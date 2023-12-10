/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: ContestMgr.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __CONTESTMGR_H__
#define __CONTESTMGR_H__

//#define FIVEPOINT_OPTIMIZER

#include "Trace.h"
#include "NavFunctions.h"
#include <map>
#include <memory>


#ifdef TEST_CONTEST
class CTestContest;
#endif

/** 
 * @brief Contest Manager
 * 
 * CContestMgr class is responsible for all contests specific operations.
 * All calculations are triggered by providing a new GPS fix. Manager stores
 * those GPS fixes inside traces that are later used in contests results
 * calculations. Several different traces are needed to handle all the 
 * contest types. In order to save memory traces store smart references
 * to provided GPS fixes.
 *
 * Contest Manager remembers the best results obtained for each contest during
 * a flight.
 */


class CContestMgr {
public:
  /** 
   * @brief The type of contest
   */

  enum TType {
    TYPE_OLC_CLASSIC,                             /**< @brief OLC-Classic part of OLC-Plus */
    TYPE_OLC_FAI,                                 /**< @brief FAI-OLC part of OLC-Plus */
    TYPE_OLC_PLUS,                                /**< @brief OLC-Plus contest (score only) */
    TYPE_OLC_CLASSIC_PREDICTED,                   /**< @brief OLC-Classic predicted for returning to start */
    TYPE_OLC_FAI_PREDICTED,                       /**< @brief FAI-OLC part of OLC-Plus predicted for returning to start */
    TYPE_OLC_PLUS_PREDICTED,                      /**< @brief OLC-Plus predicted for returning to start (score only) */
    TYPE_OLC_LEAGUE,                              /**< @brief OLC-League (Sprint) */
    TYPE_FAI_3_TPS,                               /**< @brief FAI with 3 turnpoints */
    TYPE_FAI_3_TPS_PREDICTED,                     /**< @brief FAI with 3 turnpoints predicted for returning to start */
    TYPE_FAI_ASSISTANT,                           /**< @brief FAI ASSISTANT for the Analisys page */
    TYPE_XC_FREE_TRIANGLE,                         /**< @brief XC Free Triangle e valueted only in PG mode */
    TYPE_XC_FAI_TRIANGLE,                          /**< @brief XC FAI Triangle evalueted only in PG mode for score only */
    TYPE_XC_FREE_FLIGHT ,                          /**< @brief XC Free Flight evaluated only in PG mode for score only */
    TYPE_XC ,                                       /**< @brief XC total scoring evaluated only in PG mode for score only */
    TYPE_NUM,                                      /**< @brief Do not use it! */
    TYPE_INVALID = TYPE_NUM
  } ;

  enum class XCFlightType : uint8_t {
    XC_FREE_TRIANGLE,
    XC_FAI_TRIANGLE,
    XC_FREE_FLIGHT,
    XC_INVALID
  };

  enum class ContestRule : uint8_t {
    NONE,
    FAI_ASSISTANT,
    OLC,
    XContest2018,
    XContest2019,
    CFD,
    LEONARDO_XC,
    UK_NATIONAL_LEAGUE,
    NUM_OF_XC_RULES // must be the last !
  };

  enum class XCTriangleStatus : uint8_t {
    INVALID,
    VALID,
    CLOSED
  };


  struct TriangleLeg {
    int    LegIdx;  // 0 used as not jet valid. Valid will be 1st,2nd,3th leg
    double LegDist;
    double LegAngle;
    double Lat1;
    double Lon1;
    double Lat2;
    double Lon2;
    TriangleLeg() :LegIdx(0),LegDist(0), LegAngle(0), Lat1(0),Lon1(0), Lat2(0), Lon2(0) {};
    inline void FillLeg(const int& legN, const CPointGPS& firstPoint, const CPointGPS& secondPoint) {
      LegIdx = legN;
      Lat1 = firstPoint.Latitude();
      Lon1 = firstPoint.Longitude();
      Lat2 = secondPoint.Latitude();
      Lon2 = secondPoint.Longitude();
      DistanceBearing(firstPoint.Latitude(), firstPoint.Longitude(), secondPoint.Latitude(), secondPoint.Longitude(), &LegDist, &LegAngle);
    }
  };

