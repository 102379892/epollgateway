#include <iostream>
#include <string>
#include <string.h>
#include "curl/curl.h"
#include "http/httpclient.h"
#include "clog/CLog.h"

using namespace std;

unsigned char ToHex(unsigned char x) 
{ 
	return  x > 9 ? x + 55 : x + 48; 
}

unsigned char FromHex(unsigned char x) 
{ 
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (isalnum((unsigned char)str[i]) || 
			(str[i] == '-') ||
			(str[i] == '_') || 
			(str[i] == '.') || 
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ')
			strTemp += "+";
		else
		{
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] % 16);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (str[i] == '+') strTemp += ' ';
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high*16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

void HttpClient::setHttpHeader(const std::map<std::string, std::string> &header)
{
    m_mHeader = header;
}

void HttpClient::addHttpHeader(const std::string& key, const std::string& value)
{
	m_mHeader.insert(std::pair<std::string, std::string>(key, value));
}

int HttpClient::initcurl(int nConnTimeout, int nOverTimeout, bool bPersistent)
{
    m_bPersistent = bPersistent;
    m_nConTimeout = nConnTimeout;
    m_nOverTimeout = nOverTimeout;

    if (m_bPersistent)
    {
        m_pCurl = curl_easy_init();
    }
    else
    {
        m_pCurl = NULL;
    }

    return 0;
}

int HttpClient::destorycurl()
{
    if (NULL != m_pCurl)
    {
        curl_easy_cleanup(m_pCurl);
    }

	if (m_HttpHeaderList != NULL)
	{
		curl_slist_free_all(m_HttpHeaderList);
		m_HttpHeaderList = NULL;
	}

    m_bPersistent = false;

    return 0;
}

void HttpClient::global_init()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void HttpClient::global_cleanup()
{
    curl_global_cleanup();
}

HttpClient::HttpClient()
{
	m_pCurl = NULL;
    m_lHttpcode = 0;
    m_dContentSize = 0;
    memset(m_sErrorBuffer, '\0', CURL_ERROR_SIZE);
    m_sBuffer = "";
    m_sContentType = "";
    m_sContentLength = "";
    m_sHttpHeaderLength = "";
	m_bPersistent = false;
	m_nConTimeout = 50;
	m_nOverTimeout = 50;

    m_HttpHeaderList = NULL;
    m_mResponseHeader.clear();
	m_mHeader.clear();
}

HttpClient::~HttpClient()
{
	destorycurl();
	m_pCurl = NULL;
    m_lHttpcode = 0;
    m_dContentSize = 0;
    memset(m_sErrorBuffer, '\0', CURL_ERROR_SIZE);
    m_sBuffer = "";
    m_sContentType = "";
    m_sContentLength = "";
    m_sHttpHeaderLength = "";

    m_HttpHeaderList = NULL;
    m_mResponseHeader.clear();
	m_mHeader.clear();
}

int HttpClient::getHttpHeader(std::map<std::string, std::string> &header)
{
    if (m_HttpHeaderList == NULL) return 0;
    struct curl_slist *ptr = m_HttpHeaderList;
    size_t firstPos, endPos;
    int headerSize = 0;
    string headerLine;
    string nameStr;
    string valueStr;
    while(ptr != NULL)
    {
        headerLine.assign(ptr->data, strlen(ptr->data));
        firstPos = headerLine.find(':');
        
        if(firstPos != std::string::npos)
        {
            endPos = headerLine.find("\r\n", firstPos);
            if(endPos == std::string::npos)
            {
                endPos = headerLine.find('\n', firstPos);
                if(endPos == std::string::npos)
                {
                    endPos = headerLine.length();
                }
            }
            nameStr = headerLine.substr(0, firstPos);
            valueStr = headerLine.substr(firstPos+1,endPos-1-firstPos);

            nameStr = strTrim(nameStr);
            valueStr = strTrim(valueStr);
            
            if(valueStr.length()!=0)
            {
                for(size_t i=0; i <= nameStr.size();i++)
                {
                    if(nameStr[i]>='A'&&nameStr[i]<='Z')
                    {
                        nameStr[i] += 32;
                        continue;
                    }
                }
                if(!CHECKHEADCONTENT(nameStr))
                {
                    header.insert(std::pair<string, string>(nameStr, valueStr));
                    headerSize++;
                }
            }
        }
        ptr = ptr->next;
    }

    return headerSize;
}

string HttpClient::get(const string &url)
{
    string params ="";
    return easycurl(url, false, params.c_str(), 0);
}

string HttpClient::get(const string &url, const string& params)
{
	return easycurl(url, false, params.c_str(), 0);
}

string HttpClient::get(const string &url, map<string, string> &m)
{
    string poststring="";

	if (m.size() > 0)
	{
		map<string, string>::iterator iter = m.begin();
		poststring += iter->first + "=" + UrlEncode(iter->second);
		++iter;
		for (; iter != m.end(); ++iter)
		{
			poststring += "&";
			poststring += iter->first + "=" + UrlEncode(iter->second);
		}
	}

    return easycurl(url, false, poststring.data(), poststring.size());
}

string HttpClient::post(const string &url, map<string, string> &m)
{
    string poststring="";

	if (m.size() > 0)
	{
		map<string, string>::iterator iter = m.begin();
		poststring += iter->first + "=" + UrlEncode(iter->second);
		++iter;
		for (; iter != m.end(); ++iter)
		{
			poststring += "&";
			poststring += iter->first + "=" + UrlEncode(iter->second);
		}
	}

    return easycurl(url, true, poststring.data(), poststring.size());
}

string HttpClient::post(const string &url, const string &m)
{
    return post(url, m.data(), m.size());
}

string HttpClient::post(const string &url, const char* data, size_t dataLen)
{
    return easycurl(url, true, data, dataLen);
}

int HttpClient::checkUpLoad(curl_slist *http_headers, CURLcode& r)
{
	int result = -1;
	if (r == CURLE_OK)
	{
		curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &m_lHttpcode);

		// FTP upload retCode:226 stands for "Requested file action successful"
		if (m_lHttpcode == 200 || m_lHttpcode == 201 || m_lHttpcode == 204 || m_lHttpcode == 206 || m_lHttpcode == 226)
		{
			m_lHttpcode = 200;
			result = 0;
		}
		else
		{
			result = -1;
		}

		/* now extract transfer info */
		double speed_upload = 0.0;
		double total_time = 0.0;
		curl_easy_getinfo(m_pCurl, CURLINFO_SPEED_UPLOAD, &speed_upload);
		curl_easy_getinfo(m_pCurl, CURLINFO_TOTAL_TIME, &total_time);
		curl_easy_getinfo(m_pCurl, CURLINFO_SIZE_UPLOAD, &m_dContentSize);
	}
	else
	{
		result = -1;
	}

	if (http_headers != NULL)
	{
		curl_slist_free_all(http_headers);
	}

	if (!m_bPersistent)
	{
		// 非持久连接，关闭当前连接
		curl_easy_cleanup(m_pCurl);
		m_pCurl = NULL;
	}

	return result;
}

