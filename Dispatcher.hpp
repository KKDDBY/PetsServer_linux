//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   Dispatcher.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-1
//FileDescription	:	消息分发器

//模块说明：解码编码器（codec）通过解码之后调用分发器，分发器通过提前注册的功能回调函数，对不同类型消息做逻辑处理
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include<map> 
#include<functional>
#include<queue>

#include "muduo/base/noncopyable.h"
#include "muduo/base/Mutex.h"

#include"SMessage.hpp"
#include"CELLTimestamp.hpp"
#include"Define.hpp"

class Pet;

struct SPacket
{
    DNID dnid;
    void* buffer;
    void* context;
};

class Dispatcher:public muduo::noncopyable
{
public:
    //typedef void(*DispatcherCallBack)(SMessage* msg);
    typedef std::function<void (DNID,SMessage*,Pet*)> DispatcherCallBack;
    void InitDispatcher();   
    void BindCB(MESSAGETYPE type,const DispatcherCallBack& cb); 
    void PushPack(SPacket *pack);  
    bool GetPack(SPacket **pack);
    int  DispatchMsg(); //消息分发
private:
    std::map<MESSAGETYPE,DispatcherCallBack> m_callBackMap; //消息类型回调映射
    std::queue<SPacket*> m_MsgQueue;    //封包队列
    CELLTimestamp m_dispcTimmer;        //消息分发器定时器
    muduo::MutexLock m_msgQueueMtx;     //封包队列互斥量
};

#endif