  /**
   * @brief Contest Result
   * 
   * CContestMgr::CResult class stores the results of one contest.
   */
  class CResult {
    friend class CContestMgr;
    TType          _type = TYPE_INVALID;    /**< @brief The type of the contest (TYPE_INVALID if result invalid) */
    bool           _predicted = false;      /**< @brief @c true if a result is based on prediction */
    unsigned       _distance = 0;           /**< @brief Contest covered distance in meters 0 for XC if invalid triangle*/
    unsigned       _current_distance = 0;   /**< @brief Contest covered current distance in meters even if invalid*/
    unsigned       _predicted_distance = 0; /**< @brief Contest closed maximum distance in meters */
    float          _score = 0;              /**< @brief Contest score (if exists) */
    unsigned       _duration = 0;           /**< @brief Contest duration */
    float          _speed = 0;              /**< @brief Contest speed */
    CPointGPSArray _pointArray;             /**< @brief The list of contest result points */

    void Update();
  public:
    CResult() = default;
    CResult(TType type, bool predicted, unsigned distance, float score, const CPointGPSArray &pointArray);
    CResult(TType type, bool predicted, unsigned distance, unsigned current_distance, unsigned predicted_distance ,float score, const CPointGPSArray &pointArray);
    CResult(TType type, const CResult &ref);
    CResult(const CResult &ref, bool fillArray);
    TType Type() const         { return _type; }
    bool Predicted() const     { return _predicted; }
    unsigned Distance() const  { return _distance; }
    unsigned PredictedDistance() const  { return _predicted_distance; }
    float Score() const        { return _score; }
    unsigned Duration() const  { return _duration; }
    float Speed() const        { return _speed; }

    const CPointGPSArray &PointArray() const { return _pointArray; }
    void UpdateDistance(double current_distance) {
      _current_distance=current_distance;
      _duration = _pointArray.empty() ? 0 : (_pointArray.back().TimeDelta(_pointArray.front()));
      _speed = _pointArray.empty() ? 0 : ((float)current_distance / _duration);
    }
    // used only by TYPE_XC_FREE_TRIANGLE
    void UpdateDistancesAndArray( unsigned current_distance, unsigned predicted_distance, CPointGPSArray &&pointArray ){
      _type = TYPE_XC_FREE_TRIANGLE;
      _current_distance=current_distance;
      _predicted_distance=predicted_distance;
      std::swap(_pointArray, pointArray);      
      _duration = _pointArray.empty() ? 0 : (_pointArray.back().TimeDelta(_pointArray.front()));
      _speed = _pointArray.empty() ? 0 : ((float)_current_distance / _duration);
    };

  };
  
  typedef std::vector<CResult> CResultArray;
  
private:
#ifdef TEST_CONTEST
  friend class CTestContest;
#endif
  typedef std::unique_ptr<CTrace> CTracePtr;
  typedef std::multimap<unsigned, const CTrace::CPoint *> CDistanceMap;

  // Performance knobs
  static constexpr unsigned TRACE_FIX_LIMIT = 100;              /**< @brief The number of GPS fixes to store in the main trace */
  static constexpr unsigned TRACE_TRIANGLE_FIX_LIMIT = 20;      /**< @brief The number of GPS fixes to store in the FAI-OLC trace */
  static constexpr unsigned TRACE_SPRINT_FIX_LIMIT = 100;       /**< @brief The number of GPS fixes to store in the OLC-League trace */
  static constexpr unsigned TRACE_TRIANGLE_MIN_TIME = 5 * 60;   /**< @brief The minimum detected trace loop length
                                                                        (to filter out small thermalling circles) */
  static constexpr unsigned COMPRESSION_ALGORITHM = CTrace::ALGORITHM_DISTANCE | CTrace::ALGORITHM_TIME_DELTA; /**< @brief Traces compression algorithm */
  
  // Contest specific defines
  static constexpr short TRACE_START_FINISH_ALT_DIFF = 1000;    /**< @brief Maximum difference of altitude between contest start
                                                                        and end points */
  static constexpr unsigned TRACE_CLOSED_MAX_DIST = 1000;       /**< @brief Maximum distance between 2 GPS fixes that form a closed path */
  static constexpr unsigned TRACE_SPRINT_TIME_LIMIT = 150 * 60; /**< @brief Time limit for OLC-League trace */
  static constexpr unsigned TRACE_FAI_BIG_TRIANGLE_LENGTH = 500 * 1000; /**< @brief Minimum distance for big FAI triangle with lees strict rules */