int HttpClient::upload(const std::string &url, const std::string &userPwd, const std::string &localFile)
{
	m_lHttpcode = 0;

	bool httpUpload = (url.find("http://") != string::npos);
	bool ftpUpload = (url.find("ftp://") != string::npos);
	if (!httpUpload && !ftpUpload)
	{
		return -1;
	}

	if (m_bPersistent)
	{
		if (m_pCurl == NULL)
		{
			destorycurl();
			initcurl();
		}
	}
	else
	{
		m_pCurl = curl_easy_init();
	}

	CURLcode r = CURLE_GOT_NOTHING;
	FILE*  f = fopen((localFile).c_str(), "rb");
	if (f == NULL)
	{
		mglog(LL_ERROR, "<HttpClient_I::upload> OPEN FILE FAILE.");
		perror(NULL);
		return -1;
	}

	struct stat file_info;
	if (stat(localFile.c_str(), &file_info))
	{
		mglog(LL_ERROR, "<HttpClient_I::upload> OPEN FILE ERROR[localFile:%s].", localFile.c_str());
		return 1;
	}

	curl_slist *http_headers = NULL;
	curl_off_t fsize = (curl_off_t)file_info.st_size;
	http_headers = curl_slist_append(http_headers, m_sContentLength.c_str());

	for (std::map<string, string>::iterator it = m_mHeader.begin(); it != m_mHeader.end(); ++it)
	{
		http_headers = curl_slist_append(http_headers, (it->first + ":" + it->second).c_str());
	}

	if (!m_bPersistent)
	{
		http_headers = curl_slist_append(http_headers, "Connection: close");
	}
	else
	{
		http_headers = curl_slist_append(http_headers, "Connection: Keep-Alive");
	}

	curl_easy_setopt(m_pCurl, CURLOPT_ERRORBUFFER, m_sErrorBuffer);
	curl_easy_setopt(m_pCurl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, filereadfunc);
	curl_easy_setopt(m_pCurl, CURLOPT_READDATA, f);
	curl_easy_setopt(m_pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
	curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, m_nConTimeout);
	curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, m_nOverTimeout);
	curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 0);
	curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);

	if (ftpUpload)
	{
		//curl_easy_setopt(_curl, CURLOPT_POSTQUOTE, http_headers);
		// if userpwd is not set, here need not set CURLOPT_USERPWD
		if (!userPwd.empty())
		{
			curl_easy_setopt(m_pCurl, CURLOPT_USERPWD, userPwd.c_str());
		}
		curl_easy_setopt(m_pCurl, CURLOPT_FTPPORT, "-"); /* disable passive mode */
		curl_easy_setopt(m_pCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
		curl_easy_setopt(m_pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, m_nOverTimeout);
	}

	if (httpUpload)
	{
		curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, http_headers);
		curl_easy_setopt(m_pCurl, CURLOPT_PUT, 1L);
	}

	r = curl_easy_perform(m_pCurl);
	fclose(f);

	return checkUpLoad(http_headers, r);
}

