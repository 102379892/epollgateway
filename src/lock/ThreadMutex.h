#ifndef __H__THREADMUTEX__H__
#define __H__THREADMUTEX__H__

#include <pthread.h>

class CThreadMutex
{
public:
	CThreadMutex();
	~CThreadMutex();

	int Init();
	int Destroy();
	int acquire();
	int tryacquire();
	int release();
	int remove();

	int acquire_read();
	int acquire_write();
	int tryacquire_read();
	int tryacquire_write();

private:
	pthread_mutexattr_t m_Mattr;
	pthread_mutex_t m_Mutex;
	bool m_bRemoved;
};

#endif