  // Other
  static constexpr unsigned DEFAULT_HANDICAP = 100;

  static CContestMgr _instance;                       /**< @brief Singleton */
  unsigned _handicap = DEFAULT_HANDICAP;              /**< @brief Glider handicap */
  CTracePtr _trace = std::make_unique<CTrace>(TRACE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM); /**< @brief Main trace */
  CTracePtr _traceSprint = std::make_unique<CTrace>(TRACE_SPRINT_FIX_LIMIT, TRACE_SPRINT_TIME_LIMIT,
                                                    COMPRESSION_ALGORITHM); /**< @brief Trace for OLC-League */
  CTracePtr _traceFreeTriangle = std::make_unique<CTrace>(
      TRACE_TRIANGLE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM); /**< @brief Trace for XContest Free Triangle */
  CTracePtr _traceLoop =
      std::make_unique<CTrace>(TRACE_TRIANGLE_FIX_LIMIT, 0, COMPRESSION_ALGORITHM); /**< @brief Trace for OLC-League */

  std::unique_ptr<CPointGPS> _prevFAIFront;           /**< @brief Last reviewed OLC-FAI loop end points */
  std::unique_ptr<CPointGPS> _prevFAIBack;            /**< @brief Last reviewed OLC-FAI loop end points */
  std::unique_ptr<CPointGPS> _prevFAIPredictedFront;  /**< @brief Last reviewed OLC-FAI Predicted loop end points */
  std::unique_ptr<CPointGPS> _prevFAIPredictedBack;   /**< @brief Last reviewed OLC-FAI Predicted loop end points */
  std::unique_ptr<CPointGPS> _prevFreeTriangleFront;  /**< @brief Last reviewed XContest Free Triangle loop end points */
  std::unique_ptr<CPointGPS> _prevFreeTriangleBack;   /**< @brief Last reviewed XContest Free Triangle loop end points */

  XCFlightType     _bestXCType = XCFlightType::XC_INVALID;         /**< @brief the XC type that has maximum scoring */
  XCFlightType     _bestXCTriangleType = XCFlightType::XC_INVALID; // Best triangle type ( FAI and FT )
  XCTriangleStatus _XCFAIStatus = XCTriangleStatus::INVALID;       // Status of the current FAI best triangle ( INVALID,VALID,CLOSED)
  XCTriangleStatus _XCFTStatus = XCTriangleStatus::INVALID;        // Status of the current FREE best triangle ( INVALID,VALID,CLOSED)

  // Precalculated XC infobox values
  double _XCTriangleClosurePercentage = 0.;             // Percentage to go for  the current best XC triangle
  double _XCTriangleClosureDistance = 0.;               // Distance to go for  the current best XC triangle
  double _XCTriangleDistance = 0.;                      // Total predicted distance for  the current best XC triangle
  double _XCMeanSpeed = 0.;                             // XC  mean speed calculater on 3TP ( Like XCTrack does )

  CPointGPS _pgpsFreeTriangleClosePoint = {0, 0, 0, 0}; /**< @brief Point to close the Free Triangle on track*/
  double _fFreeTriangleTogo = 0.;                       /**< @brief current closing distance for XContest FREE TRIANGLE*/
  double _fFreeTriangleBestTogo = 0.;                   /**< @brief best achieved closing distance for XContest FREE TRIANGLE*/
  CPointGPS _pgpsFAITriangleClosePoint = {0, 0, 0, 0};  /**< @brief Point to close the FAI Triangle on track*/
  double _fFAITriangleTogo = 0.;                        /**< @brief current closing distance for XContest FAI TRIANGLE*/
  double _fFAITriangleBestTogo = 0.;                    /**< @brief best achieved closing distance for XContest FAI TRIANGLE*/

  bool _bFAI = false;                                   /**< @brief is the FREE Triangle a FAI one ?*/

  TriangleLeg* _maxFAILeg = nullptr;                    // pointer to the longest current leg of the triangle to be promoted into a FAI one
  bool _bLooksLikeAFAITriangle = false;                 /**< @brief does the  FREE Triangle looks like a FAI attempt ?*/
  int _dFAITriangleClockwise = 0;                       /**< @brief 1 clockwise. 1 counter clockwise*/

