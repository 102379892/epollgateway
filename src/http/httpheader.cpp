#include <stdio.h>
#include <string.h>
#include "http/httpheader.h"
#include "clog/CLog.h"

HTTPHeaderHandler::HTTPHeaderHandler()
{
}

HTTPHeaderHandler::~HTTPHeaderHandler()
{
}

size_t HTTPHeaderHandler::size()
{
    return m_headers.size();
}

int HTTPHeaderHandler::parse(char * content, size_t len)
{
    // 提取消息头的内容
    char head[MacHTTPHeaderLen+1];
    head[0] = 0;
    
    char *p = strstr(content, MacHTTPHeaderEnd);
    if (NULL != p)
    {
        //http消息头大于最大值
        if (p - content > MacHTTPHeaderLen)
        {
			mglog(LL_ERROR, "request(%s) http header (len=%ld) is too large", p, p-content);
            return -1;
        }
        //获取http消息头
        memcpy(head, content, p-content);
        head[p-content] = '\0';
    }
    else
    {
        //只有消息头
        int nLen = len<MacHTTPHeaderLen? len : MacHTTPHeaderLen;
        memcpy(head, content, nLen);
        head[nLen] = 0;
    }

    //截取http消息头的每一行
    char *ptrptr = NULL;
    p = strtok_r(head, MacHTTPLineEnd, &ptrptr);
    while (NULL != p)
    {
        std::string copyHeader = p;
        size_t pos = copyHeader.find(MacHttpHeaderSep);
        if (pos != std::string::npos)
        {
            std::string key = copyHeader.substr(0, pos);
            std::string value = copyHeader.substr(pos+1);
            size_t spos = key.find_first_not_of(" \t\n\r");
            size_t epos = key.find_last_not_of(" \t\n\r");
            if(spos==std::string::npos || epos==std::string::npos) 
            {
                key.clear();
            }
            else
            {
                key = key.substr(spos, epos - spos + 1); 
            }

            spos = value.find_first_not_of(" \t\n\r");
            epos = value.find_last_not_of(" \t\n\r");
            if(spos==std::string::npos || epos==std::string::npos) 
            {
                value.clear();
            }
            else
            {
                value = value.substr(spos, epos - spos + 1); 
            }
            
            if(!key.empty() && !value.empty())
            {
                m_headers.insert(std::pair<std::string, std::string>(key, value));
            }
        }
        
        p = strtok_r(NULL, MacHTTPLineEnd, &ptrptr);
    }
     
    return 0;
}

int HTTPHeaderHandler::add(const std::string& name, const std::string& value)
{
    m_headers.insert(std::pair<std::string, std::string>(name, value));
    return 0;
}

int HTTPHeaderHandler::remove(const std::string& name)
{
    m_headers.erase(name);
    return 0;
}

int HTTPHeaderHandler::update(const std::string& name, const std::string& value)
{
    std::map<std::string, std::string>::iterator ite = m_headers.find(name);
    if(ite != m_headers.end())
    {
        (*ite).second = value;
        return 0;
    }
    return -1;
}

int HTTPHeaderHandler::query(const std::string& name, std::string& value)
{
    std::map<std::string, std::string>::iterator ite = m_headers.find(name);
    if(ite != m_headers.end())
    {
        value = (*ite).second;
        return 0;
    }
    return -1;
}


int HTTPHeaderHandler::compose(std::string& content)
{
    std::map<std::string, std::string>::iterator ite;
    for (ite = m_headers.begin(); ite != m_headers.end(); ++ite)
    {
        char oneheader[512];
        memset(oneheader, 0, sizeof(oneheader));
        snprintf(oneheader, sizeof(oneheader), 
            "%s%c %s%s", 
            (*ite).first.c_str(), MacHttpHeaderSep,
            (*ite).second.c_str(), MacHTTPLineEnd);
         content += oneheader;
    }

    content += MacHTTPLineEnd;

    return 0;
}

