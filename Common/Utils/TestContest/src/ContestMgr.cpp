/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "ContestMgr.h"


unsigned CContestMgr::COMPRESSION_ALGORITHM;


/** 
 * @brief Default constructor
 * 
 * Constructor to create dummy invalid contest result object.
 */
CContestMgr::CResult::CResult():
  _type(TYPE_NUM), _distance(0), _score(0)
{
}


/** 
 * @brief Primary constructor
 * 
 * @param type The type of the contest
 * @param distance Contest covered distance
 * @param score Contest score (if exists)
 * @param pointArray The list of contest result points
 */
CContestMgr::CResult::CResult(TType type, unsigned distance, float score, const CPointGPSArray &pointArray):
  _type(type), _distance(distance), _score(score), _pointArray(pointArray)
{
}


/** 
 * @brief "Copy" constructor
 * 
 * @param type The type of the contest results
 * @param ref The results data to copy (beside type)
 */
CContestMgr::CResult::CResult(TType type, const CResult &ref):
  _type(type), _distance(ref._distance), _score(ref._score), _pointArray(ref._pointArray)
{
}



/** 
 * @brief Constructor
 * 
 * @param handicap Glider handicap
 * @param startAltitudeLoss The loss of altitude needed to detect the start of powerless flight
 */
CContestMgr::CContestMgr(unsigned handicap, short startAltitudeLoss):
  _handicap(handicap),
  _trace(TRACE_FIX_LIMIT, 0, startAltitudeLoss, COMPRESSION_ALGORITHM),
  _traceSprint(TRACE_SPRINT_FIX_LIMIT, TRACE_SPRINT_TIME_LIMIT, 0, COMPRESSION_ALGORITHM),
  _traceLoop(TRACE_TRIANGLE_FIX_LIMIT, 0, 0, COMPRESSION_ALGORITHM)
{
  for(unsigned i=0; i<TYPE_NUM; i++)
    _resultArray.push_back(CResult());
}


/** 
 * @brief Returns the string representation of contest type
 * 
 * @param type Contest type to return
 * 
 * @return String representation of contest type
 */
const char *CContestMgr::TypeToString(TType type)
{
  const char *typeStr[] = {
    "OLC-Classic",
    "OLC-Classic (P)",
    "FAI-OLC",
    "OLC-Plus",
    "OLC-League",
    "FAI 3 TPs",
    "FAI 3 TPs (P)",
    "[invalid]" 
  };
  return typeStr[type];
}


/** 
 * @brief Searches the trace for biggest allowed closed loop
 * 
 * Method searches provided trace to find the biggest closed loop allowed by
 * the contest rules. The last point of provided trace is used as a reference
 * to search for the closure. Method returns start and end points of the loop
 * and the number of GPS fixes included in the loop.
 * 
 * @param trace The trace to search in
 * @param [out] start Detected start point of the loop
 * @param [out] end   Detected start point of the loop
 * 
 * @return The number of GPS fixes included in the loop.
 */