  mutable Mutex _mainCS;                                /**< @brief Main critical section that prevents Reset() and Add() at the same time */
  mutable Mutex _traceCS;                               /**< @brief Main trace critical section for returning _trace points */
  mutable Mutex _resultsCS;                             /**< @brief Contests results critical section for returning results */

  std::array<CResult, TYPE_NUM> _resultArray;           /**< @brief Array of results */
  std::array<TriangleLeg, 3> _faiAssistantTriangleLegs; /** To store data and speedup rendering of the FAI Assistant */
  CResult _resultFREETriangle = {};                     /**< @brief private results for  XContest Free Triangle */

  // member functions
  bool BiggestLoopFind(const CTrace &trace, const CTrace::CPoint *&start, const CTrace::CPoint *&end) const;
  bool BiggestLoopFind(const CTrace &traceIn, CTrace &traceOut, bool predicted) const;
  bool FAITriangleEdgeCheck(unsigned length1, unsigned length2, unsigned length3) const;
  bool FAITriangleEdgeCheck(unsigned length, unsigned best) const;
  void PointsResultOLC(TType type, const CTrace &traceResult);
  void SolvePoints(const CTrace &trace, bool sprint, bool predicted);
  void SolveFAITriangle(const CTrace &trace,const CPointGPS *prevFront,const CPointGPS *prevBack,bool predicted);
  void SolveFREETriangle(const CTrace &trace,const CPointGPS *prevFront,const CPointGPS *prevBack);
  void SolveOLCPlus(bool predicted);
  void SolveXC();
  double ScoreXC(double current_distance,double total_distance ,XCFlightType type, bool update_status);
  double ScoreXContest2018(double current_distance, double total_distance_, XCFlightType type, bool update_status);
  double ScoreXContest2019(double current_distance, double total_distance_, XCFlightType type, bool update_status);
  double ScoreCDF(double current_distance,double total_distance ,XCFlightType type, bool update_status);
  double ScoreLEONARDO_XC(double current_distance,double total_distance ,XCFlightType type, bool update_status);
  double ScoreUK_NATIONAL_LEAGUE(double current_distance,double total_distance ,XCFlightType type, bool update_status);
  double ScoreFAI(double current_distance,double total_distance ,XCFlightType type, bool update_status);
  bool FREETriangleEdgeCheck(unsigned length1, unsigned length2, unsigned length3)  const;
  void UpdateFAIAssistantData() ;
  void FindFAITriangleClosingPoint();
  void FindFREETriangleClosingPoint();

 public:

  CContestMgr() = default;

  TriangleLeg* GetFAIAssistantMaxLeg() {return _maxFAILeg;};
  TriangleLeg* GetFAIAssistantLeg(int i) {return &_faiAssistantTriangleLegs[i];};
  bool LooksLikeAFAITriangleAttempt() { return _bLooksLikeAFAITriangle;};
  int isFAITriangleClockwise() { return _dFAITriangleClockwise;};
  bool hasValidPath(TType type);
  static CContestMgr &Instance() { return _instance; }
  static const TCHAR *TypeToString(TType type);
  static const TCHAR *XCRuleToString(ContestRule type);

  // Used by old OLC functions and now just call XC function ( to be refactor . Tony  2019) )
  CPointGPS GetClosingPoint(void )         {return GetFAITriangleClosingPoint();};
  CPointGPS GetBestClosingPoint(void )     {return GetFAITriangleClosingPoint();};
  double GetClosingPointDist(void) { return GetFAITriangleClosingPointDist()  ;};
  double GetBestClosingPointDist(void) { return  GetFAITriangleBestClosingPointDist()  ;};
  bool FAI(void) {return _bFAI;};

  // Used by XC
  double GetFreeTriangleClosingPointDist(void) { return _fFreeTriangleTogo;};
  double GetFreeTriangleBestClosingPointDist(void) { return  _fFreeTriangleBestTogo ;};
  CPointGPS GetFreeTriangleClosingPoint(void )         {return _pgpsFreeTriangleClosePoint;};
  CPointGPS GetFAITriangleClosingPoint(void )         {return _pgpsFAITriangleClosePoint;};
  double GetFAITriangleClosingPointDist(void) { return _fFAITriangleTogo;};
  double GetFAITriangleBestClosingPointDist(void) { return  _fFAITriangleBestTogo ;};

  void Reset(unsigned handicap);
  void Add(unsigned time, double lat, double lon, int alt);

  CResult Result(TType type, bool fillArray) const;
  void Trace(CPointGPSArray &array) const;

