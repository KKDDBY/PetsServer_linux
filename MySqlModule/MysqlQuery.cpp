#include<stdio.h>
#include<iostream>
#include<stdarg.h>
#include<assert.h>
#include<string>
#include<chrono>
#include<thread>
#include<algorithm>
#include <sstream>

#include"muduo/base/Logging.h"

#include"MysqlQuery.hpp"
#include"ProcedureHelper.hpp"

#pragma warning(disable:4996 )

ErrMsg::ErrMsg()
{
	memset(m_ErrInfo, 0, sizeof(m_ErrInfo));
}

void ErrMsg::LogMsg(char* info,...)
{
	//这里使用到 c库中的几个宏，实现变参的应用
	va_list vl;		//一个带有偏移量的指针  char* dst(待处理参数) ,int offset(偏移量)
	va_start(vl, info);	//将info绑定到va_list中
	vsprintf(m_ErrInfo, info, vl);		//vsprintf 跟 sparintf函数功能类似，不过参数vsprintf使用的是va_list
	va_end(vl);	//使用完毕记得清空 va_list
	std::cout << m_ErrInfo << std::endl;	
}

KKDD_SQL_MODULE::MyQuery::MyQuery()
{
	m_pMysql = nullptr;
	m_pMysqlStmt = nullptr;
	m_exitFlag = false;
}

KKDD_SQL_MODULE::MyQuery::~MyQuery()
{
	if (m_pMysqlStmt != nullptr)
	{
		mysql_stmt_close(m_pMysqlStmt);
		m_pMysqlStmt = nullptr;
	}

	if (m_pMysql != nullptr)
	{
		mysql_close(m_pMysql);
		m_pMysql = nullptr;
	}
}

bool KKDD_SQL_MODULE::MyQuery::Init(char* UserName, char* PassWord, char* DBIP, char* DBName, unsigned int dwPort)
{
	m_pMysql = mysql_init(NULL);
	if (m_pMysql == nullptr)
	{
		m_errmsg.LogMsg("init mysql Error");
		return false;
	}
	LOG_INFO << "mysql_init successfully";

	//设置mysql
	mysql_options(m_pMysql, MYSQL_OPT_CONNECT_TIMEOUT, "120");
	mysql_options(m_pMysql, MYSQL_SET_CHARSET_NAME, "utf8"); //utf8

	//初始化成功，调用connect连接mysql
	if (!Connect(UserName, PassWord, DBIP, DBName))
		return false;
		

	//连接成功 随后启动mysql_stmt以及将数据库信息保存
	m_pMysqlStmt = mysql_stmt_init(m_pMysql);
	if (m_pMysqlStmt == nullptr)
	{
		m_errmsg.LogMsg("init_mysql_stmt Error");
		return false;
	}

	LOG_INFO << "mysql_stmt_init successfully";

	m_dbInfo.SetInfo(UserName, PassWord, DBIP, DBName, dwPort);

	return true;
}

void KKDD_SQL_MODULE::MyQuery::Realse()
{

}

bool KKDD_SQL_MODULE::MyQuery::Connect(char* UserName, char* PassWord, char* DBIP, char* DBName, unsigned int dwPort, unsigned int client_flag)
{
	if (nullptr == mysql_real_connect(m_pMysql, DBIP, UserName, PassWord, DBName, dwPort,NULL,CLIENT_MULTI_RESULTS))
	{
		m_errmsg.LogMsg("mysql_real_connect Error [%s]",mysql_error(m_pMysql)); 
		return false;
	}

	LOG_INFO << "mysql_real_connect successfully";
	return true;
}

