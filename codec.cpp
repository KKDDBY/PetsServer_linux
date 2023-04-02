#include<thread>
#include<iostream>

#include"codec.hpp"
#include"stdAfx.h"
#include"LoginMsg.hpp"
#include"Pet.hpp"

Codec::Codec()
{
    m_dispc = nullptr;
    InitCodecMem();
}

void Codec::InitCodecMem()
{
    m_dispc = new Dispatcher;
}

Dispatcher* &Codec::GetDispatcher()
{
    return m_dispc;
}

void Codec::onMessage(const muduo::net::TcpConnectionPtr &conn, 
                      muduo::net::Buffer *buf,
                      muduo::Timestamp receiveTime)
{
    //std::cout << "onMessage: " << std::this_thread::get_id() << std::endl;
    while(buf->readableBytes() >= HEADMSGLEN)
    {
        const void* header = buf->peek();
        DWORD msgLen = static_cast<const DWORD*>(header)[0]; //前四个字节是数据头，也就是消息长度
        const int32_t len = muduo::net::sockets::networkToHost32(msgLen);
        if (len > 65536 || len < 0)
        {
            LOG_ERROR << "Invalid length " << len;
            conn->shutdown();  // FIXME: disable reading
            break;
        }
        else if (buf->readableBytes() >= len + HEADMSGLEN)
        {
            buf->retrieve(HEADMSGLEN);
            std::string data(buf->peek(), len);
            //线程安全的组包
            {
                if(__UNUSEFUL__)
                  LOG_INFO << "OnRecv msg len: " << len;

                //这里很不明智，仅仅为了登录注册两个消息就要每次都调用getContext转换，有点浪费效率，最好的办法是吧登录注册两个消息单独提出来做处理，其余消息都是connected
                SPacket* pack = new SPacket;
                //pack->m_dindClient = 
                Pet* pet = const_cast<Pet*>(boost::any_cast<const Pet>(&conn->getContext()));
                if(pet)
                {
                  pack->dnid = pet->GetDnid();
                  pack->context = pet;
                }
                else
                {
                 pack->dnid = boost::any_cast<DNID>(conn->getContext());
                 pack->context = nullptr;  
                }
                pack->buffer = malloc(len);
                memcpy(pack->buffer,data.c_str(),len);
                m_dispc->PushPack(pack);
            }
            buf->retrieve(len);
        }
        else
        {
            break;
        }
    }

     
}

void Codec::SendMsg(muduo::net::TcpConnection *conn, const void *data, DWORD size)
{
    muduo::net::Buffer buf;
    buf.append(data,size);
    int32_t len = static_cast<int32_t>(size);
    int32_t be32 = muduo::net::sockets::hostToNetwork32(len);   //主机字节序转网络字节序
    buf.prepend(&be32, sizeof be32);
    conn->send(&buf);
    return;
}
