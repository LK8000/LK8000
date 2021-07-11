/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#endif

#include "Defines.h" // VENTA3

#include "Utils.h"
#include "Utils2.h"

// #include "resource.h"
#include "Units.h"

double AngleLimit360(double theta) {
  while (theta>=360.0) {
    theta-= 360.0;
  }
  while (theta<0.0) {
    theta+= 360.0;
  }
  return theta;
}


void DistanceBearing(double lat1, double lon1, double lat2, double lon2,
                     double *Distance, double *Bearing) {

// incomplete, test does not show benefits, low hits
#if (LK_CACHECALC && LK_CACHECALC_DBE)
  #define CASIZE_DBE 50
  static bool doinit=true;
  static int  cacheIndex;
  static double cur_checksum;
  static double cur_lat1, cur_lat2, cur_lon1, cur_lon2;
  bool cacheFound;
  int i;

  static double cache_checksum[CASIZE_DBE];
  static double cache_lat1[CASIZE_DBE];
  static double cache_lat2[CASIZE_DBE];
  static double cache_lon1[CASIZE_DBE];
  static double cache_lon2[CASIZE_DBE];
  static double cache_Distance[CASIZE_DBE];
  static double cache_Bearing[CASIZE_DBE];

  if (doinit) {
	cacheIndex=0;
	for (i=0; i<CASIZE_DBE; i++) {
		cache_checksum[i]=0;
		cache_lat1[i]=0;
		cache_lat2[i]=0;
		cache_lon1[i]=0;
		cache_lon2[i]=0;
		cache_Distance[i]=0;
		cache_Bearing[i]=0;
	}
	doinit=false;
  }

  Cache_Calls_DBE++;
  cur_checksum=lat1+lat2+lon1+lon2;
  cacheFound=false;

  for (i=0; i<CASIZE_DBE; i++) {
	if ( cache_checksum[i] != cur_checksum ) continue;
	if ( cache_lat1[i] != lat1 ) {
		Cache_False_DBE++;
		continue;
	}
	if ( cache_lat2[i] != lat2 ) {
		Cache_False_DBE++;
		continue;
	}
	if ( cache_lon1[i] != lon1 ) {
		Cache_False_DBE++;
		continue;
	}
	if ( cache_lon2[i] != lon2 ) {
		Cache_False_DBE++;
		continue;
	}
	cacheFound=true;
	break;
  }

  if (cacheFound) {
	Cache_Hits_DBE++;
  }  else {
	cur_lat1=lat1;
	cur_lat2=lat2;
	cur_lon1=lon1;
	cur_lon2=lon2;
  }
#endif


  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;

  double clat1 = cos(lat1);
  double clat2 = cos(lat2);
  double dlon = lon2-lon1;

  if (Distance) {
    double s1 = sin((lat2-lat1)/2);
    double s2 = sin(dlon/2);
    double a= max(0.0,min(1.0,s1*s1+clat1*clat2*s2*s2));
    *Distance = 6371000.0*2.0*atan2(sqrt(a),sqrt(1.0-a));
  }
  if (Bearing) {
    double y = sin(dlon)*clat2;
    double x = clat1*sin(lat2)-sin(lat1)*clat2*cos(dlon);
    *Bearing = (x==0 && y==0) ? 0:AngleLimit360(atan2(y,x)*RAD_TO_DEG);
  }

#if (LK_CACHECALC && LK_CACHECALC_DBE)
  if (!cacheFound) {
	if (++cacheIndex==CASIZE_DBE) cacheIndex=0;
	cache_checksum[cacheIndex]=cur_checksum;
	cache_lat1[cacheIndex]=cur_lat1;
	cache_lat2[cacheIndex]=cur_lat2;
	cache_lon1[cacheIndex]=cur_lon1;
	cache_lon2[cacheIndex]=cur_lon2;
  }
#endif

}
