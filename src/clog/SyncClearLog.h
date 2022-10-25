#ifndef __H__SYNCCLEARLOG__H__
#define __H__SYNCCLEARLOG__H__

#include <string>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <vector>

class CSyncClearLog;
class CClearLogCallback
{
public:
	CClearLogCallback(CSyncClearLog* pClearLogHandle);
	~CClearLogCallback();

	void runTimerTask();
	void FetchFile(std::string sFilePath);
	void delFile(std::string sFilePath, std::string sFileName);

private:
	CSyncClearLog* m_pClearLogHandle;
};

class CSyncClearLog
{
public:
	CSyncClearLog(std::string sLogBckPath, std::string sModleName, int nLogTimeout, int nCheckLogTime);
	~CSyncClearLog();

	int init();

private:
	friend class CClearLogCallback;
	
	std::string m_sLogBckPath;
	std::string m_sModleName;
	int m_nLogTimeout;
	int m_nCheckLogTime;
};

#endif
