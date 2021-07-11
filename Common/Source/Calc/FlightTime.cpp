/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
static
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
	if (ISCAR) Calculated->FlightTime = 0; // see later in FlightDuration also
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
	DoStatusMessage(MsgToken(1527)); // TRIP COMPUTER RESET
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

static
int FlightDuration(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static double FlightTime = 0;
  /*
   * Flight duration only depends of TakeoffTime and CurrentTime
   *
   * To display flight time until next takeoff after landing, duration
   * is only updated when flying.
   */
  if (Calculated->Flying) {
    FlightTime = Basic->Time - Calculated->TakeOffTime;
  }
  return FlightTime;
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


  TakeoffLanding(Basic, Calculated);

  Calculated->FlightTime = FlightDuration(Basic, Calculated);

  if (ISCAR) {
    TripTimes(Basic, Calculated);
  }

  return true;
}

