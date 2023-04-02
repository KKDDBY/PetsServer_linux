//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   LoginMsg.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-2-28
//FileDescription	:	登录消息结构体定义

//模块说明：该文件定义了所有登录消息

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __LOGINMSG_HPP__
#define __LOGINMSG_HPP__

#include"SMessage.hpp"

DECLARE_MSG_MAP_BEGIN(SLoginBaseMsg,SMessage,SMessage::KKDD_LOGIN_MESSAGE_BASE)
KKDD_LOGINMSG_LOGIN,    //用于账号登录
KKDD_LOGINMSG_REGISTER, //账号注册消息
KKDD_LOGINMSG_SETINFO,  //设置账号信息
KKDD_LOGINMSG_GEITINFO, //获取账号信息
DECLARE_MSG_MAP_END()

//登录消息
DECLARE_MSG(SLoginMsg,SLoginBaseMsg,SLoginBaseMsg::KKDD_LOGINMSG_LOGIN)
struct SQLoginMsg:public SLoginMsg
{
    char str[0];    //账号&密码 使用柔性数组
};
struct SALoginMsg:public SLoginMsg
{
    enum
    {
        LOGIN_SUCCESS,          //登录成功
        INVALID_ACCOUNT,        //错误的账号
        INVALID_PASSWORD,       //错误的密码
        LOGIN_ALREADY_LOGIN,    //重复登录
        ACCOUNT_BLOCKED,        //账号被封了
    };
    BYTE LoginRet;
};

//注册消息
DECLARE_MSG(SRegisterMsg,SLoginBaseMsg,SLoginBaseMsg::KKDD_LOGINMSG_LOGIN)
struct SQRegisterMsg:public SRegisterMsg
{
    char str[0];    //账号&密码 使用柔性数组
};
struct SARegisterMsg:public SRegisterMsg
{
    enum
    {
        REGINTER_SUCCESS,       //注册成功
        ACCOUNT_EXIST,          //账号已存在
    };
    BYTE RegisterRet;
};


#endif
