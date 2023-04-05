//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   MysqlQuery.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-26
//FileDescription	:	mysql模块

//模块说明：封装mysql的接口，通过代码复用的方式做到，查询sql的代码解耦

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef KKDD__MYSQLQUERY_H
#define KKDD__MYSQLQUERY_H

//#include "mysql.h"
#include<iostream>
#include<mysql/mysql.h>
#include<mutex>
#include<functional>
#include<queue>
#include<condition_variable>
#include<string>
#include<string.h> 

#include<boost/any.hpp>

#include"Define.hpp"
#include"SQLDefine.hpp"


//根据新侠实现自定义错误类 sd
class ErrMsg
{
public:
	ErrMsg();
	void LogMsg(char* info, ...);

private:
	char m_ErrInfo[ERROR_MESSAGE_BUFFER_MAX_LENGTH];
};

struct SqlPack;
namespace KKDD_SQL_MODULE		//自定义命名空间
{
	class MyQuery			//封装mysql查询类，提供接口，方面提供查询
	{
	public:
		class MySTMTBind	//mysql Stmt 参数绑定
		{
		public:
			MySTMTBind(MYSQL_BIND & mybind, my_bool & mybool, unsigned long& mylen);	//这里的构造函数很重要，在参数绑定的前提赋值

			bool IsNull() { return m_null; }
			size_t Length() { return m_lens; }

			//改进这里使用 泛型绑定，减少代码冗余量
			void bind_param(enum_field_types buffer_type, void* buffer,unsigned int buffer_length, my_bool* is_null);

			void BindBlob(void *data, size_t size);					//绑定bigobject
			void BindString(const char *data, size_t size);		//绑定string
			void BindLong(long *data);										//绑定
			void BindULong(unsigned long *data);					//...
			void BindLongLong(long long *data);							//...
			void BindULongLong(unsigned long long *data);		//...
			void BindUTiny(unsigned char *data);					//...

		private:
			MYSQL_BIND    &  m_bind;
			my_bool        & m_null;
			unsigned long  & m_lens;
		};
		
		//返回绑定参数辅助类	用于绑定类的隐式转换
		struct Arguments
		{
			Arguments(unsigned int bindnum= 0, MYSQL_BIND* binds = 0) :m_bindnum(bindnum), m_pbinds(binds) {}

			unsigned int m_bindnum;
			MYSQL_BIND * m_pbinds;
		};


		//使用模板类来绑定查询参数（配合上面的单个参数绑定类使用）
		//template<int BIND_NUM>	//模板类型是需要绑定参数的个数
		class QueryBind
		{
		public:
			QueryBind(BYTE paramNumbers);
			~QueryBind();
			MySTMTBind operator [] (int bindWitch) { return MySTMTBind(m_binds[bindWitch], m_nulls[bindWitch], m_lens[bindWitch]); }	//这里做赋值处理
			operator Arguments () { return Arguments(m_paramNumbers, m_binds); }	//隐式类型转换 将querybind对象转换成 Arguments对象，方便获取绑定参数
			operator MYSQL_BIND* () {  return m_binds; }
			MYSQL_BIND* GetBind() { return m_binds; }
			BYTE GetParamNumCounter() { return m_paramNumbers;}
		private:
			MYSQL_BIND* m_binds;		//绑定的每个参数(数组)
			BYTE m_paramNumbers;		//参数个数	
			unsigned long* m_lens;		//每个参数的长度(数组)
			my_bool* m_nulls;			//每个参数是否为空(数组)
		};


		//处理返回值类 包含预处理 跟普通查询
		struct QueryResult
		{
			~QueryResult() { Release(); }
			QueryResult() { memset(this, 0, sizeof *this); }
			void Reset() { Release(); memset(this, 0, sizeof *this); }
			void Release();
			int AffectedRows();
			int FetchResult(QueryBind& bind);			//用来解释stmt结果 加强版
			int FetchResult();							//用来解释 普通sql结果

			MYSQL_STMT    	*stmt;       	// 预处理执行句柄
			MYSQL_RES       *res;           // 本次操作执行后保存下来的结果                                                                                                                                                                                  
			MYSQL_BIND      *binds;         // 绑定参数
			const char		*sql;			// 本次操作语句 
			my_ulonglong    nrow;           // 本次操作的影响行数 返回上次执行语句更改、删除或插入的总行数。对于UPDATE、DELETE或INSERT语句，可在mysql_stmt_execute()之后立刻调用它们对于SELECT语句，mysql_stmt_affected_rows()的工作方式类似于mysql_num_rows()。

