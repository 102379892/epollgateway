#include <sstream>
#include "json/json.h"
#include "prase/CmdPrase.h"
#include "clog/CLog.h"
#include "common/Common.h"

CCmdPrase::CCmdPrase()
{

}

CCmdPrase::~CCmdPrase()
{

}

bool CCmdPrase::IsCmd(const char* recvBuf, const int recvLen, stCmdInfo& cmd)
{
    Json::Value root;
    Json::Reader reader;
    std::istringstream is;
    is.str(std::string(recvBuf, recvLen));
    if (!reader.parse(is, root))
    {
        mglog(LL_INFO, "IsCmd(recvBuf:%s).", recvBuf);
        return false;
    }

    if (root.isMember("imei"))
    {
        cmd.imei = root["imei"].asString();
    }
    if (root.isMember("content"))
    {
        cmd.content = root["content"].asString();
    }
    
    mglog(LL_INFO, "IsCmd(imei:%s,content:%s).", cmd.imei.c_str(), cmd.content.c_str());
    return true;
}

int CCmdPrase::DealCmd(const stCmdInfo& cmd, char*& cmdStream, int& cmdLen)
{
    std::vector<std::string> tokenlist;
    strSplit(cmd.content, ",", tokenlist);

    if (tokenlist.size() == 0)
    {
        mglog(LL_INFO, "DealCmd content is empty(imei:%s,content:%s).", cmd.imei.c_str(), cmd.content.c_str());
        return -1;
    }

    bool ReplyFlag = false;
    std::string instructId = "";
    std::string content = "";
    const char* headstr = tokenlist.at(0).c_str();
    if (strcasecmp(headstr, "Live") == 0)		
    {
        //直播指令下发
        ReplyFlag = true;
        instructId = "BAF";
        if (tokenlist.size() == 8)
        {
            //开始直播指令格式：Live,${imei},${seq},1,${channel},${url},${rate},${resolution}
            std::string imei = tokenlist.at(1);
            std::string seq = tokenlist.at(2);
            std::string flag = tokenlist.at(3);
            if (flag.compare("1") != 1)
            {
                flag = "1";
            }

            std::string channel = tokenlist.at(4);
            std::string url = tokenlist.at(5);
            std::string rate = tokenlist.at(6);
            std::string resolution = tokenlist.at(7);

            content += "," + imei;
            content += "," + seq;
            content += "," + flag;
            content += "," + channel;
            content += "," + url;
            content += "," + rate;
            content += "," + resolution;
        }
        else if(tokenlist.size() == 5)
        {
            //结束直播指令格式：Live,${imei,${seq},0,${channel}
            std::string imei = tokenlist.at(1);
            std::string seq = tokenlist.at(2);
            std::string flag = tokenlist.at(3);
            if (flag.compare("0") != 0)
            {
                flag = "0";
            }

            std::string channel = tokenlist.at(4);

            content += "," + imei;
            content += "," + seq;
            content += "," + flag;
            content += "," + channel;
        } 
        else
        {
            mglog(LL_INFO, "DealCmd unknow cmd type(imei:%s,content:%s).", cmd.imei.c_str(), cmd.content.c_str());
            return -2;
        }
    }
	else if (strcasecmp(headstr, "LiveSwitch") == 0)		
    {
        //直播切换通道指令下发
        ReplyFlag = true;
        instructId = "BAL";
        if (tokenlist.size() == 8)
        {
            std::string imei = tokenlist.at(1);
            std::string seq = tokenlist.at(2);
            std::string prechannel = tokenlist.at(3);
            std::string channel = tokenlist.at(4);
            std::string url = tokenlist.at(5);
            std::string rate = tokenlist.at(6);
            std::string resolution = tokenlist.at(7);

            content += "," + imei;
            content += "," + seq;
            content += "," + prechannel;
            content += "," + channel;
            content += "," + url;
            content += "," + rate;
            content += "," + resolution;
        }
        else
        {
            mglog(LL_INFO, "DealCmd unknow cmd type(imei:%s,content:%s).", cmd.imei.c_str(), cmd.content.c_str());
            return -3;
        }
    }
    else if (strcasecmp(headstr, "ConfigChanged") == 0)	
	{
        //配置变更
		ReplyFlag = true;
        instructId = "BAK";
		if (tokenlist.size() == 3)
		{
			std::string imei = tokenlist.at(1);
            std::string seq = tokenlist.at(2);

			content += "," + imei;
            content += "," + seq;
		}
        else
        {
            mglog(LL_INFO, "DealCmd unknow cmd type(imei:%s,content:%s).", cmd.imei.c_str(), cmd.content.c_str());
            return -3;
        }
	}

    std::string pkgStart = std::string("*") + std::string("MG") + "2011";
	std::string pkgEnd = "#";
    std::string pkgcontent = pkgStart + instructId + content + pkgEnd;
    cmdLen = (int)pkgcontent.length();
    cmdStream = new char[cmdLen + 1];
    memset(cmdStream, '\0', cmdLen + 1);
    memcpy(cmdStream, pkgcontent.data(), cmdLen);
    mglog(LL_INFO, "DealCmd(pkgcontent:%s).", pkgcontent.c_str());
    return 0;
}
