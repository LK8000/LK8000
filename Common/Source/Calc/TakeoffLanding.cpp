/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "InputEvents.h"
#include "Waypointparser.h"




extern DERIVED_INFO Finish_Derived_Info;
extern void ResetFlightStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void DoAutoQNH(NMEA_INFO *Basic, DERIVED_INFO *Calculated);



void TakeoffLanding(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  static bool getTakeOffPosition=true;

  if (!Basic->NAVWarning) {

	if ( Basic->Speed> TakeOffSpeedThreshold ) {
		time_in_flight++;
		time_on_ground=0;
	} else {
		if ( (Calculated->AltitudeAGL<300)&&(Calculated->TerrainValid)&&(!ISPARAGLIDER) ) {
			time_in_flight--;
		} else if ( (!Calculated->TerrainValid) || ISPARAGLIDER) {
			time_in_flight--;
		}
		time_on_ground++;
		// Continue to update in order to have the most correct last point position and altitude
		// Do that only when we are not flying
		if ( getTakeOffPosition ) {
			WayPointList[RESWP_TAKEOFF].Latitude=Basic->Latitude;
			WayPointList[RESWP_TAKEOFF].Longitude=Basic->Longitude;
			// use NavAltitude

			WayPointList[RESWP_TAKEOFF].Altitude=0;
			WaypointAltitudeFromTerrain(&WayPointList[RESWP_TAKEOFF]);
			if(WayPointList[RESWP_TAKEOFF].Altitude == 0) {
				// if no terrain data, use GPS Altitude
				//RESWP_TAKEOFF.Altitude is used for AutoQNH, don't use baro for this...
				WayPointList[RESWP_TAKEOFF].Altitude=Basic->Altitude;
			}

			if (WayPointList[RESWP_TAKEOFF].Altitude==0) WayPointList[RESWP_TAKEOFF].Altitude=0.001; // 100227 BUGFIX?
			_tcscpy(WayPointList[RESWP_TAKEOFF].Comment,_T("LAST GROUND POSITION"));
			WayPointList[RESWP_TAKEOFF].Reachable=TRUE;
			WayPointList[RESWP_TAKEOFF].Visible=TRUE;
			if ( (!ValidWayPoint(HomeWaypoint)) || (ISPARAGLIDER && !SIMMODE) ) {
				HomeWaypoint=RESWP_TAKEOFF;
				TakeOffWayPoint=true;
			}
			if ((HomeWaypoint!=RESWP_TAKEOFF) && !ISPARAGLIDER) {
				TakeOffWayPoint=false;
			}
			WayPointList[RESWP_TAKEOFF].Format = LKW_VIRTUAL; 
			
		}
			
	}
  }


  // time_in_flight is always 0-60" or 90" and it is a countdown
  // time_on_ground is always 0-30"  counting up
  if (ISPARAGLIDER) {
	time_in_flight = min(600, max(0,time_in_flight));
	time_on_ground = min(30, max(0,time_on_ground));
  } else {
	if (ISCAR) {
		time_in_flight = min(900, max(0,time_in_flight));
		time_on_ground = min(30, max(0,time_on_ground));
	} else {
		time_in_flight = min(60, max(0,time_in_flight));
		time_on_ground = min(30, max(0,time_on_ground));
	}
  }

  // JMW logic to detect takeoff and landing is as follows:
  //   detect takeoff when above threshold speed for 10 seconds
  //
  //   detect landing when below threshold speed for xx seconds 
  //
  // TODO accuracy: make this more robust by making use of terrain height data 
  // if available

  if ((time_on_ground<=10)||(ReplayLogger::IsEnabled())) {
	// Don't allow 'OnGround' calculations if in IGC replay mode
	Calculated->OnGround = FALSE;
  }

  if (!Calculated->Flying) {
	// Check for an abnormal status
	if (LKSW_ForceLanding) {
		LKSW_ForceLanding=FALSE;
		#if TESTBENCH
		StartupStore(_T("... Not flying, but ForceLanding request found! Cleared.\n"));
		#endif
	}
	// detect takeoff
	if (time_in_flight>10) {
		InputEvents::processGlideComputer(GCE_TAKEOFF);
		_tcscpy(LANDINGWP_Name,_T(""));

		int j=FindNearestFarVisibleWayPoint(Basic->Longitude,Basic->Latitude,3000,WPT_UNKNOWN);
		if (j<0)
			_tcscpy(TAKEOFFWP_Name,_T("???"));
		else {
			_tcscpy(TAKEOFFWP_Name,WayPointList[j].Name);
		}

		StartupStore(_T(". TAKEOFF @%s from <%s>%s"), WhatTimeIsIt(),TAKEOFFWP_Name,NEWLINE);

		// Reset stats on takeoff, but not in SIM mode 
		// If replaying a file, we do reset of course.
		if (!SIMMODE || ReplayLogger::IsEnabled())
		  ResetFlightStats(Basic, Calculated);

		Calculated->Flying = TRUE;

		// For paragliders assign ff values here, because the ff is not calculated, being always true
		if (ISPARAGLIDER) {
			Calculated->FreeFlying=true;
			Calculated->FreeFlightStartTime=Basic->Time;
			Calculated->FreeFlightStartQNH=Basic->Altitude;
			Calculated->FreeFlightStartQFE=Basic->Altitude;
		}

		WasFlying=true;
		Calculated->TakeOffTime= Basic->Time;
		TakeOffWayPoint=true;
		// wait before getting a new takeoff until we are no more flying
		getTakeOffPosition=false;

		// save stats in case we never finish
		memcpy(&Finish_Derived_Info, Calculated, sizeof(DERIVED_INFO));

	}
	if (time_on_ground>10) {
		Calculated->OnGround = TRUE;
		DoAutoQNH(Basic, Calculated);
		// Do not reset QFE after landing.
		if (!WasFlying) QFEAltitudeOffset=ALTITUDEMODIFY*Calculated->NavAltitude;
	}
  } else {
	// detect landing
	if (LKSW_ForceLanding) {
		#if TESTBENCH
		StartupStore(_T("... Force Landing detected\n"));
		#endif
		time_in_flight=0;
		LKSW_ForceLanding=FALSE;
	}

	if (time_in_flight==0 && !ISCAR) { 
		// have been stationary for a minute
		InputEvents::processGlideComputer(GCE_LANDING);

		int j=FindNearestFarVisibleWayPoint(Basic->Longitude,Basic->Latitude,3000,WPT_UNKNOWN);
		if (j<0)
			_tcscpy(LANDINGWP_Name,_T("???"));
		else {
			_tcscpy(LANDINGWP_Name,WayPointList[j].Name);
		}

		StartupStore(_T(". LANDED @%s at <%s>%s"), WhatTimeIsIt(),LANDINGWP_Name,NEWLINE);
		UpdateLogBook(true); // we landed for sure

		// JMWX  restore data calculated at finish so
		// user can review flight as at finish line
		// VENTA3 TODO maybe reset WasFlying to false, so that QFE is reset
		// though users can reset by hand anyway anytime.. 

		if (Calculated->ValidFinish) {
			double flighttime = Calculated->FlightTime;
			double takeofftime = Calculated->TakeOffTime;
			memcpy(Calculated, &Finish_Derived_Info, sizeof(DERIVED_INFO));
			Calculated->FlightTime = flighttime;
			Calculated->TakeOffTime = takeofftime;
		}
		Calculated->Flying = FALSE;
		getTakeOffPosition=true;
	}

  }
}


