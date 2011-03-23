/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __POINTGPS_H__
#define __POINTGPS_H__

#include "SmartPtr.h"
#include "Utils.h"
#include <vector>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD .0174532925199432958
#endif

/** 
 * @brief GPS fix data
 * 
 * CPointGPS class stores all data obtained from GPS fix. It also
 * provides basic operations on GPS fixes.
 */
class CPointGPS {
  static const unsigned MAX_TIME_DELTA = 16 * 3600; // 16h
  
  // data from GPS
  unsigned _time;
  int _alt;
  double _lat;
  double _lon;
  int _x;
  int _y;
  int _z;
  
public:
  static const unsigned DAY_SECONDS  = 24 * 3600; // 24h
  static const double EARTH_RADIUS   = 6371000.0;
  
  CPointGPS(unsigned time, double lat, double lon, int alt):
    _time(time), _alt(alt), _lat(lat), _lon(lon)
  {
    lat *= DEG_TO_RAD;
    lon *= DEG_TO_RAD;
    double clat = cos(lat);
    _x = -EARTH_RADIUS * clat * cos(lon);
    _y = EARTH_RADIUS * sin(lat);
    _z = EARTH_RADIUS * clat * sin(lon);
  }
  
  unsigned Time() const      { return _time; }
  double Latitude() const    { return _lat; }
  double Longitude() const   { return _lon; }
  int Altitude() const       { return _alt; }
  
  unsigned Distance(double lat, double lon) const;
  unsigned Distance(const CPointGPS &ref) const;
  unsigned Distance(const CPointGPS &seg1, const CPointGPS &seg2) const;
  unsigned Distance3D(int x, int y, int z) const;
  unsigned Distance3D(const CPointGPS &ref) const;
  unsigned Distance3D(const CPointGPS &seg1, const CPointGPS &seg2) const;
  int TimeDelta(unsigned ref) const;
  int TimeDelta(const CPointGPS &ref) const { return TimeDelta(ref.Time()); }
  
  bool operator==(const CPointGPS &ref) const { return _time == ref._time; }
  bool operator<(const CPointGPS &ref) const  { return TimeDelta(ref) < 0; }
  bool operator>(const CPointGPS &ref) const  { return TimeDelta(ref) > 0; }
};

typedef CSmartPtr<const CPointGPS> CPointGPSSmart;
typedef std::vector<CPointGPS> CPointGPSArray;


/** 
 * @brief Calculates the distance between GPS fix and a given point
 * 
 * @param lat Latitude of distant point
 * @param lon Longitude of distant point
 * 
 * @return Calculated distance
 */
inline unsigned CPointGPS::Distance(double lat, double lon) const
{
  lat *= DEG_TO_RAD;
  double lat2 = _lat * DEG_TO_RAD;
  lon *= DEG_TO_RAD;
  double lon2 = _lon * DEG_TO_RAD;
  
  double clat1 = cos(lat);
  double clat2 = cos(lat2);
  double dlon = lon2-lon;
  
  double s1 = sin((lat2-lat)/2);
  double s2 = sin(dlon/2);
  double a= std::max(0.0, std::min(1.0,s1*s1+clat1*clat2*s2*s2));
  
  return 6371000.0*2.0*atan2(sqrt(a),sqrt(1.0-a));
}


/** 
 * @brief Calculates the distance between 2 GPS fixes
 * 
 * @param ref Other GPS fix to use in calculations
 * 
 * @return Calculated distance
 */
inline unsigned CPointGPS::Distance(const CPointGPS &ref) const
{
  return Distance(ref._lat, ref._lon);
}


/** 
 * @brief Calculates approximated distance of GPS fix from a line segment
 * 
 * @param seg1 First end of line segment
 * @param seg2 Second end of line segment
 * 
 * @return Calculated distance
 */
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


inline unsigned CPointGPS::Distance3D(int x, int y, int z) const
{
  double dx = _x - x;
  double dy = _y - y;
  double dz = _z - z;
  return sqrt(dx*dx + dy*dy + dz*dz);
}


/** 
 * @brief Calculates the distance between 2 GPS fixes
 * 
 * @param ref Other GPS fix to use in calculations
 * 
 * @return Calculated distance
 */
inline unsigned CPointGPS::Distance3D(const CPointGPS &ref) const
{
  return Distance3D(ref._x, ref._y, ref._z);
}


/** 
 * @brief Calculates approximated distance of GPS fix from a line segment
 * 
 * @param seg1 First end of line segment
 * @param seg2 Second end of line segment
 * 
 * @return Calculated distance
 */
inline unsigned CPointGPS::Distance3D(const CPointGPS &seg1, const CPointGPS &seg2) const
{
  int X1 = _x - seg1._x;
  int Y1 = _y - seg1._y;
  int Z1 = _z - seg1._z;
  int DX = seg2._x - seg1._x;
  int DY = seg2._y - seg1._y;
  int DZ = seg2._z - seg1._z;
  
  double dot = X1*DX + Y1*DY + Z1*DZ;
  double len_sq = DX*DX + DY*DY + DZ*DZ;
  double param = dot / len_sq;
  
  int x, y, z;
  
  if(param < 0) {
    x = seg1._x;
    y = seg1._y;
    z = seg1._z;
  }
  else if(param > 1) {
    x = seg2._x;
    y = seg2._y;
    z = seg2._z;
  }
  else {
    x = seg1._x + param * DX;
    y = seg1._y + param * DY;
    z = seg1._z + param * DZ;
  }
  
  return Distance3D(x, y, z);
}


/** 
 * @brief Calculates time difference between 2 GPS fixes
 * 
 * @param ref The second time to use
 * 
 * @return Calculated time difference
 */
inline int CPointGPS::TimeDelta(unsigned ref) const
{
  int delta = _time - ref;
  if(delta < 0) {
    if(delta < -(int)(DAY_SECONDS - MAX_TIME_DELTA))
      delta += DAY_SECONDS;
  }
  return delta;
}


#endif /* __POINTGPS_H__ */