unsigned CContestMgr::BiggestLoopFind(const CTrace &trace, const CTrace::CPoint *&start, const CTrace::CPoint *&end) const
{
  unsigned pointsCount = trace.Size();
  const CTrace::CPoint *back = trace.Back();
  
  // try to find the biggest closed path possible
  const CTrace::CPoint *point = trace.Front();
  CTrace::CPoint *next = point->Next();
  while(next && next != back) {
    pointsCount--;
    if((unsigned)back->GPS().TimeDelta(next->GPS()) < TRACE_TRIANGLE_MIN_TIME)
      // filter too small circles from i.e. thermalling
      return 0;
    
    if(back->GPS().Altitude() + TRACE_START_FINISH_ALT_DIFF >= next->GPS().Altitude()) {
      // valid points altitudes combination
      unsigned dist = back->GPS().Distance(point->GPS(), next->GPS());
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


/** 
 * @brief Searches the trace for biggest allowed closed loop
 * 
 * Method searches provided trace to find the biggest closed loop allowed by
 * the contest rules. If the loop is found, method returns @c true and all
 * points of the loop are put to output trace.
 * 
 * @param traceIn Trace in which a loop should be found.
 * @param traceOut Output trace with all points of the loop.
 * 
 * @return @c true if a loop was detected.
 */
bool CContestMgr::BiggestLoopFind(const CTrace &traceIn, CTrace &traceOut) const
{
  bool updated = false;
  
  if(traceIn.Size() > 2) {
    const CTrace::CPoint *start = 0;
    const CTrace::CPoint *end = 0;
    if(BiggestLoopFind(traceIn, start, end)) {
      // new loop found - copy the points to output trace
      traceOut.Clear();
      const CTrace::CPoint *point = start;
      while(point) {
        traceOut.Push(new CTrace::CPoint(traceOut, *point, traceOut._back));
        if(point == end)
          break;
        point = point->Next();
      }
      // compress resulting trace
      traceOut.Compress();
      updated = true;
    }
  }
  
  return updated;
}


/** 
 * @brief Verifies FAI triangle edge.
 * 
 * Method checks if provided edge can create bigger FAI triangle than
 * the provided one.
 * 
 * @param length The length on a FAI triangle to verify.
 * @param best Previous best FAI circumference.
 * 
 * @return @c true if a bigger FAI triangle can be constructed from the edge.
 */
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


/** 
 * @brief Verifies if provided edges form a valid FAI triangle
 * 
 * Method checks if a valid triangle can be constructed from provided
 * triangle edges.
 * 
 * @param length1 First edge
 * @param length2 Second edge
 * @param length3 Third edge
 * 
 * @return @c true if a valid FAI triangle can be constructed from provided edges
 */
bool CContestMgr::FAITriangleEdgeCheck(unsigned length1, unsigned length2, unsigned length3) const
{
  unsigned length = length1 + length2 + length3;
  if(length < TRACE_FAI_BIG_TRIANGLE_LENGTH) {
    unsigned lengthMin = std::min(length1, std::min(length2, length3));
    return lengthMin * 25 > length * 7; // 28%
  }
  else {
    unsigned lengthMin = std::min(length1, std::min(length2, length3));
    unsigned lengthMax = std::max(length1, std::max(length2, length3));
    return lengthMin * 4 > length && lengthMax * 20 < length * 9; // 25% && 45%
  }
}


/** 
 * @brief Sets a result for points based contests
 * 
 * @param type The type of the contest
 * @param traceResult The result data to set
 */
void CContestMgr::PointsResult(TType type, const CTrace &traceResult)
{
  // prepare result
  CPointGPSArray pointArray;
  const CTrace::CPoint *point = traceResult.Front();
  unsigned distance = 0;
  while(point) {
    if(pointArray.size())
      distance += point->GPS().Distance(pointArray.back());
    pointArray.push_back(point->GPS());
    point = point->Next();
  }
  
  // store result
  if(distance > _resultArray[type].Distance()) {
    float score;
    switch(type) {
    case TYPE_OLC_CLASSIC:
    case TYPE_OLC_CLASSIC_PREDICTED:
      score = distance / 1000.0 * 100 / _handicap;
      break;
    case TYPE_OLC_LEAGUE:
      score = distance / 1000.0 / 2.5 * 200 / (_handicap + 100);
      break;
    default:
      score = 0;
    }
    _resultArray[type] = CResult(type, distance, score, pointArray);
  }
}


/** 
 * @brief Solve point based contest
 * 
 * @param trace The trace to use
 * @param sprint @c true if a sprint contest
 * @param predicted @c true if a predicted path to the trace start should be calculated
 */
void CContestMgr::SolvePoints(const CTrace &trace, bool sprint, bool predicted)
{
  if(!trace.Size())
    return;
  
  // find the last point meeting criteria
  short finishAltDiff = sprint ? 0 : TRACE_START_FINISH_ALT_DIFF;
  const CTrace::CPoint *point = trace.Back();
  const CTrace::CPoint *last = 0;
  short startAltitude = trace.Front()->GPS().Altitude();
  while(point) {
    if(point->GPS().Altitude() + finishAltDiff >= startAltitude) {
      last = point;
      break;
    }
    point = point->Previous();
  }
  
  // create result trace
  CTrace traceResult(sprint ? 5 : 7, sprint ? TRACE_SPRINT_TIME_LIMIT : 0, 0, CTrace::ALGORITHM_DISTANCE);
  
  // add points to result trace
  point = trace.Front();
  while(point && point != last->Next()) {
    traceResult.Push(new CTrace::CPoint(traceResult, *point, traceResult._back));
    point = point->Next();
  }
  
  if(predicted) {
    // predict GPS data of artificial point in the location of the trace start
    const CPointGPS &start = traceResult.Front()->GPS();
    const CResult &result = _resultArray[TYPE_OLC_CLASSIC];
    unsigned time = start.Time();
    if(result.Speed()) {
      time += result.PointArray().back().Distance(start) / result.Speed();
      time %= CPointGPS::DAY_SECONDS;
    }
    
    // add predicted point
    traceResult.Push(new CPointGPS(time, start.Latitude(), start.Longitude(), start.Altitude()));
  }
  
  // compress trace to obtain the result
  traceResult.Compress();
  
  // store result
  TType type = sprint ? TYPE_OLC_LEAGUE : (predicted ? TYPE_OLC_CLASSIC_PREDICTED : TYPE_OLC_CLASSIC);
  if(predicted)
    // do it just in a case if predicted trace is worst than the current one
    _resultArray[TYPE_OLC_CLASSIC_PREDICTED] = CResult(TYPE_OLC_CLASSIC_PREDICTED, _resultArray[TYPE_OLC_CLASSIC]);
  PointsResult(type, traceResult);
  
  if(!sprint) {
    // calculate TYPE_FAI_3_TPS
    traceResult.Compress(5);
    // store result
    if(predicted)
      // do it just in a case if predicted trace is worst than the current one
      _resultArray[TYPE_FAI_3_TPS_PREDICTED] = CResult(TYPE_FAI_3_TPS_PREDICTED, _resultArray[TYPE_FAI_3_TPS]);
    PointsResult(predicted ? TYPE_FAI_3_TPS_PREDICTED : TYPE_FAI_3_TPS, traceResult);
  }
}


/** 
 * @brief Solves FAI triangle based contest
 * 
 * @param trace The trace to use
 * @param prevFront Loop front point of previous iteration
 * @param prevBack Loop back point of previous iteration
 */
void CContestMgr::SolveTriangle(const CTrace &trace, const CPointGPS *prevFront, const CPointGPS *prevBack)
{
  CResult &result = _resultArray[TYPE_OLC_FAI];
  if(trace.Size() > 2) {
    // check for every trace point
    const CTrace::CPoint *point1st = trace.Front();
    while(point1st) {
      // check if all edges should be analysed
      bool skip1 = prevFront && prevBack;
      if(skip1 && (point1st->GPS() < *prevFront || point1st->GPS() > *prevBack))
        skip1 = false;
      
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
        bool skip2 = skip1;
        if(skip2 && (it1st->second->GPS() < *prevFront || it1st->second->GPS() > *prevBack))
          skip2 = false;
        
        unsigned dist1st = it1st->first;
        if(!FAITriangleEdgeCheck(dist1st, result.Distance()))
          // better solution found in the meantime
          break;
        
        // create a map of points that may form second edge of a better triangle
        CDistanceMap distanceMap2nd;
        const CTrace::CPoint *point2nd = it1st->second;
        for(next=point2nd->Next(); next; next=next->Next()) {
          bool skip3 = skip2;
          if(skip3 && (next->GPS() < *prevFront || next->GPS() > *prevBack))
            skip3 = false;
          if(skip3)
            // that triangle was analysed already
            continue;
          
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
              pointArray.push_back(trace.Front()->GPS());
              pointArray.push_back(point1st->GPS());
              pointArray.push_back(point2nd->GPS());
              pointArray.push_back(point3rd->GPS());
              pointArray.push_back(trace.Back()->GPS());
              _resultArray[TYPE_OLC_FAI] = CResult(TYPE_OLC_FAI, distance, score, pointArray);
            }
          }
        }
      }
      
      point1st = point1st->Next();
    }
  }
}


