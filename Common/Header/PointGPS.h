/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: PointGPS.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __POINTGPS_H__
#define __POINTGPS_H__

#include "Point3D.h"

/** 
 * @brief GPS fix data
 * 
 * CPointGPS class stores all data obtained from GPS fix. It also
 * provides basic operations on GPS fixes.
 */
class CPointGPS : public CPoint3D {
  unsigned _time;
  
public:
  static const unsigned DAY_SECONDS  = 24 * 3600; // 24h
  
  CPointGPS(unsigned time, double lat, double lon, int alt):
    CPoint3D(lat, lon, alt), _time(time) {}
  unsigned Time() const      { return _time; }
  int TimeDelta(unsigned ref) const;
  int TimeDelta(const CPointGPS &ref) const { return TimeDelta(ref.Time()); }
  bool operator==(const CPointGPS &ref) const { return _time == ref._time; }
  bool operator<(const CPointGPS &ref) const  { return TimeDelta(ref) < 0; }
  bool operator>(const CPointGPS &ref) const  { return TimeDelta(ref) > 0; }
};

typedef std::shared_ptr<const CPointGPS> CPointGPSSmart;
typedef std::vector<CPointGPS> CPointGPSArray;

inline
CPointGPSSmart make_CPointGPSSmart(unsigned time, double lat, double lon, int alt) {
  return std::make_shared<CPointGPSSmart::element_type>(time, lat, lon, alt);
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
  return _time - ref;
}


#endif /* __POINTGPS_H__ */
