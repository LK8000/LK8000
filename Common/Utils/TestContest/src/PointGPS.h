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

class CPointGPS {
  static const unsigned MAX_TIME_DELTA = 12 * 3600; // 12h
  
  // data from GPS
  unsigned _time;
  short _alt;
  double _lat;
  double _lon;
  
public:
  static const unsigned DAY_SECONDS    = 24 * 3600; // 24h
  
  CPointGPS(unsigned time, double lat, double lon, short alt):
    _time(time), _alt(alt), _lat(lat), _lon(lon) {}
  
  unsigned Time() const      { return _time; }
  double Latitude() const    { return _lat; }
  double Longitude() const   { return _lon; }
  short Altitude() const     { return _alt; }
  
  unsigned Distance(double lat, double lon) const;
  unsigned Distance(const CPointGPS &ref) const;
  unsigned Distance(const CPointGPS &seg1, const CPointGPS &seg2) const;
  int TimeDelta(const CPointGPS &ref) const;
  
  bool operator==(const CPointGPS &ref) const { return _time == ref._time; }
  bool operator<(const CPointGPS &ref) const { return TimeDelta(ref) < 0; }
  bool operator>(const CPointGPS &ref) const { return TimeDelta(ref) > 0; }
};

typedef CSmartPtr<const CPointGPS> CPointGPSSmart;
typedef std::vector<CPointGPS> CPointGPSArray;


inline unsigned CPointGPS::Distance(double lat, double lon) const
{
  double dist;
  DistanceBearing(lat, lon, _lat, _lon, &dist, 0);
  return dist;
}


inline unsigned CPointGPS::Distance(const CPointGPS &ref) const
{
  return Distance(ref._lat, ref._lon);
}


inline unsigned CPointGPS::Distance(const CPointGPS &seg1, const CPointGPS &seg2) const
{
  double A = _lon - seg1._lon;
  double B = _lat - seg1._lat;
  double C = seg2._lon - seg1._lon;
  double D = seg2._lat - seg1._lat;
  
  double dot = A * C + B * D;
  double len_sq = C * C + D * D;
  double param = dot / len_sq;
  
  double lon, lat;
  
  if(param < 0) {
    lon = seg1._lon;
    lat = seg1._lat;
  }
  else if(param > 1) {
    lon = seg2._lon;
    lat = seg2._lat;
  }
  else {
    lon = seg1._lon + param * C;
    lat = seg1._lat + param * D;
  }
  
  return Distance(lat, lon);
}


inline int CPointGPS::TimeDelta(const CPointGPS &ref) const
{
  int delta = _time - ref._time;
  if(delta < 0) {
    if(delta < -(int)(DAY_SECONDS - MAX_TIME_DELTA))
      delta += DAY_SECONDS;
  }
  return delta;
}


#endif /* __POINTGPS_H__ */
