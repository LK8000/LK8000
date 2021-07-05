#ifndef FLARMCALCULATIONS_H
#define FLARMCALCULATIONS_H

#include <math.h>
#include <map>
#include <stdint.h>
#include "ClimbAverageCalculator.h"

class FlarmCalculations
{
public:
  FlarmCalculations(void);
  ~FlarmCalculations(void);
  double Average30s(uint32_t RadioId, double curTime, double curAltitude);
private:
  typedef std::map<uint32_t, ClimbAverageCalculator<30> > AverageCalculatorMap;
  AverageCalculatorMap averageCalculatorMap;
};

#endif
