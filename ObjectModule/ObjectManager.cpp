#include"ObjectManager.hpp"

CObjectManager::CObjectManager(void)
{
}

CObjectManager::~CObjectManager(void)
{
	Clear();
}

bool CObjectManager::Initialize(long lMaxObject)
{
    m_lMaxSize = lMaxObject;
    return true;
}

bool CObjectManager::Run()
{
	std::map<DNID, Object*>::iterator it = m_ObjectMap.begin();
    for(; it != m_ObjectMap.end();it++ )
	{
		Object *pObject = it->second;
		if (pObject == NULL)
			m_RemoveList.push_back(it->first);
	}

	// 角色清除
	it = m_ObjectMap.end();
	while (m_RemoveList.size())
	{
		it = m_ObjectMap.find(m_RemoveList.front());
		if (it != m_ObjectMap.end())
			m_ObjectMap.erase(it);

		m_RemoveList.pop_front();
	}

    //Unlock();

    return true;
}

bool CObjectManager::Destroy()
{
    Clear();
    return true;
}

bool CObjectManager::RealseObject(DNID dnid)
{	
	Object* ret = nullptr;		//这里一定要用引用，否者就算delete ret，m_ObjectMap的second也不一定为nullptr
    std::map<DNID,Object*>::iterator it = m_ObjectMap.find(dnid);

    if(it != m_ObjectMap.end())
        ret = it->second;

	if(ret == nullptr)
    	return false;

	delete ret;
	it->second = nullptr;
	
	return true;
}

Object* CObjectManager::_Locate(const DWORD GID)
{
    Object *ret = NULL;
    //Lock();
    std::map<DNID,Object*>::iterator it = m_ObjectMap.find(GID);
    if(it != m_ObjectMap.end())
        ret = it->second;
    //Unlock();

    return ret;
}

int CObjectManager::TraversalObjects(TRAVERSALFUNC TraversalFunc, void *Param)
{
   for ( std::map<DNID, Object*>::iterator it = m_ObjectMap.begin(); it != m_ObjectMap.end();it++)
	{
		Object *pObject = it->second;
		if (pObject == NULL)
		{
			// m_RemoveList.push_back(it->first);
			continue;
		}

		if (!pObject)
			continue; // m_RemoveList.push_back(it->first);
		else if (TraversalFunc(it->second, Param) == 0)
			break;
	}

    return (int)m_ObjectMap.size();
}

void CObjectManager::Clear()
{
    m_lMaxSize = 0;
	for ( std::map<DNID, Object*>::iterator it = m_ObjectMap.begin(); it != m_ObjectMap.end();it++)
	{
		Object *pObject = it->second;
		if (pObject)
        {
			delete pObject;
            pObject = nullptr;
        }
	}
}
