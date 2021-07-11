/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "windanalyser.h"

extern WindAnalyser *windanalyser;

void  SetWindEstimate(const double wind_speed, 
		      const double wind_bearing, 
		      const int quality) {
  Vector v_wind;
  v_wind.x = wind_speed*cos(wind_bearing*3.1415926/180.0);
  v_wind.y = wind_speed*sin(wind_bearing*3.1415926/180.0);
  LockFlightData();
  if (windanalyser) {
    windanalyser->slot_newEstimate(&GPS_INFO, &CALCULATED_INFO, v_wind, quality);
  }
  UnlockFlightData();
}
