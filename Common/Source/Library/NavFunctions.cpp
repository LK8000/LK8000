/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: NavFunctions.cpp,v 8.2 2010/12/12 15:50:41 root Exp root $
*/

#include "externs.h"
#include "NavFunctions.h"
#include <cmath>


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
  double ResultLat;
  double ResultLon;

  Lat *= DEG_TO_RAD;
  Lon *= DEG_TO_RAD;
  Bearing *= DEG_TO_RAD;
  Distance = Distance/6371000;

  const double sinDistance = sin(Distance);
  const double cosDistance = cos(Distance);

  const double cosLat = cos(Lat);
  const double sinLat = sin(Lat);

  ResultLat = asin(sinLat*cosDistance+cosLat*sinDistance*cos(Bearing)); 
  if (lat_out) {
    *lat_out = ResultLat*RAD_TO_DEG;
  }
  if (lon_out) {
    if(cosLat==0)
      ResultLon = Lon;
    else {
      ResultLon = atan2(sin(Bearing)*sinDistance*cosLat,cosDistance-sinLat*sin(ResultLat));
      ResultLon = fmod((Lon+ResultLon+M_PI),(M_2PI)) - M_PI;
    }
    *lon_out = ResultLon*RAD_TO_DEG;
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
	 LKASSERT(Sin_Lat_TP!= -1);
	 LKASSERT(Sin_Lat_TP!= 1);
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

  if (fabs(x)>0.00000001 || fabs(y)>0.00000001){
    Rad_Bearing = atan2(y, x);
  } else {
    Rad_Bearing = 0;
  }

  if (Rad_Bearing < 0) {
    Rad_Bearing += Rad_360;
  }
  *Bearing = (double)(RAD_TO_DEG * Rad_Bearing);
  *Range = (double) (std::hypot((double) (X_2 - X_1), (double) (Y_2 - Y_1)));
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



#if 0 // Currently duplicated and inlined inside LKAirspaces
bool CheckInsideLongitude(double longitude,
                         const double lon_min, const double lon_max) {
  if (lon_min<=lon_max) {
    // normal case
    return ((longitude>lon_min) && (longitude<lon_max));
  } else {
    // area goes across 180 degree boundary, so lon_min is +ve, lon_max is -ve (flipped)
    return ((longitude>lon_min) || (longitude<lon_max));
  }
}
#endif


/////////////////////////////////////////////////////////////////////////////////

// this is a slow function
// adapted from The Aviation Formulary 1.42

// finds the point along a distance dthis between p1 and p2, which are
// separated by dtotal
void IntermediatePoint(double lon1, double lat1,
		       double lon2, double lat2,
		       double dthis,
		       double dtotal,
		       double *lon3, double *lat3) {
  double A, B, x, y, z, d, f;
  /*
  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  */

  LKASSERT(lat3 != NULL);
  LKASSERT(lon3 != NULL);

  if ((lon1 == lon2) && (lat1 == lat2)){
    *lat3 = lat1;
    *lon3 = lon1;
    return;
  }

  if (dtotal>0) {
    f = dthis/dtotal;
    d = dtotal;
  } else {
    d = 1.0e-7;
    f = 0.0;
  }
  f = min(1.0,max(0.0,f));

  double coslat1 = cos(lat1);
  double coslat2 = cos(lat2);

  A=sin((1-f)*d)/sin(d);
  B=sin(f*d)/sin(d);
  x = A*coslat1*cos(lon1) +  B*coslat2*cos(lon2);
  y = A*coslat1*sin(lon1) +  B*coslat2*sin(lon2);
  z = A*sin(lat1)           +  B*sin(lat2);
  *lat3=atan2(z,sqrt(x*x+y*y))*RAD_TO_DEG;
  *lon3=atan2(y,x)*RAD_TO_DEG;
}

// finds cross track error in meters and closest point p4 between p3 and
// desired track p1-p2.
// very slow function!
double CrossTrackError(double lon1, double lat1,
                       double lon2, double lat2,
                       double lon3, double lat3,
                       double *lon4, double *lat4) {

  double dist_AD, crs_AD;
  DistanceBearing(lat1, lon1, lat3, lon3, &dist_AD, &crs_AD);
  dist_AD/= (RAD_TO_DEG * 111194.9267); crs_AD*= DEG_TO_RAD;

  double dist_AB, crs_AB;
  DistanceBearing(lat1, lon1, lat2, lon2, &dist_AB, &crs_AB);
  dist_AB/= (RAD_TO_DEG * 111194.9267); crs_AB*= DEG_TO_RAD;

  lat1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lat3 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  lon3 *= DEG_TO_RAD;

  double XTD; // cross track distance
  double ATD; // along track distance
  //  The "along track distance", ATD, the distance from A along the
  //  course towards B to the point abeam D

  double sindist_AD = sin(dist_AD);

  XTD = asin(sindist_AD*sin(crs_AD-crs_AB));

  double sinXTD = sin(XTD);
  ATD = asin(sqrt( sindist_AD*sindist_AD - sinXTD*sinXTD )/cos(XTD));
  
  if (lon4 && lat4) {
    IntermediatePoint(lon1, lat1, lon2, lat2, ATD, dist_AB,
		      lon4, lat4);
  }

  // units
  XTD *= (RAD_TO_DEG * 111194.9267);

  return XTD;
}


void ScreenClosestPoint(const POINT &p1, const POINT &p2, 
			const POINT &p3, POINT *p4, int offset) {

  int v12x, v12y, v13x, v13y;

  v12x = p2.x-p1.x; v12y = p2.y-p1.y;
  v13x = p3.x-p1.x; v13y = p3.y-p1.y;

  int mag12 = isqrt4(v12x*v12x+v12y*v12y);
  if (mag12>1) {
    // projection of v13 along v12 = v12.v13/|v12|
    int proj = (v12x*v13x+v12y*v13y)/mag12;
    // fractional distance
    double f;
    if (offset>0) {
      if (offset*2<mag12) {
	proj = max(0, min(proj, mag12));
	proj = max(offset, min(mag12-offset, proj+offset));
      } else {
	proj = mag12/2;
      }
    } 
    f = min(1.0,max(0.0,(double)proj/mag12));

    // location of 'closest' point 
    p4->x = lround(v12x*f)+p1.x;
    p4->y = lround(v12y*f)+p1.y;
  } else {
    p4->x = p1.x;
    p4->y = p1.y;
  }
}


// Calculates projected distance from P3 along line P1-P2
// The cross track error (xtd) is also available (for TaskStatistics.cpp),
// since xtd is calculated anyway in this function calling CrossTrackError()
// crs is the true course to P2 from the intermediate point P4 on the great circle route
double ProjectedDistance(double lon1, double lat1,
                         double lon2, double lat2,
                         double lon3, double lat3,
                         double *xtd, double *crs) {
  double lon4, lat4;
  double tmpd;

  tmpd = CrossTrackError(lon1, lat1,
                  lon2, lat2,
                  lon3, lat3,
                   &lon4, &lat4);

  if (xtd) *xtd= tmpd;

  DistanceBearing(lat1, lon1, lat4, lon4, &tmpd, crs); // null crs managed by distancebearing
  return tmpd;
}



void LatLon2Flat(double lon, double lat, int *scx, int *scy) {
  *scx = (int)(lon*fastcosine(lat)*100);
  *scy = (int)(lat*100);
}
