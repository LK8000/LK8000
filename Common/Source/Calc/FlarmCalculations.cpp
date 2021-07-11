/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

*/

#include "options.h"
#include "FlarmCalculations.h"

#include "utils/stl_utils.h"
#include <algorithm>

FlarmCalculations::FlarmCalculations(void)
{
}

FlarmCalculations::~FlarmCalculations(void)
{
}


double FlarmCalculations::Average30s(uint32_t RadioId, double curTime, double curAltitude)
{
  return averageCalculatorMap[RadioId].GetAverage(curTime, curAltitude);
}
