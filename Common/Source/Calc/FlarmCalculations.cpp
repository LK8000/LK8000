/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "options.h"
#include "FlarmCalculations.h"

#include "utils/heapcheck.h"
#include "utils/stl_utils.h"
#include <algorithm>

FlarmCalculations::FlarmCalculations(void)
{
}

FlarmCalculations::~FlarmCalculations(void)
{
	std::for_each(averageCalculatorMap.begin(), averageCalculatorMap.end(), safe_delete());
}


double FlarmCalculations::Average30s(long flarmId, double curTime, double curAltitude)
{
  ClimbAverageCalculator *itemTemp = NULL;
  AverageCalculatorMap::iterator iterFind = averageCalculatorMap.find(flarmId);
  if( iterFind != averageCalculatorMap.end() )
    {
      itemTemp = averageCalculatorMap[flarmId];		
    }
  else
    {
      itemTemp = new ClimbAverageCalculator();
      averageCalculatorMap[flarmId] = itemTemp;		
    }
  return itemTemp->GetAverage(curTime, curAltitude, 30);
}

