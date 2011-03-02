#include "ContestMgr.h"


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
  const CTrace::CPoint *point = _trace.Back();
  const CTrace::CPoint *last = 0;
  double startAltitude = _trace.Front()->GPS().Altitude();
  while(point) {
    if(point->GPS().Altitude() >= startAltitude - finishAltDiff) {
      last = point;
      break;
    }
    point = point->Previous();
  }
  
  // create result trace
  CTrace traceResult(rules.TPNum(), CTrace::ALGORITHM_DISTANCE, 0, false);
  
  // add points to result trace
  point = _trace.Front();
  while(point && point != last->Next()) {
    traceResult.Push(new CTrace::CPoint(traceResult, *point, traceResult._back));
    point = point->Next();
  }
  traceResult.Compress();

  // copy result
  CPointGPSArray pointArray;
  point = traceResult.Front();
  double length = 0;
  
  while(point) {
    if(pointArray.size())
      length += point->GPS().Distance(pointArray.back());
    pointArray.push_back(point->GPS());
    point = point->Next();
  }
  
  _resultArray[TYPE_OLC_CLASSIC] = CResult(length, length * 100 / _handicap, pointArray);
}


void CContestMgr::Add(const CPointGPSSmart &gps)
{
  _trace.Push(gps);
  _trace.Compress();
  
  _traceSprint.Push(gps);
  _traceSprint.Compress();
  
  if(_trace.Size()) {
    CRules rules(7, 0, 1000);
    UpdateOLCClassic(rules);
  }
}
