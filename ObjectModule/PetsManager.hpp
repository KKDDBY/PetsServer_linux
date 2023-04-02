//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   PetsManager.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-12
//FileDescription	:	pets对象管理器

//模块说明：宠物对象管理器

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __PETSMANAGER_HPP__
#define __PETSMANAGER_HPP__

#include<map>
#include<list>

#include "muduo/net/TcpConnection.h"

#include"stdAfx.h"
#include"ObjectManager.hpp"
#include"LoginMsg.hpp"
#include"MoveMsg.hpp"
#include"ChatMsg.hpp"
#include"Pet.hpp"

class PetsManager:public CObjectManager
{
public:
    PetsManager();
    ~PetsManager();

  //  void OnHeartbeat(const muduo::net::TcpConnectionPtr& conn, muduo::Timestamp);   //客户端发来心跳消息
    bool Run(); 
    Object *CreateObject(const DWORD IID);   // 生成一个IID类型的 对象

    //消息分类  
    bool OnDispatchLoginMsg(DNID dnid, SMessage* basemsg,Pet* pet);    //登录消息类处理 注意登录消息pet是为空的
    bool OnDispatchChatMsg(DNID dnid, SMessage* basemsg,Pet* pet);     //聊天消息类处理
    bool OnDispatchMoveMsg(DNID dnid, SMessage* basemsg,Pet* pet);     //移动消息类处理   

    

private:


};

#endif