#ifndef WINDZIGZAG_H
#define WINDZIGZAG_H

#include "lk8000.h"
#include "Parser.h"

int WindZigZagUpdate(NMEA_INFO* Basic, DERIVED_INFO* Calculated,
		      double *zzwindspeed, double *zzwindbearing);


#endif
