#ifndef __H__REPORTPRASE__H__
#define __H__REPORTPRASE__H__

#include <string>
#include <map>

const int MIN_MAPGOO_HEADER_LEN = 3;

const std::string PROTO_VERSION = "20";

const std::string TRACK_BG_FLAG = "BG";
const std::string TRACK_BH_FLAG = "BH";
const std::string TRACK_BI_FLAG = "BI";
const std::string TRACK_BL_FLAG = "BL";

const std::string MG_PROTO_FUNC_AH = "AH";
const std::string MG_PROTO_FUNC_AB = "AB";

const int MAX_TRACK_BITCH_COUNT = 2000;
const int BG_SINGLE_TRACK_ITEM_LEN = 23;
const int BH_SINGLE_TRACK_ITEM_LEN = 27;
const int BI_SINGLE_TRACK_ITEM_LEN = 44;
const int BL_SINGLE_TRACK_ITEM_LEN = 32;

//协议包中定位模式的值
const int GPS_LOCATION_FLAG_V = 0x30;
const int BS_LOCATION_FLAG_V = 0x40;

//解析到后台的定位模式值
const int GPS_LOCATION_FLAG = 0;	//精确GPS定位
const int BS_LOCATION_FLAG = 2;		//基站定位

struct stTrackInfo {
    //定位时间，6个字节
    long gps_time;
    
    //系统时间，6个字节
    long system_time;

    //纬度，4个字节
    int lat;

    //经度，4个字节
    int lng;

    //定位标记，1个字节
    int gps_flag ;

    //速度，1个字节，单位是2节
    int speed;

    //方向，1个字节
    int direct;

    //海拔高度，2个字节
    int altiude;

    //GPS卫星颗数，1个字节
    int gps_satelite_num;

    //北斗卫星颗数，1个字节
    int beidou_satelite_num;

    //状态位，1个字节
    int status;
    //状态位的第0个bit位代表acc状态，0-acc off 1-acc on
    int acc;
    //状态位的第1个bit位代表要素开启状态，0-off 1-on
    int recstatus;

    //里程值 4个字节
    int mileage;

    stTrackInfo();
	int get1(const char* str);
	int get2(const char* str);
	int get4(const char* str);
	long get6(const char* str);
    int transpos(const int value);
    int getgpsflag(const int flag);
    int decode(const char* recvBuf, const int recvLen);
    std::string print();
};

class CReportPrase {
public:
	CReportPrase();
	~CReportPrase();
    
	int DealReport(const char* recvBuf, const int recvLen, std::string& imei, char*& sendBuf, int& sendLen);
	int PraseReport(const char* recvBuf, const int recvLen, std::string& imei);
};

#endif

