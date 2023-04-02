//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   ObjectManager.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-14
//FileDescription	:	基本对象类型管理器

//模块说明：用来管理对象的模块，包括对象的创建删除，修改,遍历等

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __OBJECTMANAGER_HPP__
#define __OBJECTMANAGER_HPP__

#include<map>
#include<list>

#include"Define.hpp"
#include"Object.hpp"


typedef bool ( *TRAVERSALFUNC) (Object* &, void*);

class CObjectManager
{
public:
    CObjectManager(void);
    virtual ~CObjectManager(void);

public:
    // 根据最大容纳数目（lMaxObject）
    bool Initialize(long lMaxObject=0); 

    // 执行一些必要的操作
    bool Run(); 

    // 释放所有对象，关闭管理器
    bool Destroy(); 

    //释放一个对象
    bool RealseObject(DNID dnid);

protected:
    virtual Object *CreateObject(const DWORD IID) = 0;   // 生成一个IID类型的 对象

public:
    // 用关键字来查找目标
    Object *_Locate(const DWORD GID); 

    // 遍历列表回调函数
    int TraversalObjects(TRAVERSALFUNC TraversalFunc, void* Param);

    // 清空该容器的所有IObject子对象
    void Clear(); 

    // 确认当前容器中有多少个对象
    size_t size() { return m_ObjectMap.size(); }

protected:
    long m_lMaxSize;                          // 该管理器能容纳对象的最大数目，为0的时候忽略该限制
    std::map<DNID, Object*> m_ObjectMap;      // 容纳对象的容器
    std::list<unsigned DNID> m_RemoveList;    // 待删除列表

};


#endif