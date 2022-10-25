#ifndef __H__CLOG__H__
#define __H__CLOG__H__

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
#include "clog/CLogHandle.h"

bool LogInit(LogLevel emLogLevel, const char* pTempDir, const char* pBackDir, const char* pModuleName, const char* pLogStream = "file");
bool LogStop();
bool SetLogLevel(LogLevel emLogLevel);

char*& DealDir(char*& sPath);
int AddLogType(const char* log_type);
void WriteLog(LogLevel log_level, const char* log_fmt, ...);
void WriteRunLog(LogLevel log_level, const char* log_fmt, ...);
void WriteDefLog(const char* log_type, LogLevel log_level, const char* log_fmt, ...);

#define mglog(log_level, log_fmt, log_arg...) \
	WriteLog(log_level, "[%s:%d][%s]" log_fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##log_arg)

#define mgrunlog(log_fmt, log_arg...) \
	WriteRunLog(LL_RUN, "[%s:%d][%s]" log_fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##log_arg)

#define mgdeflog(log_type, log_fmt, log_arg...) \
	WriteDefLog(log_type, LL_RUN, "[%s:%d][%s]" log_fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##log_arg)

#endif
