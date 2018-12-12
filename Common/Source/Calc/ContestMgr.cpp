/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: ContestMgr.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/


#include "externs.h"

#include "ContestMgr.h"
#include <memory>
#include "NavFunctions.h"
#include "RasterTerrain.h"


//#define MAX_EARTH_DIST_IN_M   40000000.0
CContestMgr CContestMgr::_instance;

/** 
 * @brief Returns the string representation of contest type
 * 
 * @param type Contest type to return
 * 
 * @return String representation of contest type
 */
const TCHAR *CContestMgr::TypeToString(TType type)
{
  const TCHAR *typeStr[] = {
    _T("OLC-Classic"),
    _T("FAI-OLC"),
    _T("OLC-Plus"),
    _T("OLC-Classic (P)"),
    _T("FAI-OLC (P)"),
    _T("OLC-Plus (P)"),
    _T("OLC-League"),
    _T("FAI 3 TPs"),
    _T("FAI 3 TPs (P)"),
    MsgToken(1297) ,   //    _@M1297_ "FAI Start on turnpoint"
    MsgToken(1298)  ,  //    _@M1298_ "FAI Start on leg"
#ifdef   FIVEPOINT_OPTIMIZER
  MsgToken(1299)  ,  //    _@M1299_ "FAI Mercedes star"
#endif
    _T("XC FREE Triangle") ,
    _T("XC FAI Triangle"),
    _T("XC FREE Fligth"),
    _T("XC Total"),
    _T("[invalid]")
  };
  return typeStr[type];
}


/**
 * @brief Returns the string representation of current contest rule
 *
 * @param rule ID
 *
 * @return String representation of current contest rule . No need to translate here
 */
const TCHAR *CContestMgr::XCRuleToString(int rule)
{
  const TCHAR *typeStr[] = {
      _T("OFF"),                      // Onlt OLC evalueted
      _T("XContest 2018"),
      _T("XContest 2019"),
      _T("French CFD ligue"),
      _T("Leonardo XC"),
      _T("UK National League"),
      _T("[INVALID]")                 // Never use this. Used for iterate over rules
  };
  return typeStr[rule];
}

/** 
 * @brief Constructor
 */
CContestMgr::CContestMgr():
  _handicap(DEFAULT_HANDICAP),
  _trace(new CTrace(TRACE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM)),
  _traceSprint(new CTrace(TRACE_SPRINT_FIX_LIMIT, TRACE_SPRINT_TIME_LIMIT, COMPRESSION_ALGORITHM)),
  _traceFreeTriangle(new CTrace(TRACE_TRIANGLE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM)),
  _traceLoop(new CTrace(TRACE_TRIANGLE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM)),
  _prevFAIFront(), _prevFAIBack(),
  _prevFAIPredictedFront(), _prevFAIPredictedBack(),
  _pgpsClosePoint(0,0,0,0),
  _pgpsBestClosePoint(0,0,0,0),
  _pgpsNearPoint(0,0,0,0),
  _pgpsFreeTriangleClosePoint(0,0,0,0),
  _pgpsFreeTriangleBestClosePoint(0,0,0,0)
{
  _fTogo     =  0;
  _fBestTogo =  0;
  _uiFAIDist  =0;
  _bFAI              =false;
  _resultFREETriangle = CResult();
  _bestXCTriangleType = XCFlightType::XC_INVALID;
  _bestXCType = XCFlightType::XC_INVALID;
  _XCFAIStatus= XCTriangleStatus::INVALID;
  _XCFTStatus= XCTriangleStatus::INVALID;
  _XCTriangleClosurePercentage = 0;
  _XCTriangleDistance = 0;
  _XCMeanSpeed = 0;

  ScopeLock guard(_resultsCS);
  for(unsigned i=0; i<TYPE_NUM; i++)
    _resultArray.push_back(CResult());
}

/** 
 * @brief Resets Contest Manager
 * 
 * @param handicap Glider handicap
 */
