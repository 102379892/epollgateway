#include "lock/ThreadMutex.h"

CThreadMutex::CThreadMutex()
:m_bRemoved(false)
{

}

CThreadMutex::~CThreadMutex()
{

}

int CThreadMutex::Init()
{
	//初始化互斥锁属性对象,缺省值为PTHREAD_PROCESS_PRIVATE
	int nRet = pthread_mutexattr_init(&m_Mattr);
	if (nRet < 0)
	{
		return -1;
	}

	//设置互斥锁的范围
	nRet = pthread_mutexattr_setpshared(&m_Mattr, PTHREAD_PROCESS_PRIVATE);
	if (nRet < 0)
	{
		pthread_mutexattr_destroy(&m_Mattr);
		return -1;
	}
	
	//设置互斥锁类型的属性,缺省值为PTHREAD_MUTEX_DEFAULT
	//PTHREAD_MUTEX_ERRORCHECK类型的互斥锁可提供错误检查
	nRet = pthread_mutexattr_settype(&m_Mattr, PTHREAD_MUTEX_DEFAULT);
	if (nRet < 0)
	{
		pthread_mutexattr_destroy(&m_Mattr);
		return -1;
	}
	
	nRet = pthread_mutex_init(&m_Mutex, &m_Mattr);
	if (nRet < 0)
	{
		pthread_mutexattr_destroy(&m_Mattr);
		return -1;
	}

	return 0;
}

int CThreadMutex::Destroy()
{
	int nRntCode = 0;
	//销毁互斥锁属性对象
	int nRet = pthread_mutexattr_destroy(&m_Mattr);
	if (nRet < 0)
	{
		nRntCode = -1;
	}

	nRet = pthread_mutex_destroy(&m_Mutex);
	if (nRet < 0)
	{
		nRntCode = -1;
	}

	m_bRemoved = true;
	return nRntCode;
}

int CThreadMutex::acquire()
{
	return pthread_mutex_lock(&m_Mutex);
}

int CThreadMutex::tryacquire()
{
	return pthread_mutex_trylock(&m_Mutex);
}

int CThreadMutex::release()
{
	return pthread_mutex_unlock(&m_Mutex);
}

int CThreadMutex::remove()
{
	if (!m_bRemoved)
	{
		return Destroy();
	}

	return 0;
}

int CThreadMutex::acquire_read()
{
	return 0;
}

int CThreadMutex::acquire_write()
{
	return 0;
}
int CThreadMutex::tryacquire_read()
{
	return 0;
}
int CThreadMutex::tryacquire_write()
{
	return 0;
}

