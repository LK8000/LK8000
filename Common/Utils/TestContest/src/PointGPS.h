/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __POINTGPS_H__
#define __POINTGPS_H__

#include "Tools.h"
#include "Utils.h"
#include <vector>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD					0.0174532925199432958
#define RAD_TO_DEG					57.2957795131
#endif

class CPointGPS {
  static const unsigned MAX_TIME_DELTA = 20 * 3600; // 20h
  static const unsigned DAY_SECONDS    = 24 * 3600; // 24h
  
  // data from GPS
  unsigned _time;
  unsigned _alt;
  double _lat;
  double _lon;
  
public:
  CPointGPS(unsigned time, double lat, double lon, unsigned alt):
    //_time(time), _lat(lat * DEG_TO_RAD), _lon(lon * DEG_TO_RAD), _alt(alt)
    _time(time), _alt(alt), _lat(lat), _lon(lon)
  {}
  
  unsigned Time() const      { return _time; }
  // double Latitude() const  { return _lat * RAD_TO_DEG; }
  // double Longitude() const { return _lon * RAD_TO_DEG; }
  double Latitude() const    { return _lat; }
  double Longitude() const   { return _lon; }
  unsigned Altitude() const  { return _alt; }
  
  unsigned Distance(const CPointGPS &ref) const;
  unsigned TimeDelta(const CPointGPS &ref) const;
};

typedef CSmartPtr<const CPointGPS> CPointGPSSmart;
typedef std::vector<CPointGPS> CPointGPSArray;


inline unsigned CPointGPS::Distance(const CPointGPS &ref) const
{
  double dist;
  DistanceBearing(ref._lat, ref._lon, _lat, _lon, &dist, 0);
  return dist;
}


// inline double CPointGPS::Distance(const CPointGPS &ref) const
// { 
//   using namespace std;
//   double clat1 = cos(_lat);
//   double clat2 = cos(ref._lat);
//   double dlon = ref._lon - _lon;
//   double s1 = sin((ref._lat - _lat) / 2);
//   double s2 = sin(dlon / 2);
//   double a = max(0.0,min(1.0,s1*s1+clat1*clat2*s2*s2));
//   return 6371000.0*2.0*atan2(sqrt(a),sqrt(1.0-a));
// }


inline unsigned CPointGPS::TimeDelta(const CPointGPS &ref) const
{
  unsigned delta = _time - ref._time;
  if(delta < 0) {
    if(delta < -(DAY_SECONDS - MAX_TIME_DELTA))
      delta += DAY_SECONDS;
  }
  return delta;
}


#endif /* __POINTGPS_H__ */
