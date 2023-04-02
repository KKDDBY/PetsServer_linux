//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   petsServer.cpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-2-28
//FileDescription	:	游戏主服务

//模块说明：游戏主要逻辑定义

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __PETSSERVER_HPP__
#define __PETSSERVER_HPP__

//c/c++
#include <unordered_map>
#include <unordered_set>
#include <string.h>
#include <map>
#include <queue>
//#include <pthread.h>
#include<thread>

//thrid-party
#include "muduo/net/TcpServer.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Logging.h"
//own
#include "codec.hpp"
#include"CELLTimestamp.hpp"
#include"PetsManager.hpp"
#include"MysqlQuery.hpp"
#include"SQLDefine.hpp"

using namespace muduo;
using std::placeholders::_4;
using namespace KKDD_SQL_MODULE;    //使用mysql模块命名空间

class PetsServer: public muduo::noncopyable
{
public:
    struct Connection
    {
        net::TcpConnectionPtr conn;  //muduoTCP连接对象
        int ttl;                     //心跳ttl
        Connection(){ttl = 3; }
        Connection(const net::TcpConnectionPtr& _conn):
            conn(_conn), ttl(3){}
    };
    typedef std::map<DNID, Connection> ConnectionMap;  

public:
    PetsServer(net::EventLoop* loop, const net::InetAddress& listenAddr);
    ~PetsServer();
    bool InitServer();  //服务器初始化
    bool start();       //服务器开启
    bool ServerLoop();  //服务器循环 20fps
    //bool MysqlLoop();   //mysql循环

private:
    void onConnection(const net::TcpConnectionPtr& conn);                           //客户端连接触发的回调
    void checkAlive();      //心跳机制 定时器定时处理任务 20s触发一次，3次没有响应，关闭连接 数据入库 释放资源。
    DNID GreateDnid();      //DNID网络链接号生成 前十六位time时间戳的后16位，后十六位随机数
   
private:
    ConnectionMap connections;            //连接对象池
    Codec* codec;                         //编码解码器
    net::TcpServer server;                //muduo tcp服务
    CELLTimestamp m_servertimer;          //时钟
    std::thread* m_mainThread;            //游戏逻辑线程
    PetsManager m_petsManager;            //宠物管理器(玩家管理器)
    std::thread* m_mysqlThread;           //数据库交互线程
    MyQuery*    m_sqlIns;                 //数据库模块 注意这里仅仅保存  sql模块的指针，不对他做资源释放操作，因为他是单例
    bool m_exitFlag;                      //线程退出标记

};

#endif