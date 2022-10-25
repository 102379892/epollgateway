#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include "clog/CLogHandle.h"

#ifndef THREAD_NO_OPTIMIZE
__thread char CLogWriter::m_sLogThreadBuf[LOG_BUFFSIZE+1];
#endif

void localtime_safe(const time_t curtime, const long timezone, struct tm* tm_time)
{
	unsigned int n32hpery;
	unsigned int n32pass4year;
	const char days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	time_t time = curtime + (timezone * 60 * 60);
	if (time < 0)
	{
		time = 0;
	}

	tm_time->tm_sec = (int)(time % 60);

	time /= 60;
	tm_time->tm_min = (int)(time % 60);

	time /= 60;
	n32pass4year = ((unsigned int)time / (1461L * 24L));
	tm_time->tm_year = (n32pass4year << 2) + 70;

	time %= 1461L * 24L;
	for (;;)
	{
		n32hpery = 365 * 24;
		if ((tm_time->tm_year & 3) == 0)
		{
			n32hpery += 24;
		}

		if (time < n32hpery)
		{
			break;
		}

		tm_time->tm_year++;
		time -= n32hpery;
	}

	tm_time->tm_hour = (int)(time % 24);

	time /= 24;
	time++;
	if ((tm_time->tm_year & 3) == 0)
	{
		if (time > 60)
		{
			time--;
		}
		else
		{
			if (time == 60)
			{
				tm_time->tm_mon = 1;
				tm_time->tm_mday = 29;
				return;
			}
		}
	}

	for (tm_time->tm_mon = 0; days[tm_time->tm_mon] < time; tm_time->tm_mon++)
	{
		time -= days[tm_time->tm_mon];
	}

	tm_time->tm_mday = (int)(time);
	return;
}

CLogWriter::CLogWriter()
:m_fpCur(NULL), m_bIsSync(true), m_bIsAppend(true),
m_emLogLevel(LL_DEBUG), m_nPid(0), m_sTempDir(NULL), 
m_sBackDir(NULL), m_sTempFullName(NULL), m_sBackFullName(NULL), 
m_sBackFileName(NULL), m_sHostName(NULL), m_sFileNamePrefix(NULL), 
m_sTemp(NULL), m_nSeq(1), m_nCheckTime(10),
m_nLastTime(0), m_nMoveFileMinSize(0), m_nStreamType(0)
{
}

CLogWriter::~CLogWriter()
{
	if (NULL != m_fpCur)
	{
		fflush(m_fpCur);
		fclose(m_fpCur);
		m_fpCur = NULL;
	}
	
	distroyMember();
}

bool CLogWriter::initMember()
{
	m_sTempDir = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sTempDir)
	{
		return false;
	}
	memset(m_sTempDir, '\0', FILE_PATH_LEN + 1);

	m_sBackDir = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sBackDir)
	{
		return false;
	}
	memset(m_sBackDir, '\0', FILE_PATH_LEN + 1);

	m_sTempFullName = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sTempFullName)
	{
		return false;
	}
	memset(m_sTempFullName, '\0', FILE_PATH_LEN + 1);

	m_sHostName = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sHostName)
	{
		return false;
	}
	memset(m_sHostName, '\0', FILE_PATH_LEN + 1);

	memset(m_sIP, '\0', HOST_IP);

	m_sFileNamePrefix = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sFileNamePrefix)
	{
		return false;
	}
	memset(m_sFileNamePrefix, '\0', FILE_PATH_LEN + 1);

	m_sBackFullName = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sBackFullName)
	{
		return false;
	}
	memset(m_sBackFullName, '\0', FILE_PATH_LEN + 1);

	m_sBackFileName = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sBackFileName)
	{
		return false;
	}
	memset(m_sBackFileName, '\0', FILE_PATH_LEN + 1);

	m_sTemp = new char[FILE_PATH_LEN + 1];
	if (NULL == m_sTemp)
	{
		return false;
	}
	memset(m_sTemp, '\0', FILE_PATH_LEN + 1);

	m_nCheckTime = 10;
	m_nLastTime = time(NULL);
	m_nMoveFileMinSize = 1024 * 1024 * 300;
	m_nStreamType = 0;

	int ret = pthread_mutex_init(&m_Mutex, NULL);
	if (ret != 0)
	{
		return false;
	}

	ret = pthread_rwlock_init(&m_RWLock, NULL);
	if (ret != 0)
	{
		return false;
	}

	return true;
}

