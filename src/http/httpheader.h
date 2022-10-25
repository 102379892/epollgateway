#ifndef __H__HTTP_HEADER__H__
#define __H__HTTP_HEADER__H__

#include <string>
#include <map>

#define MacHTTPHeaderLen        2048
#define MacHTTPHeaderItemLen    128
#define MacHTTPHeaderEnd        "\r\n\r\n"
#define MacHTTPLineEnd          "\r\n"
#define MacHttpHeaderSep        ':'
#define MacMethodHeader         "method"
#define MacContentTypeHeader    "Content-Type"
#define MacStatusCodeHeader     "status"
#define MacURIPathHeader        "pathinfo"
#define MacCookieHeader         "Cookie"
#define MacSetCookieHeader      "Set-Cookie"
#define MacErrorInfoHeader      "Invalid Receivers:"

class HTTPHeaderHandler
{
public:
    HTTPHeaderHandler();
	~HTTPHeaderHandler();

	size_t size();
    int parse(char *content, size_t len);
    int add(const std::string& name, const std::string& value);
    int remove(const std::string& name);
    int update(const std::string& name, const std::string& value);
    int query(const std::string& name, std::string& value);
    int compose(std::string& content);

public:
    std::map<std::string, std::string> m_headers;
};

#endif
