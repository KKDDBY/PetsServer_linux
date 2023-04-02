//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//FileName		    :   Object.hpp
//FileAuthor		:	KKDD
//FileCreateDate	:	2023-3-14
//FileDescription	:	基本对象类型

//模块说明：所有对象类型的基类,封装了对象最基本的行为

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include"Define.hpp"

class Object
{
public:
    Object():m_IID(0){}
    Object *DynamicCast(const DWORD IID) { if (this == NULL) return NULL; return (IID == m_IID) ? this : VDC(IID); }
	virtual Object *VDC(const DWORD IID) { return NULL; }

private:
    DWORD m_IID;    //类型ID
};

#endif