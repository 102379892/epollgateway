#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include "prase/ReportPrase.h"
#include "clog/CLog.h"
#include "common/point.h"
#include "json/json.h"
#include "http/httpclient.h"

std::map<std::string, std::string> MAPGOO_PROTO_HEADER_TAILER_PAIR = {
    {"*MG", "#"},
    {"[MG", "]"},	//%是string.find的转移字符，[ ]在string.find内是做模式匹配字符
    {"*HQ", "#"}
};

stTrackInfo::stTrackInfo()
{
    gps_time = 0;
    system_time = 0;
    lat = 0;
    lng = 0;
    gps_flag = 0;
    speed = 0;
    direct = 0;
    altiude = 0;
    gps_satelite_num = 0;
    beidou_satelite_num = 0;
    status = 0;
    acc = 0;
    recstatus = 0;
    mileage = 0;
}

int stTrackInfo::get1(const char* str)
{
	int ret = (int)str[0] & 0x000000ff;
	return ret;
}

int stTrackInfo::get2(const char* str)
{
	int ret = 0;
	ret |= (int)str[0] & 0x000000ff;
	ret |= ((int)str[1] << 8) & 0x0000ff00;
	return ret;
}


int stTrackInfo::get4(const char* str)
{
	int ret = 0;
	ret |= (int)str[0] & 0x000000ff;
	ret |= ((int)str[1] << 8) & 0x0000ff00;
	ret |= ((int)str[2] << 16) & 0x00ff0000;
	ret |= ((int)str[3] << 24) & 0xff000000;
	return ret;
}

long stTrackInfo::get6(const char* str)
{
	long ret = 0;
	ret |= (long)str[0] & 0x00000000000000ff;
	ret |= ((long)str[1] << 8) & 0x000000000000ff00;
	ret |= ((long)str[2] << 16) & 0x0000000000ff0000;
	ret |= ((long)str[3] << 24) & 0x00000000ff000000;
	ret |= ((long)str[4] << 32) & 0x000000ff00000000;
	ret |= ((long)str[5] << 40) & 0x0000ff0000000000;
	return ret;
}

int stTrackInfo::transpos(const int value)
{
    int degree = (int)(value / 1000000);
    double min_sec = (value % 1000000) / 10000.0;
    return (int)((degree * 1.0 + min_sec/60) * 1000000);
}

int stTrackInfo::getgpsflag(const int flag) 
{
    if ((flag & GPS_LOCATION_FLAG_V) == GPS_LOCATION_FLAG_V)
    {
        return GPS_LOCATION_FLAG;
    }
    else if ((flag & BS_LOCATION_FLAG_V) == BS_LOCATION_FLAG_V)
    {
        return BS_LOCATION_FLAG;
    }
    else 
    {
        return GPS_LOCATION_FLAG;
    }
}

int stTrackInfo::decode(const char* recvBuf, const int recvLen)
{
    if (recvLen < BL_SINGLE_TRACK_ITEM_LEN)
    {
        return -1;
    }

    int pos = 0;
    gps_time = get6(recvBuf + pos);
    pos += 6;

    system_time = get6(recvBuf + pos);
    pos += 6;

    lat = transpos(get4(recvBuf + pos));
    pos += 4;
    
    lng = transpos(get4(recvBuf + pos));
    pos += 4;

    gps_flag = getgpsflag(get1(recvBuf + pos));
    pos += 1;

    speed = get1(recvBuf + pos);
    pos += 1;
    
    direct = get1(recvBuf + pos);
    pos += 1;
    
    altiude = get2(recvBuf + pos);
    pos += 2;

    gps_satelite_num = get1(recvBuf + pos);
    pos += 1;

    beidou_satelite_num = get1(recvBuf + pos);
    pos += 1;

    status = get1(recvBuf + pos);
    pos += 1;
    
    acc = status & 0x00000001;
    recstatus = status & 0x00000002;

    mileage = get4(recvBuf + pos);
    pos += 4;
    
    return 0;
}

std::string stTrackInfo::print()
{
    std::ostringstream os("");
    os << "gps_time=" << gps_time 
    << ",system_time=" << system_time
    << ",lat=" << lat
    << ",lng=" << lng
    << ",gps_flag=" << gps_flag 
    << ",speed=" << speed
    << ",direct=" << direct
    << ",altiude=" << altiude
    << ",gps_satelite_num=" << gps_satelite_num
    << ",beidou_satelite_num=" << beidou_satelite_num
    << ",status=" << status
    << ",acc=" << acc
    << ",recstatus=" << recstatus
    << ",mileage=" << mileage
    << ";";

    GeoPointFloat point(lng, lat);
    GeoPointFloat bdpoint = WGS2BD(point);
    std::string url = "https://api.map.baidu.com/geocoder";
    char temp[1024] = {'\0'};
    sprintf(temp, "location=%.6f,%.6f&output=json&key=8cb976834235d8cbcde2dce4835ae191", bdpoint.Lat, bdpoint.Lng);
    std::string params = temp;

    mglog(LL_INFO, "get url(url:%s,params:%s).", url.c_str(), params.c_str());

    HttpClient client;
    client.initcurl();
    std::string resp = client.get(url, params);

    Json::Value root;
    Json::Reader reader;
    std::istringstream is;
    is.str(resp);
    if (!reader.parse(is, root))
    {
        mglog(LL_ERROR, "prase point(resp:%s).", resp.c_str());
    }
    else
    {
        /*
        {
            "status":"OK",
            "result":{
                "location":{
                    "lng":113.515186,
                    "lat":22.362154
                },
                "formatted_address":"广东省珠海市香洲区会同南路",
                "business":"",
                "addressComponent":{
                    "city":"珠海市",
                    "direction":"",
                    "distance":"",
                    "district":"香洲区",
                    "province":"广东省",
                    "street":"会同南路",
                    "street_number":""
                },
                "cityCode":140
            }
        }
        */
        if (root.isMember("result") && root["result"].isObject())
        {
            Json::Value result = root["result"];
            if (result.isMember("formatted_address"))
            {
                os << result["formatted_address"];
            }
        }
    }

    return os.str();
}

