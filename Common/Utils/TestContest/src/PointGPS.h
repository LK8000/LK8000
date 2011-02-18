/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __POINTGPS_H__
#define __POINTGPS_H__

#include "Utils.h"
#include <iosfwd>


class CPointGPS {
  double _time;
  double _lat;
  double _lon;
  double _alt;
public:
  CPointGPS(double time, double lat, double lon, double alt):
    _time(time), _lat(lat), _lon(lon), _alt(alt) {}
  double Time() const { return _time; }
  double Latitude() const { return _lat; }
  double Longitude() const { return _lon; }
  double Altitude() const { return _alt; }
  double Distance(const CPointGPS &ref) const
  { 
    double dist;
    DistanceBearing(ref._lat, ref._lon, _lat, _lon, &dist, 0);
    return dist;
  }
  double TimeDelta(const CPointGPS &ref) const { return _time - ref._time; }
  
  friend std::ostream &operator<<(std::ostream &stream, const CPointGPS &point);
};


#endif /* __POINTGPS_H__ */
