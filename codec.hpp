//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   codec.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-2-28
//FileDescription	:	解码编码器

//模块说明：底层网络消息调用该模块的回调，然后将网络TCP包，进行拆包,将拆好的包放入封包队列，交友游戏逻辑线程处理
/*Codec 是 encoder 和 decoder 的缩写，这是一个到软硬件都在使用的术语，这里我借指“把网络数据和业务消息之间互相转换”的代码。*/
/*codec 的基本功能之一是做 TCP 分包：确定每条消息的长度，为消息划分界限。在 non-blocking 网络编程中，codec 几乎是必不可少的。
如果只收到了半条消息，那么不会触发消息回调，数据会停留在 Buffer 里（数据已经读到 Buffer 中了），等待收到一个完整的消息再通知处理函数。*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CLIPSERVER_CODEC_H__
#define __CLIPSERVER_CODEC_H__

//c/c++
#include <string>
#include <functional>
#include <fstream>


//third-part
#include "muduo/net/TcpConnection.h"
#include "muduo/net/Buffer.h"
#include "muduo/net/Endian.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
//own
#include "Dispatcher.hpp"

class Codec
{
public:
    Codec();

    void InitCodecMem();

    Dispatcher* & GetDispatcher();

    //网络消息回调
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp receiveTime);

    //发送消息给客户端
    void  SendMsg(muduo::net::TcpConnection* conn,
                 const void* data,DWORD size);      
    

private:
    Dispatcher* m_dispc;
};

#endif
