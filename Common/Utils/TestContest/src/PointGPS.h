/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __POINTGPS_H__
#define __POINTGPS_H__

#include "Utils.h"


class CPointGPS {
  static const unsigned MAX_TIME_DELTA = 20 * 3600; // 20h
  static const unsigned DAY_SECONDS    = 24 * 3600; // 24h
  
  // data from GPS
  const double _time;
  const double _lat;
  const double _lon;
  const double _alt;
  
public:
  CPointGPS(double time, double lat, double lon, double alt):
    _time(time), _lat(lat), _lon(lon), _alt(alt) {}
  
  double Time() const      { return _time; }
  double Latitude() const  { return _lat; }
  double Longitude() const { return _lon; }
  double Altitude() const  { return _alt; }
  
  double Distance(const CPointGPS &ref) const;
  double TimeDelta(const CPointGPS &ref) const;
};



inline double CPointGPS::Distance(const CPointGPS &ref) const
{ 
  double dist;
  DistanceBearing(ref._lat, ref._lon, _lat, _lon, &dist, 0);
  return dist;
}


inline double CPointGPS::TimeDelta(const CPointGPS &ref) const
{
  double delta = _time - ref._time;
  if(delta < 0) {
    if(delta < -(DAY_SECONDS - MAX_TIME_DELTA))
      delta += DAY_SECONDS;
  }
  return delta;
}


#endif /* __POINTGPS_H__ */
