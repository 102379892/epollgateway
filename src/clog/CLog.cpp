#include "clog/CLog.h"

#define MAX_HANDLE_NUM 20

//日志类型
const char* LOGTYPE_DBG = "dbg";
const char* LOGTYPE_RUN = "run";

//日志索引
const int LOG_DBG_INDEX = 0;
const int LOG_RUN_INDEX = 1;

bool IS_INIT = false;
int MAX_LOG_INDEX = 2;
const char* LOG_LIST_KEY[MAX_HANDLE_NUM];
CLogWriter* LOG_LIST[MAX_HANDLE_NUM];

int AddLogType(const char* log_type)
{
	if (NULL == log_type || strlen(log_type) == 0)
	{
		fprintf(stderr, "log_type is null.\n");
		return -1;
	}

	if (MAX_LOG_INDEX + 1 > MAX_HANDLE_NUM)
	{
		fprintf(stderr, "over max log handle.\n");
		return -2;
	}

	if (true == IS_INIT)
	{
		fprintf(stderr, "log init is finished.\n");
		return -3;
	}

	LOG_LIST_KEY[MAX_LOG_INDEX] = log_type;
	++MAX_LOG_INDEX;
	return 0;
}

void WriteLog(LogLevel log_level, const char* log_fmt, ...)
{
	if (LOG_LIST[LOG_DBG_INDEX] != NULL)
	{
		va_list args;
		va_start(args, log_fmt);
		LOG_LIST[LOG_DBG_INDEX]->Log(log_level, log_fmt, args);
		va_end(args);
	}
	else
	{
		char temp[100] = { 0 };
		sprintf(temp, "%d", getpid());
		LogInit(LL_DEBUG, "./logtemp", "./logback", temp);
	}
}

void WriteRunLog(LogLevel log_level, const char* log_fmt, ...) 
{
	if (LOG_LIST[LOG_RUN_INDEX] != NULL)
	{
		va_list args;
		va_start(args, log_fmt);
		LOG_LIST[LOG_RUN_INDEX]->Log(log_level, log_fmt, args);
		va_end(args);
	}
	else
	{
		char temp[100] = { 0 };
		sprintf(temp, "%d", getpid());
		LogInit(LL_DEBUG, "./logtemp", "./logback", temp);
	}
}

void WriteDefLog(const char* log_type, LogLevel log_level, const char* log_fmt, ...)
{
	for (int i = 2; i < MAX_LOG_INDEX; ++i)
	{
		if (LOG_LIST_KEY[i] == NULL || strlen(LOG_LIST_KEY[i]) == 0)
		{
			fprintf(stderr, "not find this logtype.\n");
			return;
		}

		if (strcmp(LOG_LIST_KEY[i], log_type) == 0)
		{
			if (LOG_LIST[i] == NULL)
			{
				fprintf(stderr, "LOG_LIST is null.\n");
				return;
			}

			va_list args;
			va_start(args, log_fmt);
			LOG_LIST[i]->Log(log_level, log_fmt, args);
			va_end(args);

			return;
		}
	}
}

char*& DealDir(char*& sPath)
{
	size_t nLen = strlen(sPath);
	for (size_t i = 0; i <= nLen; ++i)
	{
		if ('\0' == sPath[i] && i > 0 && '/' == sPath[i-1])
		{
			sPath[i-1] = '\0';
		}
	}

	return sPath;
}

