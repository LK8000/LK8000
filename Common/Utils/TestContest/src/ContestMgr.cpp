/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "ContestMgr.h"
#include <iostream>


CContestMgr::CContestMgr(unsigned handicap, unsigned startAltitudeLoss):
  _handicap(handicap),
  _trace(TRACE_FIX_LIMIT, 0, startAltitudeLoss, COMPRESSION_ALGORITHM),
  _traceSprint(TRACE_SPRINT_FIX_LIMIT, TRACE_SPRINT_TIME_LIMIT, 0, COMPRESSION_ALGORITHM)
{
  for(unsigned i=0; i<TYPE_NUM; i++)
    _resultArray.push_back(CResult());
}


void CContestMgr::UpdateOLCClassic(const CRules &rules)
{
  // find the last point meeting criteria
  double finishAltDiff = rules.FinishAltDiff();
  const CTrace::CPoint *point = rules.Trace().Back();
  const CTrace::CPoint *last = 0;
  double startAltitude = rules.Trace().Front()->GPS().Altitude();
  while(point) {
    if(point->GPS().Altitude() >= startAltitude - finishAltDiff) {
      last = point;
      break;
    }
    point = point->Previous();
  }
  
  // create result trace
  CTrace traceResult(rules.TPNum(), rules.TimeLimit(), 0, CTrace::ALGORITHM_DISTANCE);
  
  // add points to result trace
  point = rules.Trace().Front();
  while(point && point != last->Next()) {
    traceResult.Push(new CTrace::CPoint(traceResult, *point, traceResult._back));
    point = point->Next();
  }
  traceResult.Compress();

  // copy result
  CPointGPSArray pointArray;
  point = traceResult.Front();
  double distance = 0;
  
  while(point) {
    if(pointArray.size())
      distance += point->GPS().Distance(pointArray.back());
    pointArray.push_back(point->GPS());
    point = point->Next();
  }
  if(distance > _resultArray[rules.Type()].Distance()) {
    double result;
    if(rules.Type() == TYPE_OLC_LEAGUE)
      result = distance / 2.5 * 200 / (_handicap + 100);
    else
      result = distance * 100 / _handicap;
    _resultArray[rules.Type()] = CResult(rules.Type(), distance, result, pointArray);
  }
}


void CContestMgr::Add(const CPointGPSSmart &gps)
{
  _trace.Push(gps);
  _trace.Compress();
  
  _traceSprint.Push(gps);
  _traceSprint.Compress();
  
  if(_trace.Size())
    UpdateOLCClassic(CRules(TYPE_OLC_CLASSIC, _trace, 7, 0, 1000));
  
  if(_traceSprint.Size())
    UpdateOLCClassic(CRules(TYPE_OLC_LEAGUE, _traceSprint, 5, TRACE_SPRINT_TIME_LIMIT, 0));
}
