/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __TESTCONTEST_H__
#define __TESTCONTEST_H__

#include "ContestMgr.h"
#include "TimeStamp.h"
#include <fstream>
#include <vector>


class CReplayLogger;


/** 
 * @brief Main contests test class
 */
class CTestContest {
  
  /** 
   * @brief Google Earth KML wrapper class
   */
  class CKMLWrapper {
    mutable std::ofstream _stream;
    
  public:
    CKMLWrapper(const std::string &path);
    ~CKMLWrapper();
    
    void Dump(const CTrace &trace) const;
    void Dump(const CContestMgr::CResult &result) const;
  };
  
  typedef std::vector<CTimeStamp> CTimeStampArray;
  
  static const unsigned TIME_ANALYSIS_STEP = 500;
  
  const std::string _igcFile;                     /**< @brief IGC file to analyse */
  const unsigned _handicap;                       /**< @brief Glider handicap */
  CReplayLogger &_replay;                         /**< @brief IGC file replay logger */
  CKMLWrapper _kml;                               /**< @brief Google Earth KML wraper */
  CContestMgr _contestMgr;                        /**< @brief Contests Manager */
  CTimeStampArray _timeArray;                     /**< @brief Performance measurements time array */
  unsigned _interruptFix;                         /**< @brief GPS fix at which a test should be interrupted */
  unsigned _maxIterProcessPeriod;                 /**< @brief Maximum iteration processing period */
  unsigned _maxIterProcessTime;                   /**< @brief Time of GPS fix of maximum iteration processing */
  short _startAltitudeLoss;                       /**< @brief The loss of altitude needed to detect the start of powerless flight */
  bool _startDetected;                            /**< @brief @c true if a start of powerless flight was detected */
  int _startMaxAltitude;                          /**< @brief The maximum altitude of a takeoff */
 
  static void GPSHandler(void *user, unsigned time, double latitude, double longitude, short altitude);
  
  void Dump(const CContestMgr::TType type) const;
  
public:
  CTestContest(const std::string &igcFile, unsigned handicap, unsigned startAltitudeLoss, unsigned interruptFix);
  void Run();
};

#endif /* __TESTCONTEST_H__ */
