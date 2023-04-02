//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   MoveMsg.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-1
//FileDescription	:	登录消息结构体定义

//模块说明：该文件定义了所有登录消息

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __MOVEMSG_HPP__
#define __MOVEMSG_HPP__

#include"SMessage.hpp"
#include"Define.hpp"

DECLARE_MSG_MAP_BEGIN(SMoveBaseMsg,SMessage,SMessage::KKDD_MOVE_MESSAGE_BASE)
KKDD_MOVEMSG_MOVE,      //普通移动消息
KKDD_MOVEMSG_POSITION,  //服务器校正数据消息
KKDD_MOVEMSG_JUMP,      //跳跃消息
DECLARE_MSG_MAP_END()

//移动消息
DECLARE_MSG(SMoveMoveBaseMsg,SMoveBaseMsg,SMoveBaseMsg::KKDD_MOVEMSG_MOVE)
struct SQMoveMsg:public SMoveMoveBaseMsg
{
    QWORD  roomID;   //房间ID （通过petsID+时间戳的方式生成，能有效的删除和后续的创建），如果使用++WORD的方式，不太好回收房间号。 
    POINT pos;       //1.0版本，暂时用像素点来作为移动，后续1.1版本将改为tile的形式
};
struct SAMoveMsg:public SMoveMoveBaseMsg
{
    POINT pos;      //将当前玩家的Pos同步给当前房间的所有玩家
};



#endif
