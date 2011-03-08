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

class CContestMgr; 

class CTrace {
public:
  enum TAlgorithm {
    ALGORITHM_DISTANCE                = 0x0001,
    ALGORITHM_TRIANGLES               = 0x0002,
    
    ALGORITHM_INHERITED               = 0x0010,
    
    ALGORITHM_TIME_DELTA              = 0x0100
  };
  
  class CPoint;
  
private:
  friend class CContestMgr; 

  /**
   * @brief A map of GPS points sorted from the most importand to the least important ones.
   * 
   */
  typedef std::set<CPoint *, CPtrCmp<CPoint *> > CPointCostSet;
  
  const unsigned _maxSize;
  const unsigned _timeLimit;
  const unsigned _startAltitudeLoss;
  const unsigned _algorithm;
  unsigned _size;
  unsigned _analyzedPointCount;
  CPointCostSet _compressionCostSet;
  CPoint *_front;
  CPoint *_back;
  
  bool _startDetected;
  unsigned _startMaxAltitude;
  
  CTrace(const CTrace &);              /**< @brief Disallowed */
  CTrace &operator=(const CTrace &);   /**< @brief Disallowed */

  void Push(CPoint *point);
  
public:
  CTrace(unsigned maxSize, unsigned timeLimit, unsigned startAltitudeLoss, unsigned algorithm);
  ~CTrace();
  
  void Push(const CPointGPSSmart &gps);
  void Compress();
  
  unsigned Size() const               { return _size; }
  unsigned AnalyzedPointCount() const { return _analyzedPointCount; }
  
  const CPoint *Front() const         { return _front; }
  const CPoint *Back() const          { return _back; }
  
  friend std::ostream &operator<<(std::ostream &stream, const CTrace &trace);
};


class CTrace::CPoint {
  friend class CTrace;
  friend class CTestContest;
  
  const CTrace &_trace;
  CPointGPSSmart _gps;
  
  // trace compression values
  unsigned _prevDistance;
  unsigned _inheritedCost;
  unsigned _distanceCost;
  unsigned _timeCost;
  
  // list iterators
  CPoint *_prev;
  CPoint *_next;
  
  CPoint(const CPoint &);              /**< @brief Disallowed */
  CPoint &operator=(const CPoint &);   /**< @brief Disallowed */
  
  void Reduce();
  void AssesCost();
  
public:
  CPoint(const CTrace &trace, const CPointGPSSmart &gps, CPoint *prev);
  CPoint(const CTrace &trace, const CPoint &ref, CPoint *prev);
  ~CPoint();
  
  const CPointGPS &GPS() const { return *_gps; }
  
  CPoint *Next() const { return _next; }
  CPoint *Previous() const { return _prev; }
  
  bool operator==(const CPoint &ref) const { return _gps->Time() == ref._gps->Time(); }
  bool operator<(const CPoint &ref) const;
  
  friend std::ostream &operator<<(std::ostream &stream, const CPoint &point);
};


inline bool CTrace::CPoint::operator<(const CPoint &ref) const
{
  unsigned leftCost = 0;
  unsigned rightCost = 0;
      
  leftCost += _distanceCost;
  rightCost += ref._distanceCost;
      
  if(_trace._algorithm & ALGORITHM_INHERITED) {
    leftCost += _inheritedCost;
    rightCost += ref._inheritedCost;
  }
  if(_trace._algorithm & ALGORITHM_TIME_DELTA) {
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
