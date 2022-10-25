#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include "common/ErrorCode.h"
#include "clog/CLog.h"
#include "sock/BaseSocket.h"
#include "common/Singleton.h"

CTcpBaseHandle::CTcpBaseHandle()
:m_threadNum(0), m_listenPort(0), m_listenAddr(""),
m_listenFd(-1)
{

}

CTcpBaseHandle::~CTcpBaseHandle()
{
	Stop();
}

int CTcpBaseHandle::Init(const int threadNum)
{
	m_threadNum = threadNum;
	m_poolList = new CEpoolCtl[m_threadNum];
	if (m_poolList == NULL)
	{
		mglog(LL_ERROR, "new CEpoolCtl failed.");
		return Comm_New_Err;
	}

	int ret = -1;
	for (int i = 0; i < m_threadNum; ++i)
	{
		ret = m_poolList[i].Init();
		if (ret != 0)
		{
			mglog(LL_ERROR, "init CEpoolCtl failed(ret:%d).", ret);
			return Comm_Other_Err;
		}
		m_poolList[i].Start();
	}

	return Comm_Success;
}

void CTcpBaseHandle::Stop()
{
	for (int i = 0; i < m_threadNum; ++i)
	{
		m_poolList[i].Stop();
	}

	if (m_listenFd != -1)
	{
		close(m_listenFd);
	}

	if (m_poolList != NULL)
	{
		delete [] m_poolList;
		m_poolList = NULL;
	}
}

bool CTcpBaseHandle::isListenFd(const int fd)
{
	return (m_listenFd == fd);
}

int CTcpBaseHandle::Listen(const std::string& ip, const int port)
{
	m_listenAddr = ip;
	m_listenPort = port;

	m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == m_listenFd)
	{
		mglog(LL_ERROR, "Listen socket failed(err:%m).");
		return IAS_CREATESOCKET_ERR;
	}

	SetReuseAddr(m_listenFd);
	SetNonblock(m_listenFd);

	sockaddr_in servaddr;
	memset((char*)&servaddr, 0, sizeof(sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip.c_str());

	int ret = bind(m_listenFd, (sockaddr*)&servaddr, sizeof(servaddr));
	if (-1 == ret)
	{
		mglog(LL_ERROR, "Listen bind failed(err:%m).");
		return IAS_SOCKETBIND_ERR;
	}

	ret = listen(m_listenFd, SOMAXCONN);
	if (-1 == ret)
	{
		mglog(LL_ERROR, "Listen listen failed(err:%m).");
		return IAS_SOCKETLISTEN_ERR;
	}

	int index = m_listenFd % m_threadNum;
	ret = m_poolList[index].AddPool(m_listenFd);
	if (ret != 0)
	{
		mglog(LL_ERROR, "Listen epoll_ctl add failed(err:%m).");
		return IAS_EPOLLADD_ERR;
	}

	mglog(LL_INFO, "Listen success(ip:%d,port:%d).", ip.c_str(), port);
	return Comm_Success;
}

int CTcpBaseHandle::AcceptConnect()
{
	int fd = -1;
	sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(sockaddr_in);

	fd = accept(m_listenFd, (sockaddr*)&clientAddr, &addrLen);
	if (-1 == fd)
	{
		mglog(LL_ERROR, "AcceptConnect accept failed(errno:%m).");
		return IAS_SOCKETACCEPT_ERR;
	}

	SetNoDelay(fd);
	SetNonblock(fd);

	int index = fd % m_threadNum;
	int ret = m_poolList[index].AddPool(fd);
	if (ret != 0)
	{
		mglog(LL_ERROR, "AcceptConnect epoll_ctl add failed(err:%m).");
		return IAS_EPOLLADD_ERR;
	}

	uint32_t ip = ntohl(clientAddr.sin_addr.s_addr);
	uint16_t port = ntohs(clientAddr.sin_port);
	std::ostringstream os("");
	os <<  (ip >> 24) << "." <<  ((ip >> 16) & 0xFF) << "." << ((ip >> 8) & 0xFF) << "." << (ip & 0xFF);
	m_poolList[index].AddConnect(fd, os.str(), port);
	
	mglog(LL_INFO, "AcceptConnect accept success(clientfd:%d,ip:%s,port:%d).", fd, os.str().c_str(), port);
	return Comm_Success;
}

void CTcpBaseHandle::SetSendBufSize(const int fd, const int sendSize)
{
	int ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendSize, 4);
	if (-1 == ret) 
	{
		mglog(LL_ERROR, "SetSendBufSize failed(fd:%d,err:%m).", fd);
	}

	socklen_t len = 4;
	int size = 0;
	getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, &len);
}

void CTcpBaseHandle::SetRecvBufSize(const int fd, const int recvSize)
{
	int ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &recvSize, 4);
	if (-1 == ret) 
	{
		mglog(LL_ERROR, "SetRecvBufSize failed(fd:%d,err:%m).", fd);
	}

	socklen_t len = 4;
	int size = 0;
	getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, &len);
}

void CTcpBaseHandle::SetNonblock(const int fd)
{
	int ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
	if (-1 == ret)
	{
		mglog(LL_ERROR, "SetNonblock failed(fd:%d,err:%m).", fd);
	}
}

void CTcpBaseHandle::SetReuseAddr(const int fd)
{
	int reuse = 1;
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
	if (-1 == ret)
	{
		mglog(LL_ERROR, "SetReuseAddr failed(fd:%d,err:%m).", fd);
	}
}

void CTcpBaseHandle::SetNoDelay(const int fd)
{
	int nodelay = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
	if (-1 == ret)
	{
		mglog(LL_ERROR, "SetNoDelay failed(fd:%d,err:%m).", fd);
	}
}