CReportPrase::CReportPrase()
{
    HttpClient::global_init();
}

CReportPrase::~CReportPrase()
{
    HttpClient::global_cleanup();
}

int CReportPrase::DealReport(const char* recvBuf, const int recvLen, std::string& imei, char*& sendBuf, int& sendLen)
{
    if (recvLen < (MIN_MAPGOO_HEADER_LEN + PROTO_VERSION.length()))
    {
        mglog(LL_ERROR, "DealReport len is not invaild(recvBuf:%s,imei:%s,recvLen:%d).", recvBuf, imei.c_str(), recvLen);
        return -1;
    }

    int lastpos = 0;
    std::map<std::string, std::string>::iterator iter = MAPGOO_PROTO_HEADER_TAILER_PAIR.begin();
    for (; iter != MAPGOO_PROTO_HEADER_TAILER_PAIR.end(); ++iter)
    {
        int startindex = 0;
        int endindex = 0;
        bool isfindstart = false;
        int pos = lastpos;
        while(pos < recvLen)
        {
            if (isfindstart == false)
            {
                std::string headstr(recvBuf + pos, MIN_MAPGOO_HEADER_LEN);
                if (iter->first == headstr)
                {
                    isfindstart = true;
                    startindex = pos;
                    pos += MIN_MAPGOO_HEADER_LEN;
                    continue;
                }
            }
            else
            {
                std::string headstr(recvBuf + pos, 1);
                if (iter->second == headstr)
                {
                    endindex = pos;
                    int templen = endindex - startindex + 1;
                    char* tempbuf = new char[templen + 1];
                    memset(tempbuf, '\0', templen + 1);
                    memcpy(tempbuf, recvBuf + startindex, templen);
                    PraseReport(tempbuf, templen, imei);
                    delete [] tempbuf;
                    tempbuf = NULL;
                    lastpos = pos + 1;

                    isfindstart = false;
                    startindex = 0;
                    endindex = 0;
                }
            }
            ++pos;
        }
    }
    
    return 0;
}

int CReportPrase::PraseReport(const char* recvBuf, const int recvLen, std::string& imei)
{
    std::ostringstream os("");
    for (int i = 0; i < recvLen; i++)
    {
        unsigned int a = ((unsigned int)recvBuf[i]) & 0x000000ff;
        os << std::setfill('0') << std::setw(2) << std::hex << a << " ";
    }
    mglog(LL_INFO, "PraseReport(recvBuf:%s,recvLen:%d,hex:%s).", recvBuf, recvLen, os.str().c_str());
    
    bool isexitimei = false; 
    int imeilen = 0;
    int startlen = MIN_MAPGOO_HEADER_LEN + PROTO_VERSION.length() + 1;
    for (int i = startlen; i < recvLen; ++i)
    {
        if (recvBuf[i] == ',')
        {
            isexitimei = true;
            break;
        }
        else
        {
            imeilen++;
        }
    }

    std::string temp(recvBuf+startlen, imeilen);
    imei = temp;
    if (isexitimei == false && recvBuf[startlen] == 'Y')
    {
        mglog(LL_INFO, "DealReport dev resp(recvBuf:%s,imei:%s,recvLen:%d).", recvBuf, imei.c_str(), recvLen);
        return 0;
    }
    mglog(LL_INFO, "PraseReport imei(recvBuf:%s,imei:%s,recvLen:%d).", recvBuf, imei.c_str(), recvLen);

    int startindex = 0;
    int endindex = 0;
    for (int i = 0; i < recvLen; ++i)
    {
        if (recvBuf[i] == ',')
        {
            startindex = i + 1;
        }
        else if (recvBuf[i] == '#' || recvBuf[i] == ']')
        {
            endindex = i;
        }
    }

    std::string version = "2";
    std::string datatype(recvBuf + startindex, 2);
    if (datatype == TRACK_BL_FLAG)
    {
        startindex += TRACK_BL_FLAG.length() + version.length() + 2;
        int datalen = endindex - startindex;
       
        int n = datalen/BL_SINGLE_TRACK_ITEM_LEN;
        for (int i = 0; i < n; ++i)
        {
            char* databuf = new char[BL_SINGLE_TRACK_ITEM_LEN + 1];
            memset(databuf, '\0', BL_SINGLE_TRACK_ITEM_LEN + 1);
            memcpy(databuf, recvBuf + startindex + i*BL_SINGLE_TRACK_ITEM_LEN, BL_SINGLE_TRACK_ITEM_LEN);

            stTrackInfo track;
            int ret = track.decode(databuf, BL_SINGLE_TRACK_ITEM_LEN);
            if (ret != 0)
            {
                return ret;
            } 
            else
            {
                mglog(LL_INFO, "DealReport track(%s).", track.print().c_str());
            }
        }
    }
    return 0;
}
