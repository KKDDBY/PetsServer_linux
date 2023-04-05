#include<algorithm>
#include<sstream>

#include "stdAfx.h"
#include "ProcedureHelper.hpp"

int ProcedureHelper::Prepare(const char *procName)
{
	strProc = procName;
	char sql_paramlist[1024] = {};
	sprintf(sql_paramlist, SELECT_PROCEDURE_PARAMLIST, KKDD_SQL_MODULE::MyQuery::GetQueryInstance()->GetDBName(), procName);
	QueryResult qr;
	if (KKDD_SQL_MODULE::MyQuery::GetQueryInstance()->ExecuteSQL(sql_paramlist,&qr) != 0)

	{
		LOG_ERROR << "存储过程[" << sql_paramlist << "]获取参数列表失败";
		return -4;
	}

	if (qr.AffectedRows() < 1)
	{
		LOG_ERROR << "存储过程[" << sql_paramlist << "]获取参数列表返回值行数错误";
		return -1; // 影响行数错误
	}

	if (qr.FetchResult() != 0)
	{
		LOG_ERROR << "存储过程[" << sql_paramlist << "]参数列表遍历结果失败";
		return -2;
	}

	strArgs = qr.rows[0];

	if (__UNUSEFUL__)
		LOG_INFO << "Procedure params: " << strArgs;

	std::transform(strArgs.begin(), strArgs.end(), strArgs.begin(), ::tolower); // 转小写，方便后续遍历
	std::stringstream strStream(strArgs);

	std::string parampart; // MySQL参数的一部分 (int _XXX varchar(50))
	int whichPart = 0;	   // 0 -> int,1 -> _XXX, 2 -> varchar(50)
	strStream >> parampart;
	while (!parampart.empty())
	{
		switch (whichPart)
		{
		case 0: // 类型
		{
			whichPart = 1;
			if (parampart == "in")
			{
				++inCount;
				parameters[totalCount].direction = SqlParam::DIRECTION::V_IN;
			}
			else if (parampart == "out")
			{
				++outCount;
				parameters[totalCount].direction = SqlParam::DIRECTION::V_OUT;
			}
			else if (parampart == "inout")
			{
				++inoutCount;
				parameters[totalCount].direction = SqlParam::DIRECTION::V_INOUT;
			}
			else // 没写参数传递方向 默认in
			{
				++inCount;
				parameters[totalCount].direction = SqlParam::DIRECTION::V_IN;
				whichPart = 2;
			}
		}
		break;
		case 1: // 字段名
		{
			whichPart = 2;
		}
		break;
		case 2: // 字段类型跟长度 varchar(50)
		{
			size_t last = parampart.length();
			if (parampart.at(last - 1) == ',')
				last -= 1;

			// 是否还有长度信息？
			if (parampart.at(last - 1) == ')')
			{
				last = parampart.find_first_of('(');
				if (last != std::string::npos)
					parameters[totalCount].lens = atoi(parampart.c_str() + last + 1); // atoi将123XX转换成123,x123x转化失败0
			}

			parameters[totalCount].type = GetMySqlType(parampart.substr(0, last));
			if (parameters[totalCount].type == -1)
			{
				LOG_ERROR << "存储过程[" << sql_paramlist << "]参数列表转换成MySQL参数类型时失败";
				return -3; // MySQL 参数转换失败
			}

			totalCount++;
			whichPart = 0;
		}
		break;
		}
		parampart.clear();
		strStream >> parampart;
	}
	return 0;
}

