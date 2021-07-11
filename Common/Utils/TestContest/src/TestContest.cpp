/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "TestContest.h"
#include "ReplayLogger.h"
#include "Tools.h"
#include <iostream>
#include <iomanip>



/** 
 * @brief Constructor
 * 
 * @param igcFile IGC file to analyse
 * @param handicap Glider handicap
 * @param startAltitudeLoss The loss of altitude needed to detect the start of powerless flight
 * @param interruptFix GPS fix at which a test should be interrupted
 */
CTestContest::CTestContest(const std::string &igcFile, unsigned handicap, unsigned startAltitudeLoss, unsigned interruptFix):
  _igcFile(igcFile),
  _handicap(handicap),
  _replay(CReplayLogger::Instance()),
  _kml(igcFile + ".kml"),
  _contestMgr(),
  _interruptFix(interruptFix),
  _maxIterProcessPeriod(0),
  _maxIterProcessTime(0),
  _startAltitudeLoss(startAltitudeLoss), 
  _startDetected(false), _startMaxAltitude(-1000)
{
  _contestMgr.Reset(handicap);
  std::wstring wname(_igcFile.begin(), _igcFile.end());
  _replay.Filename(wname.c_str());
  _replay.Register(this, GPSHandler);
}


/** 
 * @brief GPS data receive handler
 * 
 * Main entry point to the test application.
 *
 * @param user User context
 * @param time GPS fix time
 * @param latitude GPS fix latitude
 * @param longitude GPS fix longitude
 * @param altitude GPS fix altitude
 */
void CTestContest::GPSHandler(void *user, unsigned time, double latitude, double longitude, short altitude)
{
  if(latitude == 0 && longitude == 0)
    // filter invalid GPS fixes
    return;
  
  // obtain test object
  CTestContest *test = static_cast<CTestContest *>(user);
  
  if(test->_interruptFix && test->_contestMgr._trace->AnalyzedPointCount() > test->_interruptFix)
    // interrupt the test after requested number of GPS fixes
    return;
  
  if(!test->_startDetected) {
    if(test->_startAltitudeLoss > 0) {
      test->_startMaxAltitude = std::max(test->_startMaxAltitude, (int)altitude);
      if(altitude + test->_startAltitudeLoss > test->_startMaxAltitude)
        return;
    }
    test->_startDetected = true;
  }
  CTimeStamp iterBegin;
  
  // add new GPS point to the analysis
  test->_contestMgr.Add(new CPointGPS(time, latitude, longitude, altitude));
  
  CTimeStamp iterEnd;
  unsigned iterTime = iterEnd - iterBegin;

  // store the longest GPS fix processing period
  if(iterTime > test->_maxIterProcessPeriod) {
    test->_maxIterProcessPeriod = iterTime;
    test->_maxIterProcessTime = time;
  }
  
  // meassure performance
  unsigned analyzedPointCount = test->_contestMgr._trace->AnalyzedPointCount();
  if(analyzedPointCount && (analyzedPointCount % TIME_ANALYSIS_STEP == 0)) {
    putchar('.');
    fflush(stdout);
    test->_timeArray.push_back(CTimeStamp());
  }
}


/** 
 * @brief Dumps contest results
 * 
 * @param type The type of the contest to dump
 */
void CTestContest::Dump(const CContestMgr::TType type) const
{
  CContestMgr::CResult result = _contestMgr.Result(type, true);
  const CPointGPSArray &array = result.PointArray();
  unsigned distance = 0;
  if(array.size()) {
    bool triangle = (type == CContestMgr::TYPE_OLC_FAI || type == CContestMgr::TYPE_OLC_FAI_PREDICTED);
    for(unsigned i=0; i<array.size() - 1; i++) {
      if((i==0 || i==array.size() - 2) && triangle)
        continue;
      distance += array[i].Distance(array[i+1]);
    }
    if(triangle)
      distance += array[3].Distance(array[1]);
  }
  
  std::cout << std::endl;
  std::wstring typeStr = CContestMgr::TypeToString(type);
  std::cout << "Contest '" << std::string(typeStr.begin(), typeStr.end()) << "':" << std::endl;
  std::cout << " - Distance: " << result.Distance() << " (error: " << (int)distance - (int)result.Distance() << ")" << std::endl;
  std::cout << " - Score: " << result.Score() << std::endl;
  std::cout << " - Predicted: " << result.Predicted() << std::endl;
  for(CPointGPSArray::const_iterator it=result.PointArray().begin(); it!=result.PointArray().end(); ++it)
    std::cout << " - " << TimeToString(it->Time()) << std::endl;
 
  _kml.Dump(result);
}


/** 
 * @brief Main test run method
 */
void CTestContest::Run()
{
  std::cout << "Contests analysis for: " << _igcFile << " (handicap: " << _handicap << ")" << std::endl;
  
  // start replay
  _replay.Start();
  
  // store start timestamp
  _timeArray.push_back(CTimeStamp());
  
  while(_replay.Update()) {}
  putchar('\n');
  
  // store finish timestamp
  _timeArray.push_back(CTimeStamp());
  
  // dump compressed trace
  const CTrace &trace = *_contestMgr._trace;
  const CTrace &traceSprint = *_contestMgr._traceSprint;
  _kml.Dump(trace);
  //  std::cout << _trace << std::endl;
  
  // dump performance stats
  std::cout << std::endl;
  std::cout << "Performance stats:" << std::endl;
  std::cout << "------------------" << std::endl;
  std::cout << " - fix data size:                " << sizeof(CTrace::CPoint) + sizeof(unsigned) + sizeof(CPointGPS) << std::endl;
  std::cout << " - number of analysed fixes:     " << trace.AnalyzedPointCount() << std::endl;;
  std::cout << " - number of trace fixes:        " << trace.Size() << std::endl;;
  std::cout << " - number of FAI trace fixes:    " << CContestMgr::TRACE_TRIANGLE_FIX_LIMIT << std::endl;;
  std::cout << " - number of sprint trace fixes: " << traceSprint.Size() << std::endl;;
  std::cout << " - execution time:               " <<
    std::fixed << std::setprecision(2) << (_timeArray.back() - _timeArray.front()) / 1000.0 << "s" << std::endl;;
  std::cout << " - max iteration time:           " << _maxIterProcessPeriod << "ms (" << TimeToString(_maxIterProcessTime) << ")" << std::endl;
  if(_timeArray.size() > 2) {
    std::cout << " - execution time of " << TIME_ANALYSIS_STEP << " fixes periods: " << std::endl;
    
    for(unsigned i=1; i<_timeArray.size() - 1; i++)
      std::cout << "   * " << std::setw(5) << TIME_ANALYSIS_STEP * (i - 1) << " - " << std::setw(5) << TIME_ANALYSIS_STEP * i << ": " <<
        std::setw(4) << _timeArray[i] - _timeArray[i - 1] << "ms" << std::endl;
  }
  
  // dump contest results
  Dump(CContestMgr::TYPE_OLC_CLASSIC);
  Dump(CContestMgr::TYPE_OLC_FAI);
  Dump(CContestMgr::TYPE_OLC_PLUS);
  Dump(CContestMgr::TYPE_OLC_CLASSIC_PREDICTED);
  Dump(CContestMgr::TYPE_OLC_FAI_PREDICTED);
  Dump(CContestMgr::TYPE_OLC_PLUS_PREDICTED);
  Dump(CContestMgr::TYPE_OLC_LEAGUE);
  Dump(CContestMgr::TYPE_FAI_3_TPS);
  Dump(CContestMgr::TYPE_FAI_3_TPS_PREDICTED);
}
