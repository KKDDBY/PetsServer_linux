//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   ChatMsg.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-2-28
//FileDescription	:	聊天消息结构体定义

//模块说明：该文件定义了所有聊天消息
//结构体中单独只有一个柔性数组，那么柔性数组的大小就是结构体的大小，分配内存时，按柔性数组大小分配，不需要额外一个字节的分配

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CHATMSG_HPP__
#define __CHATMSG_HPP__

#include"SMessage.hpp"

DECLARE_MSG_MAP_BEGIN(SChatBaseMsg,SMessage,SMessage::KKDD_CHAT_MESSAGE_BASE)
KKDD_CHATMSG_WHISPER,    //私聊
KKDD_CHATMSG_BROADCAST,  //广播
KKDD_CHATMSG_SYSTEM,     //系统消息
KKDD_CHATMSG_ROOM,       //房间消息
DECLARE_MSG_MAP_END()

//私聊消息
DECLARE_MSG(SChatBaseWhisperMsg,SChatBaseMsg,SChatBaseMsg::KKDD_CHATMSG_WHISPER)
struct SQChatWhisperMsg:public SChatBaseWhisperMsg
{
    DNID dnid;
    char chatData[0];   //消息格式:指定发送人&消息
};
struct SAChatWhisperMsg:public SChatBaseWhisperMsg
{
    char chatData[0];   //消息格式:消息         私聊消息直接转发
};

//广播消息
DECLARE_MSG(SChatBaseBroadcastMsg,SChatBaseMsg,SChatBaseMsg::KKDD_CHATMSG_BROADCAST)
struct SQChatBroadcastMsg:public SChatBaseBroadcastMsg
{
    char broadcastData[0];    //消息格式:消息                 广播的消息 使用柔性数组
};
struct SAChatBroadcastMsg:public SChatBaseBroadcastMsg
{
   char broadcastData[0];    //消息格式:消息                  广播的消息 使用柔性数组
};

//系统消息  
DECLARE_MSG(SChatBaseSystemMsg,SChatBaseMsg,SChatBaseMsg::KKDD_CHATMSG_SYSTEM)
struct SQChatSystemMsg:public SChatBaseSystemMsg
{
    char systemData[0];        //消息格式:消息                 广播的消息 使用柔性数组
};
struct SAChatSystemMsg:public SChatBaseSystemMsg
{
    char systemData[0];        //消息格式:消息                  广播的消息 使用柔性数组
};

//房间消息  
DECLARE_MSG(SChatBaseRoomMsg,SChatBaseMsg,SChatBaseMsg::KKDD_CHATMSG_ROOM)
struct SQChatRoomMsg:public SChatBaseRoomMsg
{
    WORD roomID;             //2个字节-16位-0xFFFF-最大能开65535个房间完全够用
    char roomData[0];        //消息格式:消息                 广播的消息 使用柔性数组
};
struct SAChatRoomMsg:public SChatBaseRoomMsg
{
   char roomData[0];        //消息格式:消息                  广播的消息 使用柔性数组
};





#endif
