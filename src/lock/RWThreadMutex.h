#ifndef __H__RWTHREADMUTEX__H__
#define __H__RWTHREADMUTEX__H__

#include <pthread.h>

class CRwThreadMutex
{
public:
	CRwThreadMutex();
	~CRwThreadMutex();

	int Init();
	int Destroy();
	int remove();
	int acquire_read();
	int acquire_write();
	int tryacquire_read();
	int tryacquire_write();
	int acquire();
	int tryacquire();
	int release();

private:
	pthread_rwlockattr_t m_RwAttr;
	pthread_rwlock_t m_RwLock;
	bool m_bRemoved;
};

#endif
