/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: NavFunctions.cpp,v 8.2 2010/12/12 15:50:41 root Exp root $
*/

#include "StdAfx.h"
#include "NavFunctions.h"
#include "lk8000.h"
#include "Utils.h"

#include "utils/heapcheck.h"


#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif



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


double DoubleDistance(double lat1, double lon1, double lat2, double lon2,
		      double lat3, double lon3) {

  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lat3 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  lon3 *= DEG_TO_RAD;

  double clat1 = cos(lat1);
  double clat2 = cos(lat2);
  double clat3 = cos(lat3);
  double dlon21 = lon2-lon1;
  double dlon32 = lon3-lon2;

  double s21 = sin((lat2-lat1)/2);
  double sl21 = sin(dlon21/2);
  double s32 = sin((lat3-lat2)/2);
  double sl32 = sin(dlon32/2);

  double a12 = max(0.0,min(1.0,s21*s21+clat1*clat2*sl21*sl21));
  double a23 = max(0.0,min(1.0,s32*s32+clat2*clat3*sl32*sl32));
  return 6371000.0*2.0*(atan2(sqrt(a12),sqrt(1.0-a12))
			+atan2(sqrt(a23),sqrt(1.0-a23)));

}




void FindLatitudeLongitude(double Lat, double Lon, 
                           double Bearing, double Distance,
                           double *lat_out, double *lon_out)
{
  double result;

  Lat *= DEG_TO_RAD;
  Lon *= DEG_TO_RAD;
  Bearing *= DEG_TO_RAD;
  Distance = Distance/6371000;

  double sinDistance = sin(Distance);
  double cosLat = cos(Lat);

  if (lat_out) {
    result = (double)asin(sin(Lat)*cos(Distance)
                          +cosLat*sinDistance*cos(Bearing));
    result *= RAD_TO_DEG;
    *lat_out = result;
  }
  if (lon_out) {
    if(cosLat==0)
      result = Lon;
    else {
      result = Lon+(double)asin(sin(Bearing)*sinDistance/cosLat);
      result = (double)fmod((result+M_PI),(M_2PI));
      result = result - M_PI;
    }
    result *= RAD_TO_DEG;
    *lon_out = result;
  }
}



void xXY_to_LL(double Lat_TP, double Long_TP, double X_int, double Y_int, double *Lat, double *Long)
{
   double X, Y, Ynp, Ysp;
   double Sin_Lat_TP, Cos_Lat_TP;
   double Sin_DLng, Cos_DLng;
   double Lat_Num, Lat_Denum;
   double Long_Num, Long_Denum;
   double Temp; 
   double   Delta_Long;
 //  long   X_Own = 0;
 //  long   Y_Own = 0;

   /* Adjust the X,Y coordinate's according to the "global" X,Y    */
   /* coordinate system.                                           */

   //xrTP_Position(&X_Own, &Y_Own);
   X = (double) (X_int);
   Y = (double) (Y_int);
   if ( Lat_TP != 90 ) {

      /* Non polar projection */

      if ( X != 0.0 ) 
	  {

         Cos_Lat_TP = cos((double) DEG_TO_RAD * (double) Lat_TP);
         Sin_Lat_TP = sin((double) DEG_TO_RAD * (double) Lat_TP);

         Temp = EARTH_DIAMETER * Cos_Lat_TP;
         Ynp =  Temp/(1.0 + Sin_Lat_TP);
         Ysp = -Temp/(1.0 - Sin_Lat_TP);

         Long_Num = X * (Ynp - Ysp);
         Long_Denum = (Ynp - Y)*(Y - Ysp) - X*X;
         Delta_Long = (double)(RAD_TO_DEG * atan2(Long_Num, Long_Denum));
         *Long = Long_TP + Delta_Long;

         Cos_DLng = cos((double) DEG_TO_RAD * (double) Delta_Long);
         Sin_DLng = sin((double) DEG_TO_RAD * (double) Delta_Long);
         Lat_Num   = Y*Sin_DLng + X*Sin_Lat_TP*Cos_DLng;
         Lat_Denum = X*Cos_Lat_TP;

         *Lat  = (double)(RAD_TO_DEG * atan2(Lat_Num, Lat_Denum));
         if ( *Lat > 90 ) {
               *Lat -= 180;
         }
         else {
            if ( *Lat < -90 ) {
               *Lat += 180;
            }
         }
      }
      else {
         *Lat  = Lat_TP + (double)(RAD_TO_DEG * atan2(Y, EARTH_DIAMETER) * 2);
         *Long = Long_TP;
      } 
   }
   else {

      /* Polar projection     */

      Delta_Long = (double)(RAD_TO_DEG * atan2(-X, Y));

      Lat_Num = X*X + Y*Y;
      Lat_Denum = SQUARED_EARTH_DIAMETER;

      *Lat = 90 -(double)(RAD_TO_DEG * atan2(Lat_Num, Lat_Denum) * 2);
      *Long = Long_TP + Delta_Long;
   }
}


