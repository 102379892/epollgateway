#include <iostream>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iomanip>
#include "point.h"

GeoPointFloat::GeoPointFloat()
{
	Lat = 0;
	Lng = 0;
}

GeoPointFloat::GeoPointFloat(const double lng, const double lat)
{
	Lng = lng;
	Lat = lat;
}

GeoPointFloat::GeoPointFloat(const int lng, const int lat)
{
	Lng = (double)lng / 1000000;
	Lat = (double)lat / 1000000;
}

GeoPointFloat::GeoPointFloat(const GeoPointFloat& point)
{
	Lng = point.Lng;
	Lat = point.Lat;
}
	
GeoPointFloat LocationMake(double lng, double lat)
{
	GeoPointFloat loc;
	loc.Lng = lng;
	loc.Lat = lat;
	return loc;
}

bool outOfChina(double lat, double lon)
{
	if (lon < 72.004 || lon > 137.8347)
		return true;
	if (lat < 0.8293 || lat > 55.8271)
		return true;
	return false;
}

double transformLat(double x, double y)
{
	double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(x > 0 ? x : -x);
	ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
	ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
	ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
	return ret;
}

double transformLon(double x, double y)
{
	double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(x > 0 ? x : -x);
	ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
	ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
	ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
	return ret;
}

GeoPointFloat WGS2GCJ(GeoPointFloat wgLoc)
{
	GeoPointFloat mgLoc;
	if (outOfChina(wgLoc.Lat, wgLoc.Lng))
	{
		mgLoc = wgLoc;
		return mgLoc;
	}
	double dLat = transformLat(wgLoc.Lng - 105.0, wgLoc.Lat - 35.0);
	double dLon = transformLon(wgLoc.Lng - 105.0, wgLoc.Lat - 35.0);
	double radLat = wgLoc.Lat / 180.0 * pi;
	double magic = sin(radLat);
	magic = 1 - ee * magic * magic;
	double sqrtMagic = sqrt(magic);
	dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
	dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
	mgLoc.Lat = wgLoc.Lat + dLat;
	mgLoc.Lng = wgLoc.Lng + dLon;

	return mgLoc;
}

GeoPointFloat GCJ2WGS(GeoPointFloat gcLoc)
{
	if (outOfChina(gcLoc.Lat, gcLoc.Lng))
	{
		return gcLoc;
	}

	GeoPointFloat wgLoc;
	GeoPointFloat temp = gcLoc;
	GeoPointFloat currGcLoc = WGS2GCJ(temp);
	wgLoc.Lat = gcLoc.Lat - (currGcLoc.Lat - gcLoc.Lat);
	wgLoc.Lng = gcLoc.Lng - (currGcLoc.Lng - gcLoc.Lng);
	return wgLoc;
}

//  Transform GCJ-02 to BD-09
GeoPointFloat GCJ2BD(GeoPointFloat gcLoc)
{
	if (outOfChina(gcLoc.Lat, gcLoc.Lng))
	{
		return gcLoc;
	}
	double x = gcLoc.Lng, y = gcLoc.Lat;
	double z = sqrt(x * x + y * y) + 0.00002 * sin(y * x_pi);
	double theta = atan2(y, x) + 0.000003 * cos(x * x_pi);
	return LocationMake(z * cos(theta) + 0.0065, z * sin(theta) + 0.006);
}

//  Transform BD-09 to GCJ-02
GeoPointFloat BD2GCJ(GeoPointFloat bdLoc)
{
	if (outOfChina(bdLoc.Lat, bdLoc.Lng))
	{
		return bdLoc;
	}
	double x = bdLoc.Lng - 0.0065, y = bdLoc.Lat - 0.006;
	double z = sqrt(x * x + y * y) - 0.00002 * sin(y * x_pi);
	double theta = atan2(y, x) - 0.000003 * cos(x * x_pi);
	return LocationMake(z * cos(theta), z * sin(theta));
}

//	百度转真实坐标
//  Transform BD-09 to WGS-84
GeoPointFloat BD2WGS(GeoPointFloat bdLoc)
{
	if (outOfChina(bdLoc.Lat, bdLoc.Lng))
	{
		return bdLoc;
	}
	return GCJ2WGS(BD2GCJ(bdLoc));
}

//	真实坐标转百度
//  Transform WGS-84 to BD-09
GeoPointFloat WGS2BD(GeoPointFloat gcLoc)
{
	if (outOfChina(gcLoc.Lat, gcLoc.Lng))
	{
		return gcLoc;
	}
	return GCJ2BD(WGS2GCJ(gcLoc));
}

bool isInRegion(const GeoPointInt &point, const std::vector<GeoPointInt>& points)
{
	if(points.size() < 3)
		return false;

	int crossNum = 0;
	for(std::vector<GeoPointInt>::const_iterator it = points.begin();
		it != points.end();
		++it)
	{
		GeoPointInt p1 = *it;
		GeoPointInt p2;
		if((it+1) != points.end())
			p2 = *(it+1);
		else
			p2 = *(points.begin());

		if(point.lat < std::min(p1.lat, p2.lat) || point.lat > std::max(p1.lat, p2.lat))
			continue;

		if(point.lng > std::max(p1.lng, p2.lng))
			continue;

		if(point.lng < std::min(p1.lng, p2.lng))
		{
			if(point.lat < std::max(p1.lat, p2.lat))
				++crossNum;
			continue;
		}

		int deltaLat = p1.lat - p2.lat;
		int deltaLng = p1.lng - p2.lng;
		if(deltaLat == 0 || deltaLng == 0) // point is in the edge
			return true;

		double lngOfPointInEdge = p1.lng + (double)(point.lat - p1.lat)*deltaLng/deltaLat;
		if(fabs(point.lng - lngOfPointInEdge) < 1) // approximately conclude that point is in the edge
		{
			return true;
		}
		else if(point.lng < lngOfPointInEdge)
		{
			if(point.lat < std::max(p1.lat, p2.lat))
			{
				++crossNum;
			}
		}
	}

	return (crossNum&1);
}

int getDistance(const GeoPointInt &p1, const GeoPointInt &p2)
{
	double lng1 = ((double)p1.lng / 1000000) * cg_PI / 180.0;
	double lat1 = ((double)p1.lat / 1000000) * cg_PI / 180.0;
	double lng2 = ((double)p2.lng / 1000000) * cg_PI / 180.0;
	double lat2 = ((double)p2.lat / 1000000) * cg_PI / 180.0;

	double lonDist = lng1 - lng2;
	double latDist = lat1 - lat2;

	double s1 = sin(latDist / 2.0);
	double s2 = sin(lonDist / 2.0);
	double a = s1 * s1 + cos(lat1) * cos(lat2) *s2 * s2;

	double b = sqrt(a);

	if (b > 1.0) {
		b = 1.0;
	}

	double c = 2 * atan2(b, sqrt(1 - a));

	return (int)(round(cg_EARTH_RADIUS * c *10000 / 10 ));
}
