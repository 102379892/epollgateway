#ifndef __H__BASESOCKET__H__
#define __H__BASESOCKET__H__

#include <string>
#include <netinet/in.h>
#include "sock/EpoolCtl.h"

class CTcpBaseHandle
{
public:
	CTcpBaseHandle();
	~CTcpBaseHandle();

	int Init(const int threadNum = 1);
	void Stop();
	int Listen(const std::string& ip, const int port);
	bool isListenFd(const int fd);

	void SetSendBufSize(const int fd, const int sendSize);
	void SetRecvBufSize(const int fd, const int recvSize);
	void SetNonblock(const int fd);
	void SetReuseAddr(const int fd);
	void SetNoDelay(const int fd);

	int AcceptConnect();

private:
	int m_threadNum;
	int m_listenPort;
	std::string m_listenAddr;

	int m_listenFd;
	CEpoolCtl* m_poolList;
};

#endif