			my_ulonglong    num_rows;       // 结果集的数据总行数 返回结果集中行的数目，仅对 SELECT语句有效
			unsigned int    num_fields;     // 单个数据的字段数 
			unsigned long   *lens;          // 当前行每个字段数据的长度！ 
			MYSQL_ROW       rows;           // 当前行的数据

		};

		struct GarbageCollection 			//垃圾回收器
		{
			GarbageCollection(){}
			~GarbageCollection(){if(MyQuery::m_queryIns) delete MyQuery::m_queryIns;}
		
		};
		
		struct DBInfo						//用来存放连接数据库的信息
		{
		public:
			DBInfo() { m_user = m_password = m_dbname = m_host = {}; m_port = 0; }
			void SetInfo(char* UserName, char* PassWord, char* DBIP, char* DBName, unsigned int dwPort);

			std::string m_user;				//用户名
			std::string m_password;			//密码
			std::string m_dbname;			//db名称
			std::string m_host;				//主机名称ipd地址
			int m_port;						//端口号
		};
		
	public:
		//初始化mysql
		bool Init(char* UserName, char* PassWord, char* DBIP, char* DBName, unsigned int dwPort = 3306);
		
		void Realse();

		// 连接数据库 在Init里面已调用
		bool Connect(char* UserName, char* PassWord, char* DBIP, char* DBName, unsigned int dwPort = 3306, unsigned int client_flag = 0);

		// 执行 stmt 过程 将执行的stmt结果放进res中，通过相关函数获取查询的值,这里args如果size为0 就没有必要去绑定了
		int ExecuteStmt(const char* sql,QueryResult& qr, Arguments args = Arguments(0, 0));	

		//执行 mysql普通查询过程
		int ExecuteSQL(const char* sql, QueryResult* qr);
		int ExecuteSQLEx(const char* sql, QueryResult& qr);

		//获取单例
		static MyQuery* GetQueryInstance();

		//sql线程函数
		void MysqlLoop();

		void PushSqlPack(SqlPack* pack);
		bool GetSqlPack(SqlPack** pack);

		//执行语句
		void ExecuteSentence(SqlPack* pack);

		//执行存储过程
		bool ExecuteProcedure(SqlPack* pack);

		const char* GetDBName() {return m_dbInfo.m_dbname.c_str();}	//获取db名称

	//单例模式
	private:
		MyQuery();
		~MyQuery();
		MyQuery(const MyQuery&) = delete;
		MyQuery(const MyQuery&&) = delete;
		MyQuery& operator=(const MyQuery&) = delete;
		static MyQuery* m_queryIns;		//唯一句柄
	public:
		MYSQL * m_pMysql;				//mysql 连接句柄
		MYSQL_STMT *m_pMysqlStmt;		//mysql stmt连接句柄
		ErrMsg m_errmsg;				//sql专用错误消息输出类
		std::condition_variable m_cv;	//条件变量，后续开启多个sql线程，并发执行sql请求(注意临界区访问)
	private:	
		DBInfo m_dbInfo;				//DB连接信息
		static GarbageCollection* m_gc;	//单例模式内置的垃圾回收器

		std::queue<SqlPack*> m_sqlTaskQueue; //sql任务队列
		std::mutex m_sqlQueueMtx;			 //mysql任务队列锁
		bool m_exitFlag;					 //线程退出标志
	
	//测试函数 不重要
	public:	
		void writeMysqlStmt();
		void readMysqlStmt();		
		void testProcedure();		
	};
};



typedef std::function<void(KKDD_SQL_MODULE::MyQuery::QueryResult*)> SqlCallBack;
//typedef std::function<void(KKDD_SQL_MODULE::MyQuery::QueryResult*,ProcedureOutParams*)> SqlCallBack_procedure;

struct SqlPack						//mysql任务
{
	SQL_PACK_TYPE pack_type;		//操作类型
	const char* sentence;			//sql语句，或者stmt语句，或者存储过程语句
	union 
	{
		KKDD_SQL_MODULE::MyQuery::QueryBind* query_bind;	//注意释放资源
		boost::any* any_arr;						//注意释放资源
	};
	SqlCallBack sql_callback;						//本次sql执行完之后的回调函数
	BYTE paramNums;									//联合体指针的数组成员个数	QueryBind不需要,内部内置了个数
};

#endif