/** 
 * @brief Sets dummy OLC-Plus results data
 * 
 * Method sets OLC-Plus result based on the results of OLC-Classic and FAI-OLC.
 */
void CContestMgr::SolveOLCPlus()
{
  CResult &classic = _resultArray[TYPE_OLC_CLASSIC];
  CResult &fai = _resultArray[TYPE_OLC_FAI];
  _resultArray[TYPE_OLC_PLUS] = CResult(TYPE_OLC_PLUS, 0, classic.Score() + fai.Score(), CPointGPSArray());
}


/** 
 * @brief Adds a new GPS fix to analysis
 * 
 * @param gps New GPS fix to use in analysis
 */
void CContestMgr::Add(const CPointGPSSmart &gps)
{
  static unsigned step = 0;
  const unsigned STEPS_NUM = 4;
  
  // OLC-Plus
  _trace.Push(gps);
  if(step % STEPS_NUM == 0) {
    // Solve FAI-OLC
    std::auto_ptr<CPointGPS> prevFront(_traceLoop.Front() ? new CPointGPS(_traceLoop.Front()->GPS()) : 0);
    std::auto_ptr<CPointGPS> prevBack(_traceLoop.Back() ? new CPointGPS(_traceLoop.Back()->GPS()) : 0);
    if(BiggestLoopFind(_trace, _traceLoop))
      SolveTriangle(_traceLoop, prevFront.get(), prevBack.get());
  }
  _trace.Compress();
  if(step % STEPS_NUM == 1) {
    // Solve OLC-Classic and FAI 3TPs
    SolvePoints(_trace, false, false);
  }
  SolveOLCPlus();

  if(step % STEPS_NUM == 2) {
    // Solve OLC-Classic and FAI 3TPs for predicted path
    SolvePoints(_trace, false, true);
  }
  
  // OLC-League
  _traceSprint.Push(gps);
  _traceSprint.Compress();
  if(step % STEPS_NUM == 3)
    // Solve OLC-Sprint
    SolvePoints(_traceSprint, true, false);
  
  step++;
}
