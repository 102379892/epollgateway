#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <algorithm>
#include "config/IniReader.h"

void trubstr(string& sBuf, const string& sTrubBuf)
{
	size_t spos = sBuf.find_first_not_of(sTrubBuf);
	size_t epos = sBuf.find_last_not_of(sTrubBuf);
	if ((spos == string::npos) || (epos == string::npos)) 
	{
		sBuf.clear();
	}
	else
	{
		sBuf = sBuf.substr(spos, epos - spos + 1); 
	}
	
	return;
}

CIniReader::CIniReader()
{
	m_vtConfig.clear();
}

CIniReader::~CIniReader()
{

}

int CIniReader::Print(string& sBugBuf)
{
	vector<CRecord>::iterator iter = m_vtConfig.begin();
    for (; iter != m_vtConfig.end(); ++iter)
    {
        sBugBuf += iter->sSection;
		sBugBuf += "|";
		sBugBuf += iter->sKey;
		sBugBuf += "|";
		sBugBuf += iter->sValue;
		sBugBuf += "|";
		sBugBuf += "\n";
    }
    return 0;
}

int CIniReader::Prase(string& sLineBuf, CRecord& Record)
{
	string::size_type nIndex = 0;
	nIndex = sLineBuf.find_first_of("#");
	if (nIndex != string::npos)
	{
		sLineBuf = sLineBuf.substr(0, nIndex);
	}
	
	nIndex = sLineBuf.find_first_of(")");
	if (nIndex != string::npos)
	{
		string::size_type nBeIndex = sLineBuf.find_first_of("(");
		if (nBeIndex != string::npos)
		{
			sLineBuf = sLineBuf.substr(nBeIndex+1, nIndex - nBeIndex);
		}
	}
	
	trubstr(sLineBuf, " \t\n\r");
	if (sLineBuf == "")
	{
		return 0;
	}
	 
	if ('[' == sLineBuf[0] && ']' == sLineBuf[sLineBuf.length() - 1])
	{
		fprintf(stderr, "sLineBuf:%s\n",sLineBuf.c_str());
		trubstr(sLineBuf, "[]");
		trubstr(sLineBuf, " \t\n\r");
		Record.sSection = sLineBuf;
		transform(Record.sSection.begin(), Record.sSection.end(), Record.sSection.begin(), ::tolower);
		return 0;
	}
	
	if (Record.sSection == "")
	{
		//return 0;
	}
	
	nIndex = 0;
	nIndex = sLineBuf.find_first_of("=");
	if (nIndex == string::npos)
	{
		return 0;
	}
	
	Record.sKey = sLineBuf.substr(0, nIndex);
	
	trubstr(Record.sKey, " \t\n\r");
	transform(Record.sKey.begin(), Record.sKey.end(), Record.sKey.begin(), ::tolower);

	fprintf(stderr, "sKey:%s,",Record.sKey.c_str());
	
	Record.sValue = sLineBuf.substr(nIndex + 1, sLineBuf.length());
	trubstr(Record.sValue, " \t\n\r");
	
	fprintf(stderr, "sValue:%s\n",Record.sValue.c_str());
	
	if (!Record.sKey.empty() && !Record.sValue.empty())
		m_vtConfig.push_back(Record);
	
	return 0;
}

int CIniReader::Load(const string& sFileName)
{
	int fd = open(sFileName.c_str(), O_RDONLY);
	if (fd < 0)
	{
		fprintf(stderr, "[CIniReader] open file failed(%d,%s,%s).\n", errno, strerror(errno), sFileName.c_str());
		return -1;
	}
	
	struct stat stbuf;
	memset(&stbuf, 0, sizeof(stbuf));
	if (fstat(fd, &stbuf) != 0)
	{
		fprintf(stderr, "[CIniReader] fstat failed(%d,%s,%s).\n", errno, strerror(errno), sFileName.c_str());
		close(fd);
		return -1;
	}

	char *sb = NULL;
	if ( (sb = (char *)mmap(0, stbuf.st_size, PROT_READ , MAP_PRIVATE,  fd, 0)) == MAP_FAILED)
	{
		fprintf(stderr, "[CIniReader] mmap failed(%d,%s,%s).\n", errno, strerror(errno), sFileName.c_str());
		close(fd);
		return -1;
	}
	close(fd);
	
	CRecord Record;
	char* pLineBegin = sb;
	char* pLineEnd = NULL;
	char* pEnd = sb + stbuf.st_size;
	do
	{
		if (NULL == pLineBegin)
		{
			break;
		}
		
		pLineEnd = strstr(pLineBegin, "\n");
		if (NULL == pLineEnd)
		{
			pLineEnd = pEnd + 1;
		}
		string sLineBuf(pLineBegin, 0, pLineEnd - pLineBegin);
		Prase(sLineBuf, Record);
			
		pLineBegin = pLineEnd + 1;
	}while(pLineBegin < pEnd);

	munmap(sb, stbuf.st_size);
    return 0;
}

int CIniReader::Get(const string& sSection, vector<CRecord>& vtConfig)
{
	int nRet = -1;
    vector<CRecord>::iterator iter = m_vtConfig.begin();
    for (; iter != m_vtConfig.end(); ++iter)
    {
        if (sSection == iter->sSection)
        {
             vtConfig.push_back(*iter);
			 nRet = 0;
        }
    }
    return nRet;
}

int CIniReader::Get(const string& sSection, const string& skey, string& sValue)
{
	int nRet = -1;
    vector<CRecord>::iterator iter = m_vtConfig.begin();
    for (; iter != m_vtConfig.end(); ++iter)
    {
        if (sSection == iter->sSection && skey == iter->sKey)
        {
            sValue = iter->sValue;
			nRet = 0;
        }
    }
    return nRet;
}
