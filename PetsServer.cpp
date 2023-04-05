//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   petsServer.cpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-2-28
//FileDescription	:	游戏主服务

//模块说明：游戏主要逻辑定义

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

//c/c++
#include<chrono>

#include"stdAfx.h"
#include"PetsServer.hpp"
#include"SMessage.hpp"
#include"Algorithm.hpp"
#include"MysqlQuery.hpp"

using namespace KKDD_SQL_MODULE;

PetsServer::PetsServer(net::EventLoop *loop, const net::InetAddress &listenAddr):
        server(loop, listenAddr, "PetsServer"),
        codec(new Codec)
{
    m_mainThread = nullptr;
    m_mysqlThread = nullptr;
    m_sqlIns = nullptr;
    m_exitFlag = false;
}

PetsServer::~PetsServer()
{
    if(m_mainThread && m_mainThread->joinable())
    {
        m_mainThread->join();
        delete m_mainThread;
        m_mainThread = nullptr;
    }  

    if(m_mysqlThread && m_mysqlThread->joinable())
    {
        m_mysqlThread->join();
        delete m_mysqlThread;
        m_mysqlThread = nullptr;
    }

    if(codec)
        delete codec;
}

bool PetsServer::InitServer()
{
    server.setConnectionCallback(std::bind(&PetsServer::onConnection, this, _1));                        //OK 当有连接上来，保存用户连接
    server.setMessageCallback(std::bind(&Codec::onMessage, codec, _1, _2, _3));    //当有消息发上来，将消息
    server.setThreadNum(20);

    //注册消息转发函数
    codec->GetDispatcher()->BindCB(SMessage::KKDD_CHAT_MESSAGE_BASE,std::bind(&PetsManager::OnDispatchChatMsg,&this->m_petsManager,_1,_2,_3));
    codec->GetDispatcher()->BindCB(SMessage::KKDD_MOVE_MESSAGE_BASE,std::bind(&PetsManager::OnDispatchMoveMsg,&this->m_petsManager,_1,_2,_3));
    codec->GetDispatcher()->BindCB(SMessage::KKDD_LOGIN_MESSAGE_BASE,std::bind(&PetsManager::OnDispatchLoginMsg,&this->m_petsManager,_1,_2,_3));

    //mysql模块初始化
    m_sqlIns = MyQuery::GetQueryInstance();
    if(!m_sqlIns)
    {   
        LOG_INFO << "MyQuery::GetQueryInstance() failed";return false;
    }

    if(!m_sqlIns->Init(DB_USERNAME,DB_PASSWORD,DB_IPADDRESS,DB_DBNAME,DB_IPPORT))
        return false;

    return true;
}

bool PetsServer::start()
{
    if(!InitServer())
        return false;

    //创建服务器逻辑线程，专门用来跑服务器逻辑
    m_mainThread = new std::thread(&PetsServer::ServerLoop,this);
    if(!m_mainThread)
    {
        LOG_ERROR << "m_mainThread start failed...";
        return false;
    }   
    LOG_ERROR << "m_mainThread start successfully...";

    //创建数据库服务线程，用来处理数据库相关逻辑
    m_mysqlThread = new std::thread(&MyQuery::MysqlLoop,m_sqlIns);
    if(!m_mysqlThread)
    {
        LOG_ERROR << "m_mysqlThread start failed...";
        return false;
    }   
    LOG_INFO << "m_mysqlThread start successfully...";
    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    server.start();
    net::EventLoop* loop = server.getLoop();
    if(!loop)
        return false;
    loop->runEvery(2, std::bind(&PetsServer::checkAlive, this));
    LOG_INFO << "pets server start";
    if(__UNUSEFUL__)
        LOG_INFO << "KKDD_TEST DEFINE OPEN";
    else
        LOG_INFO << "KKDD_TEST DEFINE CLOSE";


    //MyQuery::QueryResult qr;
    //m_sqlIns->ExecuteSQL("select @_1",&qr);
    //m_sqlIns->ExecuteSQL("set @_1 = null",&qr);

    MyQuery::QueryBind* bind = new MyQuery::QueryBind(2);
    (*bind)[0].BindString("TEST",strlen("TEST"));
    (*bind)[1].BindString("123456789",strlen("123456789"));
    Pet pet;
    pet.PrepareSQL(SQL_PACK_TYPE::SQL_STMT_TYPE,SELECT_PETSLOGIN_STMT,bind,2,std::bind(&Pet::OnMysqlLoginMsg,&pet,_1));

    boost::any* bind1 = new  boost::any[2];
    bind1[0] = (char*)"TEST";   //只需要准备in inout数据
    char* s = boost::any_cast<char*>(bind1[0]);
    LOG_INFO << s ;
    pet.PrepareSQL(SQL_PACK_TYPE::SQL_PROCEDURE_TYPE,"p_test",bind1,2,std::bind(&Pet::TESTFUNC,&pet,_1));

    return true;
}