string HttpClient::sendcurl(const string &url, bool post, const char* data, size_t dataLen)
{
	string sUrlTemp = url;
	CURLcode curlcode = CURLE_GOT_NOTHING;
	curl_slist *http_headers = NULL;
	http_headers = curl_slist_append(http_headers, m_sContentLength.c_str());

	for (std::map<string, string>::iterator it = m_mHeader.begin(); it != m_mHeader.end(); ++it)
	{
		http_headers = curl_slist_append(http_headers, (it->first + ":" + it->second).c_str());
	}

	if (!m_bPersistent)
	{
		http_headers = curl_slist_append(http_headers, "Connection: close");
	}
	else
	{
		http_headers = curl_slist_append(http_headers, "Connection: Keep-Alive");
	}

	if (post)
	{
		curl_easy_setopt(m_pCurl, CURLOPT_POST, 1);
		curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, dataLen);
		curl_easy_setopt(m_pCurl, CURLOPT_URL, sUrlTemp.data());
	}
	else
	{
		sUrlTemp += "?";
		sUrlTemp += data;
		curl_easy_setopt(m_pCurl, CURLOPT_URL, sUrlTemp.data());
	}

	bool ishttps = (url.find("https://") != string::npos);
	if (ishttps)
	{
		curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, http_headers);
	curl_easy_setopt(m_pCurl, CURLOPT_ERRORBUFFER, m_sErrorBuffer);

	curl_easy_setopt(m_pCurl, CURLOPT_HEADER, 0);
	curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);

	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, bufferwriterfunc);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &m_sBuffer);

	curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, httpheadreaderfunc);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEHEADER, &m_HttpHeaderList);

	curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, m_nOverTimeout);
	curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, m_nConTimeout);
	curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);

	curlcode = curl_easy_perform(m_pCurl);
	if (curlcode == CURLE_OK)
	{
		curl_easy_getinfo(m_pCurl, CURLINFO_RESPONSE_CODE, &m_lHttpcode);
	}
	else
	{
		mglog(LL_ERROR, "doGet: curl_easy_perform failed. URL:%s, Desc:%s", sUrlTemp.c_str(), curl_easy_strerror(curlcode));
	}

	if (http_headers != NULL)
	{
		curl_slist_free_all(http_headers);
	}

	if (!m_bPersistent)
	{
		curl_easy_cleanup(m_pCurl);
		m_pCurl = NULL;
	}

	if (m_lHttpcode == 200 || m_lHttpcode == 206)
	{
		return m_sBuffer;
	}
	
	mglog(LL_ERROR, "HTTP REQUEST RETURN CODE[lHttpcode:%ld,ErrorBuffer:%s,sBuffer:%.3000s,curlcode:%d,url:%s,reqLen:%ld,req:%s].", m_lHttpcode, m_sErrorBuffer, m_sBuffer.c_str(), curlcode, sUrlTemp.c_str(), dataLen, data);
	return "";
}

string HttpClient::easycurl(const string &url, bool post, const char* data, size_t dataLen)
{
    m_sBuffer = "";
    if (m_HttpHeaderList != NULL)
    {
        curl_slist_free_all(m_HttpHeaderList);
        m_HttpHeaderList = NULL;
    }
    m_sErrorBuffer[0] = 0;
    m_lHttpcode = 0;

    if (m_bPersistent)
    {    
        if (m_pCurl == NULL)
        {
            destorycurl();
            initcurl();
        }
    }
    else
    {
		m_pCurl = curl_easy_init();
    }

    if (m_pCurl)
    {
		if (post)
		{
			setContentLength(dataLen);
		}
		return sendcurl(url, post, data, dataLen);
    }

	return "";
}

void HttpClient::setContentLength(const int contentLength)
{
	m_sContentLength = "";
	char temp[128];
	sprintf(temp, "Content-Length: %d", contentLength);
	m_sContentLength = temp;
}

string HttpClient::getErrorInfo()
{
    return m_sErrorBuffer;
}

long HttpClient::getHttpCode()
{
    return m_lHttpcode;
}

std::string HttpClient::strLtrim(std::string str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
                                        std::not1(std::ptr_fun(::isspace))));
    return str;
}

std::string HttpClient::strRtrim(std::string str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(),
                           std::not1(std::ptr_fun(::isspace))).base(), str.end());
    return str;

}

std::string HttpClient::strTrim(std::string str)
{
    return strRtrim(strLtrim(str));
}

int bufferwriterfunc(char *data, size_t size, size_t nmemb, string *buffer)
{
    int result = 0;
    if (buffer != NULL)
    {
        buffer->append(data, size * nmemb);
        result = size * nmemb;
    }
    return result;
}

int httpheadreaderfunc(char *data, size_t size, size_t nmemb, struct curl_slist **buffer)
{
    int result = 0;
    *buffer = curl_slist_append(*buffer, data);
    result = size * nmemb;
    return result;
}

size_t filewritefunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

size_t filereadfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
    FILE *f = (FILE*)stream;
    size_t n;
    if (ferror(f))
        return CURL_READFUNC_ABORT;
    n = fread(ptr, size, nmemb, f) * size;
    return n;
}