int KKDD_SQL_MODULE::MyQuery::ExecuteStmt(const char * sql,QueryResult & qr, Arguments args)
{
	LOG_INFO << sql ;
	//判断边界
	if (sql == NULL || m_pMysqlStmt == NULL)
	{
		return  m_errmsg.LogMsg("sql or Mysql_STMT is NULL"),-1;
	}

	//stmt准备	返回值为0才是成功
	int result = mysql_stmt_prepare(m_pMysqlStmt, sql, strlen(sql));
	if (result != 0)
		return m_errmsg.LogMsg("Execute mysql_stmt_prepare is Error [ reason:%s --- sql:%s]", mysql_stmt_error(m_pMysqlStmt), sql),result;

	//stmt 参数绑定 有参数才能绑定
	if (args.m_bindnum != 0)
	{
		result = mysql_stmt_bind_param(m_pMysqlStmt, args.m_pbinds);	//绑定参数由外部准备好
		if (result != 0)
			return m_errmsg.LogMsg("Execute mysql_stmt_bind_param is Error [reason:%s --- sql:%s]", mysql_stmt_error(m_pMysqlStmt), sql), result;
	}

	//执行 
	result = mysql_stmt_execute(m_pMysqlStmt);
	if (result != 0)
		return m_errmsg.LogMsg("Execute mysql_stmt_execute is Error [reason:%S --- sql:%s]",mysql_stmt_error(m_pMysqlStmt),sql),result;

	// 如果成功,完成了对结果的缓冲处理 从mysql server缓冲取回来
	result = mysql_stmt_store_result(m_pMysqlStmt);
	if(result != 0)
		return m_errmsg.LogMsg("Execute mysql_stmt_store_result is Error [reason:%S --- sql:%s]", mysql_stmt_error(m_pMysqlStmt), sql), result;


	//执行成功之后，将结果存进 queryresult中
	qr.sql = sql;
	qr.stmt = m_pMysqlStmt;
	//qr.binds = args.m_pbinds;
	qr.res = mysql_stmt_result_metadata(m_pMysqlStmt);
	qr.nrow = mysql_stmt_affected_rows(m_pMysqlStmt);


	if (qr.res)
	{
		qr.num_rows = mysql_num_rows(qr.res);
		qr.num_fields = mysql_num_fields(qr.res);
	}

	return 0;
}

int KKDD_SQL_MODULE::MyQuery::ExecuteSQL(const char * sql, QueryResult* qr)
{
	if (sql == NULL || m_pMysql == NULL)
		return m_errmsg.LogMsg("sql or MySQL is NULL!!!"),-1;

	//执行查询语句
	int result = mysql_real_query(m_pMysql, sql, strlen(sql));
	if (result != 0)
		return m_errmsg.LogMsg("Execute mysql_real_query is Error [reason:%s --- sql:%s]",mysql_error(m_pMysql),sql),result;

	//将查询内容缓冲到本地
	MYSQL_RES* tempSQL_RES = mysql_store_result(m_pMysql);
	
	if(qr != NULL)
	{
		qr->sql = sql;
		qr->res = tempSQL_RES;
        qr->nrow = mysql_affected_rows(m_pMysql);

		if(qr->res)
        {
            qr->num_rows = mysql_num_rows( qr->res );
            qr->num_fields = mysql_num_fields( qr->res );
        }
        tempSQL_RES = NULL;
	}
	if(tempSQL_RES != NULL)
		mysql_free_result(tempSQL_RES)/*, OutputDebugString( "release\r\n" )*/;
	return 0;
}

int KKDD_SQL_MODULE::MyQuery::ExecuteSQLEx(const char *sql, QueryResult &qr)
{
    if (sql == NULL || m_pMysql == NULL)
		return m_errmsg.LogMsg("sql or MySQL is NULL!!!"),-1;

	//执行查询语句
	int result = mysql_real_query(m_pMysql, sql, strlen(sql));
	if (result != 0)
		return m_errmsg.LogMsg("Execute mysql_real_query is Error [reason:%s --- sql:%s]",mysql_error(m_pMysql),sql),result;
	return 0;
}

