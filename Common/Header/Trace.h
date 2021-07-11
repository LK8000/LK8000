/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Trace.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __TRACE_H__
#define __TRACE_H__

#include "PointGPS.h"
#include <set>

class CContestMgr; 

/** 
 * @brief Pointers compare object
 */
template<class T>
struct CPtrCmp {
  bool operator()(const T &left, const T &right) const { return *left < *right; }
};


/** 
 * @brief GPS path trace
 * 
 * CTrace class is responsible for storing the best glider path representation
 * as possible. The trace have a defined maximum size. If more points are added
 * they may be compressed to the required size. During compression the least
 * important points are removed.
 */
class CTrace {
public:
  /**
   * @brief Compression algorithm knobs
   */
  enum TAlgorithm {
    ALGORITHM_DISTANCE        = 0x0001,           /**< @brief AB + BC - AC */
    // ALGORITHM_TRIANGLES       = 0x0002,           /**< @brief ABC triangle area */
    
    // ALGORITHM_INHERITED       = 0x0010,           /**< @brief Remebers the cost of GPS fixes removed during compression */
    
    ALGORITHM_TIME_DELTA      = 0x0100            /**< @brief Uses the time between GPS fixes */
  };
  
  class CPoint;
  
private:
  friend class CContestMgr; 

  /**
   * @brief A map of GPS points sorted from the most importand to the least important ones.
   * 
   */
  typedef std::set<CPoint *, CPtrCmp<CPoint *> > CPointCostSet;
  
  unsigned _maxSize;                              /**< @brief Maximum number of GPS fixes to store inside a trace */
  const unsigned _timeLimit;                      /**< @brief Maximum time period of a trace */
  const unsigned _algorithm;                      /**< @brief The compression algorithm of a trace */
  bool _valid;                                    /**< @brief Informs that a trace is invalid */
  unsigned _size;                                 /**< @brief Current number of fixes stored in a trace */
  unsigned _analyzedPointCount;                   /**< @brief The number of analysed GPS fixes */
  CPointCostSet _compressionCostSet;              /**< @brief The sorted set of GPS fixes */
  CPoint *_front;                                 /**< @brief The first GPS fix in a trace */
  CPoint *_back;                                  /**< @brief The last GPS fix in a trace */
  
  CTrace(const CTrace &);                         /**< @brief Disallowed */
  CTrace &operator=(const CTrace &);              /**< @brief Disallowed */

  void Push(CPoint *point);
  
public:
  CTrace(unsigned maxSize, unsigned timeLimit, unsigned algorithm);
  ~CTrace();
  
  void Clear();
  
  void Push(const CPointGPSSmart &gps);
  void Compress(unsigned maxSize = 0);
  
  unsigned Size() const               { return _size; }
  unsigned AnalyzedPointCount() const { return _analyzedPointCount; }
  
  const CPoint *Front() const         { return _front; }
  const CPoint *Back() const          { return _back; }
};


/**
 * @brief Trace point class
 * 
 * CTrace::CPoint class is responsible for storing GPS fix and trace compression
 * data.
 */
class CTrace::CPoint {
  friend class CTrace;
  friend class CTestContest;
  
  const CTrace &_trace;                           /**< @brief Parent trace */
  CPointGPSSmart _gps;                            /**< @brief Contained GPS fix */
  
  // trace compression values
  unsigned _prevDistance;                         /**< @brief The distance from the previous GPS fix */
  //  float _inheritedCost;                           /**< @brief The cost inherited from compressed (removed) GPS fixes */
  unsigned _distanceCost;                         /**< @brief The distance related compression cost */
  unsigned _timeCost;                             /**< @brief Time related compression cost */
  
  // list iterators
  CPoint *_prev;                                  /**< @brief Previous point in time domain */
  CPoint *_next;                                  /**< @brief Next point in time domain */
  
  CPoint(const CPoint &);                         /**< @brief Disallowed */
  CPoint &operator=(const CPoint &);              /**< @brief Disallowed */
  
  void Reduce();
  void AssesCost();
  
public:
  CPoint(const CTrace &trace, const CPointGPSSmart &gps, CPoint *prev);
  CPoint(const CTrace &trace, const CPoint &ref, CPoint *prev);
  ~CPoint();

  const CPointGPS &GPS() const { return *_gps; }
  
  CPoint *Next() const         { return _next; }
  CPoint *Previous() const     { return _prev; }
  
