#include"muduo/base/Logging.h"

#include "Dispatcher.hpp"
#include"stdAfx.h"


void Dispatcher::InitDispatcher()
{

}

void Dispatcher::BindCB(MESSAGETYPE type,const DispatcherCallBack &cb)
{
    m_callBackMap[type] = cb;
}

void Dispatcher::PushPack(SPacket *pack)
{
    m_msgQueueMtx.lock();
    m_MsgQueue.push(pack);
    m_msgQueueMtx.unlock();

    if(__UNUSEFUL__)
        LOG_INFO << "Dispatcher has push a pack,now size is: " << m_MsgQueue.size();
}

bool Dispatcher::GetPack(SPacket **pack)
{
    if(m_MsgQueue.empty())
        return false;

    m_msgQueueMtx.lock();       //上锁
    *pack = m_MsgQueue.front(); //出包
    m_MsgQueue.pop();           //出队
    m_msgQueueMtx.unlock();     //解锁
    
    if(__UNUSEFUL__)
        LOG_INFO << "Dispatcher has pop a pack,now size is: " << m_MsgQueue.size();

    return true;
}

int Dispatcher::DispatchMsg()
{
    //处理极限为1s,超时剩下的包下一帧处理
    int msgCounter = 0;
    QWORD beginTimeStamp = m_dispcTimmer.getCurrentTimeStampMiliSec();
    while(m_dispcTimmer.getCurrentTimeStampMiliSec() - beginTimeStamp <= 1000)  
    {
        SPacket *pack = nullptr;
        if(!GetPack(&pack))
            break;
        
        if(!pack->buffer)
            continue;
            
        //消息分发
        SMessage* pmsg = static_cast<SMessage*>(pack->buffer);
        m_callBackMap[pmsg->_protocol](pack->dnid,pmsg,static_cast<Pet*>(pack->context));

        //pack资源释放
        if(pack->buffer)
            delete pack->buffer;
        delete pack;
        ++msgCounter;
    }
    return msgCounter;
}