void CContestMgr::Reset(unsigned handicap)
{
  ScopeLock guard(_mainCS);
  _handicap = handicap;
  {
    ScopeLock TraceGuard(_traceCS);
    _trace.reset(new CTrace(TRACE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM));
  }
  _traceSprint.reset(new CTrace(TRACE_SPRINT_FIX_LIMIT, TRACE_SPRINT_TIME_LIMIT, COMPRESSION_ALGORITHM));
  _traceLoop.reset(new CTrace(TRACE_TRIANGLE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM));
  _traceFreeTriangle.reset(new CTrace(TRACE_TRIANGLE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM));
  _prevFAIFront.reset(0);
  _prevFAIBack.reset(0);
  _prevFAIPredictedFront.reset(0);
  _prevFAIPredictedBack.reset(0);
  _prevFreeTriangleFront.reset(0);
  _prevFreeTriangleBack.reset(0);
  _bFAI = false;
  _uiFAIDist =0;
  _fTogo     = 0;
  _fBestTogo = 0;
  _resultFREETriangle = CResult();
  {
    ScopeLock Resultguard(_resultsCS);
    for(unsigned i=0; i<TYPE_NUM; i++)
      _resultArray[i] = CResult();
  }
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
 * @return @c true if a loop found
 */
bool CContestMgr::BiggestLoopFind(const CTrace &trace, const CTrace::CPoint *&start, const CTrace::CPoint *&end) const
{
  const CTrace::CPoint *back = trace.Back();
  
  // try to find the biggest closed path possible
  const CTrace::CPoint *point = trace.Front();
  CTrace::CPoint *next = point->Next();
  while(next && next != back) {
    if((unsigned)back->GPS().TimeDelta(next->GPS()) < TRACE_TRIANGLE_MIN_TIME)
      // filter too small circles from i.e. thermalling
      return false;
    
    if(back->GPS().Altitude() + TRACE_START_FINISH_ALT_DIFF >= next->GPS().Altitude()) {
      // valid points altitudes combination
      unsigned dist = back->GPS().DistanceXYZ(point->GPS(), next->GPS());
      if(dist < TRACE_CLOSED_MAX_DIST) {
        start = next;
        end = back;
        return true;
      }
    }
    point = next;
    next = point->Next();
  }
  
  return false;
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
 * @param predicted @c true if a predicted path to the trace start should be calculated
 * 
 * @return @c true if a loop was detected.
 */
bool CContestMgr::BiggestLoopFind(const CTrace &traceIn, CTrace &traceOut, bool predicted) const
{
  bool updated = false;
  
  if(traceIn.Size() > 2) {
    const CTrace::CPoint *start = traceIn.Front();
    const CTrace::CPoint *end = traceIn.Back();
    if((unsigned)end->GPS().TimeDelta(start->GPS()) < TRACE_TRIANGLE_MIN_TIME)
      // filter too small circles from i.e. thermalling
      return false;
    
    if(predicted || BiggestLoopFind(traceIn, start, end)) {
      // new valid loop found - copy the points to output trace
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
 * @brief Verifies if provided edges form a valid FLAT triangle
 *
 * @param length1 First edge
 * @param length2 Second edge
 * @param length3 Third edge
 *
 * @return @c true if a valid FAI triangle can be constructed from provided edges
 */
bool CContestMgr::FREETriangleEdgeCheck(unsigned length1, unsigned length2, unsigned length3) const
{
  bool valid = false;

  switch (AdditionalContestRule) {
    case XContest2018:
      valid= true;            // No constrain on edge here
      break;
    case XContest2019:
      valid= true;            // No constrain on edge here
      break;
    case CFD:
      valid= true;            // No constrain on edge here
      break;
    case LEONARDO_XC:
      valid= true;            // No constrain on edge here
      break;
    case UK_NATIONAL_LEAGUE:
      valid= true;            // and check
      const unsigned length = length1 + length2 + length3;
      const unsigned lengthMin = std::min(length1, std::min(length2, length3));
      if ( lengthMin * 3 < length * 20 ) {    // 15%
        valid = false;
      }
      else  {
        const unsigned lengthMax = std::max(length1, std::max(length2, length3));
        if ( lengthMax * 9 > length * 20 )   // 45%
          valid = false;
      }
      break;
  }
  return valid;
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
      distance += point->GPS().DistanceXYZ(pointArray.back());
    pointArray.push_back(point->GPS());
    point = point->Next();
  }
  
  // store result
  if(distance > _resultArray[type].Distance()) {
    float score;
    LKASSERT(_handicap>0);
    if (_handicap==0) return; // UNMANAGED
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
    bool predicted = pointArray.back().TimeDelta(_trace->Back()->GPS()) > 0;
    ScopeLock guard(_resultsCS);
    _resultArray[type] = CResult(type, predicted, distance, score, pointArray);
  }
}


void CContestMgr::RefreshFAIOptimizer(void)
{
      ScopeLock guard(_mainCS);
      {
        ScopeLock Traceguard(_traceCS);
        _trace->Compress();
      }
    SolvePoints(*_trace, false, true);
    SolveOLCPlus(true);

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
  
  // find the first point meeting criteria
  short finishAltDiff = sprint ? 0 : TRACE_START_FINISH_ALT_DIFF;
  const CTrace::CPoint *first = 0;
  const CTrace::CPoint *point = trace.Front();
  const CTrace::CPoint *last = trace.Back();
  short endAltitude = last->GPS().Altitude();
  while(point) {
    if(point->GPS().Altitude() <= endAltitude + finishAltDiff) {
      first = point;
      break;
    }
    point = point->Next();
  }
  
  if(!first)
    // no points matching heights constrain
    return;
  
  // create result trace
  CTrace traceResult(sprint ? 5 : 7, sprint ? TRACE_SPRINT_TIME_LIMIT : 0, CTrace::ALGORITHM_DISTANCE);
  
  // add points to result trace
  point = first;
  while(point && point != last->Next()) {
    traceResult.Push(new CTrace::CPoint(traceResult, *point, traceResult._back));
    point = point->Next();
  }
  
  if(predicted) {
    // predict GPS data of artificial point in the location of the trace start
    const CPointGPS &start = traceResult.Front()->GPS();
    const CPointGPS &end = traceResult.Back()->GPS();
    float speed = _resultArray[TYPE_OLC_CLASSIC].Speed();
    unsigned time = end.Time();
    if(speed) {
      time += static_cast<unsigned>(end.DistanceXYZ(start) / speed);
      LKASSERT(CPointGPS::DAY_SECONDS!=0);
      if (CPointGPS::DAY_SECONDS!=0) // UNMANAGED
      time %= CPointGPS::DAY_SECONDS;
    }
    
    // add predicted point
    traceResult.Push(make_CPointGPSSmart(time, start.Latitude(), start.Longitude(), start.Altitude()));
  }
  
  // compress trace to obtain the result
  traceResult.Compress();
  
  // store result
  TType type = sprint ? TYPE_OLC_LEAGUE : (predicted ? TYPE_OLC_CLASSIC_PREDICTED : TYPE_OLC_CLASSIC);
  if(predicted) {
    // do it just in a case if predicted trace is worst than the current one
    ScopeLock guard(_resultsCS);
    _resultArray[TYPE_OLC_CLASSIC_PREDICTED] = CResult(TYPE_OLC_CLASSIC_PREDICTED, _resultArray[TYPE_OLC_CLASSIC]);
  }
  PointsResult(type, traceResult);
  

  if(!sprint) {
    // calculate TYPE_FAI_3_TPS
	  /*
if(result.Type() ==ContestMgr::TYPE_FAI_TRIANGLE)
   traceResult.Compress(3);
else*/
   traceResult.Compress(5);
    // store result
    if(predicted) {
      // do it just in a case if predicted trace is worst than the current one
      ScopeLock guard(_resultsCS);
      _resultArray[TYPE_FAI_3_TPS_PREDICTED] = CResult(TYPE_FAI_3_TPS_PREDICTED, _resultArray[TYPE_FAI_3_TPS]);
    }

    PointsResult(predicted ? TYPE_FAI_3_TPS_PREDICTED : TYPE_FAI_3_TPS, traceResult);

    traceResult.Compress(FAI_OptimizerMode);
    if(predicted)
    {
      // do it just in a case if predicted trace is worst than the current one
      ScopeLock guard(_resultsCS);
      _resultArray[TYPE_FAI_TRIANGLE] = CResult(TYPE_FAI_TRIANGLE, _resultArray[TYPE_OLC_FAI_PREDICTED]);
    }

    PointsResult(TYPE_FAI_TRIANGLE, traceResult);


    const CPointGPSArray &points = _resultArray[TYPE_FAI_TRIANGLE]._pointArray;
	double fDist=0, fAngle =0.0;
	 _bFAI = false;
	if(points.size() > 3)
	{
	  double fRelMin = FAI_NORMAL_PERCENTAGE; //   0.28    /*  % of FAI triangle if distance < 750km/500km */
	  _bFAI = true;
	  double fLeg1 = points[1].Distance(points[2]);
	  double fLeg2 = points[2].Distance(points[3]);
	  double fLeg3 = points[3].Distance(points[1]);
	  double fFAIDist = fLeg1 + fLeg2 + fLeg3;
	  if(fFAIDist <=0.0) fFAIDist = 1.0;
	  if(fFAIDist > FAI28_45Threshold)
		  fRelMin = FAI_BIG_PERCENTAGE; //   0.25    /*  % of FAI triangle if distance > 750km/500km */
	  if ((fLeg1 / fFAIDist) < fRelMin) _bFAI = false;
	  if ((fLeg2 / fFAIDist) < fRelMin) _bFAI = false;
	  if ((fLeg3 / fFAIDist) < fRelMin) _bFAI = false;

	  point = trace.Front();
	  if(point)
	  {
	    DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude, point->GPS().Latitude() , point->GPS().Longitude() , &_fTogo, &fAngle);
		_pgpsClosePoint = point->GPS();
	  }

	  if((unsigned int)(_uiFAIDist*100.0) != (unsigned int)(fFAIDist*100.0))  /* copare if still the same FAI ? */
	  {
		_uiFAIDist = fFAIDist;     /* reset previous  infos */
		_pgpsBestClosePoint = _pgpsClosePoint;
		_fBestTogo = _fTogo;
		_pgpsBestClosePoint = _pgpsClosePoint;
		_pgpsNearPoint      = trace.Back()->GPS();
	  }

	  while( (point->GPS().Time() <=  points[1].Time()) && (point != last) && point)
	  {
		DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude, point->GPS().Latitude() , point->GPS().Longitude() , &fDist, &fAngle);
	    if(fDist < _fTogo )
	    {
		  _pgpsClosePoint = point->GPS();
		  _fTogo = fDist;
	    }
	    point = point->Next();
	  }

	  if((_fTogo < _fBestTogo) && trace.Back())     /* shorter than before ? */
	  {
		_fBestTogo          = _fTogo;               /* remember closest distance */
		_pgpsBestClosePoint = _pgpsClosePoint;      /* best closing point        */
		_pgpsNearPoint      = trace.Back()->GPS();  /* with current position     */

	  }
	}

	static bool needfilling_notriangle=true;
	static bool needfilling_yestriangle=true;
	static double oldFAIDist=0;

	if (_bFAI && (_uiFAIDist != oldFAIDist)) {
		needfilling_yestriangle=true;
		oldFAIDist=_uiFAIDist;
	}

    if ( ! AdditionalContestRule) {       // If XC is active RESWP_FAIOPTIMIZED is set by the XCSolve function

      if(_bFAI)
      {
        needfilling_notriangle=true;
        WayPointList[RESWP_FAIOPTIMIZED].Latitude=_pgpsClosePoint.Latitude();
        WayPointList[RESWP_FAIOPTIMIZED].Longitude=_pgpsClosePoint.Longitude();
        WayPointList[RESWP_FAIOPTIMIZED].Altitude=_pgpsClosePoint.Altitude();
        if (WayPointList[RESWP_FAIOPTIMIZED].Altitude==0) WayPointList[RESWP_FAIOPTIMIZED].Altitude=0.001;
        WayPointList[RESWP_FAIOPTIMIZED].Reachable=TRUE;
        WayPointList[RESWP_FAIOPTIMIZED].Visible=TRUE;

        if(needfilling_yestriangle) {
          _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Comment,_T("%-.1f %s %s"),_uiFAIDist *DISTANCEMODIFY,Units::GetDistanceName(), MsgToken(1816)); // FAI triangle closing point
          needfilling_yestriangle=false;
        }

      }
      else
      {
        //WayPointList[RESWP_FAIOPTIMIZED].Latitude=RESWP_INVALIDNUMBER;
        //WayPointList[RESWP_FAIOPTIMIZED].Longitude=RESWP_INVALIDNUMBER;
        WayPointList[RESWP_FAIOPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
        WayPointList[RESWP_FAIOPTIMIZED].Reachable=false;
        WayPointList[RESWP_FAIOPTIMIZED].Visible=false;
        if (needfilling_notriangle) {
          _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Comment,_T("no triangle closing point found!"));
          needfilling_notriangle=false;
        }
      }
    }
  }
}


/** 
 * @brief Solves FAI triangle based contest
 * 
 * @param trace The trace to use
 * @param prevFront Loop front point of previous iteration
 * @param prevBack Loop back point of previous iteration
 * @param predicted @c true if a predicted path to the trace start should be calculated
 */
void CContestMgr::SolveFAITriangle(const CTrace &trace,
                                   const CPointGPS *prevFront,
                                   const CPointGPS *prevBack,
                                   bool predicted)
{
  TType type = predicted ? TYPE_OLC_FAI_PREDICTED : TYPE_OLC_FAI;
  CResult bestResult = _resultArray[type];
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
        unsigned dist = point1st->GPS().DistanceXYZ(next->GPS());
        // check if 1st edge not too short
        if(!FAITriangleEdgeCheck(dist, bestResult.Distance()))
          continue;
        distanceMap1st.insert(std::make_pair(dist, next));
      }
      
      // check all possible first edges of the triangle
      for(CDistanceMap::reverse_iterator it1st=distanceMap1st.rbegin(); it1st!=distanceMap1st.rend(); ++it1st) {
        bool skip2 = skip1;
        if(skip2 && (it1st->second->GPS() < *prevFront || it1st->second->GPS() > *prevBack))
          skip2 = false;
        
        unsigned dist1st = it1st->first;
        if(!FAITriangleEdgeCheck(dist1st, bestResult.Distance()))
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
          
          unsigned dist = point2nd->GPS().DistanceXYZ(next->GPS());
          // check if 2nd edge not too long
          if(dist * 14 > dist1st * 20) // 45% > 25%
            continue;
          // check if 2nd edge not too short
          if(!FAITriangleEdgeCheck(dist, bestResult.Distance()))
            continue;
          distanceMap2nd.insert(std::make_pair(dist, next));
        }
        
        // check all possible second and third edges of the triangle
        for(CDistanceMap::reverse_iterator it2nd=distanceMap2nd.rbegin(); it2nd!=distanceMap2nd.rend(); ++it2nd) {
          unsigned dist2nd = it2nd->first;
          if(!FAITriangleEdgeCheck(dist2nd, bestResult.Distance()))
            // better solution found in the meantime
            break;
          
          const CTrace::CPoint *point3rd = it2nd->second;
          unsigned dist3rd = point3rd->GPS().DistanceXYZ(point1st->GPS());
          unsigned distance = dist1st + dist2nd + dist3rd;
          if(distance > bestResult.Distance()) {
            // check if valid FAI triangle
            if(FAITriangleEdgeCheck(dist1st, dist2nd, dist3rd)) {
              // store new result
              LKASSERT(_handicap>0);
              if (_handicap==0) return; // UNMANAGED
              float score = distance / 1000.0 * 0.3 * 100 / _handicap;
              CPointGPSArray pointArray;
              pointArray.push_back(trace.Front()->GPS());
              pointArray.push_back(point1st->GPS());
              pointArray.push_back(point2nd->GPS());
              pointArray.push_back(point3rd->GPS());
              pointArray.push_back(trace.Back()->GPS());
              
              bool predictedFAI = false;
              if(type == TYPE_OLC_FAI_PREDICTED) {
                const CResult &resultFAI = _resultArray[TYPE_OLC_FAI];
                if(resultFAI.Type() == TYPE_OLC_FAI) {
                  // check time range
                  const CPointGPSArray &pointsFAI = resultFAI.PointArray();
                  if(pointsFAI[0].TimeDelta(pointArray[1]) > 0 ||
                     pointArray[3].TimeDelta(pointsFAI[4]) > 0)
                    // result outside of not predicted loop
                    predictedFAI = true;
                }
                else
                  // has to be predicted triangle as OLC-FAI invalid
                  predictedFAI = true;
              }
              
              bestResult = CResult(type, predictedFAI, distance, score, pointArray);
            }
          }
        }
      }
      
      point1st = point1st->Next();
    }
  }
  
  if(predicted && bestResult.Predicted()) {
    // modify the last point and recalulate the result
    const CPointGPS &start = bestResult.PointArray().front();
    const CPointGPS &end = trace.Back()->GPS();
    float speed = _resultArray[TYPE_OLC_CLASSIC].Speed();
    unsigned time = end.Time();
    if(speed) {
      time += static_cast<unsigned>(end.DistanceXYZ(start) / speed);
      time %= CPointGPS::DAY_SECONDS;
    }
    
    ScopeLock guard(_resultsCS);
    bestResult._pointArray[4] = CPointGPS(time, start.Latitude(), start.Longitude(), start.Altitude());
    bestResult.Update();
  }
  
  if(bestResult.Type() != TYPE_INVALID) {
    ScopeLock guard(_resultsCS);
    _resultArray[type] = bestResult;
  }
}