// 准备一个 64K 的 BUFFER 来作为 real_query 的缓存
char PrepareBuffer[0xFA00];
int ProcedureHelper::Execute()
{
	DWORD prepareIdx = 0; // PrepareBuffer下标索引
	// char PrepareBuffer[1024]; // 执行存储过程前，的set变量操作
	char tempStr[256] = "";
	std::string strSelect;
	std::string pecedureCall = "call ";
	pecedureCall += this->strProc;
	pecedureCall += " ( ";
	int result = 0;

	for (int i = 0; i < totalCount; ++i)
	{
		// 如果不是第一个参数，就需要加分隔符了。。。
		if (i != 0)
			pecedureCall += ", ";

		// 如果是传入参数，不使用用户变量（no set @_xxx）
		if (parameters[i].direction == SqlParam::V_IN)
		{
			if (parameters[i].type != MYSQL_TYPE_BLOB)
			{
				// boost::any转换
				std::string ret;
				if ((ret = AnyValueToString(parameters[i])).empty())
					return -1;
				if (__UNUSEFUL__)
					LOG_INFO << "ProduceCall param:" << i << " value:" << ret;
				pecedureCall += ret;
			}
			else // 二进制数据特殊，需要单独准备参数
			{
				// 为前一个语句加上结束符
				if (prepareIdx != 0)
					PrepareBuffer[prepareIdx++] = ',';
				else
				{
					*(DWORD *)PrepareBuffer = *(DWORD *)"set";
					prepareIdx += 3;
				}

				int len = sprintf(tempStr,"@_%d = 0x", i);
				memcpy( PrepareBuffer + prepareIdx, tempStr, len );
				prepareIdx += len;

				// 这里把2进制数据转换成16进制数据
				void* pointer = boost::any_cast<void*>(parameters[i].bindValue);
				BYTE* byData = (BYTE*)pointer;
				for (int n = 0; n < parameters[i].lens; n++)
					sprintf(&PrepareBuffer[prepareIdx + n * 2], "%02x", byData[n]);
				prepareIdx += parameters[i].lens * 2;

				// 插入变量调用
				len = sprintf( tempStr, "@_%d", i );
				pecedureCall += tempStr;
			}
		}
		else	// 接下来处理的就是 IN/INOUT 不管是哪一类，都需要准备用户变量
		{	
            // 为前一个语句加上结束符
            if ( prepareIdx != 0 )
                PrepareBuffer[ prepareIdx++ ] = ',';
            else
            {
                *(DWORD*)PrepareBuffer = *(DWORD*)"set";
                prepareIdx += 3;
            }

			if (parameters[i].direction == SqlParam::V_INOUT)	 // INOUT 是需要传值的
			{
				// 插入变量调用
                if (parameters[i].type != MYSQL_TYPE_BLOB)
                {
					std::string ret;
					if ((ret = AnyValueToString(parameters[i])).empty())
						return -1;
                    int len = sprintf( PrepareBuffer + prepareIdx, " @_%d = %s", i,ret);
                    prepareIdx += len;
                }
				else
				{
					// 这里把2进制数据转换成16进制数据
					void* pointer = boost::any_cast<void*>(parameters[i].bindValue);
					BYTE* byData = (BYTE*)pointer;
					for (int n = 0; n < parameters[i].lens; n++)
						sprintf(&PrepareBuffer[prepareIdx + n * 2], "%02x", byData[n]);
					prepareIdx += parameters[i].lens * 2;
				}
			}
			else	 // 单纯的 OUT 类型，需要初始化
			{
				int len = sprintf(PrepareBuffer + prepareIdx, " @_%d = null", i);
                prepareIdx += len;
			}

			// 设置变量的调用
            int len = sprintf(tempStr, "@_%d",i);
            pecedureCall.append( tempStr, len);

            // 设置查询存储过程执行结果
            len = sprintf( tempStr, strSelect.empty() ? "select @_%d" : ", @_%d", i );
            strSelect.append( tempStr, len );
		}
	}
	
	// 加上结束括号
    pecedureCall.append(" )",2);

	 KKDD_SQL_MODULE::MyQuery* query = KKDD_SQL_MODULE::MyQuery::GetQueryInstance();	//获取单例
	if(prepareIdx != 0)
	{
		// 容错，处理越界问题！
		if(prepareIdx > 0xFA00 - 20)
		{
			LOG_ERROR << "准备存储过程参数的时候 长度大于限制";
			return -1;
		}

		PrepareBuffer[prepareIdx ] = 0;
		result = query->ExecuteSQL(PrepareBuffer,NULL);
		if ( result != 0 )
			return result;
	}

	// 执行存储过程！
	result = query->ExecuteSQL(pecedureCall.c_str(),this);
    if(result != 0)
        return LOG_ERROR<<"执行存储过程失败...", result;

	//这里是否要保存存储过程的查询内容？(因为下面的ExecuteSQL会覆盖掉查询结果集)
	if(__UNUSEFUL__)
		while (FetchResult() == 0 && num_fields == 2)
			LOG_INFO << rows[0] << "   " << rows[1];
	QueryResult::Release();

	// 获取执行结果
    if(strSelect.empty())
        return 0;

	result = query->ExecuteSQL(strSelect.c_str(),this);
    if (result != 0)
        return result;
    
    if (AffectedRows() != 1)
        return -1;

    result = FetchResult();
    if (result != 0)
        return result;
	
	for (DWORD i = 0; i < outCount + inoutCount; ++i)	//将存储过程 返回参数 绑定到参数
		SqlValueToAny(parameters[inCount + i],rows[i],lens[i]);

	// 每次用完记得清空buffer
	memset(PrepareBuffer, 0, 0xFA00);
}