bool PetsServer::ServerLoop()   
{
    LOG_INFO << "Game server frame synchronization loop start"; //游戏服务器帧同步开启
    while(!m_exitFlag)    //20fps 50ms/times 服务器帧率
    {
        //游戏循环
        if(m_servertimer.getElapsedTimeMilliSec() <= 1000/SERVER_FPS) //
        {
            //封包队列处理
            codec->GetDispatcher()->DispatchMsg();  //调用codec的消息分发器，去分发消息

            //各模块运行
            m_petsManager.Run();
            
            //LOG_INFO << "use time: " << m_servertimer.getElapsedTimeMilliSec() << "ms";
        }

        //游戏休眠
        int iUseTime = m_servertimer.getElapsedTimeMilliSec();  //返回的是毫秒值，向下取整
        if(iUseTime > (1000/SERVER_FPS))  
        {
            LOG_WARN << "警告!!! 进入等待时间累计超时，可能是服务器过载，也可能是服务器内部出错";
            LOG_WARN << "服务器帧数:" << SERVER_FPS << " 当前帧超时时间:" << (iUseTime - (1000/SERVER_FPS)) << "ms";

        }
        else    
        {
            //休眠空余时间
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/SERVER_FPS - iUseTime));
        }

        //跟新时钟，进入下一帧的循环
        m_servertimer.update();
    }
}

void PetsServer::onConnection(const net::TcpConnectionPtr &conn)
{
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
        //创建连接对象，此时单纯建立TCP三次握手连接。ConnectionMap里的tcpcon上下文，在登录成功之后设置(如果登录失败，释放响应的资源)
        DNID dnid = GreateDnid();   //生成唯一网络连接号
        Connection scon;
        conn->setContext(boost::any(dnid)); //未创建对象的时候上下文是dnid，创建对象之后重设上下文为pet对象
        scon.conn = conn;
        scon.ttl = 3;
        connections[dnid] = scon;
        LOG_INFO << "为当前网络连接创建网络ID号 DNID:[" << dnid <<"]";
    }
    else    //主动退出 释放资源
    {  
        //客户端断开连接，释放该客户端的资源...
        Pet* pet = const_cast<Pet*>(boost::any_cast<const Pet>(&conn->getContext()));
        if(pet)
        {
            m_petsManager.RealseObject(pet->GetDnid());
            connections.erase(pet->GetDnid());
            LOG_INFO << "DNID:[" << pet->GetDnid()<< "] 正常退出,name:"  << pet->m_Property.m_name ;
        }
        else    //这种情况是 pet 仅仅做了TCP三次握手，还没有创建对象的情况
        {   
            LOG_INFO << "connections.size: " << connections.size();
            DNID dnid = boost::any_cast<DNID>(conn->getContext());
            connections.erase(dnid);
            LOG_INFO << "DNID:[" << dnid << "] 正常退出,未创建对象，仅做了TCP三次握手";
            LOG_INFO << "connections.size: " << connections.size();

        }
    }
}

void PetsServer::checkAlive()
{
    //LOG_INFO <<"checkAlive: " <<  pthread_self();
}

DNID PetsServer::GreateDnid()
{
    DNID dnid = 0;
    srand(time(0));
    while(1)
    {
        dnid = ((time(NULL)<< 16)| (rand()%0XFFFF));
        if(connections.find(dnid) == connections.end()) //避免冲突
            return dnid;
        LOG_INFO << "为客户端创建网络连接号重复 DNID:[" << dnid <<"]";
    }
}
