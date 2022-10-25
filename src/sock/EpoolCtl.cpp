#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include "common/ErrorCode.h"
#include "clog/CLog.h"
#include "sock/EpoolCtl.h"
#include "sock/BaseSocket.h"
#include "common/Singleton.h"

const int MAXRECVLEN = 104857600;	//最大接收字节：100*1024*1024

CEpoolCtl::CEpoolCtl()
:m_waitTimeout(-1), m_epfd(-1), m_recvbuf(NULL)
{

}

CEpoolCtl::~CEpoolCtl()
{

}

int CEpoolCtl::Init(const int waitTimeout)
{
	m_waitTimeout = waitTimeout;

	m_epfd = epoll_create(SOMAXCONN);
	if (m_epfd == -1)
	{
		mglog(LL_ERROR, "init epoll_create failed(err:%m).");
		return IAS_CREATEEPOLL_ERR;
	}

	m_recvbuf = new char[MAXRECVLEN+1];
	if (NULL == m_recvbuf)
	{
		mglog(LL_ERROR, "init new failed.");	
		return Comm_New_Err;
	}

	int ret = SINGLETON(CSockManager)->Init();
	if (ret != 0)
	{
		mglog(LL_ERROR, "init CSockManager failed(ret:%d).", ret);
		return Comm_Other_Err;
	}

	return Comm_Success;
}

void CEpoolCtl::Stop()
{
	CThread::Stop();
	if (m_epfd != -1)
	{
		close(m_epfd);
		m_epfd = -1;
	}

	if (m_recvbuf != NULL)
	{
		delete [] m_recvbuf;
		m_recvbuf = NULL;
	}
}

void CEpoolCtl::run()
{
	m_isRun = true;
	struct epoll_event* events = new struct epoll_event[SOMAXCONN];
	while (m_isRun)
	{
		int nfds = epoll_wait(m_epfd, events, SOMAXCONN, m_waitTimeout);
		for (int i = 0; i < nfds; i++)
		{
			int ret = -1;
			int ev_fd = events[i].data.fd;
			if (SINGLETON(CTcpBaseHandle)->isListenFd(ev_fd))
			{
				if (events[i].events & EPOLLIN)
				{
					SINGLETON(CTcpBaseHandle)->AcceptConnect();
				}
				continue;
			}

#ifdef EPOLLRDHUP
			if (events[i].events & EPOLLRDHUP)
			{
				mglog(LL_ERROR, "StartDispatch recv close event(ev_fd:%d,err:%m).", ev_fd);
				CloseEvent(ev_fd);
				continue;
			}
#endif

			if (events[i].events & EPOLLIN)
			{
				RecvEvent(ev_fd);
				continue;
			}

			if (events[i].events & EPOLLOUT)
			{
				WriteEvent(ev_fd);
				continue;
			}

			if (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))
			{
				mglog(LL_ERROR, "StartDispatch recv close event(ev_fd:%d,err:%m).", ev_fd);
				CloseEvent(ev_fd);
				continue;
			}
		}
	}
	m_isStop = true;
	delete []events;
	events = NULL;
}

int CEpoolCtl::AddPool(const int fd, const int type)
{
	//EPOLLIN：触发该事件，表示对应的文件描述符上有可读数据。(包括对端SOCKET正常关闭)；	
	//EPOLLOUT：触发该事件，表示对应的文件描述符上可以写数据；
	//EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
	//EPOLLERR：表示对应的文件描述符发生错误；
	//EPOLLHUP：表示对应的文件描述符被挂断；
	//EPOLLET：将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)[EPOLLLT]来说的；
	//EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里；
	struct epoll_event ev;
	ev.data.fd = fd;
	if (0 == type)
	{
		//监听fd
		ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
	}
	else
	{
		//客户端fd
		ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP;
	}
#ifdef EPOLLRDHUP
		ev.events |= EPOLLRDHUP;
#endif
	int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
	if (ret != 0)
	{
		mglog(LL_ERROR, "Listen epoll_ctl add failed(err:%m).");
		return IAS_EPOLLADD_ERR;
	}

	return Comm_Success;
}

void CEpoolCtl::AddConnect(const int fd, const std::string ip, const int port)
{
	SINGLETON(CSockManager)->AddConnect(fd, ip, port);
}

int CEpoolCtl::RecvEvent(const int fd)
{
	memset(m_recvbuf, '\0', MAXRECVLEN+1);
	int recvlen = ::recv(fd, m_recvbuf, MAXRECVLEN, 0);
	if (recvlen < 0)
	{
		mglog(LL_ERROR, "RecvEvent failed(fd:%d,err:%m).", fd);
		return IAS_RECVDATA_ERR;
	}

	if (recvlen == 0)
	{
		return Comm_Success;
	}
	
	int ret = SINGLETON(CSockManager)->RecvData(fd, m_recvbuf, recvlen);
	if (ret != 0)
	{
		mglog(LL_ERROR, "RecvEvent RecvData failed(fd:%d,m_recvbuf:%s).", fd, m_recvbuf);
	}

	return Comm_Success;
}

int CEpoolCtl::WriteEvent(const int fd)
{
	mglog(LL_INFO, "recv WriteEvent(fd:%d).", fd);
	return 0;
}

int CEpoolCtl::CloseEvent(const int fd)
{
	mglog(LL_INFO, "recv CloseEvent(fd:%d).", fd);
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL) != 0)
	{
		mglog(LL_ERROR, "CloseEvent epoll_ctl del failed(fd:%d,err:%m).", fd);
		return IAS_EPOLLDEL_ERR;
	}
	close(fd);

	SINGLETON(CSockManager)->DelConnect(fd);

	return Comm_Success;
}
