#ifndef __H__THREAD__H__
#define __H__THREAD__H__

#include <pthread.h>
#include <unistd.h>

class CThread
{
public:
	CThread(bool detachable = true, size_t stackSize = 0);
	virtual ~CThread();

	int Start();
	void Stop();
	virtual void run() = 0;
	
private:
	void Wait();
	static void* thread_run(void* arg);

protected:
	bool m_isRun;
	bool m_isStop;
	
	bool m_detachable;
	size_t m_stackSize;
	pthread_t m_threadId;
};

#endif
