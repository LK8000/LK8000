/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "ContestMgr.h"
#include <iostream>

unsigned CContestMgr::COMPRESSION_ALGORITHM;


CContestMgr::CContestMgr(unsigned handicap, unsigned startAltitudeLoss):
  _handicap(handicap),
  _trace(TRACE_FIX_LIMIT, 0, startAltitudeLoss, COMPRESSION_ALGORITHM),
  _traceSprint(TRACE_SPRINT_FIX_LIMIT, TRACE_SPRINT_TIME_LIMIT, 0, COMPRESSION_ALGORITHM)
{
  for(unsigned i=0; i<TYPE_NUM; i++)
    _resultArray.push_back(CResult());
}


const char *CContestMgr::TypeToString(TType type)
{
  const char *typeStr[] = {
    "OLC Classic",
    "OLC FAI",
    "OLC League",
    "<invalid>" 
  };
  return typeStr[type];
}


double CContestMgr::AproxDistanceToLineSegment(const CPointGPS &point, const CPointGPS &seg1, const CPointGPS &seg2) const
{
  double A = point.Longitude() - seg1.Longitude();
  double B = point.Latitude() - seg1.Latitude();
  double C = seg2.Longitude() - seg1.Longitude();
  double D = seg2.Latitude() - seg1.Latitude();
  
  double dot = A * C + B * D;
  double len_sq = C * C + D * D;
  double param = dot / len_sq;
  
  double xx, yy;
  
  if(param < 0) {
    xx = seg1.Longitude();
    yy = seg1.Latitude();
  }
  else if(param > 1) {
    xx = seg2.Longitude();
    yy = seg2.Latitude();
  }
  else {
    xx = seg1.Longitude() + param * C;
    yy = seg1.Latitude() + param * D;
  }
  
  double dist;
  DistanceBearing(point.Latitude(), point.Longitude(), yy, xx, &dist, 0);
  return dist;
}


unsigned CContestMgr::BiggestLoopFind(const CTrace &trace, const CTrace::CPoint *&start, const CTrace::CPoint *&end) const
{
  unsigned pointsCount = trace.Size();
  const CTrace::CPoint *back = trace.Back();
  
  // try to find the biggest closed path possible
  const CTrace::CPoint *point = trace.Front();
  CTrace::CPoint *next = point->Next();
  while(next && next != back) {
    pointsCount--;
    if(back->GPS().Time() < next->GPS().Time() + TRACE_TRIANGLE_MIN_TIME)
      // filter too small circles from i.e. thermalling
      return 0;
    
    if(back->GPS().Altitude() >= next->GPS().Altitude() - TRACE_START_FINISH_ALT_DIFF) {
      // valid points altitudes combination
      // TODO: Determine if distance to line segment is really needed
      //double dist = back->GPS().Distance(next->GPS());
      double dist = AproxDistanceToLineSegment(back->GPS(), point->GPS(), next->GPS());
      if(dist < TRACE_CLOSED_MAX_DIST) {
        start = next;
        end = back;
        return pointsCount;
      }
    }
    point = next;
    next = point->Next();
  }
  
  return 0;
}


void CContestMgr::SolvePoints(const CRules &rules)
{
  if(!rules.Trace().Size())
    return;
  
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
  
  // prepare result
  CPointGPSArray pointArray;
  point = traceResult.Front();
  double distance = 0;
  while(point) {
    if(pointArray.size())
      distance += point->GPS().Distance(pointArray.back());
    pointArray.push_back(point->GPS());
    point = point->Next();
  }
  
  // store result
  if(distance > _resultArray[rules.Type()].Distance()) {
    double result;
    if(rules.Type() == TYPE_OLC_LEAGUE)
      result = distance / 2.5 * 200 / (_handicap + 100);
    else
      result = distance * 100 / _handicap;
    _resultArray[rules.Type()] = CResult(rules.Type(), distance, result, pointArray);
  }
}


void CContestMgr::SolveTriangle(const CTrace &trace)
{
  if(trace.Size() > 2) {
    const CTrace::CPoint *start = 0;
    const CTrace::CPoint *end = 0;
    if(unsigned pointsCount = BiggestLoopFind(trace, start, end)) {
      std::cout << "Loop detected (" << pointsCount << "): " << TimeToString(start->GPS().Time()) << " -> " << TimeToString(end->GPS().Time()) << std::endl;
      
      CTrace traceLoop(TRACE_TRIANGLE_FIX_LIMIT, 0, 0, COMPRESSION_ALGORITHM);
      if(pointsCount > TRACE_TRIANGLE_FIX_LIMIT * 1.5) {
        // add points to dedicated loop trace to make calulations faster
        const CTrace::CPoint *point = start;
        while(point) {
          traceLoop.Push(new CTrace::CPoint(traceLoop, *point, traceLoop._back));
          if(point == end)
            break;
          point = point->Next();
        }
        traceLoop.Compress();
        start = traceLoop.Front();
        end = traceLoop.Back();
      }
      
      // const CTrace::CPoint *current = start;
      // typedef std::vector<const CTrace::CPoint *> CPointArray;
      // CPointArray temporary;
      
      // while(current) {
      //   const CPoint *next = &current->Next();
      //   while(next) {
          
      //     next = next->Next();
      //   }
      // }
      
      // unsigned arraySize = pointsCount * (pointsCount + 1) / 2;
      // double *path = new double[arraySize];
      // CTrace::CPoint *path = new CTrace::CPoint *[arraySize];
      // for(unsigned i=0; i<arraySize; i++) {
      //   for(unsigned j=i; j<arraySize; j++) {
      //     path[i+j];
      //   }
      // }

      // for(unsigned k=0; k<arraySize; k++) {
      //   for(unsigned i=0; i<arraySize; i++) {
      //     for(unsigned j=i; j<arraySize; j++) {
            
      //     }
      //   }
      // }
      // std::cout << "arraySize: " << arraySize << std::endl;
    }
  }
}


void CContestMgr::Add(const CPointGPSSmart &gps)
{
  _trace.Push(gps);
  SolveTriangle(_trace);
  _trace.Compress();
  SolvePoints(CRules(TYPE_OLC_CLASSIC, _trace, 7, 0, TRACE_START_FINISH_ALT_DIFF));
  
  _traceSprint.Push(gps);
  _traceSprint.Compress();
  SolvePoints(CRules(TYPE_OLC_LEAGUE, _traceSprint, 5, TRACE_SPRINT_TIME_LIMIT, 0));
}
