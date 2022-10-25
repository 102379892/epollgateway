#ifndef __H__CMDPRASE__H__
#define __H__CMDPRASE__H__

#include <string>

typedef struct stCmdInfo {
    std::string imei;
    std::string content;

    stCmdInfo():imei(""), content(""){}
}stCmdInfo;

class CCmdPrase {
public:
	CCmdPrase();
	~CCmdPrase();

    bool IsCmd(const char* recvBuf, const int recvLen, stCmdInfo& cmd);
    int DealCmd(const stCmdInfo& cmd, char*& cmdStream, int& cmdLen);
};

#endif