bool createLogHandle(char* tempdir, char* backdir, char* temp, const int size,
	LogLevel emLogLevel, const char* pTempDir, const char* pBackDir, const char* pModuleName, const char* pLogStream)
{
	if (NULL == tempdir || NULL == backdir || NULL == temp)
	{
		return false;
	}

	if (MAX_LOG_INDEX > MAX_HANDLE_NUM)
	{
		return false;
	}

	memset(tempdir, '\0', FILE_PATH_LEN + 1);
	strcpy(tempdir, pTempDir);
	DealDir(tempdir);

	memset(backdir, '\0', FILE_PATH_LEN + 1);
	strcpy(backdir, pBackDir);
	DealDir(backdir);

	//如果路径存在文件夹，则判断是否存在
	memset(temp, '\0', size + 1);
	if (access(tempdir, 0) == -1)
	{
		strcpy(temp, "mkdir -p ");
		strcat(temp, tempdir);
		system(temp);
		if (access(tempdir, 0) == -1)
		{
			fprintf(stderr, "mkdir failed(%s).\n", temp);
			return false;
		}
	}

	//如果路径存在文件夹，则判断是否存在
	memset(temp, '\0', size + 1);
	if (access(backdir, 0) == -1)
	{
		strcpy(temp, "mkdir -p ");
		strcat(temp, backdir);
		system(temp);
		if (access(backdir, 0) == -1)
		{
			fprintf(stderr, "mkdir failed(%s).\n", temp);
			return false;
		}
	}

	int nRet = -1;
	for (int i = 0; i < MAX_LOG_INDEX; ++i)
	{
		if (LOG_LIST_KEY[i] == NULL || strlen(LOG_LIST_KEY[i]) == 0)
		{
			fprintf(stderr, "MAX_LOG_INDEX is invaild(i:%d,index:%d).\n", i, MAX_LOG_INDEX);
			return false;
		}

		CLogWriter* loghandle = new CLogWriter();
		if (NULL == loghandle)
		{
			fprintf(stderr, "new CLogWriter failed.\n");
			return false;
		}
		LOG_LIST[i] = loghandle;

		LogLevel loglevel = emLogLevel;
		if (i != LOG_DBG_INDEX)
		{
			loglevel = LL_RUN;
		}

		nRet = loghandle->LogInit(loglevel, tempdir, backdir, pModuleName, LOG_LIST_KEY[i], pLogStream);
		if (nRet < 0)
		{
			LogStop();
			fprintf(stderr, "%s LogInit failed.\n", LOG_LIST_KEY[i]);
			return false;
		}	
	}

	return true;
}

bool LogInit(LogLevel emLogLevel, const char* pTempDir, const char* pBackDir, const char* pModuleName, const char* pLogStream)
{
	IS_INIT = false;
	int templen = strlen(pTempDir);
	int backlen = strlen(pBackDir);
	int modulelen = strlen(pModuleName);
	if ( 
		templen <= 0 || backlen <= 0 || modulelen <= 0 ||
		(templen + modulelen + 1 > FILE_PATH_LEN) ||
		(backlen + modulelen + 1 > FILE_PATH_LEN)
	)
	{
		return false;
	}
	int size = templen > backlen ? (templen + modulelen + 1) : (backlen + modulelen + 1);

	char* tempdir = new char[FILE_PATH_LEN + 1];
	char* backdir = new char[FILE_PATH_LEN + 1];
	char* temp = new char[size + 1];

	LOG_LIST_KEY[LOG_DBG_INDEX] = LOGTYPE_DBG;
	LOG_LIST_KEY[LOG_RUN_INDEX] = LOGTYPE_RUN;
	bool brnt = createLogHandle(tempdir, backdir, temp, size, emLogLevel, pTempDir, pBackDir, pModuleName, pLogStream);
	
	if (NULL != tempdir)
	{
		delete[] tempdir;
	}
	if (NULL != backdir)
	{
		delete[] backdir;
	}
	if (NULL != temp)
	{
		delete[] temp;
	}

	IS_INIT = true;
	return brnt;
}

bool LogStop()
{
	for (int i = 0; i < MAX_LOG_INDEX; ++i)
	{
		if (LOG_LIST[i] != NULL)
		{
			delete LOG_LIST[i];
			LOG_LIST[i] = NULL;
		}
	}

	return true;
}

bool SetLogLevel(LogLevel emLogLevel)
{
	if (LOG_LIST[LOG_DBG_INDEX] != NULL)
	{
		return LOG_LIST[LOG_DBG_INDEX]->SetLogLevel(emLogLevel);
	}

	return false;
}

