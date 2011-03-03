/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __COMPETITIONMGR_H__
#define __COMPETITIONMGR_H__

#include "Trace.h"


class CContestMgr {
public:
  enum TType {
    TYPE_OLC_CLASSIC,
    TYPE_OLC_FAI,
    TYPE_OLC_LEAGUE,
    TYPE_NUM
  };
  
  class CRules {
    const TType _type;
    const CTrace &_trace;
    const unsigned _tpNum;
    const unsigned _timeLimit;
    const double _finishAltDiff;
  public:
    CRules(TType type, const CTrace &trace, unsigned tpNum, unsigned timeLimit, double finishAltDiff):
      _type(type), _trace(trace), _tpNum(tpNum), _timeLimit(timeLimit), _finishAltDiff(finishAltDiff) {}
    TType Type() const           { return _type; }
    const CTrace &Trace() const  { return _trace; }
    unsigned TPNum() const       { return _tpNum; }
    unsigned TimeLimit() const   { return _timeLimit; }
    double FinishAltDiff() const { return _finishAltDiff; }
  };
  
  
  class CResult {
    TType          _type;
    double         _distance;
    double         _score;
    CPointGPSArray _pointArray;
  public:
    CResult(): _distance(0), _score(0) {}
    CResult(TType type, double distance, double score, const CPointGPSArray &pointArray):
      _type(type), _distance(distance), _score(score), _pointArray(pointArray) {}
    TType Type() const                       { return _type; }
    double Distance() const                  { return _distance; }
    double Score() const                     { return _score / 1000; }
    double Duration() const                  { return _pointArray.empty() ? 0 : (_pointArray.back().Time() - _pointArray.front().Time()); }
    double Speed() const                     { return _pointArray.size() ? (_distance / Duration()) : 0; }
    const CPointGPSArray &PointArray() const { return _pointArray; }
  };
  
  typedef std::vector<CResult> CResultArray;
  
private:
  const unsigned _handicap;
  CTrace _trace;
  CTrace _traceSprint;
  CResultArray _resultArray;
  
  void UpdateOLCClassic(const CRules &rules);
  
public:
  static const unsigned TRACE_FIX_LIMIT = 250;
  static const unsigned TRACE_SPRINT_FIX_LIMIT = 100;
  static const unsigned TRACE_SPRINT_TIME_LIMIT = 150 * 60;
  static unsigned COMPRESSION_ALGORITHM;
  
  static const char *TypeToString(TType type);
  
  CContestMgr(unsigned handicap, unsigned startAltitudeLoss);
  
  const CTrace &Trace() const { return _trace; }
  const CTrace &TraceSprint() const { return _traceSprint; }
  
  void Add(const CPointGPSSmart &gps);
  const CResult &Result(TType type) const { return _resultArray[type]; }
};

#endif /* __CONTESTMGR_H__ */