KKDD_SQL_MODULE::MyQuery::GarbageCollection* KKDD_SQL_MODULE::MyQuery::m_gc = new KKDD_SQL_MODULE::MyQuery::GarbageCollection;	//垃圾回收期初始化
KKDD_SQL_MODULE::MyQuery* KKDD_SQL_MODULE::MyQuery::m_queryIns = new KKDD_SQL_MODULE::MyQuery;	//单例初始化
KKDD_SQL_MODULE::MyQuery* KKDD_SQL_MODULE::MyQuery::GetQueryInstance()
{
	LOG_INFO<< "GetQueryInstance";
    return m_queryIns;
}

void KKDD_SQL_MODULE::MyQuery::MysqlLoop()
{
	LOG_INFO << "MysqlLoop Start!!!";
	//do something
	while(!m_exitFlag)
	{
		std::unique_lock<std::mutex> lk(m_sqlQueueMtx);	//上锁
		m_cv.wait(lk,[this](){ return this->m_sqlTaskQueue.size() > 0;}); //尝试唤醒 如果唤醒失败，则当前线程一直阻塞在这里

		while(!m_sqlTaskQueue.empty())  
		{
			SqlPack *pack = nullptr;
			if(!GetSqlPack(&pack))
				break;
			
			if(!pack->query_bind)
				continue;
				
			//执行语句操作
			ExecuteSentence(pack);

			//pack资源释放
			if(pack->pack_type == SQL_PROCEDURE_TYPE)
				if(pack->any_arr)
					delete[] pack->any_arr;
			else
				if(pack->query_bind)
					delete pack->query_bind;
			delete pack;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));	//执行完后继续休眠
	}
	

	LOG_INFO << "MysqlLoop Stop!!!";

}

void KKDD_SQL_MODULE::MyQuery::PushSqlPack(SqlPack *pack)
{
	m_sqlQueueMtx.lock();
    m_sqlTaskQueue.push(pack);
    m_sqlQueueMtx.unlock();

    if(__UNUSEFUL__)
        LOG_INFO << "MyQuery has push a pack,now size is: " << m_sqlTaskQueue.size();
}

bool KKDD_SQL_MODULE::MyQuery::GetSqlPack(SqlPack **pack)
{
    if(m_sqlTaskQueue.empty())	
        return false;

	//使用条件变量就不需要在这里上锁了
    //m_sqlQueueMtx.lock();       //上锁
    *pack = m_sqlTaskQueue.front(); //出包
    m_sqlTaskQueue.pop();           //出队
    //m_sqlQueueMtx.unlock();     //解锁
    
    if(__UNUSEFUL__)
        LOG_INFO << "MyQuery has pop a pack,now size is: " << m_sqlTaskQueue.size();
    return true;
}

void KKDD_SQL_MODULE::MyQuery::ExecuteSentence(SqlPack *pack)
{
	if(!pack)
		return;

	switch(pack->pack_type)
	{
	case SQL_PACK_TYPE::SQL_COMMON_TYPE:
		{
			if(__UNUSEFUL__)
				LOG_INFO << "SQL_COMMON_TYPE";
			QueryResult qr;
			if(ExecuteSQL(pack->sentence,&qr) != 0)
				return;
			pack->sql_callback(&qr);
		}
	break;
	case SQL_PACK_TYPE::SQL_STMT_TYPE:
		{
			if(__UNUSEFUL__)
				LOG_INFO << "SQL_STMT_TYPE";
			QueryResult qr;
			if(ExecuteStmt(pack->sentence,qr,*pack->query_bind) != 0)
				return;
			pack->sql_callback(&qr);	
		}
	break;
	case SQL_PACK_TYPE::SQL_PROCEDURE_TYPE:
		{
			if(__UNUSEFUL__)
				LOG_INFO << "SQL_PROCEDURE_TYPE";
			if(!ExecuteProcedure(pack))
				return;
		}
	break;
	default:
		LOG_ERROR << "ExecuteSentence type undifine";
	break;
	}
}

