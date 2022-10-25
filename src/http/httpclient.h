#ifndef _T_HTTPCLIENT_T_
#define _T_HTTPCLIENT_T_

#include <map>
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <assert.h>

//http消息头字段名
#define HEADERLISTNUM 33
static std::string LINEACCEPTRANGES            = "accept-ranges";
static std::string LINEAGE                     = "age";
static std::string LINEALLOW                   = "allow";
static std::string LINECACHECONTROL            = "cache-control";
static std::string LINECONNECTION              = "connection";
static std::string LINECONTENTENCODING         = "content-encoding";
static std::string LINECONTENTLANGUAGE         = "content-language";
static std::string LINECONTENTLENGTH           = "content-length";
static std::string LINECONTENTLOCATION         = "content-location";
static std::string LINECONTENTMD5              = "content-md5";
static std::string LINECONTENTDISPOSITION      = "content-disposition";
static std::string LINECONTENTRANGE            = "content-range";
static std::string LINECONTENTTYPE             = "content-type";
static std::string LINEDATE                    = "date";
static std::string LINEETAG                    = "etag";
static std::string LINEEXPIRES                 = "expires";
static std::string LINELASTMODIFIED            = "last-modified";
static std::string LINELINK                    = "link";
static std::string LINELOCATION                = "location";
static std::string LINEP3P                     = "p3p";
static std::string LINEPRAGMA                  = "pragma";
static std::string LINEPROXYAUTHENTICATE       = "proxy-authenticate";
static std::string LINEREFRESH                 = "refresh";
static std::string LINERETRYAFTER              = "retry-after";
static std::string LINESERVER                  = "server";
static std::string LINESETCOOKIE               = "set-cookie";
static std::string LINESTRICTTRANSPORTSECURITY = "strict-transport-security";
static std::string LINETRAILER                 = "trailer";
static std::string LINETRANSFERENCODING        = "transfer-encoding";
static std::string LINEVARY                    = "vary";
static std::string LINEVIA                     = "via";
static std::string LINEWARNING                 = "warning";
static std::string LINEWWWAUTHENTICATE         = "WWW-Authenticate";
static std::string LINEKEEPALIVE               = "keep-alive";
static std::string LINEXWWWFORMURLENCODED      = "x-www-form-urlencoded";
static std::string LINEACCEPTENCODING          = "accept-encoding";
static std::string LINEHOST                    = "host";
static std::string LINEUSERAGENT               = "user-agent";

//判断http消息头字段
#define CHECKHEADCONTENT(x) ((x) == LINEACCEPTRANGES || \
(x) == LINEAGE || (x) == LINEALLOW || (x) == LINECACHECONTROL || \
(x) == LINEKEEPALIVE || (x) == LINECONNECTION || (x) == LINECONTENTENCODING || \
(x) == LINECONTENTLANGUAGE || (x) == LINECONTENTLOCATION || (x) == LINECONTENTMD5 || \
(x) == LINECONTENTDISPOSITION || (x) == LINEDATE || (x) == LINEETAG || \
(x) == LINEEXPIRES || (x) == LINELASTMODIFIED || (x) == LINELINK || \
(x) == LINELOCATION || (x) == LINEP3P || (x) == LINEPRAGMA || \
(x) == LINEPROXYAUTHENTICATE || (x) == LINEREFRESH || (x) == LINERETRYAFTER || \
(x) == LINESERVER || (x) == LINESTRICTTRANSPORTSECURITY || (x) == LINETRAILER || \
(x) == LINETRANSFERENCODING || (x) == LINEVARY || (x) == LINEVIA || \
(x) == LINEWARNING || (x) == LINEWWWAUTHENTICATE)

class HttpClient
{
public:
    HttpClient();
    ~HttpClient();
   
	static void global_init();
	static void global_cleanup();

	int destorycurl(); 
	int initcurl(int nConnTimeout = 50, int nOverTimeout = 50, bool bPersistent = false);

    std::string get(const std::string &url);
	std::string get(const std::string &url, const std::string& params);
    std::string get(const std::string &url, std::map<std::string, std::string> &m);
    std::string post(const std::string &url, std::map<std::string, std::string> &m);
    std::string post(const std::string &url, const std::string &m);
    std::string post(const std::string &url, const char* data, size_t dataLen);
	int upload(const std::string &url, const std::string &userPwd, const std::string &localFile);

    long getHttpCode();
    std::string getErrorInfo();
    void setContentLength(const int contentLength);
	void addHttpHeader(const std::string& key, const std::string& value);
    int getHttpHeader(std::map<std::string, std::string> &header);
    double getContentSize(){ return m_dContentSize; };
	void setHttpHeader(const std::map<std::string, std::string> &header);

private:
    long m_lHttpcode;
    double m_dContentSize;
    
    std::string m_sBuffer;
    std::string m_sContentType;
    std::string m_sContentLength;
    std::string m_sHttpHeaderLength;
    
	struct curl_slist* m_HttpHeaderList;
	char m_sErrorBuffer[CURL_ERROR_SIZE];
	std::map<std::string, std::string> m_mResponseHeader;

	CURL* m_pCurl;
	int m_nConTimeout;
	int m_nOverTimeout;
	bool m_bPersistent;
	std::map<std::string, std::string> m_mHeader;

	std::string strRtrim(std::string str);
	std::string strLtrim(std::string str);
	std::string strTrim(std::string str);
    std::string easycurl(const std::string& url, bool post, const char* data, size_t dataLen);
	std::string sendcurl(const std::string &url, bool post, const char* data, size_t dataLen);
	int checkUpLoad(curl_slist *http_headers, CURLcode& r);
};

size_t filewritefunc(void *ptr, size_t size, size_t nmemb, void *stream);
size_t filereadfunc(void *ptr, size_t size, size_t nmemb, void *stream);
int bufferwriterfunc(char *data, size_t size, size_t nmemb, std::string *buffer);
int httpheadreaderfunc(char *data, size_t size, size_t nmemb, struct curl_slist **httpHeaderList);
std::string UrlEncode(const std::string& str);

#endif

