#include"muduo/base/Logging.h"

#include"PetsManager.hpp"
#include "Pet.hpp"



Object *PetsManager::CreateObject(const DWORD IID)
{
    if (IID == IID_PETS)
		return new Pet();

    return NULL;
}

PetsManager::PetsManager()
{
}

PetsManager::~PetsManager()
{
}

bool PetsManager::OnDispatchLoginMsg(DNID dnid, SMessage *basemsg,Pet* pet)
{
    if(__UNUSEFUL__)
        LOG_INFO << "OnDispatchLoginMsg";
        
	SLoginBaseMsg * pMsg = static_cast<SLoginBaseMsg*>(basemsg);
    switch(pMsg->_protocol)
    {
    case SLoginBaseMsg::KKDD_LOGINMSG_LOGIN:    //账号登录消息
        {
            //连接状态处理，登录成功重新设置上下文，
            pet->OnNetLoginMsg(static_cast<SQLoginMsg*>(pMsg));
        }
        break;
    case SLoginBaseMsg::KKDD_LOGINMSG_REGISTER: //账号注册消息
        {
            //连接状态处理，登录成功重新设置上下文，
        }
        break;
    case SLoginBaseMsg::KKDD_LOGINMSG_SETINFO:  //设置账号信息
        {
            
        }
        break;
    case SLoginBaseMsg::KKDD_LOGINMSG_GEITINFO: //获取账号信息
        {
            
        }
        break;
    default:
        //log error type msg
        break;    


    }

    return false;
}

bool PetsManager::OnDispatchChatMsg(DNID dnid, SMessage *basemsg,Pet* pet)
{
	SChatBaseMsg * pMsg = static_cast<SChatBaseMsg*>(basemsg);
    switch(pMsg->_protocol)
    {
    case SChatBaseMsg::KKDD_CHATMSG_WHISPER: ////私聊
        {
            
        }
        break;
    case SChatBaseMsg::KKDD_CHATMSG_BROADCAST: //广播
        {
            
        }
        break;
    case SChatBaseMsg::KKDD_CHATMSG_SYSTEM: //系统消息
        {
            
        }
        break;
    case SChatBaseMsg::KKDD_CHATMSG_ROOM: //房间消息
        {
            
        }
        break;
    default:
        //log error type msg
        break;  
    }
    return false;
}

bool PetsManager::OnDispatchMoveMsg(DNID dnid, SMessage *basemsg,Pet* pet)
{
	SMoveBaseMsg * pMsg = static_cast<SMoveBaseMsg*>(basemsg);
    switch(pMsg->_protocol)
    {
    case SMoveBaseMsg::KKDD_MOVEMSG_MOVE: //普通移动消息
        {
            
        }
        break;
    case SMoveBaseMsg::KKDD_MOVEMSG_POSITION: //服务器校正数据消息
        {
            
        }
        break;
    case SMoveBaseMsg::KKDD_MOVEMSG_JUMP: //跳跃消息
        {
            
        }
      break;
    default:
        //log error type msg
        break;  
    }
    return false;
}

bool PetsManager::Run()
{
    //

    return CObjectManager::Run();
}
