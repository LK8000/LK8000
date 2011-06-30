/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
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
  static const unsigned MAX_TIME_DELTA = 16 * 3600; // 16h
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

typedef CSmartPtr<const CPointGPS> CPointGPSSmart;
typedef std::vector<CPointGPS> CPointGPSArray;


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
