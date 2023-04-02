//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   Pet.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-14
//FileDescription	:	桌面宠物类型

//模块说明：继承子Object，扩展宠物自身的功能

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __PET_HPP__
#define __PET_HPP__

#include<string.h>
#include<vector>

#include"Object.hpp"
#include"LoginMsg.hpp"
#include"MysqlQuery.hpp"

using namespace KKDD_SQL_MODULE;

//这一坨都用原始数据pod类型，不要用到指针，到时候直接memcpy就行了
struct SProperty    //存入数据库的属性
{
    SProperty(){memset(this,0,sizeof(SProperty));}
    DWORD m_staticID;       //静态ID，数据库分配的ID，也是宠物唯一ID
    POINT m_pos;            //宠物格子坐标
    char m_name[20];        //昵称
    char m_password[20];    //密码 md5加密
};
 

class Pet:public Object
{
public:
    Pet();    
    Object* VDC(const DWORD IID)  {if(IID == IID_PETS)return this; return nullptr;}
    DNID GetDnid();
    std::vector<std::string> Parse(std::string str ,std::string delimiter);

    void PrepareSQL(SQL_PACK_TYPE type,const char* sentence,MyQuery::QueryBind* bind,SqlCallBack cb);
//Net消息处理模块
public:
    void OnNetLoginMsg(SQLoginMsg* pMsg);

//MySQL回调模块函数，参数统一为sql的返回值，这样就跟sql模块解耦了
public:
    void OnMysqlLoginMsg(MyQuery::QueryResult& qr);


public:
    SProperty m_Property;   //存入数据库的pets属性
private:
    DWORD m_IID;            //类型ID
    DNID m_dnid;            //网络链接号
};

#endif