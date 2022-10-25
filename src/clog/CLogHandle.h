#ifndef __H__CLOGHANDLE__H__
#define __H__CLOGHANDLE__H__

#include <stdio.h>
#include <pthread.h>

//IP最大长度
const int HOST_IP = 16;

//路径最大长度
const int FILE_PATH_LEN = 500;

//日志最大长度
const int LOG_BUFFSIZE = 1024 * 10;

typedef enum LogLevel
{
	LL_RUN = 0,
	LL_DEBUG = 1,
	LL_TRACE = 2,
	LL_INFO = 3,
	LL_WARNING = 4,
	LL_ERROR = 5
}LogLevel;

class CLogWriter
{
public:
	CLogWriter();
	~CLogWriter();

	LogLevel GetLevel();
	bool SetLogLevel(LogLevel emLogLevel);
	bool CheckLevel(LogLevel emLogLevel);
	
	bool LogInit(LogLevel emLogLevel, const char* pTempDir, const char* pBackDir, const char* pModuleName, const char* pLogType, 
		const char* pLogStream = "file", bool bAppend = true, bool bIsSync = true);
	bool Log(LogLevel emLogLevel, const char* pLogFormat, va_list& args);

private:
	bool initMember();
	void distroyMember();

	const char* LogLevelToString(LogLevel emLogLevel);
	FILE* OpenFile(const char* pFileName, const bool bIsAppend);

	bool ReCreateFile();
	int CheckAndDealLogFile();
	
private:
	FILE* m_fpCur;					//当前文件句柄
	bool m_bIsSync;					//是否同步刷新磁盘
	bool m_bIsAppend;				//写文件是否追加
	enum LogLevel m_emLogLevel;		//日志级别
	pid_t m_nPid;					//进程ID
	char* m_sTempDir;				//临时文件目录
	char* m_sBackDir;				//备份文件目录
	
	char* m_sTempFullName;			//临时文件全名（m_sTempDir+"/"+"模块名_日志类型"+""+"_tmp.log"）
	char* m_sBackFullName;			//备份文件全名（m_sBackDir+"/"+"模块名_日志类型_进程ID_IP后缀"+"_日期_时间_序号"+".log"）
	char* m_sBackFileName;			//备份文件名(不含路径)
	char* m_sHostName;				//主机名
	char m_sIP[HOST_IP];			//机器IP地址

	char* m_sFileNamePrefix;		//文件名的前缀
	char* m_sTemp;					//临时使用

	int m_nSeq;						//文件名序号
	time_t m_nCheckTime;			//检查时间
	time_t m_nLastTime;				//上次检查时间
	int m_nMoveFileMinSize;			//文件备份最小的大小
	int m_nStreamType;				//日志输出的流（0：输出文件 1：输出标准输出）

	pthread_mutex_t m_Mutex;
	pthread_rwlock_t m_RWLock;

#ifndef THREAD_NO_OPTIMIZE
	static __thread char m_sLogThreadBuf[LOG_BUFFSIZE + 1];
#endif
};

#endif
