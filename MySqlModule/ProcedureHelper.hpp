//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   ProcedureHelper.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-4-4
//FileDescription	:	存储过程helper

//模块说明：封装存储过程调用过程
//  这里限定了存储过程参数列表的最基本规则：
//  不能在参数列表中使用双字节字符
//  不能在参数列表中使用注释
//  长度标识必须紧靠参数类型
//  ‘，’必须紧靠参数申明的末尾
//  参数和参数之间的‘，’后必须存在空格
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __PROCEDUREHELPER_HPP__
#define __PROCEDUREHELPER_HPP__

#include"MysqlQuery.hpp"

//注意MySQL存储过程写法，参数得严格按照in , out ,inout 顺序来写，否则调用返回值回失败
class ProcedureHelper:public KKDD_SQL_MODULE::MyQuery::QueryResult
{
public:	
	struct SqlParam	//参数定义
	{
		enum DIRECTION { V_IN, V_OUT, V_INOUT };
		DIRECTION           direction;  // 参数传递方向
		enum_field_types    type;	    // 参数类型
		size_t              lens;	    // 有效的参数长度（针对于字符串或Blob类型）
		boost::any       	bindValue;	// 参数值（包括传入传出） 根据 type进行转换
	};
	ProcedureHelper();
	~ProcedureHelper();
	int Prepare(const char* procName);		//存储过程准备工作
	int Execute();							//执行存储过程
	void Clear();							//清空
	std::string AnyValueToString(SqlParam& param);				//将boost::any类型 转换成MySQL基本数据类型，最后转换成字符串 返回.
	void SqlValueToAny(SqlParam& param,char* value,size_t size);	//将MySQL查询字段字符串，按照原来类型转换成boost::any类型
	enum_field_types GetMySqlType(const std::string& typestr);	//根据字符串判断，返回MySQL类型

	enum { MAX_PARAM_COUNT = 10 };		//现在定义的存储过程目前最多支持10个参数
public:
	std::string strProc;
	std::string strArgs;
	DWORD totalCount;
	DWORD inCount;
	DWORD outCount;
	DWORD inoutCount;
	SqlParam parameters[ MAX_PARAM_COUNT ];	//写死20个 感觉有点浪费空间哈,2023-3-28改小一点
};


#endif