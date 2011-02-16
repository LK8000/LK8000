/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __TRACE_H__
#define __TRACE_H__

#include "PointGPS.h"
#include <list>
#include <map>
#include <cmath>


class CTrace {
public:
  class CPointCost {
    double _areaCost;
    double _timeCost;
    const CPointGPS * const _ptr;  // needed to make map key unique
  public:
    static double Area(const CPointGPS &current, const CPointGPS &prev, const CPointGPS &next)
    {
      double ax = current.Longitude(); double ay = current.Latitude();
      double bx = prev.Longitude();    double by = prev.Latitude();
      double cx = next.Longitude();    double cy = next.Latitude();
      return fabs(ax*(by-cy) + bx*(cy-ay) + cx*(ay-by));
    }
    
    CPointCost(const CPointGPS &current, const CPointGPS &prev, const CPointGPS &next):
      _areaCost(Area(current, prev, next)),
      _timeCost(next.TimeDelta(current) + current.TimeDelta(prev)),
      _ptr(&current)
    {
    }
    
    bool operator==(const CPointCost &ref) const { return _ptr == ref._ptr; }
    
    bool operator<(const CPointCost &ref) const
    {
      if(_areaCost > ref._areaCost)
        return false;
      else if(_areaCost < ref._areaCost)
        return true;
      else if(_timeCost > ref._timeCost)
        return false;
      else if(_timeCost < ref._timeCost)
        return true;
      else
        return _ptr < ref._ptr;
    }
    
    friend std::ostream &operator<<(std::ostream &stream, const CPointCost &cost);
  };
  typedef std::list<const CPointGPS *> CGPSPointList;
  
private:
  /**
   * @brief A map of GPS points sorted from the most importand to the least important ones.
   * 
   */
  typedef std::map<CPointCost, CGPSPointList::iterator> CGPSPointCostMap;
  
  const unsigned _maxSize;
  unsigned _pointCount;
  CGPSPointList _pointList;
  CGPSPointCostMap _pointCostMap;
  
public:
  CTrace(unsigned maxSize);
  ~CTrace();

  const CGPSPointList &List() const { return _pointList; }
  
  unsigned Size() const { return _pointList.size(); }
  unsigned PointCount() const { return _pointCount; }
  
  void Push(const CPointGPS *point);
  
  friend std::ostream &operator<<(std::ostream &stream, const CTrace &trace);
};


#endif /* __TRACE_H__ */
