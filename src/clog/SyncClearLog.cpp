#include "SyncClearLog.h"
#include "clog/CLog.h"

CClearLogCallback::CClearLogCallback(CSyncClearLog* pClearLogHandle)
:m_pClearLogHandle(pClearLogHandle)
{
}

CClearLogCallback::~CClearLogCallback()
{
}

void CClearLogCallback::runTimerTask()
{
	std::string sFullPath = m_pClearLogHandle->m_sLogBckPath;
	FetchFile(sFullPath);
}

void CClearLogCallback::FetchFile(std::string sFilePath)
{
	//定义一个目录流;就像是定义文件流一样
	DIR* psDir = NULL;
	//定义目录结构体
	struct dirent* psPentry;

	//打开一个路径并返回一个目录流。
	psDir = opendir(sFilePath.c_str());
	if (psDir == NULL)
	{
		mglog(LL_ERROR, "[FetchFile] open file failed(%d, %s, %s).", errno, strerror(errno), sFilePath.c_str());
		return;
	}

	while ((psPentry = readdir(psDir)) != NULL)
	{
		if (strcmp(psPentry->d_name, ".") == 0 ||
			strcmp(psPentry->d_name, "..") == 0
			)
		{
			continue;
		}

		if (psPentry->d_type == DT_DIR)//判断是否是目录
		{
			std::string sFullName = sFilePath;
			sFullName += "/";
			sFullName += psPentry->d_name;

			FetchFile(sFullName);
		}
		else
		{
			delFile(sFilePath, psPentry->d_name);
		}
	}

	closedir(psDir);
}

void CClearLogCallback::delFile(std::string sFilePath, std::string sFileName)
{
	time_t now = time(NULL);
	now -= m_pClearLogHandle->m_nLogTimeout * 24 * 60 * 60;
	struct tm* t_now = localtime(&now);

	char sCurDate[10] = {0};
	snprintf(sCurDate, 9, "%04d%02d%02d",
		t_now->tm_year + 1900, t_now->tm_mon + 1, t_now->tm_mday);

	std::vector<std::string> vecTemp;
	std::string::size_type nBegin = 0;
	while (1)
	{
		std::string sTemp = "";
		std::string::size_type nIndex = sFileName.find_first_of("_", nBegin);
		if (nIndex != std::string::npos)
		{
			sTemp = sFileName.substr(nBegin, nIndex - nBegin);
			vecTemp.push_back(sTemp);
			//mglog(LL_DEBUG, "[delFile](%s).", sTemp.c_str());
			nBegin = nIndex + 1;
		}
		else
		{
			sTemp = sFileName.substr(nBegin);
			vecTemp.push_back(sTemp);
			//mglog(LL_DEBUG, "[delFile](%s).", sTemp.c_str());
			break;
		}
	}
	
	if (vecTemp[1].compare("dbg") == 0)
	{
		if (vecTemp.size() >= 5 && vecTemp[4].length() == 8 && vecTemp[4].compare(sCurDate) <= 0)
		{
			std::string sFullName = sFilePath;
			sFullName += "/";
			sFullName += sFileName;

			mglog(LL_DEBUG, "[delFile] del file(%s).", sFullName.c_str());
			unlink(sFullName.c_str());
		}
		else if (vecTemp.size() >= 4 && vecTemp[3].length() == 8 && vecTemp[3].compare(sCurDate) <= 0)
		{
			std::string sFullName = sFilePath;
			sFullName += "/";
			sFullName += sFileName;

			mglog(LL_DEBUG, "[delFile] del file(%s).", sFullName.c_str());
			unlink(sFullName.c_str());
		}
	}
}

CSyncClearLog::CSyncClearLog(std::string sLogBckPath, std::string sModleName, int nLogTimeout, int nCheckLogTime)
:m_sLogBckPath(sLogBckPath), m_sModleName(sModleName), m_nLogTimeout(nLogTimeout), m_nCheckLogTime(nCheckLogTime)
{

}

CSyncClearLog::~CSyncClearLog()
{
}

int CSyncClearLog::init()
{
	return 0;
}

