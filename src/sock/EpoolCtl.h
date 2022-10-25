#ifndef __H__EPOOLCTL__H__
#define __H__EPOOLCTL__H__

#include <string>
#include <map>
#include <netinet/in.h>
#include "recv/SockClient.h"
#include "thread/Thread.h"

class CEpoolCtl:virtual public CThread
{
public:
	CEpoolCtl();
	~CEpoolCtl();

	int Init(const int waitTimeout = -1);
	void Stop();

	virtual void run();

	int AddPool(const int fd, const int type = 0);
	void AddConnect(const int fd, const std::string ip, const int port);

	int RecvEvent(const int fd);
	int WriteEvent(const int fd);
	int CloseEvent(const int fd);

public:
	int m_waitTimeout;
	int m_epfd;
	char* m_recvbuf;
};

#endif

