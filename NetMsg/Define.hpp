//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   define.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-2-28
//FileDescription	:	KKDD定义文件

//模块说明：该文件定义了一些由功能常规需要的宏，
//头文件包含顺序：c库，c++库，三方库，当前工程头文件

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DEFINE_HPP__
#define __DEFINE_HPP__

#include<stdint.h>
#include<boost/any.hpp>
#include<iostream>
#include<fstream>

#ifndef KKDD_TEST
	#define __UNUSEFUL__ 0
#else
	#define __UNUSEFUL__ 1
#endif

//64位无符号整型定义 QWORD
typedef uint64_t QWORD;

//32位无符号整型定义 WORD
typedef uint32_t DWORD;

//16位无符号整型定义 word
typedef uint16_t WORD;

//8位无符号整型定义 BYTE
typedef uint8_t BYTE;

//聊天消息最大长度
#define MAX_CHAT_LEN 128

//tile，将屏幕分割为大小相等的格子，便于地图计算
typedef WORD TILE;

//定义一个点结构体
typedef struct
{
    WORD x;
    WORD y;
}POINT;

//定义网络消息包相关的大小，网络包头部大小，网络包体最大字节65520字节
#define HEADMSGLEN 4
#define BODYMSGMAXLEN 0xFFF0

//定义消息消息类型  最大支持65535
typedef uint16_t MESSAGETYPE;

//连接时，服务器为客户端分配的DNID 唯一ID，作为连接号(高16位时间戳+低16位随机数)
typedef  DWORD DNID;

//对象IID标识
#define IID_PETS 0x1
#define IID_ITEM 0x2

//服务器帧率
#define SERVER_FPS 20

//连接状态
enum LINKSTATUS
{
	LinkStatus_UnConnected,         // 无效的连接状态(初始化状态)
	LinkStatus_Login,	            // 连接后等待认证结果的状态（TCP三次握手成功的状态）
	LinkStatus_Connected,           // 连接后认证通过，可以正常使用的状态(登录成功状态)
	LinkStatus_Disconnected,        // 连接已断开的状态
};



#endif
