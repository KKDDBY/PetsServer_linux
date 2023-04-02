//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   SMessage.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-2-28
//FileDescription	:	网络消息基类

//模块说明：
//该模块由处理codec（编码，解码）模块处理后转换成SMessage基类类型
// 这里所创建的具体消息类型都是以SQ或者SA来开头的
// S是struct的标识
// Q&A本表示为同一种类型的请求与应答
// 现表示该消息的网络流向
// 例如：
// SQLoginMsg	在客户端生成，发往服务器，由服务器处理的消息
// SALoginMsg	在服务器生成，发往客户端，由客户端处理的消息
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __SMESSAGE_HPP__
#define __SMESSAGE_HPP__
#include"Define.hpp"

#define DECLARE_MSG(name,BaseMessage,MessageType) \
struct name:public BaseMessage \
{   \
   name()   \
   {    \
     BaseMessage::_protocol = MessageType;  \
   }    \
};  

#define DECLARE_MSG_MAP_BEGIN(name,BaseMessage,MessageType) \
struct name:public BaseMessage \
{\
    name() \
    {\
        BaseMessage::_protocol = MessageType;   \
    }\
    enum {
#define DECLARE_MSG_MAP_END() }; \
BYTE _protocol;\
};\

struct SMessage
{
    enum
    {
        KKDD_LOGIN_MESSAGE_BASE = 0x64,
        KKDD_MOVE_MESSAGE_BASE,
        KKDD_CHAT_MESSAGE_BASE,
        KKDD_FIGHT_MESSAGE_BASE,

    };
    BYTE _protocol;
};


#endif