bool KKDD_SQL_MODULE::MyQuery::ExecuteProcedure(SqlPack *pack)
{
	static ProcedureHelper pdcHelper;
	if(pdcHelper.Prepare(pack->sentence) != 0)	//调用存储过程的准备工作，获取存储过程的参数列表
		return pdcHelper.Clear(),false;

	//准备调用参数
	for(int param = 0; param < pack->paramNums; ++param)
		pdcHelper.parameters[param].bindValue =  pack->any_arr[param];

	if (pdcHelper.Execute() != 0)
		return pdcHelper.Clear(),false; 

	pack->sql_callback(&pdcHelper);
	
	pdcHelper.Clear();	//每次执行完一次
    return true;
}

void KKDD_SQL_MODULE::MyQuery::writeMysqlStmt()
{
	//旧版本
	//char *query = "insert into t_logininfo (ServerID) values ( ? ) ;";

	//if (mysql_stmt_prepare(m_pMysqlStmt, query, strlen(query)))
	//{
	//	fprintf(stderr, "mysql_stmt_prepare: %s\n", mysql_error(m_pMysql));
	//	return ;
	//}

	//long sid = 1024; 

	//QueryBind<1> binds;
	//binds[0].BindLong(&sid);

	//mysql_stmt_bind_param(m_pMysqlStmt, binds.GetBind());
	//mysql_stmt_execute(m_pMysqlStmt);           //执行与语句句柄相关的预处理

	//新版本 通过函数包装一下
	//char *query = "insert into t_logininfo (ServerID) values ( ? ) ;";
	//long sid = 9999; 
	//QueryBind<1> binds;
	//binds[0].BindLong(&sid);
	//ExecuteStmt(query, binds);
}

void KKDD_SQL_MODULE::MyQuery::readMysqlStmt()
{
	 char *query = "select * from Pets where Account = 'TEST' ";
	 QueryResult qr;
	 if (ExecuteStmt(query, qr) != 0)
	 	return;

	LOG_INFO<< "AffectedRows: " << qr.AffectedRows();
	long staticid = 0;
	char account[20] = {};
	char password[33] = {};
	void * buffer = new char[1024];

	QueryBind binds(4);
	binds[0].BindLong(&staticid);
	binds[1].BindString(account,sizeof(account));
	binds[2].BindString(password, sizeof(password));
	binds[3].BindBlob(buffer, 1024);
	qr.binds = binds.GetBind();
	while (qr.FetchResult(binds) == 0)
	{
		std::cout << staticid << " " << account << " " << password <<  std::endl;
	}
}

void KKDD_SQL_MODULE::MyQuery::testProcedure()
{
	SqlPack *pack = new SqlPack;
	pack->pack_type = SQL_PACK_TYPE::SQL_PROCEDURE_TYPE;
	pack->sentence = "p_test";
	pack->paramNums = 1;
	pack->any_arr = new boost::any;
	ExecuteSentence(pack);

}

void KKDD_SQL_MODULE::MyQuery::DBInfo::SetInfo(char* UserName, char* PassWord, char* DBIP, char* DBName, unsigned int dwPort)
{
	 m_user = UserName;				//用户名
	 m_password = PassWord;		//密码
	 m_dbname = DBName;			//db名称
	 m_host = DBIP;						//主机名称ipd地址
	 m_port = dwPort;					//端口号
	 
	 LOG_INFO << ">>> MySQL INFO BEGIN>>>";
	 LOG_INFO << "USER: " << m_password;
	 LOG_INFO << "PassWord: " << m_user;
	 LOG_INFO << "DBName: " << m_dbname;
	 LOG_INFO << "DBIP: " << m_host;
	 LOG_INFO << "dwPort: " << m_port;
	 LOG_INFO << ">>> MySQL INFO END>>>";

}

