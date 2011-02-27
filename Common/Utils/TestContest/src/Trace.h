/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __TRACE_H__
#define __TRACE_H__

#include "Tools.h"
#include "Utils.h"
#include <list>
#include <set>
#include <deque>
#include <cmath>
#include <iostream>
#include <limits>


class CTrace {
public:
  enum TAlgorithm {
    ALGORITHM_DISTANCE                = 0x0001,
    ALGORITHM_TRIANGLES               = 0x0002,
    
    ALGORITHM_INHERITED               = 0x0010,
    
    ALGORITHM_TIME_DELTA              = 0x0100
  };
  
  class CPoint;
  typedef std::deque<const CPoint *> CSolution;
  
private:
  /**
   * @brief A map of GPS points sorted from the most importand to the least important ones.
   * 
   */
  typedef std::set<CPoint *, CPtrCmp<CPoint *> > CPointCostSet;
  
  class CDijkstra;
  
  static unsigned _maxSize;
  static unsigned _algorithm;
  unsigned _size;
  unsigned _pointCount;
  CPointCostSet _pointCostSet;
  CPoint *_front;
  CPoint *_back;
  double _length;
  
  CTrace(const CTrace &);              /**< @brief Disallowed */
  CTrace &operator=(const CTrace &);   /**< @brief Disallowed */
  
  //  unsigned SolveIterate(const CPoint *pointArray[], unsigned idx, unsigned stage, CSolutionArray &solution);
  
public:
  CTrace(unsigned maxSize, unsigned algorithm);
  ~CTrace();
  
  unsigned Size() const { return _size; }
  unsigned PointCount() const { return _pointCount; }
  
  const CPoint *Front() const { return _front; }
  
  void Compress();
  void Push(double time, double lat, double lon, double alt);
  
  void DistanceVerify() const;
  
  double Solve(CSolution &solution);
  
  friend std::ostream &operator<<(std::ostream &stream, const CTrace &trace);
};


class CTrace::CPoint {
  friend class CTrace;
    
  static const unsigned AVG_TASK_TIME  =  3 * 3600; // 3h
  static const unsigned MAX_TIME_DELTA = 20 * 3600; // 20h
  static const unsigned DAY_SECONDS    = 24 * 3600; // 24h
  
  // data from GPS
  const double _time;
  const double _lat;
  const double _lon;
  const double _alt;
  
  // trace optimisation values
  double _prevDistance;
  double _inheritedCost;
  double _distanceCost;
  double _timeCost;
  
  // contest solving variables
  double _pathLength;
  const CPoint *_pathPrevious;
  
  // list iterators
  CPoint *_prev;
  CPoint *_next;
  
  CPoint(const CPoint &);              /**< @brief Disallowed */
  CPoint &operator=(const CPoint &);   /**< @brief Disallowed */
  
  void Reduce();
  void AssesCost();
  
public:
  CPoint(double time, double lat, double lon, double alt, CPoint *prev);
  ~CPoint();
    
  double Time() const      { return _time; }
  double Latitude() const  { return _lat; }
  double Longitude() const { return _lon; }
  double Altitude() const  { return _alt; }
  
  double Distance(const CPoint &ref) const;
  double TimeDelta(const CPoint &ref) const;
  
  CPoint *Next() const { return _next; }
  
  bool operator==(const CPoint &ref) const { return _time == ref._time; }
  bool operator<(const CPoint &ref) const;
  
  friend std::ostream &operator<<(std::ostream &stream, const CPoint &point);
};


class CTrace::CDijkstra {
  struct CNodeCmp {
    bool operator()(const CPoint *left, const CPoint *right) const
    {
      if(left->_pathLength > right->_pathLength)
        return true;
      if(left->_pathLength < right->_pathLength)
        return false;
      return left->_time < right->_time;
    }
  };
  
  typedef std::set<CPoint *, CNodeCmp> CNodeSet;
  CTrace &_trace;
  CNodeSet _nodeSet;
  
public:
  CDijkstra(CTrace &trace, CPoint &startPoint);
  double Solve(CSolution &solution);
};



inline double CTrace::CPoint::Distance(const CPoint &ref) const
{ 
  double dist;
  DistanceBearing(ref._lat, ref._lon, _lat, _lon, &dist, 0);
  return dist;
}


// inline double CTrace::CPoint::Distance(const CPoint &ref) const
// { 
//   double dx = fabs(ref._lon - _lon);
//   double dy = fabs(ref._lat - _lat);
//   return sqrt(dx*dx + dy*dy);
// }

inline double CTrace::CPoint::TimeDelta(const CPoint &ref) const
{
  double delta = _time - ref._time;
  if(delta < 0) {
    if(delta < -(DAY_SECONDS - MAX_TIME_DELTA))
      delta += DAY_SECONDS;
    else
      std::cerr << "Delta time calculation error: " << delta << std::endl;
  }
  return delta;
}


#endif /* __TRACE_H__ */
