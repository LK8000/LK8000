/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "windanalyser.h"
#include "ThermalHistory.h"

extern void ResetFlightStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

WindAnalyser *windanalyser = NULL;

void CloseCalculations() {
  LockFlightData();    
  if (windanalyser) {
    delete windanalyser;
    windanalyser = NULL;
  }
  UnlockFlightData();
}


void InitCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  TestLog(TEXT(". Init Calculations"));

  ResetFlightStats(Basic, Calculated);

  // Initialise calculations, on first run DoInit will make it and return
  DoRangeWaypointList(Basic,Calculated);
  DoTraffic(Basic,Calculated);
  DoAirspaces(Basic,Calculated);
  DoThermalHistory(Basic,Calculated);

  InitAlarms();

  LockFlightData();
  if (!windanalyser) {
    windanalyser = new WindAnalyser();
  }
  UnlockFlightData();

}

