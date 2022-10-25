#ifndef __T__INIREADER__T__
#define __T__INIREADER__T__

#include <vector>
#include <string>

using namespace std;

class CRecord
{
public:
    string sSection;
    string sKey;
    string sValue;

    CRecord()
    {
    }
	
	~CRecord()
    {
    }
};

class CIniReader
{
public:
	CIniReader();
	~CIniReader();
	int Print(string& sBugBuf);
	int Load(const string& sFileName);
	int Get(const string& sSection, vector<CRecord>& vtConfig);
	int Get(const string& sSection, const string& skey, string& sValue);
	int Prase(string& sLineBuf, CRecord& Record);
	
	vector<CRecord> m_vtConfig;
};

void trubstr(string& sBuf, const string& sTrubBuf = " \t\n\r");

#endif


