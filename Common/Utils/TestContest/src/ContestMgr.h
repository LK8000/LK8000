/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __CONTESTMGR_H__
#define __CONTESTMGR_H__

#include "Trace.h"
#include <map>


class CContestMgr {
public:
  enum TType {
    TYPE_OLC_CLASSIC,
    TYPE_OLC_CLASSIC_PREDICTED,
    TYPE_OLC_FAI,
    TYPE_OLC_PLUS,
    TYPE_OLC_LEAGUE,
    TYPE_FAI_3_TPS,
    TYPE_FAI_3_TPS_PREDICTED,
    TYPE_NUM
  };
  
  class CResult {
    TType          _type;
    unsigned       _distance;
    float          _score;
    CPointGPSArray _pointArray;
  public:
    CResult(): _type(TYPE_NUM), _distance(0), _score(0) {}
    CResult(TType type, unsigned distance, float score, const CPointGPSArray &pointArray):
      _type(type), _distance(distance), _score(score), _pointArray(pointArray) {}
    CResult(TType type, const CResult &ref):
      _type(type), _distance(ref._distance), _score(ref._score), _pointArray(ref._pointArray) {}
    TType Type() const                       { return _type; }
    unsigned Distance() const                { return _distance; }
    float Score() const                      { return _score; }
    unsigned Duration() const                { return _pointArray.empty() ? 0 : (_pointArray.back().Time() - _pointArray.front().Time()); }
    float Speed() const                      { return _pointArray.empty() ? 0 : ((float)_distance / Duration()); }
    const CPointGPSArray &PointArray() const { return _pointArray; }
  };
  
  typedef std::vector<CResult> CResultArray;
  typedef std::multimap<unsigned, const CTrace::CPoint *> CDistanceMap;
  
private:
  const unsigned _handicap;
  CTrace _trace;
  CTrace _traceSprint;
  CTrace _traceLoop;
  CResultArray _resultArray;
  
  unsigned BiggestLoopFind(const CTrace &trace, const CTrace::CPoint *&start, const CTrace::CPoint *&end) const;
  bool BiggestLoopFind(const CTrace &traceIn, CTrace &traceOut) const;
  bool FAITriangleEdgeCheck(unsigned length, unsigned best) const;
  bool FAITriangleEdgeCheck(unsigned length1, unsigned length2, unsigned length3) const;
  void PointsResult(TType type, const CTrace &traceResult);
  void SolvePoints(const CTrace &trace, bool sprint, bool predicted);
  void SolveTriangle(const CTrace &trace, const CPointGPS *prevFront, const CPointGPS *prevBack);
  void SolveOLCPlus();
  
public:
  static const unsigned TRACE_FIX_LIMIT = 200;
  static const unsigned TRACE_TRIANGLE_FIX_LIMIT = 50;
  static const unsigned TRACE_SPRINT_FIX_LIMIT = 100;
  
  static const short TRACE_START_FINISH_ALT_DIFF = 1000;
  static const unsigned TRACE_TRIANGLE_MIN_TIME = 10 * 60;
  static const unsigned TRACE_CLOSED_MAX_DIST = 1000;
  static const unsigned TRACE_SPRINT_TIME_LIMIT = 150 * 60;
  static const unsigned TRACE_FAI_BIG_TRIANGLE_LENGTH = 500 * 1000;

  static unsigned COMPRESSION_ALGORITHM;
  
  static const char *TypeToString(TType type);
  
  CContestMgr(unsigned handicap, short startAltitudeLoss);
  
  const CTrace &Trace() const { return _trace; }
  const CTrace &TraceSprint() const { return _traceSprint; }
  
  void Add(const CPointGPSSmart &gps);
  const CResult &Result(TType type) const { return _resultArray[type]; }
};

#endif /* __CONTESTMGR_H__ */
