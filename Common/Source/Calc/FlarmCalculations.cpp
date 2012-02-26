/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

*/

#include "options.h"
#include "FlarmCalculations.h"

#include "utils/heapcheck.h"

FlarmCalculations::FlarmCalculations(void)
{
}

FlarmCalculations::~FlarmCalculations(void)
{
  for (AverageCalculatorMap::iterator i = map.begin(),
       end = map.end(); i != end; ++i)
    delete i->second;
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