void ProcedureHelper::Clear()
{
	//父类 QueryResult 资源清空
	QueryResult::Release();

	strProc.clear();
	strArgs.clear();
	totalCount = inCount = outCount = inoutCount = 0;
	std::memset(parameters, 0, sizeof(parameters));
}

std::string ProcedureHelper::AnyValueToString(SqlParam &param)
{
	if (param.type == MYSQL_TYPE_LONG)
	{
		return std::string(std::to_string(boost::any_cast<int>(param.bindValue)));
	}
	else if (param.type == MYSQL_TYPE_LONGLONG)
	{
		return std::string(std::to_string(boost::any_cast<int64_t>(param.bindValue)));
	}
	else if (param.type == MYSQL_TYPE_FLOAT)
	{
		char buf[33] = {};
		sprintf(buf, "%f", boost::any_cast<float>(param.bindValue));
		return std::string(buf, strlen(buf));
	}
	else if (param.type == MYSQL_TYPE_DOUBLE)
	{
		char buf[33] = {};
		sprintf(buf, "%lf", boost::any_cast<double>(param.bindValue));
		return std::string(buf, strlen(buf));
	}
	else if (param.type == MYSQL_TYPE_STRING)
	{
		std::string str = "'";
		str += boost::any_cast<char*>(param.bindValue);
		str += "'";
		return str;
	}
	else if (param.type == MYSQL_TYPE_BLOB)
	{
		// 这一块，在外部处理，使用set @_xxx = xxx 的方式，设置数据库的局部变量
		LOG_ERROR << "Error param.type";
		return std::string("");
	}
	else // 目前只用得到 这几种常见的类型
	{
		LOG_ERROR << "Error param.type";
		return std::string("");
	}
}

void ProcedureHelper::SqlValueToAny(SqlParam & param, char *value, size_t size)
{
	if(__UNUSEFUL__)
		LOG_INFO << "SQL Result VALUE: " << value;
	if (param.type == MYSQL_TYPE_LONG)
		param.bindValue = boost::any_cast<int>(atoi(value));
	else if (param.type == MYSQL_TYPE_LONGLONG)
		param.bindValue = boost::any_cast<int64_t>(atoll(value));
	else if (param.type == MYSQL_TYPE_FLOAT)
		param.bindValue = boost::any_cast<float>(strtof(value,nullptr));
	else if (param.type == MYSQL_TYPE_DOUBLE)
		param.bindValue = boost::any_cast<double>(strtod(value,nullptr));
	else if (param.type == MYSQL_TYPE_STRING)
		param.bindValue = value;
	else if (param.type == MYSQL_TYPE_BLOB)
	{
		param.bindValue = boost::any_cast<void*>(value);
		param.lens = size;
	}
	else // 目前只用得到 这几种常见的类型
		LOG_ERROR << "Error param.type";
}

enum_field_types ProcedureHelper::GetMySqlType(const std::string &typestr)
{
	const enum_field_types errtype = (enum_field_types)-1;
	switch (typestr[0])
	{
	case 'b':
		switch (typestr[1])
		{
		case 'l':
			return (typestr == "blob") ? MYSQL_TYPE_BLOB : errtype;
		case 'i':
			return (typestr == "bigint") ? MYSQL_TYPE_LONGLONG : errtype;
		}
		break;
	case 'i':
		return (typestr == "int") ? MYSQL_TYPE_LONG : errtype;
	case 'c':
		return (typestr == "char") ? MYSQL_TYPE_STRING : errtype;
	case 'v':
		return (typestr == "varchar") ? MYSQL_TYPE_STRING : errtype;
	}
	return errtype;
}



ProcedureHelper::ProcedureHelper()
{
	Clear();
}

ProcedureHelper::~ProcedureHelper()
{
}

