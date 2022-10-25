#ifndef __H__POINT__H__
#define __H__POINT__H__

const static double cg_PI = 3.1415926535897932384626433832795;	//pi
const static double cg_EARTH_RADIUS = 6378.137;					//earch radius, unit:km
const double a = 6378245.0;
const double pi = 3.14159265358979324;
const double ee = 0.00669342162296594323;
const double x_pi = 3.14159265358979324 * 3000.0 / 180.0;

struct GeoPointInt
{
	int lng;
	int lat;
};

struct GeoPointFloat
{
	double Lat;
	double Lng;

	GeoPointFloat();
	GeoPointFloat(const double lng, const double lat);
	GeoPointFloat(const int lng, const int lat);
	GeoPointFloat(const GeoPointFloat& point);
};

GeoPointFloat WGS2BD(GeoPointFloat gcLoc);


#endif