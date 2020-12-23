/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: NavFunctions.cpp,v 8.2 2010/12/12 15:50:41 root Exp root $
*/

#include "externs.h"
#include "NavFunctions.h"
#include <cmath>

#ifdef _WGS84
#include <GeographicLib/Geodesic.hpp>
using GeographicLib::Geodesic;

const std::runtime_error err(std::string("")); // requiered for avoid link error with mingw 5.4.0
#endif

double StraightDistance(double lat1, double lon1, double lat2, double lon2)
{
double la = lat2 - lat1;
double lo = lon2 - lon1;

return sqrt( la*la + lo*lo);

}

void DistanceBearing(double lat1, double lon1, double lat2, double lon2,
                     double *Distance, double *Bearing) {

#ifdef _WGS84
  if(earth_model_wgs84) {
    double t;
    unsigned outmask = 0;
    const Geodesic& geod = Geodesic::WGS84();
    if(Distance) {
      outmask |= Geodesic::DISTANCE;
    } else {
      Distance = &t;
    }
    if(Bearing) {
      outmask |= Geodesic::AZIMUTH;
    } else {
      Bearing = &t;
    }
    geod.GenInverse(lat1, lon1, lat2, lon2, outmask, *Distance, *Bearing, t, t, t, t, t);
    if(outmask & Geodesic::AZIMUTH) {
        *Bearing = AngleLimit360(*Bearing);
    }
    return;
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
}


double DoubleDistance(double lat1, double lon1, double lat2, double lon2,
		      double lat3, double lon3) {
#ifdef _WGS84
  if(earth_model_wgs84) {
    const Geodesic& geod = Geodesic::WGS84();
    double s12, s23;
    geod.Inverse(lat1, lon1, lat2, lon2, s12);
    geod.Inverse(lat2, lon2, lat3, lon3, s23);

    return (s12 + s23);
  }
#endif
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
    assert(lat_out && lon_out);
#ifdef _WGS84
  if(earth_model_wgs84) {
    const Geodesic& geod = Geodesic::WGS84();
    geod.Direct(Lat, Lon, Bearing, Distance, *lat_out, *lon_out);
    return;
  }
#endif
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
  *Range = (double) (hypot((double) (X_2 - X_1), (double) (Y_2 - Y_1)));
}


/////////////////////////////////////////////////////////////////////////////////

// this is a slow function
// adapted from The Aviation Formulary 1.42

// finds the point along a distance dthis between p1 and p2, which are
// separated by dtotal
static void IntermediatePoint(double lon1, double lat1,
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
