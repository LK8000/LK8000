/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"


extern void ResetFlightStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void TakeoffLanding(NMEA_INFO *Basic, DERIVED_INFO *Calculated);


int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // we want this to display landing time until next takeoff

  static int starttime = -1;
  static int lastflighttime = -1;

  if (Calculated->Flying) {
    if (starttime == -1) {
      // hasn't been started yet

      starttime = (int)Basic->Time;

      lastflighttime = -1;
    }
    return (int)Basic->Time-starttime;

  } else {

    if (lastflighttime == -1) {
      // hasn't been stopped yet
      if (starttime>=0) {
        lastflighttime = (int)Basic->Time-starttime;
      } else {
        return 0; // no last flight time
      }
      // reset for next takeoff
      starttime = -1;
    }
  }

  // return last flighttime if it exists
  return max(0,lastflighttime);
}



bool FlightTimes(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static double LastTime = 0;

  if ((Basic->Time != 0) && (Basic->Time <= LastTime)) {
	if ( (LastTime - Basic->Time >30 ) && (!Basic->NAVWarning) && 
	// replay logger does not consider UTC 00 incrementing by 85000 or whatever the basic time.
	// Meanwhile, we may also skip the 00 utc because of interpolation. So we consider this.
	!( ReplayLogger::IsEnabled() && Basic->Time<60)  ) {
		// Reset statistics.. (probably due to being in IGC replay mode)
		StartupStore(_T("... Time is in the past! %s%s"), WhatTimeIsIt(),NEWLINE);

		ResetFlightStats(Basic, Calculated);
		time_in_flight=0;
		time_on_ground=0;
	}

	LastTime = Basic->Time; 
	return false;      
  }

  LastTime = Basic->Time;

  double t = DetectStartTime(Basic, Calculated);
  if (t>0) {
	Calculated->FlightTime = t;
  } 
  #if 0
  else {
	if (Calculated->Flying) {
		StartupStore(_T("... negative start time=%f\n"),t);
	}
  }
  #endif

  TakeoffLanding(Basic, Calculated);

  return true;
}

