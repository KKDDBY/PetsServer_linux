//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   SQLDefine.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-19
//FileDescription	:	MySQL查询相关宏定义

//模块说明：定义了一些mysql相关的查询语句 
//          作为游戏服务器，mysql相关操作，是相当耗时的，非常有必要开一个线程去单独处理mysql相关操作
//          宏定义分类 AAA_BBB_CCC
//          AAA:分别代表mysql语句的种类，比如insert，delete,update,select,procedure
//          BBB:查询的具体内容
//          CCC:扩展，如查询使用的是普通mysql查询，还是使用mysql的stmt查询
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __MYSQLQUERY_HPP__
#define __MYSQLQUERY_HPP__

/*===========================mysql数据库相关定义==============================*/

//KKDD 定义最大错误信息输出长度为1024字节
#define ERROR_MESSAGE_BUFFER_MAX_LENGTH		1024

//KKDD 遍历sql返回值的操作类型 
enum MYSQL_FETCH_TYPE { SQL_COMMON= 0,SQL_STMT};

//mysql任务类型
enum SQL_PACK_TYPE{ SQL_COMMON_TYPE,SQL_STMT_TYPE,SQL_PROCEDURE_TYPE};

//数据库连接专用
#define DB_USERNAME "XXX"
#define DB_PASSWORD "XXX"
#define DB_IPADDRESS "XXX"
#define DB_IPPORT 3306
#define DB_DBNAME "XXX"


//账号登录 返回值不为0的话，表示账号密码正确，允许登录
#define SELECT_PETSLOGIN_STMT "select StaticID from Pets where Account=? and Password=?"




#endif