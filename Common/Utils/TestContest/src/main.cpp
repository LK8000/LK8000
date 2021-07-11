/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "TestContest.h"
#include "Tools.h"
#include <iostream>


int main(int argc, char *argv[])
{
  if(argc < 6) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  TestContest.exe IGC_FILE HANDICAP START_ALT_LOSS ALGORITHM INTERRUPT_FIX" << std::endl;
    std::cout << std::endl;
    std::cout << "ALGORITHM:" << std::endl;
    std::cout << " - 1: Triangles area" << std::endl;
    std::cout << " - 2: Distance lost" << std::endl;
    std::cout << " - 3: Distance lost + Inherited error" << std::endl;
    std::cout << std::endl;
    std::cout << "INTERRUPT_FIX = 0 - no interrupt" << std::endl;
    std::cout << std::endl;
    std::cout << "EXAMPLES:" << std::endl;
    std::cout << " TestContest-PC.exe Test_1.igc 126 50 2 0" << std::endl;
    std::cout << " TestContest-PC.exe Test_2.igc 126 50 3 0" << std::endl;
    return EXIT_FAILURE;
  }
  
  try {
    // switch(Convert<unsigned>(argv[4])) {
    // case 1:
    //   CContestMgr::COMPRESSION_ALGORITHM = CTrace::ALGORITHM_TRIANGLES | CTrace::ALGORITHM_TIME_DELTA;
    //   break;
    // case 2:
    //   CContestMgr::COMPRESSION_ALGORITHM = CTrace::ALGORITHM_DISTANCE  | CTrace::ALGORITHM_TIME_DELTA;
    //   break;
    // case 3:
    //   CContestMgr::COMPRESSION_ALGORITHM = CTrace::ALGORITHM_DISTANCE  | CTrace::ALGORITHM_INHERITED | CTrace::ALGORITHM_TIME_DELTA;
    //   break;
    // default:
    //   std::cerr << "Unknown algorithm ID=" << Convert<unsigned>(argv[4]) << " provided!!!" << std::endl;
    //   return EXIT_FAILURE;
    // }
    
    CTestContest test(argv[1], Convert<unsigned>(argv[2]), Convert<unsigned>(argv[3]), Convert<unsigned>(argv[5]));
    test.Run();
  }
  catch(const std::exception &ex) {
    std::cerr << "Not handled STD exception: " << ex.what() << "!!!" << std::endl;
    return EXIT_FAILURE;
  }
  catch(...) {
    std::cerr << "Not handled unknown exception!!!" << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