/**
 * @brief Solves XContest Free Triangle based contest
 * Tony 2018
 * @param trace The trace to use
 * @param prevFront Loop front point of previous iteration
 * @param prevBack Loop back point of previous iteration
 * @param predicted @c true if a predicted path to the trace start should be calculated
 */
void CContestMgr::SolveFREETriangle(const CTrace &trace,
                                    const CPointGPS *prevFront,
                                    const CPointGPS *prevBack) {
  if (trace.Size() > 2) {
    // check for every trace point
    const CTrace::CPoint *point1st = trace.Front();
    while (point1st) {
      // check if all edges should be analysed
      bool skip1 = prevFront && prevBack;
      if (skip1 && (point1st->GPS() < *prevFront || point1st->GPS() > *prevBack))
        skip1 = false;

      // create a map of points that may form first edge of a better triangle
      CDistanceMap distanceMap1st;
      const CTrace::CPoint *next = 0;
      for (next = point1st->Next(); next; next = next->Next()) {
        unsigned dist = point1st->GPS().DistanceXYZ(next->GPS());

        distanceMap1st.insert(std::make_pair(dist, next));
      }

      // check all possible first edges of the triangle
      for (CDistanceMap::reverse_iterator it1st = distanceMap1st.rbegin(); it1st != distanceMap1st.rend(); ++it1st) {
        bool skip2 = skip1;
        if (skip2 && (it1st->second->GPS() < *prevFront || it1st->second->GPS() > *prevBack))
          skip2 = false;

        unsigned dist1st = it1st->first;
        // create a map of points that may form second edge of a better triangle
        CDistanceMap distanceMap2nd;
        const CTrace::CPoint *point2nd = it1st->second;
        for (next = point2nd->Next(); next; next = next->Next()) {
          bool skip3 = skip2;
          if (skip3 && (next->GPS() < *prevFront || next->GPS() > *prevBack))
            skip3 = false;
          if (skip3)
            // that triangle was analysed already
            continue;

          unsigned dist = point2nd->GPS().DistanceXYZ(next->GPS());
          distanceMap2nd.insert(std::make_pair(dist, next));
        }

        // check all possible second and third edges of the triangle
        for (CDistanceMap::reverse_iterator it2nd = distanceMap2nd.rbegin(); it2nd != distanceMap2nd.rend(); ++it2nd) {
          unsigned dist2nd = it2nd->first;

          const CTrace::CPoint *point3rd = it2nd->second;
          unsigned dist3rd = point3rd->GPS().DistanceXYZ(point1st->GPS());
          unsigned total_distance = dist1st + dist2nd + dist3rd;

          if ( !FREETriangleEdgeCheck(dist1st, dist2nd, dist3rd) )
            break;

          double fFreeTriangleBestTogo = 100e100; // PC does not compile  DBL_MAX;
          double fAngle = 0;
          CPointGPS pgpsBestClose(0,0,0,0) ;
          CPointGPS pgpsClose(0,0,0,0) ;
          double closure;
          if (_resultFREETriangle.PointArray().size() > 0) {
            const CTrace::CPoint *p = trace.Front();
            while (p && p->GPS().Time() <= _resultFREETriangle.PointArray()[1].Time()) {
              DistanceBearing(GPS_INFO.Latitude, GPS_INFO.Longitude, p->GPS().Latitude(), p->GPS().Longitude(), &closure, &fAngle);
              // Find best closure point
              if (closure < fFreeTriangleBestTogo) {
                pgpsBestClose = CPointGPS(GPS_INFO.Time,GPS_INFO.Latitude,GPS_INFO.Longitude,GPS_INFO.Altitude);
                pgpsClose =  CPointGPS(p->GPS().Time(),p->GPS().Latitude(),p->GPS().Longitude(),p->GPS().Altitude());
                fFreeTriangleBestTogo = closure;
              }
              p = p->Next();
            }
            _fFreeTriangleTogo = fFreeTriangleBestTogo;
          }
          if (total_distance <= _resultFREETriangle.PredictedDistance()) {   // just update distance to go with current triangle if necessary
            if (_fFreeTriangleTogo < _fFreeTriangleBestTogo) {   // need only to update result
              _fFreeTriangleBestTogo = _fFreeTriangleTogo;
              _pgpsFreeTriangleBestClosePoint =  pgpsBestClose;
              _pgpsFreeTriangleClosePoint = pgpsClose;
              _resultFREETriangle.UpdateDistance(_resultFREETriangle.PredictedDistance() - _fFreeTriangleBestTogo);
            }
          }
          else {  // bigger triangle found. Store it into pointArray
            _fFreeTriangleBestTogo = _fFreeTriangleTogo;
            const double current_distance = total_distance - _fFreeTriangleBestTogo;
            CPointGPSArray pointArray;
            pointArray.push_back(trace.Front()->GPS());
            pointArray.push_back(point1st->GPS());
            pointArray.push_back(point2nd->GPS());
            pointArray.push_back(point3rd->GPS());
            pointArray.push_back(trace.Back()->GPS());
            _pgpsFreeTriangleBestClosePoint =  pgpsBestClose;
            _pgpsFreeTriangleClosePoint = pgpsClose;

            _resultFREETriangle.UpdateDistancesAndArray(current_distance, total_distance, pointArray);
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
 *
 * @param predicted @c true if a predicted path to the trace start should be calculated
 */
void CContestMgr::SolveOLCPlus(bool predicted)
{
  ScopeLock guard(_resultsCS);
  CResult &classic = _resultArray[predicted ? TYPE_OLC_CLASSIC_PREDICTED : TYPE_OLC_CLASSIC];
  CResult &fai = _resultArray[predicted ? TYPE_OLC_FAI_PREDICTED : TYPE_OLC_FAI];
  _resultArray[predicted ? TYPE_OLC_PLUS_PREDICTED : TYPE_OLC_PLUS] =
    CResult(predicted ? TYPE_OLC_PLUS_PREDICTED : TYPE_OLC_PLUS,
            classic.Predicted() || fai.Predicted(),
            0, classic.Score() + fai.Score(), CPointGPSArray());
}

/** 
 * @brief Adds a new GPS fix to analysis
 * 
 * @param gps New GPS fix to use in analysis
 */
void CContestMgr::Add(unsigned time, double lat, double lon, int alt)
{
  static CPointGPS lastGps(0, 0, 0, 0);
  static unsigned step = 0;
  unsigned STEPS_NUM;
  if ( AdditionalContestRule == 0 ) // No additional Contest
    STEPS_NUM = 7;
  else
    STEPS_NUM = 9;

  const CPointGPSSmart gps = make_CPointGPSSmart(time, lat, lon, alt);
  
  // filter out GPS fix repeats
  if(lastGps == *gps)
    return;
  lastGps = *gps;
  
  ScopeLock guard(_mainCS);
  {
    // Update main trace
    ScopeLock Traceguard(_traceCS);
    _trace->Push(gps);
    _trace->Compress();
  }
  
  // OLC-Plus
  if(step % STEPS_NUM == 0) {
    // Solve OLC-Classic and FAI 3TPs
    SolvePoints(*_trace, false, false);
    SolveOLCPlus(false);
  }
  if(step % STEPS_NUM == 1) {
    // Find FAI-OLC loop
    _traceLoop->Clear();
    if(!BiggestLoopFind(*_trace, *_traceLoop, false))
      _traceLoop->Clear();
  }
  if(step % STEPS_NUM == 2) {
    // Solve FAI-OLC
    if(_traceLoop->Size()) {
      SolveFAITriangle(*_traceLoop, _prevFAIFront.get(), _prevFAIBack.get(), false);
      _prevFAIFront.reset(_traceLoop->Size() ? new CPointGPS(_traceLoop->Front()->GPS()) : 0);
      _prevFAIBack.reset(_traceLoop->Size() ? new CPointGPS(_traceLoop->Back()->GPS()) : 0);
      SolveOLCPlus(false);
    }
  }
  
  if(step % STEPS_NUM == 3) {
    // Solve OLC-Classic and FAI 3TPs for predicted path
    SolvePoints(*_trace, false, true);
    SolveOLCPlus(true);
  }
  if(step % STEPS_NUM == 4) {
    // Find FAI-OLC loop for predicted path
    _traceLoop->Clear();
    if(!BiggestLoopFind(*_trace, *_traceLoop, true))
      _traceLoop->Clear();
  }
  if(step % STEPS_NUM == 5) {
    // Solve FAI-OLC for predicted path
    if(_traceLoop->Size()) {
      SolveFAITriangle(*_traceLoop, _prevFAIPredictedFront.get(), _prevFAIPredictedBack.get(), true);
      _prevFAIPredictedFront.reset(_traceLoop->Size() ? new CPointGPS(_traceLoop->Front()->GPS()) : 0);
      _prevFAIPredictedBack.reset(_traceLoop->Size() ? new CPointGPS(_traceLoop->Back()->GPS()) : 0);
    }
    SolveOLCPlus(true);
  }

  // Update sprint trace
  _traceSprint->Push(gps);
  _traceSprint->Compress();
  
  // OLC-League
  if(step % STEPS_NUM == 6) {
    // Solve OLC-Sprint
    SolvePoints(*_traceSprint, true, false);
  }

  if ( AdditionalContestRule ) {

    if (step % STEPS_NUM == 7) {
      // Update XContest Free Triangle trace
      _traceFreeTriangle->Clear();
      if (!BiggestLoopFind(*_trace, *_traceFreeTriangle, true))
        _traceFreeTriangle->Clear();
    }

    if (step % STEPS_NUM == 8) {

      if (_traceFreeTriangle->Size()) {
        SolveFREETriangle(*_traceFreeTriangle,_prevFreeTriangleFront.get(), _prevFreeTriangleBack.get());
        _prevFreeTriangleFront.reset(_traceFreeTriangle->Size() ? new CPointGPS(_traceFreeTriangle->Front()->GPS()) : 0);
        _prevFreeTriangleBack.reset(_traceFreeTriangle->Size() ? new CPointGPS(_traceFreeTriangle->Back()->GPS()) : 0);
      }
      SolveXC();
    }
  }

  step++;
}



/** 
 * @brief Returns trace points
 * 
 * @param array GPS points array to fill
 */
void CContestMgr::Trace(CPointGPSArray &array) const
{
  ScopeLock guard(_traceCS);
  array.clear();
  array.reserve(_trace->Size());
  const CTrace::CPoint *point = _trace->Front();
  while(point) {
    array.push_back(point->GPS());
    point = point->Next();
  }
}

/**
 * @brief Sets all dummy  Contest results data
 *
 * @param
 */
void CContestMgr::SolveXC() {

  ScopeLock guard(_resultsCS);

  // FAI
  CResult &resfai = _resultArray[TYPE_OLC_FAI_PREDICTED];
  const double predicted_distance_fai = resfai.Distance();
  const double togo_distance_fai =  _fBestTogo;
  const double current_distance_fai = predicted_distance_fai - togo_distance_fai;
  const double score_fai = ScoreXC(current_distance_fai,predicted_distance_fai,XCFlightType::XC_FAI_TRIANGLE,true);
  _resultArray[TYPE_XC_FAI_TRIANGLE] =  CResult(TYPE_XC_FAI_TRIANGLE,_XCFAIStatus!=XCTriangleStatus::CLOSED,
                                                _XCFAIStatus==XCTriangleStatus::INVALID?0:current_distance_fai,
                                                current_distance_fai,predicted_distance_fai,score_fai,
                                                CPointGPSArray(_resultArray[TYPE_OLC_FAI_PREDICTED].PointArray()));

  // Free Triangle
  const double predicted_distance_ft = (double)_resultFREETriangle.PredictedDistance();
  const double current_distance_ft = (double)_resultFREETriangle.CurrentDistance();
  const double score_ft =  ScoreXC(current_distance_ft,predicted_distance_ft,XCFlightType::XC_FREE_TRIANGLE,true);
  _resultArray[TYPE_XC_FREE_TRIANGLE] =  CResult(TYPE_XC_FREE_TRIANGLE,_XCFTStatus!=XCTriangleStatus::CLOSED,
                                                 _XCFTStatus==XCTriangleStatus::INVALID?0:current_distance_ft,
                                                 current_distance_ft,predicted_distance_ft,score_ft,
                                                 CPointGPSArray(_resultFREETriangle.PointArray()));

  // Free flight
  CResult resff = _resultArray[TYPE_FAI_3_TPS];
  const double current_distance_ff  = resff.Distance();;
  const double score_ff = ScoreXC(current_distance_ff,current_distance_ff,XCFlightType::XC_FREE_FLIGHT,true);
  _resultArray[TYPE_XC_FREE_FLIGHT] =  CResult(TYPE_XC_FREE_FLIGHT,false,current_distance_ff,current_distance_ff,
                                               current_distance_ff,score_ff,
                                               CPointGPSArray(_resultArray[TYPE_FAI_3_TPS].PointArray()));

  // Combined Result
  if ( (score_fai >= score_ff) && (score_fai >= score_ft) ) {  // FAI
    _bestXCType = CContestMgr::XC_FAI_TRIANGLE;
    _resultArray[TYPE_XC] = CResult( _resultArray[TYPE_XC_FAI_TRIANGLE]);
  }
  else if ( (score_ff >= score_fai) && (score_ff >= score_ft) ) {   // Free Flight
    _bestXCType = CContestMgr::XC_FREE_FLIGHT;
    _resultArray[TYPE_XC] = CResult( _resultArray[TYPE_XC_FREE_FLIGHT]);
  }
  else {   // Free Triangle
    _bestXCType = CContestMgr::XC_FREE_TRIANGLE;
    _resultArray[TYPE_XC] = CResult( _resultArray[TYPE_XC_FREE_TRIANGLE]);
  }

  // Best Triangle data
  _bestXCTriangleType = XCFlightType::XC_INVALID;
  double predicted_score_fai = ScoreXC(predicted_distance_fai,predicted_distance_fai,XCFlightType::XC_FAI_TRIANGLE,false);
  double predicted_score_ft = ScoreXC(predicted_distance_ft,predicted_distance_ft,XCFlightType::XC_FREE_TRIANGLE,false);
  if (predicted_score_fai > 0 && predicted_score_fai>=predicted_score_ft) {
    _bestXCTriangleType = XCFlightType::XC_FAI_TRIANGLE;
    _XCTriangleClosurePercentage = 100. * ( (predicted_distance_fai-current_distance_fai) / predicted_distance_fai ) ;
    _XCTriangleClosureDistance = predicted_distance_fai - current_distance_fai;
    _XCTriangleDistance = predicted_distance_fai;

  }
  else if (predicted_score_ft > 0) {
    _bestXCTriangleType = XCFlightType::XC_FREE_TRIANGLE;
    _XCTriangleClosurePercentage = 100. * ( (predicted_distance_ft-current_distance_ft) / predicted_distance_ft ) ;
    _XCTriangleClosureDistance = predicted_distance_ft - current_distance_ft;
    _XCTriangleDistance = predicted_distance_ft;
  }

  // Mean Speed
  if ( resff.Duration() != 0 ) {
    _XCMeanSpeed = resff.Distance() / resff.Duration();
  }

  // Set the RESWP_FAIOPTIMIZED
  if ( _bestXCTriangleType == XCFlightType::XC_FAI_TRIANGLE) {
    short Alt = 0;
    RasterTerrain::Lock();
    RasterTerrain::SetTerrainRounding(0,0);
    Alt = RasterTerrain::GetTerrainHeight(_pgpsClosePoint.Latitude(),
                                          _pgpsClosePoint.Longitude());
    RasterTerrain::Unlock();
    WayPointList[RESWP_FAIOPTIMIZED].Latitude=_pgpsClosePoint.Latitude();
    WayPointList[RESWP_FAIOPTIMIZED].Longitude=_pgpsClosePoint.Longitude();
    WayPointList[RESWP_FAIOPTIMIZED].Altitude=Alt;
    if (WayPointList[RESWP_FAIOPTIMIZED].Altitude==0) WayPointList[RESWP_FAIOPTIMIZED].Altitude=0.001;
    WayPointList[RESWP_FAIOPTIMIZED].Reachable=TRUE;
    WayPointList[RESWP_FAIOPTIMIZED].Visible=TRUE;
    _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Comment,_T("Current FAI triangle closing point"));
    _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Name,_T("FAI Close"));
    _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Code,_T("FAI"));

  }
  else if ( _bestXCTriangleType == XCFlightType::XC_FREE_TRIANGLE) {
    short Alt = 0;
    RasterTerrain::Lock();
    RasterTerrain::SetTerrainRounding(0,0);
    Alt = RasterTerrain::GetTerrainHeight(_pgpsFreeTriangleClosePoint.Latitude(),
                                          _pgpsFreeTriangleClosePoint.Longitude());
    RasterTerrain::Unlock();
    WayPointList[RESWP_FAIOPTIMIZED].Latitude=_pgpsFreeTriangleClosePoint.Latitude();
    WayPointList[RESWP_FAIOPTIMIZED].Longitude=_pgpsFreeTriangleClosePoint.Longitude();
    WayPointList[RESWP_FAIOPTIMIZED].Altitude=Alt;
    if (WayPointList[RESWP_FAIOPTIMIZED].Altitude==0) WayPointList[RESWP_FAIOPTIMIZED].Altitude=0.001;
    WayPointList[RESWP_FAIOPTIMIZED].Reachable=TRUE;
    WayPointList[RESWP_FAIOPTIMIZED].Visible=TRUE;
    _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Comment,_T("Current Free triangle closing point"));
    _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Name,_T("TRI Close"));
    _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Code,_T("TRI"));
  }
  else {
    WayPointList[RESWP_FAIOPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
    WayPointList[RESWP_FAIOPTIMIZED].Reachable=false;
    WayPointList[RESWP_FAIOPTIMIZED].Visible=false;
    _stprintf(WayPointList[RESWP_FAIOPTIMIZED].Comment,_T("no triangle closing point found!"));
  }

}

/**
 * @brief Score the Flight
 *
 * @param current_distance distance flight so far
 * @param total_distance Second edge
 * @param type Type of flight
 *
 * @return @c return score
 */
double CContestMgr::ScoreXC(double current_distance,double total_distance ,CContestMgr::XCFlightType type, bool update_status) {

  double score = 0;

  switch (AdditionalContestRule) {
    case XContest2018:
      score =  ScoreXContest2018(current_distance, total_distance, type, update_status);
      break;
    case XContest2019:
      score =  ScoreXContest2019(current_distance, total_distance, type, update_status);
      break;
    case CFD:
      score =   ScoreCDF(current_distance, total_distance, type, update_status);
      break;
    case LEONARDO_XC:
      score =   ScoreLEONARDO_XC(current_distance, total_distance, type, update_status);
      break;
    case UK_NATIONAL_LEAGUE:
      score =   ScoreUK_NATIONAL_LEAGUE(current_distance, total_distance, type, update_status);
      break;
  }
  return score;
}

// XContest 2018
// A flight may be scored as a triangle, when the distance between start point and
// finish point (= closing distance) is less than 20% of the entire distance as given by the 3 turn points.
double CContestMgr::ScoreXContest2018(double current_distance, double total_distance, CContestMgr::XCFlightType type, bool update_status) {

  if (type == CContestMgr::XCFlightType::XC_FAI_TRIANGLE ) {
    if (total_distance == 0 ) {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double togo_distance = total_distance - current_distance;
    const double C = (togo_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::VALID;
      return (current_distance/1000.) * 1.4;
    }
  }
  else if (type == CContestMgr::XCFlightType::XC_FREE_TRIANGLE  ) {
    if (total_distance == 0 ) {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double C = (total_distance - current_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {
      if (update_status)  _XCFTStatus = XCTriangleStatus::VALID;
      return (current_distance/1000.) *   1.2;
    }
  }
  else  if (type == CContestMgr::XCFlightType::XC_FREE_FLIGHT ) {
    return current_distance/1000.;
  }

  if (update_status)  {
    _XCFTStatus = XCTriangleStatus::INVALID;
    _XCFAIStatus = XCTriangleStatus::INVALID;
  }

  return 0;
}


// XContest 2019
// A flight may be scored as a triangle, when the distance between start point and
// finish point (= closing distance) is less than 20% of the entire distance as given by the 3 turn points.
// A flight is considered closed if closing distance is less than 5%. Coefficients from XContest World Rules 2019
double CContestMgr::ScoreXContest2019(double current_distance, double total_distance, CContestMgr::XCFlightType type, bool update_status) {

  if (type == CContestMgr::XCFlightType::XC_FAI_TRIANGLE ) {
    if (total_distance == 0 ) {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double togo_distance = total_distance - current_distance;
    const double C = (togo_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {
      if (C < 0.05) {
        if (update_status)  _XCFAIStatus = XCTriangleStatus::CLOSED;
        return (current_distance/1000.) * 1.6;
      }
      else {
        if (update_status)  _XCFAIStatus = XCTriangleStatus::VALID;
        return (current_distance/1000.) * 1.4;
      }
    }
  }
  else if (type == CContestMgr::XCFlightType::XC_FREE_TRIANGLE  ) {
    if (total_distance == 0 ) {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double C = (total_distance - current_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {
      if (C < 0.05) {
        if (update_status)  _XCFTStatus = XCTriangleStatus::CLOSED;
        return (current_distance/1000.) * 1.4;
      }
      else {
        if (update_status)  _XCFTStatus = XCTriangleStatus::VALID;
        return (current_distance/1000.) *   1.2;
      }
    }
  }
  else  if (type == CContestMgr::XCFlightType::XC_FREE_FLIGHT ) {
    return current_distance/1000.;
  }

  if (update_status)  {
    _XCFTStatus = XCTriangleStatus::INVALID;
    _XCFAIStatus = XCTriangleStatus::INVALID;
  }

  return 0;
}

// CDF
// A flight may be scored as a triangle, when the distance between start point and
// finish point (= closing distance) is less than 5% ( VALID ) or 3 km ( CLOSED) of the entire distance as given by the 3 turn points.
// Pour tous les types de vol, l a distance minimale de vol prise en compte est de 15 kilomètres
double CContestMgr::ScoreCDF(double current_distance,double total_distance,CContestMgr::XCFlightType type, bool update_status) {

  if (type == CContestMgr::XCFlightType::XC_FAI_TRIANGLE ) {
    if (total_distance <  15000 ) {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double togo_distance = total_distance - current_distance;
    if ((togo_distance / total_distance) >= 0.05 && togo_distance >= 3000) {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    } else {
      if (togo_distance < 3000) {
        if (update_status)  _XCFAIStatus = XCTriangleStatus::CLOSED;
        return (current_distance * 1.4 / 1000.);
      } else {
        if (update_status)  _XCFAIStatus = XCTriangleStatus::VALID;
        return (total_distance / 1000.) * 1.4;
      }
    }
  }
  else if (type == CContestMgr::XCFlightType::XC_FREE_TRIANGLE  ) {
    if (total_distance == 0 ) {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double togo_distance = total_distance - current_distance;
    if ((togo_distance / total_distance) >= 0.05 && togo_distance >= 3000) {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    } else {
      if (togo_distance < 3000) {
        if (update_status)  _XCFTStatus = XCTriangleStatus::CLOSED;
        return (current_distance * 1.2 / 1000.);
      } else {
        if (update_status)  _XCFTStatus = XCTriangleStatus::VALID;
        return (total_distance / 1000.) * 1.2;
      }
    }
  } else if (type == CContestMgr::XCFlightType::XC_FREE_FLIGHT ) {
    return current_distance / 1000.;
  }

  if (update_status)  {
    _XCFTStatus = XCTriangleStatus::INVALID;
    _XCFAIStatus = XCTriangleStatus::INVALID;
  }

  return 0;
}

// Leonardo XC servers  (https://www.dhv-xc.de/leonardo/ , http://www.paraglidingforum.com/leonardo/tracks/world/alltimes/ )
// A flight may be scored as a triangle, when the distance between start point and
// finish point (= closing distance) is less than 20% of the entire distance as given by the 3 turn points.
double CContestMgr::ScoreLEONARDO_XC(double current_distance, double total_distance, CContestMgr::XCFlightType type, bool update_status) {

  if (type == CContestMgr::XCFlightType::XC_FAI_TRIANGLE ) {
    if (total_distance == 0 ) {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double togo_distance = total_distance - current_distance;
    const double C = (togo_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::VALID;
      return (current_distance/1000.) * 2.0;
    }
  }
  else if (type == CContestMgr::XCFlightType::XC_FREE_TRIANGLE  ) {
    if (total_distance == 0 ) {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double C = (total_distance - current_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {

      if (update_status)  _XCFTStatus = XCTriangleStatus::VALID;
      return (current_distance/1000.) *   1.75;

    }
  }
  else  if (type == CContestMgr::XCFlightType::XC_FREE_FLIGHT ) {
    return (current_distance/1000.) *   1.50 ;
  }

  if (update_status)  {
    _XCFTStatus = XCTriangleStatus::INVALID;
    _XCFAIStatus = XCTriangleStatus::INVALID;
  }

  return 0;
}

// UK NATIONAL LEAGUE  (http://www.xcleague.com/xc/info/rules-leagues.html)
// A flight may be scored as a triangle, when the distance between start point and
// finish point (= closing distance) is less than 20% of the entire distance as given by the 3 turn points.
// Flat triangles has a 15% minimum edge additional constrain and coefficient depends on total_distance
double CContestMgr::ScoreUK_NATIONAL_LEAGUE(double current_distance, double total_distance, CContestMgr::XCFlightType type, bool update_status) {

  if (type == CContestMgr::XCFlightType::XC_FAI_TRIANGLE ) {
    if (total_distance < 15000 ) {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double togo_distance = total_distance - current_distance;
    const double C = (togo_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {
      if (update_status)  _XCFAIStatus = XCTriangleStatus::VALID;
      if ( total_distance < 25000 )
        return (current_distance/1000.) * 1.7;
      else
        return (current_distance/1000.) * 2.0;
    }
  }
  else if (type == CContestMgr::XCFlightType::XC_FREE_TRIANGLE  ) {
    if (total_distance  < 15000 ) {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    const double C = (total_distance - current_distance) / total_distance;
    if ( C >= 0.2 )  {
      if (update_status)  _XCFTStatus = XCTriangleStatus::INVALID;
      return 0;
    }
    else {
      if (update_status)  _XCFTStatus = XCTriangleStatus::VALID;
      if ( total_distance < 35000 )
        return (current_distance/1000.) *   1.3;
      else
        return (current_distance/1000.) *   1.7;
    }
  }
  else  if (type == CContestMgr::XCFlightType::XC_FREE_FLIGHT ) {
    return ( current_distance/1000.) * 1.3  ;
  }

  if (update_status)  {
    _XCFTStatus = XCTriangleStatus::INVALID;
    _XCFAIStatus = XCTriangleStatus::INVALID;
  }

  return 0;
}





