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
// Reminder: to gain accuracy we cannot rely on the internal timers in Hz because
// they are only approximated.
//
void TripTimes(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static unsigned int steady=0;
  static double steady_start_time=0;
  static unsigned int old_trip_steady_time=0;

  if (LKSW_ResetTripComputer) {
	LKSW_ResetTripComputer=false;
	#if TESTBENCH
	StartupStore(_T("... Trip Computer RESET by request\n"));
	#endif
	Trip_Steady_Time=0;
	Trip_Moving_Time=0;
	steady_start_time=0;
	old_trip_steady_time=0;
	if (ISCAR) Calculated->FlightTime = 0; // see later in DetecStartTime also
	Calculated->Odometer = 0;
	Calculated->TakeOffTime = Basic->Time;
	if (ISCAR) LKSW_ResetLDRotary=true;
	steady=0;
  }

  if (Basic->NAVWarning) return;

  // For CAR mode, Flying is set true after the very first departure.
  // This is granting us a valid TakeOffTime .
  if (!Calculated->Flying) return;

  if ((steady_start_time>(Basic->Time+1))||(Basic->Time<Calculated->TakeOffTime)) {
	StartupStore(_T("... TripTimes back in time! Reset!\n"));
	DoStatusMessage(_T("TRIP COMPUTER RESET"));
	LKSW_ResetTripComputer=true;
	return;
  }

  if (Basic->Speed<0.1) {
	//
	// We are NOT moving
	//
	if (steady_start_time==0) steady_start_time=Basic->Time;
	steady=(unsigned int) (Basic->Time-steady_start_time);
	if (steady>=STEADY_MINTIME) {
		// we are really steady
		Trip_Steady_Time=old_trip_steady_time + steady;
	} 
  } else {
	// We are moving!
	// Logic is: if we moved at least in last 10 seconds, we count the entire period as moving.
	// This will smooth the slow-speed inhertial of gps fixes

	// first we reset predicted steady start time
	steady_start_time=Basic->Time+1;
	old_trip_steady_time=Trip_Steady_Time;
	Trip_Moving_Time= (unsigned int)Calculated->FlightTime - Trip_Steady_Time;
  }
}


int DetectStartTime(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // we want this to display landing time until next takeoff

  static int starttime = -1;
  static int lastflighttime = -1;

  // The DetectStartTime is called BEFORE we call TripTimes, which will take care of 
  // resetting to false the LKSW switch.
  if (LKSW_ResetTripComputer && ISCAR) {
	starttime=-1;
	lastflighttime=-1;
  }

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

  // Bad situation: current time appears to be gone back.
  // If we use two nmea sources, one of them may be stuck or faulty.
  // We must be very careful NOT to reset fligthstats in a real flight!
  // Ever.. 
  if ((Basic->Time != 0) && (Basic->Time <= LastTime)) {

	// If in real flight, never reset
	if (!ReplayLogger::IsEnabled() && Calculated->Flying) goto _noreset;

        // This can be changed in fact:
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

_noreset:
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

