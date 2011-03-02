#include "TestContest.h"
#include "ReplayLogger.h"
#include <iostream>
#include <iomanip>


CTestContest::CTestContest(const std::string &igcFile, unsigned handicap, unsigned startAltitudeLoss):
  _igcFile(igcFile),
  _handicap(handicap),
  _replay(CReplayLogger::Instance()),
  _kml(igcFile + ".kml"),
  _contestMgr(handicap, startAltitudeLoss)
{
  std::wstring wname(_igcFile.begin(), _igcFile.end());
  _replay.Filename(wname.c_str());
  _replay.Register(this, GPSHandler);
}


void CTestContest::GPSHandler(void *user, double time, double latitude, double longitude, double altitude)
{
  CTestContest *test = static_cast<CTestContest *>(user);
  
  // add new GPS point to the analysis
  test->_contestMgr.Add(new CPointGPS(time, latitude, longitude, altitude));
  
  // meassure performance
  unsigned analyzedPointCount = test->_contestMgr.Trace().AnalyzedPointCount();
  if(analyzedPointCount && (analyzedPointCount % TIME_ANALYSIS_STEP == 0)) {
    putchar('.');
    fflush(stdout);
    test->_timeArray.push_back(CTimeStamp());
  }
}


void CTestContest::Run()
{
  std::cout << "Contests analysis for: " << _igcFile << " (handicap: " << _handicap << ")" << std::endl;
  
  // start replay
  _replay.Start();
  
  // store start timestamp
  _timeArray.push_back(CTimeStamp());
  
  while(_replay.Update()) {
    // if(_trace.PointCount() >= 30)
    //   break;
  }
  putchar('\n');
  
  // store finish timestamp
  _timeArray.push_back(CTimeStamp());
  
  // dump optimized trace
  const CTrace &trace = _contestMgr.Trace();
  _kml.Dump(trace);
  
  // dump performance stats
  std::cout << std::endl;
  std::cout << "Performance stats:" << std::endl;
  std::cout << "------------------" << std::endl;
  std::cout << " - fix data size:            " << sizeof(CTrace::CPoint) + sizeof(unsigned) + sizeof(CPointGPS) << std::endl;
  std::cout << " - number of analysed fixes: " << trace.AnalyzedPointCount() << std::endl;;
  std::cout << " - number of trace fixes:    " << trace.Size() << std::endl;;
  std::cout << " - execution time:           " <<
    std::fixed << std::setprecision(2) << (_timeArray.back() - _timeArray.front()) / 1000.0 << "s" << std::endl;;
  if(_timeArray.size() > 2) {
    std::cout << " - " << TIME_ANALYSIS_STEP << " fixes periods: " << std::endl;
    
    for(unsigned i=1; i<_timeArray.size() - 1; i++)
      std::cout << "   * " << std::setw(5) << TIME_ANALYSIS_STEP * (i - 1) << " - " << std::setw(5) << TIME_ANALYSIS_STEP * i << ": " <<
        std::setw(4) << _timeArray[i] - _timeArray[i - 1] << "ms" << std::endl;
  }
  
  std::cout << std::endl;
  //  std::cout << _trace << std::endl;
  
  const CContestMgr::CResult &result = _contestMgr.Result(CContestMgr::TYPE_OLC_CLASSIC);
  std::cout << "Result:" << std::endl;
  for(CPointGPSArray::const_iterator it=result.PointArray().begin(); it!=result.PointArray().end(); ++it)
    std::cout << " - " << TimeToString(it->Time()) << std::endl;
  
  std::cout << " - Distance: " << result.Distance() << std::endl;
  std::cout << " - Score: " << result.Score() << std::endl;
  
  _kml.Dump(result);
}