void CLogWriter::distroyMember()
{
	if (NULL != m_sTempDir)
	{
		delete[] m_sTempDir;
		m_sTempDir = NULL;
	}

	if (NULL != m_sBackDir)
	{
		delete[] m_sBackDir;
		m_sBackDir = NULL;
	}

	if (NULL != m_sTempFullName)
	{
		delete[] m_sTempFullName;
		m_sTempFullName = NULL;
	}

	if (NULL != m_sHostName)
	{
		delete[] m_sHostName;
		m_sHostName = NULL;
	}

	if (NULL != m_sFileNamePrefix)
	{
		delete[] m_sFileNamePrefix;
		m_sFileNamePrefix = NULL;
	}

	if (NULL != m_sBackFullName)
	{
		delete[] m_sBackFullName;
		m_sBackFullName = NULL;
	}

	if (NULL != m_sBackFileName)
	{
		delete[] m_sBackFileName;
		m_sBackFileName = NULL;
	}

	if (NULL != m_sTemp)
	{
		delete[] m_sTemp;
		m_sTemp = NULL;
	}

	pthread_mutex_destroy(&m_Mutex);
	pthread_rwlock_destroy(&m_RWLock);
}

LogLevel CLogWriter::GetLevel()
{
	return m_emLogLevel;
}

bool CLogWriter::SetLogLevel(LogLevel emLogLevel)
{
	m_emLogLevel = emLogLevel;
	return true;
}

bool CLogWriter::CheckLevel(LogLevel emLogLevel)
{
	if (emLogLevel >= m_emLogLevel)
		return true;
	else
		return false;
}