  bool operator==(const CPoint &ref) const { return _gps->Time() == ref._gps->Time(); }
  bool operator<(const CPoint &ref) const;
};


/** 
 * @brief Adds a new GPS fix to a trace
 * 
 * @param gps GPS fix to add
 */
inline void CTrace::Push(const CPointGPSSmart &gps)
{
  // add new point
  Push(new CPoint(*this, gps, _back));
}



/** 
 * @brief Constructor
 * 
 * @param trace Parent trace
 * @param gps GPS fix to contain
 * @param prev Previous trace point
 */
inline CTrace::CPoint::CPoint(const CTrace &trace, const CPointGPSSmart &gps, CPoint *prev):
  _trace(trace), 
  _gps(gps),
  _prevDistance(prev ? prev->_gps->DistanceXYZ(*this->_gps) : 0),
  //  _inheritedCost(0),
  _distanceCost(0), _timeCost(0),
  _prev(prev), _next(0)
{
  if(_prev) {
    _prev->_next = this;
    if(_prev->_prev)
      _prev->AssesCost();
  }
}


/** 
 * @brief "Copy" constructor
 * 
 * @param trace Parent trace
 * @param ref Point to copy data from
 * @param prev Previous trace point
 */
inline CTrace::CPoint::CPoint(const CTrace &trace, const CPoint &ref, CPoint *prev):
  _trace(trace), 
  _gps(ref._gps),
  _prevDistance(ref._prevDistance),
  // _inheritedCost(ref._inheritedCost),
  _distanceCost(ref._distanceCost), _timeCost(ref._timeCost),
  _prev(prev), _next(0)
{
  if(_prev) {
    _prev->_next = this;
    if(_prev->_prev)
      _prev->AssesCost();
  }
}


/** 
 * @brief Destructor
 */
inline CTrace::CPoint::~CPoint()
{
  if(_prev)
    _prev->_next = _next;
  if(_next)
    _next->_prev = _prev;
}


/** 
 * @brief Prepares the point for removal from the trace
 * 
 * Updates the neighbors with the data of current point.
 */
inline void CTrace::CPoint::Reduce()
{
  // asses new costs & set new prevDistance for next point
  // float distanceCost;
  // if(_trace._algorithm & ALGORITHM_TRIANGLES) {
  //   unsigned newDistance = _next->_gps->Distance(*_prev->_gps);
  //   distanceCost = _prevDistance + _next->_prevDistance - newDistance;
  //   _next->_prevDistance = newDistance;
  // }
  // else {
  _next->_prevDistance = std::max(0, (int)(_prevDistance + _next->_prevDistance - _distanceCost));
    //    distanceCost = _distanceCost; 
  // }
  
  // if(_trace._algorithm & ALGORITHM_INHERITED) {
  //   float cost = (distanceCost + _inheritedCost) / 2.0;
  //   _prev->_inheritedCost += cost;
  //   _next->_inheritedCost += cost;
  // }
}


/** 
 * @brief Assesses the compression cost of a point
 */
inline void CTrace::CPoint::AssesCost()
{
  // if(_trace._algorithm & ALGORITHM_TRIANGLES) {
  //   double ax = _gps->Longitude();           double ay = _gps->Latitude();
  //   double bx = _prev->_gps->Longitude();    double by = _prev->_gps->Latitude();
  //   double cx = _next->_gps->Longitude();    double cy = _next->_gps->Latitude();
  //   _distanceCost = fabs(ax*(by-cy) + bx*(cy-ay) + cx*(ay-by));
  // }
  // else {
  _distanceCost = std::max(0, (int)(_prevDistance + _next->_prevDistance - _next->_gps->DistanceXYZ(*_prev->_gps)));
  // }
  if(_trace._algorithm & ALGORITHM_TIME_DELTA)
    _timeCost = _gps->TimeDelta(*_prev->_gps);
}


/** 
 * @brief Main compression routine
 * 
 * Main compression routine used in compression set. It checks if current point
 * is more or less important than the reference point.
 * 
 * @param ref The point to compare
 * 
 * @return @c true if current GPS point is less important than reference
 */
inline bool CTrace::CPoint::operator<(const CPoint &ref) const
{
  unsigned leftCost = 0;
  unsigned rightCost = 0;
      
  leftCost += _distanceCost;
  rightCost += ref._distanceCost;
      
  // if(_trace._algorithm & ALGORITHM_INHERITED) {
  //   leftCost += _inheritedCost;
  //   rightCost += ref._inheritedCost;
  // }
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
