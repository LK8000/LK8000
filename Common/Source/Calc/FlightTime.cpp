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

#define STEADY_MINTIME	10	// seconds

//
// This is good for car/trekking mode mainly
//
void TripTimes(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static unsigned int steady=0;

  if (Basic->NAVWarning) return;

  // For CAR mode, Flying is set true after the very first departure.
  if (!Calculated->Flying) return;

  if (Basic->Speed<0.1) {
	//
	// We are NOT moving
	//
	steady++;
	if (steady==STEADY_MINTIME) Trip_Steady_Time+=STEADY_MINTIME;
	if (steady >STEADY_MINTIME) Trip_Steady_Time++;
  } else {
	//
	// We are moving!
	//
	// Logic is: if we moved at least in last 10 seconds, we count the entire period as moving.
	// This will smooth the slow-speed inhertial of gps fixes
	if (steady>0 && steady<STEADY_MINTIME) {
		Trip_Moving_Time+=steady;
	}
	Trip_Moving_Time++;
	steady=0;
  }
}


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

  if (ISCAR) TripTimes(Basic, Calculated);

  return true;
}

