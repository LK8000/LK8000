#include "ContestMgr.h"


CContestMgr::CContestMgr(unsigned handicap, unsigned startAltitudeLoss):
  _handicap(handicap),
  _trace(TRACE_FIX_LIMIT, 0, startAltitudeLoss, COMPRESSION_ALGORITHM),
  _traceSprint(TRACE_SPRINT_FIX_LIMIT, TRACE_SPRINT_TIME_LIMIT, 0, COMPRESSION_ALGORITHM)
{
  for(unsigned i=0; i<TYPE_NUM; i++)
    _solutionArray.push_back(CSolution());
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
  
  // create solution trace
  CTrace trace(rules.TPNum(), CTrace::ALGORITHM_DISTANCE, 0, false);
  
  // add points to solution trace
  point = _trace.Front();
  while(point != last) {
    trace.Push(new CTrace::CPoint(_trace, *point, trace._back));
    point = point->Next();
  }
  trace.Compress();

  // copy solution
  CPointGPSArray pointArray;
  point = trace.Front();
  double length = 0;
  
  while(point) {
    if(pointArray.size())
      length += point->GPS().Distance(pointArray.back());
    pointArray.push_back(point->GPS());
    point = point->Next();
  }
  
  _solutionArray[TYPE_OLC_CLASSIC] = CSolution(length, length * 100 / _handicap, pointArray);
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
