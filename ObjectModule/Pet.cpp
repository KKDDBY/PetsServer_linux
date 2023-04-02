#include <functional>

#include"muduo/base/Logging.h"

#include"Pet.hpp"
#include"SQLDefine.hpp"


using namespace KKDD_SQL_MODULE;
using namespace std::placeholders;

Pet::Pet()
{
    m_IID = IID_PETS;
    m_dnid = 0XFFFFFFFF;
}

DNID Pet::GetDnid()
{
    return m_dnid;
}

std::vector<std::string> Pet::Parse(std::string str,std::string delimiter)
{
    std::vector<std::string> res;
    if(str.size() <= 1)
        return res;

    std::string strs = str + delimiter;
	size_t pos = strs.find(delimiter);
	int step = delimiter.size();

	// 若找不到内容则字符串搜索函数返回 npos
	while (pos != strs.npos)
	{
		std::string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + step, strs.size());
		pos = strs.find(delimiter);
	}
    
}

void Pet::PrepareSQL(SQL_PACK_TYPE type, const char *sentence, MyQuery::QueryBind *bind, SqlCallBack cb) //解耦
{
    SqlPack* sqlpack = new SqlPack;
    sqlpack->pack_type = SQL_PACK_TYPE::SQL_STMT_TYPE;
    sqlpack->sentence = SELECT_PETSLOGIN_STMT;
    sqlpack->query_bind = bind;
    sqlpack->sql_callback = cb;
    MyQuery::GetQueryInstance()->PushSqlPack(sqlpack);
    MyQuery::GetQueryInstance()->m_cv.notify_one(); //唤醒sql线程，开始工作辣
}

void Pet::OnNetLoginMsg(SQLoginMsg *pMsg)
{
    std::vector<std::string> vecParam = Parse(pMsg->str,"&");
    if(vecParam.empty() || vecParam.size()!= 2) //绑定参数个数必须一致
        return;
    for(int i = 0; i < vecParam.size(); ++i)
        LOG_INFO << vecParam[i];
    if(__UNUSEFUL__)
        LOG_INFO << "void Pet::OnNetLoginMsg(SQLoginMsg *pMsg)";

    //准备sql包 
    MyQuery::QueryBind* bind = new MyQuery::QueryBind(2);
    (*bind)[0].BindString(vecParam[0].c_str(),vecParam[0].size());
    (*bind)[1].BindString(vecParam[1].c_str(),vecParam[1].size());
    PrepareSQL(SQL_PACK_TYPE::SQL_STMT_TYPE,SELECT_PETSLOGIN_STMT,bind,std::bind(&Pet::OnMysqlLoginMsg,this,_1));
}

void Pet::OnMysqlLoginMsg(MyQuery::QueryResult &qr)
{
    if ( qr.AffectedRows() != 0 )
	{
        long int roleID = 0;
    	MyQuery::QueryBind bind(1);
		bind[0].BindLong( &roleID );

		while ( qr.FetchResult( bind ) == 0 )
		{
			if ( bind[0].IsNull() || roleID == 0)
				continue;
            break;
		}

        if(roleID == 0)
        {
            //告知客户端登录失败(密码错误或者账号错误)
            LOG_INFO << "密码错误或者账号错误";
        } 
        else
        {
            //查询到有账号,开始做对象初始化操作
            //告知客户端登陆成功,同时向客户端发送宠物数据 
            LOG_INFO << "客户端登陆成功";

        }
    
	}
}