const char* CLogWriter::LogLevelToString(LogLevel emLogLevel)
{
	switch (emLogLevel)
	{
	case LL_RUN:
		return "RUN";
	case LL_DEBUG:
		return "DEBUG";
	case LL_TRACE:
		return "TRACE";
	case LL_INFO:
		return "INFO";
	case LL_WARNING:
		return "WARN";
	case LL_ERROR:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}

FILE* CLogWriter::OpenFile(const char* pFileName, const bool bIsAppend)
{
	FILE* fd = fopen(pFileName, bIsAppend ? "a" : "w");
	if (fd == NULL)
	{
		return NULL;
	}

	//不能定义太大，不然超过栈空间会coredump
#ifndef THREAD_NO_OPTIMIZE
	const int CACHE_BUFFSIZE = 1024 * 10;
	setvbuf(fd, (char *)NULL, _IOLBF, CACHE_BUFFSIZE);
#endif
	return fd;
}


bool CLogWriter::LogInit(LogLevel emLogLevel, const char* pTempDir, const char* pBackDir, const char* pModuleName, const char* pLogType, const char* pLogStream, bool bAppend, bool bIsSync)
{
	if (NULL != m_fpCur)
	{
		return true;
	}
	initMember();

	m_bIsSync = bIsSync;
	m_bIsAppend = bAppend;
	m_emLogLevel = emLogLevel;
	m_nPid = getpid();
	
	strcpy(m_sTempDir, pTempDir);
	strcpy(m_sBackDir, pBackDir);
	if (NULL != pLogStream && strcmp(pLogStream, "stdout") == 0)
	{
		m_nStreamType = 1;
	}

	//生成临时文件名
	snprintf(m_sTempFullName, FILE_PATH_LEN, "%s/%s_%s_tmp.log", m_sTempDir, pModuleName, pLogType);

	//获取机器IP地址字符串
	gethostname(m_sHostName, FILE_PATH_LEN);
	struct hostent* hostent = gethostbyname(m_sHostName);
	if (NULL != hostent && NULL != hostent->h_addr_list)
	{
		for (int i = 0; NULL != hostent->h_addr_list[i]; i++)
		{
			strncpy(m_sIP, inet_ntoa(*(struct in_addr*)(hostent->h_addr_list[i])), sizeof(m_sIP) - 1);
			if (strcmp(m_sIP, "127.0.0.1") == 0)
			{
				memset(m_sIP, '\0', HOST_IP);
				continue;
			}
			else
			{
				break;
			}
		}
	}

	if (m_nStreamType == 1)
	{
		return true;
	}

	//查找最后的位置
	char* tempindex = strrchr(m_sIP, '.');
	if (NULL != tempindex)
	{
		++tempindex;
		snprintf(m_sFileNamePrefix, FILE_PATH_LEN, "%s_%s_%d_%s", pModuleName, pLogType, m_nPid, tempindex);
	}
	else
	{
		snprintf(m_sFileNamePrefix, FILE_PATH_LEN, "%s_%s_%d", pModuleName, pLogType, m_nPid);
	}
	
	m_fpCur = OpenFile(m_sTempFullName, m_bIsAppend);
	if (NULL == m_fpCur)
	{
		return false;
	}

	return true;
}

bool CLogWriter::Log(LogLevel emLogLevel, const char* pLogFormat, va_list& args)
{
	if (!CheckLevel(emLogLevel))
	{
		return false;
	}
	CheckAndDealLogFile();

#ifndef THREAD_NO_OPTIMIZE
	char* logbuf = m_sLogThreadBuf;
#else
	char* logbuf = new char[LOG_BUFFSIZE+1];
	if (NULL == logbuf)
	{
		return false;
	}
#endif
	memset(logbuf, '\0', LOG_BUFFSIZE + 1);

	struct timeval now = { 0 };
	gettimeofday(&now, NULL);

	struct tm vtm = { 0 };
	//localtime_r(&now.tv_sec, &vtm);//线程安全，信号不安全，会coredump在__tz_convert
	localtime_safe(now.tv_sec, 8, &vtm);

	snprintf(logbuf, LOG_BUFFSIZE, "%04d%02d%02d %02d:%02d:%02d.%06ld (%d)(%ld)(%s) ",
		vtm.tm_year + 1900, vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, vtm.tm_min, vtm.tm_sec,
		now.tv_usec, m_nPid, (unsigned long)pthread_self(), LogLevelToString(emLogLevel)
	);

	int buflen = strlen(logbuf);
	char* bufbegin = logbuf;
	bufbegin += buflen;

	vsnprintf(bufbegin, LOG_BUFFSIZE - buflen, pLogFormat, args);
	buflen = strlen(logbuf);
	if (logbuf[buflen-1] != '\n')
	{
		logbuf[buflen-1] = '\n';
	}

	if (m_nStreamType == 1)
	{
		pthread_rwlock_rdlock(&m_RWLock);
		fwrite(logbuf, buflen, 1, stdout);
		pthread_rwlock_unlock(&m_RWLock);
#ifdef THREAD_NO_OPTIMIZE
		delete[]logbuf;
#endif
		return true;
	}

	//防止写文件之前，文件被人为删除
	if (0 != access(m_sTempFullName, W_OK))
	{
		pthread_mutex_lock(&m_Mutex);
		//锁内校验 access 看是否在等待锁过程中被其他线程loginit了  避免多线程多次close 和init
		if (0 != access(m_sTempFullName, W_OK))
		{
			if (false == ReCreateFile())
			{
				pthread_mutex_unlock(&m_Mutex);
#ifdef THREAD_NO_OPTIMIZE
				delete []logbuf;
#endif
				return false;
			}
		}
		pthread_mutex_unlock(&m_Mutex);
	}

	pthread_rwlock_rdlock(&m_RWLock);
	fwrite(logbuf, buflen, 1, m_fpCur);
	if (m_bIsSync)
	{
		fflush(m_fpCur);
	}
	pthread_rwlock_unlock(&m_RWLock);
#ifdef THREAD_NO_OPTIMIZE
	delete[]logbuf;
#endif
	return true;
}

bool CLogWriter::ReCreateFile()
{
	//生成备份文件名（不包含路径）
	time_t now = time(NULL);
	struct tm t_now = {0};
	localtime_safe(now, 8, &t_now);
	memset(m_sBackFileName, '\0', FILE_PATH_LEN + 1);
	snprintf(m_sBackFileName, FILE_PATH_LEN, "%s_%04d%02d%02d_%02d%02d%02d_%04d.log",
		m_sFileNamePrefix, t_now.tm_year + 1900, t_now.tm_mon + 1,
		t_now.tm_mday, t_now.tm_hour,
		t_now.tm_min, t_now.tm_sec, m_nSeq
	);

	++m_nSeq;
	if (m_nSeq > 10000)
	{
		m_nSeq = 1;
	}

	//生成暂时的临时文件名
	memset(m_sTemp, '\0', FILE_PATH_LEN + 1);
	strcpy(m_sTemp, m_sTempDir);
	strncat(m_sTemp, "/", 1);
	strcat(m_sTemp, m_sBackFileName);

	//当前线程重命名时其它线程可能还在操作这个文件，所以重命名在当前目录下，避免被移走或者删除
	int nRet = rename(m_sTempFullName, m_sTemp);
	if (nRet != 0)
	{
		printf("[CLogWriter] rename failed(%d,%s,%s-->%s).", errno, strerror(errno), m_sTempFullName, m_sTemp);
		return false;
	}

	//打开临时文件
	FILE* fp = OpenFile(m_sTempFullName, m_bIsAppend);
	if (fp == NULL)
	{
		printf("[CLogWriter] fopen failed(%d,%s,%s).", errno, strerror(errno), m_sTempFullName);
		return false;
	}

	FILE* oldfd = m_fpCur;
	pthread_rwlock_wrlock(&m_RWLock);
	m_fpCur = fp;
	pthread_rwlock_unlock(&m_RWLock);

	//关闭临时文件
	if (oldfd != NULL)
	{
		fflush(oldfd);
		fclose(oldfd);
		oldfd = NULL;
	}

	//生成备份文件名（.../dbg_pid_128_20170315_122100_0001.log）文件
	memset(m_sBackFullName, '\0', FILE_PATH_LEN + 1);
	strcpy(m_sBackFullName, m_sBackDir);
	strncat(m_sBackFullName, "/", 1);
	strcat(m_sBackFullName, m_sBackFileName);

	//已经完成老文件到新文件的切换，并且文件已经被关闭，所以可以移走文件
	//注意rename函数使用时，尽量避免跨磁盘或者跨网络备份，那样会占用很长的时间，严重影响日志的性能
	if (m_sTemp != m_sBackFullName)
	{
		nRet = rename(m_sTemp, m_sBackFullName);
		if (nRet != 0)
		{
			printf("[CLogWriter] rename failed(%d,%s).", errno, strerror(errno));
			return false;
		}
	}

	return true;
}

int CLogWriter::CheckAndDealLogFile()
{
	if (m_nStreamType == 1)
	{
		return 0;
	}

	time_t tCurTime = time(NULL);
	if ((tCurTime - m_nLastTime) <= m_nCheckTime)
	{
		return 0;
	}

	struct stat fileStat;
	int nRet = stat(m_sTempFullName, &fileStat);
	if (0 == nRet && fileStat.st_size < m_nMoveFileMinSize)
	{
		return 0;
	}

	bool bRnt = true;
	pthread_mutex_lock(&m_Mutex);
	if ((tCurTime - m_nLastTime) >= m_nCheckTime)
	{
		int nRet = stat(m_sTempFullName, &fileStat);
		if (
			(
				0 == nRet && fileStat.st_size >= m_nMoveFileMinSize
			) ||
			ENOENT == nRet //指定的文件不存在
		)
		{
			bRnt = ReCreateFile();
			if (true == bRnt)
			{
				m_nLastTime = tCurTime;
			}
		}
	}
	pthread_mutex_unlock(&m_Mutex);
	
	return bRnt ? 0 : (-1);
}
