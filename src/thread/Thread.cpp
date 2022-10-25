#include <string.h>
#include <errno.h>
#include "clog/CLog.h"
#include "thread/Thread.h"

CThread::CThread(bool detachable, size_t stackSize) 
:m_isRun(false), m_isStop(true),
m_detachable(detachable), m_stackSize(stackSize), 
m_threadId(0)
{

}

CThread::~CThread()
{
}

int CThread::Start()
{
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr);
	if (ret != 0)
	{
		mglog(LL_ERROR, "CThread pthread_attr_init failed(ret:%d,err:%m).", ret);
		return -1;
	}

	if (m_detachable)
	{
		ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (ret != 0)
		{
			mglog(LL_ERROR, "CThread pthread_attr_setdetachstate failed(ret:%d,err:%m).", ret);
			pthread_attr_destroy(&attr);
			return -2;
		}
	}

	if (m_stackSize > 0)
	{
		ret = pthread_attr_setstacksize(&attr, m_stackSize);
		if (ret != 0)
		{
			mglog(LL_ERROR, "CThread pthread_attr_setstacksize failed(ret:%d,err:%m).", ret);
			pthread_attr_destroy(&attr);
			return -3;
		}
	}
	
	ret = pthread_create(&m_threadId, &attr, CThread::thread_run, (void*)this);
	if (ret != 0)
	{
		mglog(LL_ERROR, "CThread pthread_create failed(ret:%d,err:%m).", ret);
		pthread_attr_destroy(&attr);
		return -4;	
	}

	ret = pthread_attr_destroy(&attr);
	if (ret != 0)
	{
		mglog(LL_ERROR, "CThread pthread_attr_destroy failed(ret:%d,err:%m).", ret);
	}

	mglog(LL_INFO, "CThread pthread_create success(%ld).", m_threadId);
	return 0;
}

void CThread::Stop()
{
	m_isRun = true;
	Wait();
}

void CThread::Wait()
{
	while(m_isStop == true) 
	{
		mglog(LL_INFO, "CThread wait(%ld).", m_threadId);
		sleep(1);
	}

	if (m_detachable)
	{
		return;
	}

	if (m_threadId == 0)
	{
		return;
	}

	int ret = pthread_join(m_threadId, NULL);
	if (ret != 0)
	{
		mglog(LL_ERROR, "CThread pthread_join failed(ret:%d,err:%m).", ret);
		return;
	}
}

void* CThread::thread_run(void* arg)
{
	CThread* thr = (CThread*)arg;
	thr->run();
	return NULL;
}
