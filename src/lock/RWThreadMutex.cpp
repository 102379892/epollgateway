#include "lock/RWThreadMutex.h"

CRwThreadMutex::CRwThreadMutex()
:m_bRemoved(true)
{

}

CRwThreadMutex::~CRwThreadMutex()
{
	remove();
}

int CRwThreadMutex::Init()
{
	//初始化读写锁属性,缺省值为PTHREAD_PROCESS_PRIVATE
	int nRet = pthread_rwlockattr_init(&m_RwAttr);
	if (nRet < 0)
	{
		return -1;
	}

	//设置读写锁属性
	nRet = pthread_rwlockattr_setpshared(&m_RwAttr, PTHREAD_PROCESS_PRIVATE);
	if (nRet < 0)
	{
		pthread_rwlockattr_destroy(&m_RwAttr);
		return -1;
	}

	nRet = pthread_rwlock_init(&m_RwLock, &m_RwAttr);
	if (nRet < 0)
	{
		pthread_rwlockattr_destroy(&m_RwAttr);
		return -1;
	}

	m_bRemoved = false;
	return 0;
}

int CRwThreadMutex::Destroy()
{
	int nRntCode = 0;
	//销毁读写锁属性
	int nRet = pthread_rwlockattr_destroy(&m_RwAttr);
	if (nRet < 0)
	{
		nRntCode = -1;
	}

	nRet = pthread_rwlock_destroy(&m_RwLock);
	if (nRet < 0)
	{
		nRntCode = -1;
	}

	m_bRemoved = true;
	return nRntCode;
}

int CRwThreadMutex::remove()
{
	if (!m_bRemoved)
	{
		return Destroy();
	}

	return 0;
}

int CRwThreadMutex::acquire_read()
{
	return pthread_rwlock_rdlock(&m_RwLock);
}

int CRwThreadMutex::acquire_write()
{
	return pthread_rwlock_wrlock(&m_RwLock);
}

int CRwThreadMutex::tryacquire_read()
{
	return pthread_rwlock_tryrdlock(&m_RwLock);
}

int CRwThreadMutex::tryacquire_write()
{
	return pthread_rwlock_trywrlock(&m_RwLock);
}

int CRwThreadMutex::acquire()
{
	return pthread_rwlock_wrlock(&m_RwLock);
}

int CRwThreadMutex::tryacquire()
{
	return pthread_rwlock_trywrlock(&m_RwLock);
}

int CRwThreadMutex::release()
{
	return pthread_rwlock_unlock(&m_RwLock);
}
