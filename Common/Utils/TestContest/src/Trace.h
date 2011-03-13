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
    ALGORITHM_TRIANGLES       = 0x0002,           /**< @brief ABC triangle area */
    
    ALGORITHM_INHERITED       = 0x0010,           /**< @brief Remebers the cost of GPS fixes removed during compression */
    
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
  const short _startAltitudeLoss;                 /**< @brief The loss of altitude needed to detect the start of powerless flight */
  const unsigned _algorithm;                      /**< @brief The compression algorithm of a trace */
  unsigned _size;                                 /**< @brief Current number of fixes stored in a trace */
  unsigned _analyzedPointCount;                   /**< @brief The number of analysed GPS fixes */
  CPointCostSet _compressionCostSet;              /**< @brief The sorted set of GPS fixes */
  CPoint *_front;                                 /**< @brief The first GPS fix in a trace */
  CPoint *_back;                                  /**< @brief The last GPS fix in a trace */
  
  bool _startDetected;                            /**< @brief @c true if a start of powerless flight was detected */
  short _startMaxAltitude;                        /**< @brief The maximum altitude of a takeoff */
  
  CTrace(const CTrace &);                         /**< @brief Disallowed */
  CTrace &operator=(const CTrace &);              /**< @brief Disallowed */

  void Push(CPoint *point);
  
public:
  CTrace(unsigned maxSize, unsigned timeLimit, short startAltitudeLoss, unsigned algorithm);
  ~CTrace();
  
  void Clear();
  
  void Push(const CPointGPSSmart &gps);
  void Compress(unsigned maxSize = 0);
  
  unsigned Size() const               { return _size; }
  unsigned AnalyzedPointCount() const { return _analyzedPointCount; }
  
  const CPoint *Front() const         { return _front; }
  const CPoint *Back() const          { return _back; }
  
  friend std::ostream &operator<<(std::ostream &stream, const CTrace &trace);
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
  float _prevDistance;                            /**< @brief The distance from the previous GPS fix */
  float _inheritedCost;                           /**< @brief The cost inherited from compressed (removed) GPS fixes */
  float _distanceCost;                            /**< @brief The ditance related compression cost */
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
  
  friend std::ostream &operator<<(std::ostream &stream, const CPoint &point);
};


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
  float leftCost = 0;
  float rightCost = 0;
      
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