  CContestMgr::XCFlightType GetBestXCType() {return _bestXCType;};
  CContestMgr::XCFlightType GetBestXCTriangleType() {return _bestXCTriangleType;};
  CContestMgr::XCTriangleStatus  GetFTTriangleStatus() { return _XCFTStatus;}
  CContestMgr::XCTriangleStatus  GetFAITriangleStatus() { return _XCFAIStatus;}
  double GetXCValidRadius();
  double GetXCClosedRadius();

  double GetXCTriangleClosureDistance(){return _XCTriangleClosureDistance;};
  double GetXCTriangleClosurePercentage(){return _XCTriangleClosurePercentage;};
  double GetXCTriangleDistance(){return _XCTriangleDistance;};
  double GetXCMeanSpeed(){return _XCMeanSpeed;};
  CPointGPS GetXCTriangleClosingPoint();
};

extern CContestMgr::ContestRule AdditionalContestRule;  	// Enum to Rules to use for the addition contest CContestMgr::ContestRule

/** 
 * @brief Primary constructor
 * 
 * @param type The type of the contest
 * @param predicted @c true if a result is based on prediction
 * @param distance Contest covered distance
 * @param score Contest score (if exists)
 * @param pointArray The list of contest result points
 */
inline CContestMgr::CResult::CResult(TType type, bool predicted, unsigned distance, float score, const CPointGPSArray &pointArray):
  _type(type), _predicted(predicted), _distance(distance),  _current_distance(0), _predicted_distance(0),_score(score),
  _duration(pointArray.empty() ? 0 : (pointArray.back().TimeDelta(pointArray.front()))),
  _speed(pointArray.empty() ? 0 : ((float)_distance / _duration)),
  _pointArray(pointArray)

{
}


/**
 * @brief  constructor for XContest
 *
 * @param type The type of the contest
 * @param predicted @c true if a result is based on prediction
 * @param distance Contest covered distance
 * @param total distance of the clsed triagle
 * @param score Contest score (if exists)
 * @param pointArray The list of contest result points
 */
inline CContestMgr::CResult::CResult(TType type, bool predicted, unsigned distance,unsigned current_distance, unsigned predicted_distance, float score, const CPointGPSArray &pointArray):
    _type(type), _predicted(predicted), _distance(distance),  _current_distance(current_distance),_predicted_distance(predicted_distance),_score(score),
    _duration(pointArray.empty() ? 0 : (pointArray.back().TimeDelta(pointArray.front()))),
    _speed(pointArray.empty() ? 0 : ((float)_distance / _duration)),
    _pointArray(pointArray)

{
}

/** 
 * @brief "Copy" constructor
 * 
 * @param type The type of the contest results
 * @param ref The results data to copy (beside type)
 */
inline CContestMgr::CResult::CResult(TType type, const CResult &ref):
  _type(type), _predicted(ref._predicted), _distance(ref._distance), _current_distance(ref._current_distance),_predicted_distance(ref._predicted_distance),
  _score(ref._score), _duration(ref._duration), _speed(ref._speed),
  _pointArray(ref._pointArray)
{
}


/** 
 * @brief "Copy" constructor
 * 
 * @param ref The results data to copy (beside type)
 * @param fillArray When @c true GPS points will be copied
 */
inline CContestMgr::CResult::CResult(const CResult &ref, bool fillArray):
  _type(ref._type), _predicted(ref._predicted), _distance(ref._distance), _current_distance(ref._current_distance) , _predicted_distance(ref._predicted_distance),
  _score(ref._score), _duration(ref._duration), _speed(ref._speed),
  _pointArray(fillArray ? ref._pointArray : CPointGPSArray())
{
}



/** 
 * @brief Recalculated contest results data
 */
inline void CContestMgr::CResult::Update()
{
  if(_pointArray.size()) {
    _duration = _pointArray.back().TimeDelta(_pointArray.front());
    _speed = (float)_distance / _duration;
  }
}


/** 
 * @brief Returns requested contest result
 * 
 * @param type Contest type to return
 * @param fillArray When @c true GPS points will be copied
 */
inline CContestMgr::CResult CContestMgr::Result(TType type, bool fillArray) const
{
  ScopeLock guard(_resultsCS);
  return CResult(_resultArray[type], fillArray);
}




#endif /* __CONTESTMGR_H__ */