void xLL_to_XY(double Lat_TP, double Long_TP, double Lat_Pnt, double Long_Pnt, double *X, double *Y)
{
   double       Delta_Long;
   double     sin_TP_Lat;
   double     cos_TP_Lat;
   double    sin_Lat_Pnt;
   double    cos_Lat_Pnt;
   double sin_Delta_Long;
   double cos_Delta_Long;
   double          Denom;



   Delta_Long = Long_Pnt - Long_TP;

   sin_TP_Lat = sin((double)(Lat_TP * DEG_TO_RAD));
   cos_TP_Lat = cos((double)(Lat_TP * DEG_TO_RAD));
   sin_Lat_Pnt = sin((double)(Lat_Pnt * DEG_TO_RAD));
   cos_Lat_Pnt = cos((double)(Lat_Pnt * DEG_TO_RAD));
   sin_Delta_Long = sin((double)(Delta_Long * DEG_TO_RAD));
   cos_Delta_Long = cos((double)(Delta_Long * DEG_TO_RAD));

   Denom = 1.0 + sin_TP_Lat*sin_Lat_Pnt + cos_TP_Lat*cos_Lat_Pnt*cos_Delta_Long;

  // xrTP_Position(&X_Own, &Y_Own);
   *X = (double) (((EARTH_DIAMETER*cos_Lat_Pnt*sin_Delta_Long)/Denom));

   *Y = (double) ((((EARTH_DIAMETER*(cos_TP_Lat*sin_Lat_Pnt -
                   sin_TP_Lat*cos_Lat_Pnt*cos_Delta_Long))/Denom)));
}


void xXY_Brg_Rng(double X_1, double Y_1, double X_2, double Y_2, double *Bearing, double *Range)
{
  double  Rad_Bearing;
  double Rad_360 = (2 * PI);

  double y = (X_2 - X_1);
  double x = (Y_2 - Y_1);

  if (fabs(x)>0.00000001 && fabs(y)>0.00000001){
    Rad_Bearing = atan2(y, x);
  } else {
    Rad_Bearing = 0;
  }

  if (Rad_Bearing < 0) {
    Rad_Bearing += Rad_360;
  }
  *Bearing = (double)(RAD_TO_DEG * Rad_Bearing);
  *Range = (double) (_hypot((double) (X_2 - X_1), (double) (Y_2 - Y_1)));
}

void xBrg_Rng_XY(double X_RefPos, double Y_RefPos, double Bearing, double Range, double *X, double *Y)
{
  (void)X_RefPos;
  (void)Y_RefPos;	


double V = Bearing / RAD_TO_DEG;

	*X = (double) ( (sin(V) * (double) Range) + 0.5 );
	*Y = (double) ( (cos(V) * (double) Range) + 0.5 );

}




void xCrs_Spd_to_VxVy(double Crs, double Spd, double *Vx, double *Vy)
{
  double Crs_rad;
  double   Tmp_Vx, Tmp_Vy;

  Crs_rad = DEG_TO_RAD * (double) Crs;

  Tmp_Vx = (double) ( (sin(Crs_rad) * (double) Spd) + 0.5 );
  Tmp_Vy = (double) ( (cos(Crs_rad) * (double) Spd) + 0.5 );
  *Vx = Tmp_Vx;
  *Vy = Tmp_Vy;
}


void xVxVy_to_Crs_Spd(double Vx, double Vy, double *Crs, double *Spd)
{
  double Tmp_Spd;

  *Crs = ((double) ((RAD_TO_DEG * 0.5 * atan2((double) Vx, (double) Vy))+ 0.5)) * 0.5;

  Tmp_Spd = (double) (sqrt(((double) Vx * (double) Vx) + ((double) Vy * (double) Vy)) + 0.5);
  *Spd = Tmp_Spd;
}


void LL_to_BearRange(double Lat_TP, double Long_TP, double Lat_Pnt, double Long_Pnt, double *Bearing, double *Range)
{
	double pos_X, pos_Y;

	xLL_to_XY(Lat_TP, 
		Long_TP,
		Lat_Pnt, 
		Long_Pnt,
		&pos_X, &pos_Y);

	double bea;
	double ran;
	xXY_Brg_Rng(0,0,pos_X, pos_Y, &bea, &ran);
	*Bearing = bea;
	*Range = ran;
}
