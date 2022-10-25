#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include "clog/CLog.h"
#include "config/Config.h"
#include "sock/BaseSocket.h"
#include "common/Singleton.h"

int main(int argc, char* argv[])
{
	//daemon(1, 0);
	int ret = SINGLETON(CConfig)->init("/data/live/epollgateway/main.conf");
	if (ret != 0)
	{
		printf("CConfig Init failed(ret:%d).\n", ret);
		return -1;
	}

	std::string logpath = SINGLETON(CConfig)->m_logPath;
	std::string logtype = SINGLETON(CConfig)->m_logType;
	LogLevel level = (LogLevel)SINGLETON(CConfig)->m_level;
	bool nret = LogInit(level, logpath.c_str(), logpath.c_str(), "gateway", logtype.c_str());
	if (nret != true)
	{
		printf("LogInit failed.\n");
		return -1;
	}

	mglog(LL_INFO, "start gateway.");

	ret = SINGLETON(CTcpBaseHandle)->Init(SINGLETON(CConfig)->m_recvThreadNum);
	if (ret != 0)
	{
		mglog(LL_ERROR, "CTcpBaseHandle Init failed(ret:%d).", ret);
		return -1;
	}

	ret = SINGLETON(CTcpBaseHandle)->Listen(SINGLETON(CConfig)->m_listenAddr, SINGLETON(CConfig)->m_listenPort);
	if (ret != 0)
	{
		mglog(LL_ERROR, "CTcpBaseHandle Listen failed(ret:%d).", ret);
		return -1;
	}

	while(1)
	{
		mglog(LL_INFO, "sleep gateway.");
		sleep(10);
	}

	SINGLETON(CTcpBaseHandle)->Stop();
	LogStop();
}
