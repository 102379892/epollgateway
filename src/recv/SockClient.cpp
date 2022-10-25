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
#include "recv/SockClient.h"
#include "common/Singleton.h"
#include "common/Singleton.h"
#include "prase/CmdPrase.h"
#include "prase/ReportPrase.h"
#include "lock/Guard.h"

CSockClient::CSockClient(const int fd, const std::string& addr, const int port)
:m_fd(fd), m_addr(addr), m_port(port), m_imei("")
{

}

CSockClient::CSockClient(const CSockClient& handle)
{
	m_fd = handle.m_fd;
	m_addr = handle.m_addr;
	m_port = handle.m_port;
	m_imei = handle.m_imei;
}

CSockClient::~CSockClient()
{

}

CSockClient& CSockClient::operator=(const CSockClient& handle)
{
	m_fd = handle.m_fd;
	m_addr = handle.m_addr;
	m_port = handle.m_port;
	m_imei = handle.m_imei;
	return *this;
}

std::string CSockClient::GetClientStr()
{
	std::ostringstream os("");
	os << "fd:" << m_fd << ",addr:" << m_addr << ",port:" << m_port << ",imei:" << m_imei;
	return os.str();
}

void CSockClient::SetImei(const std::string& imei)
{
	m_imei = imei;
}

CSockManager::CSockManager()
{

}

CSockManager::~CSockManager()
{

}

int CSockManager::Init()
{
	int ret = m_mutex.Init();
	if (ret != 0)
	{
		mglog(LL_ERROR, "init mutex failed(ret:%d).", ret);
		return Comm_Other_Err;
	}

	return Comm_Success;
}

void CSockManager::AddConnect(const int fd, const std::string ip, const int port)
{
	CSockClient client(fd, ip, port);
	mglog(LL_INFO, "AddConnect(%s).", client.GetClientStr().c_str());
	Guard<CThreadMutex> lock(m_mutex);
	m_clientList.insert(std::pair<int, CSockClient>(fd, client));
}

int CSockManager::SendData(const int fd, const char* sendBuf, const int sendLen)
{	 
	int ret = ::send(fd, sendBuf, sendLen, 0);
	if (ret < 0)
	{
		mglog(LL_ERROR, "SendData failed(fd:%d,err:%m).", fd);
		return IAS_SENDDATA_ERR;
	}
	mglog(LL_INFO, "SendData success(ret:%d,sendBuf:%s,cmdlen:%d).", ret, sendBuf, sendLen);
	
	return Comm_Success;
}

int CSockManager::RecvData(const int fd, const char* recvBuf, const int recvLen)
{
    mglog(LL_INFO, "-------RecvData----------(fd:%d,recvBuf:%s).", fd, recvBuf);
	int ret = -1;
	stCmdInfo cmd;
	bool iscmd = SINGLETON(CCmdPrase)->IsCmd(recvBuf, recvLen, cmd);
	if (iscmd)
	{
		int cmdlen = 0;
		char* cmdstream = NULL;
		ret = SINGLETON(CCmdPrase)->DealCmd(cmd, cmdstream, cmdlen);
		if (ret == 0)
		{
			Guard<CThreadMutex> lock(m_mutex);
			std::map<std::string, int>::iterator iter = m_devList.find(cmd.imei);
			if (iter != m_devList.end())
			{
				int sockfd = iter->second;
				ret = SendData(sockfd, cmdstream, cmdlen);
			}
		}
		if (cmdstream != NULL)
		{
			delete []cmdstream;
			cmdstream = NULL;
		}
		return ret;
	}

	int resplen = 0;
	char* respstream = NULL;
	std::string imei = "";
	ret = SINGLETON(CReportPrase)->DealReport(recvBuf, recvLen, imei, respstream, resplen);
	if (ret == 0)
	{
		m_devList[imei] = fd;
		std::map<int, CSockClient>::iterator iter = m_clientList.find(fd);
		if (iter != m_clientList.end())
		{
			if (imei != "")
			{
				iter->second.SetImei(imei);
			}
		}
		
		if (respstream != NULL)
		{
			ret = SendData(fd, respstream, resplen);
			delete []respstream;
			respstream = NULL;
		}
		return ret;
	}
	if (respstream != NULL)
	{
		delete []respstream;
		respstream = NULL;
	}

	return ret;
}

void CSockManager::DelConnect(const int fd)
{
	Guard<CThreadMutex> lock(m_mutex);
	std::map<int, CSockClient>::iterator iter = m_clientList.find(fd);
	if (iter != m_clientList.end())
	{
		mglog(LL_INFO, "DelConnect(%s).", iter->second.GetClientStr().c_str());
		if (iter->second.m_imei != "")
		{
			std::map<std::string, int>::iterator imeiiter = m_devList.find(iter->second.m_imei);
			if (imeiiter != m_devList.end())
			{
				m_devList.erase(iter->second.m_imei);
			}
		}
		m_clientList.erase(fd);
	}
}
