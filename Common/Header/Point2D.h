/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Point2D.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __POINT2D_H__
#define __POINT2D_H__

#include "SmartPtr.h"
#include <cmath>
#include <vector>

#ifndef DEG_TO_RAD
#define DEG_TO_RAD  (PI / 180)
#define RAD_TO_DEG  (180 / PI)
#endif

/** 
 * @brief 2D Point data
 * 
 * CPoint2D class stores the location of a geo point. It also provides
 * basic operations on those points.
 */
class CPoint2D {
  double _lat;
  double _lon;
  int _x;
  int _y;
  int _z;
  
public:
  //static const double EARTH_RADIUS  = 6371000.0;
  #define EARTH_RADIUS 6371000.0
  
  CPoint2D(double lat, double lon):
    _lat(lat), _lon(lon)
  {
    lat *= DEG_TO_RAD;
    lon *= DEG_TO_RAD;
    double clat = cos(lat);
    _x = static_cast<int>(-EARTH_RADIUS * clat * cos(lon));
    _y = static_cast<int>(EARTH_RADIUS * clat * sin(lon));
    _z = static_cast<int>(EARTH_RADIUS * sin(lat));
  }
  
  CPoint2D(unsigned x, unsigned y, unsigned z):
    _x(x), _y(y), _z(z)
  {
    
    _lat = asin(_z / EARTH_RADIUS) * RAD_TO_DEG;
    if(_y == 0)
      _lon = 0;
    else {
      _lon = atan(float(_x) / _y) * RAD_TO_DEG;
      _lon += (_y>=0) ? 90 : -90;
    }
  }
  
  double Latitude() const    { return _lat; }
  double Longitude() const   { return _lon; }
  
  unsigned Distance(double lat, double lon) const;
  unsigned Distance(const CPoint2D &ref) const;
  unsigned Distance(const CPoint2D &seg1, const CPoint2D &seg2) const;
  unsigned DistanceXYZ(int x, int y, int z) const;
  unsigned DistanceXYZ(const CPoint2D &ref) const;
  unsigned DistanceXYZ(const CPoint2D &seg1, const CPoint2D &seg2, int *nearest_x = 0, int *nearest_y = 0, int *nearest_z = 0) const;
};

typedef CSmartPtr<const CPoint2D> CPoint2DSmart;
typedef std::vector<CPoint2D> CPoint2DArray;


/** 
 * @brief Calculates the distance between current and a given point
 * 
 * @param lat Latitude of distant point
 * @param lon Longitude of distant point
 * 
 * @return Calculated distance
 */
inline unsigned CPoint2D::Distance(double lat, double lon) const
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
  
  return static_cast<unsigned>(6371000.0*2.0*atan2(sqrt(a),sqrt(1.0-a)));
}


/** 
 * @brief Calculates the distance between 2 points
 * 
 * @param ref Other point to use in calculations
 * 
 * @return Calculated distance
 */
inline unsigned CPoint2D::Distance(const CPoint2D &ref) const
{
  return Distance(ref._lat, ref._lon);
}


/** 
 * @brief Calculates approximated distance of point from a line segment
 * 
 * @param seg1 First end of line segment
 * @param seg2 Second end of line segment
 * 
 * @return Calculated distance
 */
inline unsigned CPoint2D::Distance(const CPoint2D &seg1, const CPoint2D &seg2) const
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


/** 
 * @brief Calculates the distance between current and a given point
 * 
 * @param x X coordinate of a distant point
 * @param y Y coordinate of a distant point
 * @param z Z coordinate of a distant point
 * 
 * @return Calculated distance
 */
inline unsigned CPoint2D::DistanceXYZ(int x, int y, int z) const
{
  double dx = _x - x;
  double dy = _y - y;
  double dz = _z - z;
  return static_cast<unsigned>(sqrt(dx*dx + dy*dy + dz*dz));
}


/** 
 * @brief Calculates the distance between 2 points
 * 
 * @param ref Other point to use in calculations
 * 
 * @return Calculated distance
 */
inline unsigned CPoint2D::DistanceXYZ(const CPoint2D &ref) const
{
  return DistanceXYZ(ref._x, ref._y, ref._z);
}


/** 
 * @brief Calculates distance of a point from a line segment
 * 
 * @param seg1 First end of line segment
 * @param seg2 Second end of line segment
 * @param nearest_x Nearest point's x on the line segment to return if not 0
 * @param nearest_y Nearest point's x on the line segment to return if not 0
 * @param nearest_z Nearest point's x on the line segment to return if not 0
 * 
 * @return Calculated distance
 */
inline unsigned CPoint2D::DistanceXYZ(const CPoint2D &seg1, const CPoint2D &seg2, int *nearest_x /* = 0 */, int *nearest_y /* = 0 */, int *nearest_z /* = 0 */) const
{
  int X1 = _x - seg1._x;
  int Y1 = _y - seg1._y;
  int Z1 = _z - seg1._z;
  double DX = seg2._x - seg1._x;
  double DY = seg2._y - seg1._y;
  double DZ = seg2._z - seg1._z;
  
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
    x = static_cast<int>(seg1._x + param * DX);
    y = static_cast<int>(seg1._y + param * DY);
    z = static_cast<int>(seg1._z + param * DZ);
  }
  
  if(nearest_x) *nearest_x = x;
  if(nearest_y) *nearest_y = y;
  if(nearest_z) *nearest_z = z;
  
  return DistanceXYZ(x, y, z);
}

#endif /* __POINT2D_H__ */
