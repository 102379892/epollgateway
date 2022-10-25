#ifndef __H__SOCKCLIENT__H__
#define __H__SOCKCLIENT__H__

#include <string>
#include <map>
#include <netinet/in.h>
#include "lock/ThreadMutex.h"

class CSockClient {
public:
	CSockClient(const int fd, const std::string& addr, const int port);
	CSockClient(const CSockClient& handle);
	~CSockClient();

	CSockClient& operator=(const CSockClient& handle);
	std::string GetClientStr();

	void SetImei(const std::string& imei);
private:
	CSockClient():m_fd(0), m_addr(""), m_port(0) {}

public:
	int m_fd;
	std::string m_addr;
	int m_port;
	std::string m_imei;
};

class CSockManager {
public:
	CSockManager();
	~CSockManager();

	int Init();

	void AddConnect(const int fd, const std::string ip, const int port);
	int SendData(const int fd, const char* sendBuf, const int sendLen);
	int RecvData(const int fd, const char* recvBuf, const int recvLen);
	void DelConnect(const int fd);

public:
	CThreadMutex m_mutex;
	std::map<int, CSockClient> m_clientList;
	std::map<std::string, int> m_devList;
};

#endif

