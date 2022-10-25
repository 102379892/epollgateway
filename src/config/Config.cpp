#include "config/Config.h"
#include "clog/CLog.h"
#include "config/IniReader.h"
#include "common/Singleton.h"

CConfig::CConfig()
{
	
}

CConfig::~CConfig()
{

}

int CConfig::init(const std::string filename)
{
	int ret = SINGLETON(CIniReader)->Load(filename);
	if (ret != 0)
	{
		mglog(LL_ERROR, "Load config failed(ret:%d,filename:%s).", ret, filename.c_str());
		return ret;
	}

	std::string temp = "";
	SINGLETON(CIniReader)->Get("sock", "ip", temp);
	m_listenAddr = temp;

	temp = "";
	SINGLETON(CIniReader)->Get("sock", "port", temp);
	m_listenPort = atoi(temp.c_str());

	temp = "";
	SINGLETON(CIniReader)->Get("common", "recv.thread.num", temp);
	m_recvThreadNum = atoi(temp.c_str());

	temp = "";
	SINGLETON(CIniReader)->Get("log", "path", temp);
	m_logPath = temp;

	temp = "";
	SINGLETON(CIniReader)->Get("log", "type", temp);
	m_logType = temp;

	temp = "";
	SINGLETON(CIniReader)->Get("log", "level", temp);
	m_level = atoi(temp.c_str());

	return 0;
}
