#ifndef __H__CONFIG__H__
#define __H__CONFIG__H__

#include <string>

class CConfig
{
public:
	CConfig();
	~CConfig();

	int init(const std::string filename);

public:
	std::string m_listenAddr;
	int m_listenPort;

	int m_recvThreadNum;

	std::string m_logPath;
	std::string m_logType;
	int m_level;
};

#endif
