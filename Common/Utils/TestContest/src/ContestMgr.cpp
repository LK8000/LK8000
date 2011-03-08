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


unsigned CContestMgr::AproxDistanceToLineSegment(const CPointGPS &point, const CPointGPS &seg1, const CPointGPS &seg2) const
{
  double lat = point.Latitude();
  double lon = point.Longitude();
  double seg1Lat = seg1.Latitude();
  double seg1Lon = seg1.Longitude();
  double seg2Lat = seg2.Latitude();
  double seg2Lon = seg2.Longitude();
  
  double A = lon - seg1Lon;
  double B = lat - seg1Lat;
  double C = seg2Lon - seg1Lon;
  double D = seg2Lat - seg1Lat;
  
  double dot = A * C + B * D;
  double len_sq = C * C + D * D;
  double param = dot / len_sq;
  
  double xx, yy;
  
  if(param < 0) {
    xx = seg1Lon;
    yy = seg1Lat;
  }
  else if(param > 1) {
    xx = seg2Lon;
    yy = seg2Lat;
  }
  else {
    xx = seg1Lon + param * C;
    yy = seg1Lat + param * D;
  }
  
  double dist;
  DistanceBearing(lat, lon, yy, xx, &dist, 0);
  return dist;
  
  //  return point.Distance(CPointGPS(0, yy, xx, 0));
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
    
    if(back->GPS().Altitude() + TRACE_START_FINISH_ALT_DIFF >= next->GPS().Altitude()) {
      // valid points altitudes combination
      // TODO: Determine if distance to line segment is really needed
      //unsigned dist = back->GPS().Distance(next->GPS());
      unsigned dist = AproxDistanceToLineSegment(back->GPS(), point->GPS(), next->GPS());
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


void CContestMgr::BiggestLoopFind(const CTrace &traceIn, CTrace &traceOut) const
{
  if(traceIn.Size() > 2) {
    const CTrace::CPoint *start = 0;
    const CTrace::CPoint *end = 0;
    if(BiggestLoopFind(traceIn, start, end)) {
      const CTrace::CPoint *point = start;
      while(point) {
        traceOut.Push(new CTrace::CPoint(traceOut, *point, traceOut._back));
        if(point == end)
          break;
        point = point->Next();
      }
      traceOut.Compress();
    }
  }
}


bool CContestMgr::FAITriangleEdgeCheck(unsigned length, unsigned best) const
{
  unsigned length4 = length * 4;
  if(length4 < TRACE_FAI_BIG_TRIANGLE_LENGTH) { 
    // triangle to small to be a big FAI
    if(best * 7 > length * 25) // 28%
      // path too short to have a chance to be a result - skip it
      return false;
  }
  else {
    if(best > length4) // 25%
      // path too short to have a chance to be a result - skip it
      return false;
  }
  return true;
}


bool CContestMgr::FAITriangleEdgeCheck(unsigned length1, unsigned length2, unsigned length3) const
{
  unsigned length = length1 + length2 + length3;
  if(length < TRACE_FAI_BIG_TRIANGLE_LENGTH) {
    unsigned lengthMin = std::min(length1, std::min(length2, length3));
    return lengthMin * 4 > length;
  }
  else {
    unsigned lengthMin = std::min(length1, std::min(length2, length3));
    unsigned lengthMax = std::min(length1, std::min(length2, length3));
    return lengthMin * 25 > length * 7 && lengthMax * 20 < length * 9;
  }
}


void CContestMgr::SolvePoints(const CRules &rules)
{
  if(!rules.Trace().Size())
    return;
  
  // find the last point meeting criteria
  unsigned finishAltDiff = rules.FinishAltDiff();
  const CTrace::CPoint *point = rules.Trace().Back();
  const CTrace::CPoint *last = 0;
  unsigned startAltitude = rules.Trace().Front()->GPS().Altitude();
  while(point) {
    if(point->GPS().Altitude() + finishAltDiff >= startAltitude) {
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
  unsigned distance = 0;
  while(point) {
    if(pointArray.size())
      distance += point->GPS().Distance(pointArray.back());
    pointArray.push_back(point->GPS());
    point = point->Next();
  }
  
  // store result
  if(distance > _resultArray[rules.Type()].Distance()) {
    float score;
    if(rules.Type() == TYPE_OLC_LEAGUE)
      score = distance / 1000.0 / 2.5 * 200 / (_handicap + 100);
    else
      score = distance / 1000.0 * 100 / _handicap;
    _resultArray[rules.Type()] = CResult(rules.Type(), distance, score, pointArray);
  }
}


void CContestMgr::SolveTriangle(const CTrace &trace)
{
  CResult &result = _resultArray[TYPE_OLC_FAI];
  if(trace.Size() > 2) {
    // check for every trace point
    const CTrace::CPoint *point1st = trace.Front();
    while(point1st) {
      // create a map of points that may form first edge of a better triangle
      CDistanceMap distanceMap1st;
      const CTrace::CPoint *next = 0;
      for(next=point1st->Next(); next; next=next->Next()) {
        unsigned dist = point1st->GPS().Distance(next->GPS());
        // check if 1st edge not too short
        if(!FAITriangleEdgeCheck(dist, result.Distance()))
          continue;
        distanceMap1st.insert(std::make_pair(dist, next));
      }
      
      // check all possible first edges of the triangle
      for(CDistanceMap::reverse_iterator it1st=distanceMap1st.rbegin(); it1st!=distanceMap1st.rend(); ++it1st) {
        unsigned dist1st = it1st->first;
        if(!FAITriangleEdgeCheck(dist1st, result.Distance()))
          // better solution found in the meantime
          break;
        
        // create a map of points that may form second edge of a better triangle
        CDistanceMap distanceMap2nd;
        const CTrace::CPoint *point2nd = it1st->second;
        for(next=point2nd->Next(); next; next=next->Next()) {
          unsigned dist = point2nd->GPS().Distance(next->GPS());
          // check if 2nd edge not too long
          if(dist * 14 > dist1st * 20) // 45% > 25%
            continue;
          // check if 2nd edge not too short
          if(!FAITriangleEdgeCheck(dist, result.Distance()))
            continue;
          distanceMap2nd.insert(std::make_pair(dist, next));
        }
        
        // check all possible second and third edges of the triangle
        for(CDistanceMap::reverse_iterator it2nd=distanceMap2nd.rbegin(); it2nd!=distanceMap2nd.rend(); ++it2nd) {
          unsigned dist2nd = it2nd->first;
          if(!FAITriangleEdgeCheck(dist2nd, result.Distance()))
            // better solution found in the meantime
            break;
          
          const CTrace::CPoint *point3rd = it2nd->second;
          unsigned dist3rd = point3rd->GPS().Distance(point1st->GPS());
          unsigned distance = dist1st + dist2nd + dist3rd;
          if(distance > result.Distance()) {
            // check if valid FAI triangle
            if(FAITriangleEdgeCheck(dist1st, dist2nd, dist3rd)) {
              // store new result
              float score = distance / 1000.0 * 0.3 * 100 / _handicap;
              CPointGPSArray pointArray;
              pointArray.push_back(point1st->GPS());
              pointArray.push_back(point2nd->GPS());
              pointArray.push_back(point3rd->GPS());
              _resultArray[TYPE_OLC_FAI] = CResult(TYPE_OLC_FAI, distance, score, pointArray);
            }
          }
        }
      }
      
      point1st = point1st->Next();
    }
  }
}


void CContestMgr::Add(const CPointGPSSmart &gps)
{
  // OLC Plus
  _trace.Push(gps);
  CTrace traceLoop(TRACE_TRIANGLE_FIX_LIMIT, 0, 0, COMPRESSION_ALGORITHM);
  BiggestLoopFind(_trace, traceLoop);
  if(traceLoop.Size())
    SolveTriangle(traceLoop); 
  _trace.Compress();
  SolvePoints(CRules(TYPE_OLC_CLASSIC, _trace, 7, 0, TRACE_START_FINISH_ALT_DIFF));
  
  // OLC League
  _traceSprint.Push(gps);
  _traceSprint.Compress();
  SolvePoints(CRules(TYPE_OLC_LEAGUE, _traceSprint, 5, TRACE_SPRINT_TIME_LIMIT, 0));
}
