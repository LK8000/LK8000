/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __TRACE_H__
#define __TRACE_H__

#include "PointGPS.h"
#include "Tools.h"
#include <set>
#include <vector>
//#include <cmath>
//#include <iostream>


class CPointGPS;

class CTrace {
public:
  enum TAlgorithm {
    ALGORITHM_DISTANCE                = 0x0001,
    ALGORITHM_TRIANGLES               = 0x0002,
    
    ALGORITHM_INHERITED               = 0x0010,
    
    ALGORITHM_TIME_DELTA              = 0x0100
  };
  
  class CPoint;
  typedef std::vector<const CPointGPS *> CSolution;
  
private:
  /**
   * @brief A map of GPS points sorted from the most importand to the least important ones.
   * 
   */
  typedef std::set<CPoint *, CPtrCmp<CPoint *> > CPointCostSet;
  
  static unsigned _maxSize;
  static unsigned _algorithm;
  const bool _gpsPointsOwner;
  const unsigned _startHeightLoss;
  unsigned _size;
  unsigned _analyzedPointCount;
  CPointCostSet _compressionCostSet;
  CPoint *_front;
  CPoint *_back;
  
  bool _startDetected;
  double _startAltitude;
  
  CTrace(const CTrace &);              /**< @brief Disallowed */
  CTrace &operator=(const CTrace &);   /**< @brief Disallowed */

  void Push(CPoint *gps);
  
public:
  CTrace(unsigned maxSize, unsigned algorithm, unsigned startHeightLoss = 100, bool gpsPointsOwner = true);
  ~CTrace();
  
  void Push(double time, double lat, double lon, double alt);
  void Compress();
  
  unsigned Size() const               { return _size; }
  unsigned AnalyzedPointCount() const { return _analyzedPointCount; }
  
  const CPoint *Front() const         { return _front; }
  
  double Solve(CSolution &solution);
  
  friend std::ostream &operator<<(std::ostream &stream, const CTrace &trace);
};


class CTrace::CPoint {
  friend class CTrace;
  friend class CTestContest;
  
  const CPointGPS * const _gps;
  
  // trace compression values
  double _prevDistance;
  double _inheritedCost;
  double _distanceCost;
  double _timeCost;
  
  // list iterators
  CPoint *_prev;
  CPoint *_next;
  
  CPoint(const CPoint &);              /**< @brief Disallowed */
  CPoint &operator=(const CPoint &);   /**< @brief Disallowed */
  
  void Reduce();
  void AssesCost();
  
public:
  CPoint(const CPointGPS *pointGPS, CPoint *prev);
  CPoint(const CPoint &ref, CPoint *prev);
  ~CPoint();
  
  CPoint *Next() const { return _next; }
  
  bool operator==(const CPoint &ref) const { return _gps->Time() == ref._gps->Time(); }
  bool operator<(const CPoint &ref) const;
  
  friend std::ostream &operator<<(std::ostream &stream, const CPoint &point);
};


inline bool CTrace::CPoint::operator<(const CPoint &ref) const
{
  double leftCost = 0;
  double rightCost = 0;
      
  leftCost += _distanceCost;
  rightCost += ref._distanceCost;
      
  if(CTrace::_algorithm & ALGORITHM_INHERITED) {
    leftCost += _inheritedCost;
    rightCost += ref._inheritedCost;
  }
  if(CTrace::_algorithm & ALGORITHM_TIME_DELTA) {
    leftCost *= _timeCost;
    rightCost *= ref._timeCost;
  }
      
  if(leftCost > rightCost)
    return false;
  else if(leftCost < rightCost)
    return true;
  else if(_timeCost > ref._timeCost)
    return false;
  else if(_timeCost < ref._timeCost)
    return true;
  else
    return _gps->Time() > ref._gps->Time();
}


#endif /* __TRACE_H__ */