KKDD_SQL_MODULE::MyQuery::MySTMTBind::MySTMTBind(MYSQL_BIND & mybind, my_bool & mybool, unsigned long & mylen):m_bind(mybind),m_null(mybool),m_lens(mylen){}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::bind_param(enum_field_types buffer_type, void * buffer, unsigned int buffer_length, my_bool * is_null)
{
	m_lens = (unsigned long)buffer_length;
	m_bind.buffer_type = buffer_type;
	m_bind.buffer =/* (char*)*/buffer;
	m_bind.buffer_length = buffer_length; //这个长度是指缓冲区的最大长度
	m_bind.is_null = is_null;
	m_bind.length = &m_lens;//这个指数据的实际长度
}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::BindBlob(void * data, size_t size)
{
	bind_param(MYSQL_TYPE_BLOB, data, size, 0);
}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::BindString(const char * data, size_t size)
{
	bind_param(MYSQL_TYPE_STRING, (void*)data, size, 0);
}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::BindLong(long * data)
{
	bind_param(MYSQL_TYPE_LONG, (void*)data, 0, 0);

}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::BindULong(unsigned long * data)
{
	bind_param(MYSQL_TYPE_LONG, (void*)data, 0, 0);
}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::BindLongLong(long long * data)
{
	bind_param(MYSQL_TYPE_LONGLONG, (void*)data, 0, 0);
}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::BindULongLong(unsigned long long * data)
{
	bind_param(MYSQL_TYPE_LONGLONG, (void*)data, 0, 0);
}

void KKDD_SQL_MODULE::MyQuery::MySTMTBind::BindUTiny(unsigned char * data)
{
	bind_param(MYSQL_TYPE_TINY, (void*)data, 0, 0);
}


void KKDD_SQL_MODULE::MyQuery::QueryResult::Release()	//释放 结果集资源
{
	if (res)
		mysql_free_result(res);

	if (stmt != NULL)
		mysql_stmt_free_result(stmt);
}

int KKDD_SQL_MODULE::MyQuery::QueryResult::AffectedRows()
{
	return nrow;		//返回 本次操作影响的行数 如果是select则为查询到的行数
}

int KKDD_SQL_MODULE::MyQuery::QueryResult::FetchResult(QueryBind& bind)
{
	//首先判断bind的字段数跟,查询返回的字段数是否匹配，这个操作必做，否者程序崩溃
	if(bind.GetParamNumCounter() != num_fields)
	{
		LOG_ERROR << "stmt bind params not match num_fields";
		return 1;
	}

    if (bind.GetBind() != NULL)	
	{
		int result = mysql_stmt_bind_result(stmt, bind.GetBind());
		if (result != 0)
			return result;
	}
	return mysql_stmt_fetch(stmt);
}

int KKDD_SQL_MODULE::MyQuery::QueryResult::FetchResult()
{
	if (res == NULL || num_rows == 0 || num_fields == 0)	//说明查询没有结果，直接
		return 1;

	rows = mysql_fetch_row(res);
	lens = mysql_fetch_lengths(res);

	return	(rows != NULL && lens != NULL) ? 0 : 1;
}

KKDD_SQL_MODULE::MyQuery::QueryBind::QueryBind(BYTE paramNumbers)
{
	m_binds = new MYSQL_BIND[paramNumbers];
	m_lens  = new unsigned long[paramNumbers];
	m_nulls = new my_bool[paramNumbers];
	m_paramNumbers = paramNumbers;
	LOG_INFO << "QueryBind() 构造了:" << m_paramNumbers << "个参数......";

}

KKDD_SQL_MODULE::MyQuery::QueryBind::~QueryBind()
{
	if(m_binds)
		delete[] m_binds;
	if(m_lens)
		delete[] m_lens;
	if(m_nulls)
		delete[] m_nulls;
	LOG_INFO << "~QueryBind() 释放了:" << m_paramNumbers << "个参数......";
}


