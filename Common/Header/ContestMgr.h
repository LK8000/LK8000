/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __CONTESTMGR_H__
#define __CONTESTMGR_H__

#include "Trace.h"
#include "CriticalSection.h"
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
    TYPE_OLC_CLASSIC_PREDICTED,                   /**< @brief OLC-Classic predicted for returning to start */
    TYPE_OLC_FAI,                                 /**< @brief FAI-OLC part of OLC-Plus */
    TYPE_OLC_PLUS,                                /**< @brief OLC-Plus contest (score only) */
    TYPE_OLC_PLUS_PREDICTED,                      /**< @brief OLC-Plus predicted for returning to start (score only) */
    TYPE_OLC_LEAGUE,                              /**< @brief OLC-League (Sprint) */
    TYPE_FAI_3_TPS,                               /**< @brief FAI with 3 turnpoints */
    TYPE_FAI_3_TPS_PREDICTED,                     /**< @brief FAI with 3 turnpoints predicted for returning to start */
    
    TYPE_NUM                                      /**< @brief Do not use it! */
  };
  
  /**
   * @brief Contest Result
   * 
   * CContestMgr::CResult class stores the results of one contest.
   */
  class CResult {
    TType          _type;                         /**< @brief The type of the contest (TYPE_NUM if result invalid) */
    unsigned       _distance;                     /**< @brief Contest covered distance */
    float          _score;                        /**< @brief Contest score (if exists) */
    CPointGPSArray _pointArray;                   /**< @brief The list of contest result points */
  public:
    CResult();
    CResult(TType type, unsigned distance, float score, const CPointGPSArray &pointArray);
    CResult(TType type, const CResult &ref);
    TType Type() const         { return _type; }
    unsigned Distance() const  { return _distance; }
    float Score() const        { return _score; }
    unsigned Duration() const  { return _pointArray.empty() ? 0 : (_pointArray.back().TimeDelta(_pointArray.front())); }
    float Speed() const        { return _pointArray.empty() ? 0 : ((float)_distance / Duration()); }
    const CPointGPSArray &PointArray() const { return _pointArray; }
  };
  
  typedef std::vector<CResult> CResultArray;
  
private:
#ifdef TEST_CONTEST
  friend class CTestContest;
#endif
  typedef std::auto_ptr<CTrace> CTracePtr;
  typedef std::multimap<unsigned, const CTrace::CPoint *> CDistanceMap;

  // Performance knobs
  static const unsigned TRACE_FIX_LIMIT = 200;              /**< @brief The number of GPS fixes to store in the main trace */
  static const unsigned TRACE_TRIANGLE_FIX_LIMIT = 30;      /**< @brief The number of GPS fixes to store in the FAI-OLC trace */
  static const unsigned TRACE_SPRINT_FIX_LIMIT = 100;       /**< @brief The number of GPS fixes to store in the OLC-League trace */
  static const unsigned TRACE_TRIANGLE_MIN_TIME = 10 * 60;  /**< @brief The minimum detected trace loop length
                                                                        (to filter out small thermalling circles) */
  static const unsigned COMPRESSION_ALGORITHM = CTrace::ALGORITHM_DISTANCE  | CTrace::ALGORITHM_TIME_DELTA; /**< @brief Traces compression algorithm */
  
  // Contest specific defines
  static const short TRACE_START_FINISH_ALT_DIFF = 1000;    /**< @brief Maximum difference of altitude between contest start
                                                                        and end points */
  static const unsigned TRACE_CLOSED_MAX_DIST = 1000;       /**< @brief Maximum distance between 2 GPS fixes that form a closed path */
  static const unsigned TRACE_SPRINT_TIME_LIMIT = 150 * 60; /**< @brief Time limit for OLC-League trace */
  static const unsigned TRACE_FAI_BIG_TRIANGLE_LENGTH = 500 * 1000; /**< @brief Minimum distance for big FAI triangle with lees strict rules */
  
  static CContestMgr _instance;                   /**< @brief Singleton */
  unsigned _handicap;                             /**< @brief Glider handicap */
  short _startAltitudeLoss;                       /**< @brief The loss of altitude needed to detect the start of powerless flight */
  bool _startDetected;                            /**< @brief @c true if a start of powerless flight was detected */
  int _startMaxAltitude;                          /**< @brief The maximum altitude of a takeoff */
  CTracePtr _trace;                               /**< @brief Main trace */
  CTracePtr _traceSprint;                         /**< @brief Trace for OLC-League */
  CTracePtr _traceLoop;                           /**< @brief Trace for FAI-OLC */
  CResultArray _resultArray;                      /**< @brief Array of results */
  
  mutable CCriticalSection _mainCS;               /**< @brief Main critical section that prevents Reset() and Add() at the same time */
  mutable CCriticalSection _traceCS;              /**< @brief Main trace critical section for returning _trace points */
  mutable CCriticalSection _resultsCS;            /**< @brief Contests results critical section for returning results */
  
  unsigned BiggestLoopFind(const CTrace &trace, const CTrace::CPoint *&start, const CTrace::CPoint *&end) const;
  bool BiggestLoopFind(const CTrace &traceIn, CTrace &traceOut) const;
  bool FAITriangleEdgeCheck(unsigned length, unsigned best) const;
  bool FAITriangleEdgeCheck(unsigned length1, unsigned length2, unsigned length3) const;
  void PointsResult(TType type, const CTrace &traceResult);
  void SolvePoints(const CTrace &trace, bool sprint, bool predicted);
  void SolveTriangle(const CTrace &trace, const CPointGPS *prevFront, const CPointGPS *prevBack);
  void SolveOLCPlus(bool predicted);
  
public:
  static const unsigned DEFAULT_START_ALTITUDE_LOSS = 25;
  static const unsigned DEFAULT_HANDICAP = 100;
  
  static CContestMgr &Instance() { return _instance; }
  static const TCHAR *TypeToString(TType type);
  
  CContestMgr(unsigned handicap = DEFAULT_HANDICAP, short startAltitudeLoss = DEFAULT_START_ALTITUDE_LOSS);
  
  void Reset(unsigned handicap, short startAltitudeLoss);
  void Add(const CPointGPSSmart &gps);
  
  void Result(TType type, CResult &result) const;
  void Trace(CPointGPSArray &array) const;
};

#endif /* __CONTESTMGR_H__ */
