/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "InputEvents.h"
#include "DoInits.h"
#include "MathFunctions.h"
#include "Radio.h"



extern double SpeedHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void TerrainFootprint(NMEA_INFO *Basic, DERIVED_INFO *Calculated);



void DoCalculationsSlow(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static double LastSearchBestTime = 0; 
  static bool	validHomeWaypoint=false;
  static bool	gotValidFix=false;

  if (DoInit[MDI_DOCALCULATIONSSLOW]) {
	LastSearchBestTime = 0; 
	validHomeWaypoint=false;
	gotValidFix=false;
	DoInit[MDI_DOCALCULATIONSSLOW]=false;
  }

  // See also same redundant check inside AirspaceWarning
  // calculate airspace warnings - multicalc approach embedded in CAirspaceManager
    CAirspaceManager::Instance().AirspaceWarning( Basic, Calculated);


    if (FinalGlideTerrain) {
        TerrainFootprint(Basic, Calculated);
    }

	// If we started a replay, we need to reset last time
	if (ReplayLogger::IsEnabled()) {
		if ( (Basic->Time - LastDoRangeWaypointListTime) <0 ) LastDoRangeWaypointListTime=0;
	}

	// Update search list only every x minutes :
	// At 180kmh in 10 minutes we do 30km so DSTRANGETURNPOINT to include in the nearest TP &co. 
	// should account 25km more than what we really want for range search each second 
	// Only if no data available, force action every 3 seconds
	// We are updating every 3 minutes, which makes it good also for GA 
	if (  (RangeLandableNumber<=0 && RangeTurnpointNumber<=0 && (Basic->Time > (LastDoRangeWaypointListTime + 3.0))) ||
	      (Basic->Time > (LastDoRangeWaypointListTime + 180.0)) ||
		((!validHomeWaypoint) && (Basic->Time > (LastDoRangeWaypointListTime + 15.0)))
	) {  


		// We are assigning TAKEOFF to Homewaypoint, so this is practically always true
		// but better keep it because we might not assign TAKEOFF all the times
		// Basically before a valid GPS fix is found, we are assigning either the DEM middle terrain position
		// or a real home waypoint position. Which is OK, but only until a real FIX is found!
		if (HomeWaypoint!=-1) validHomeWaypoint=true;

		if ( DoRangeWaypointList(Basic,Calculated) )
			LastDoRangeWaypointListTime=Basic->Time;

		if ( !Basic->NAVWarning ) gotValidFix=true;
	} else {
		// So we should not recalculate, apparently.. But did we ever get a valid fix?
		if (!gotValidFix) { // nope!
			if ( !Basic->NAVWarning ) { // and do we have a valid fix now?
				// YES, so recalculate to update the very first position and avoid waiting for 
				// 3 minutes until next pass!
				#if TESTBENCH
				StartupStore(_T("...... Got first valid FIX, we need to DoRangeWaypoint!\n"));
				#endif
				if ( DoRangeWaypointList(Basic,Calculated) )
					LastDoRangeWaypointListTime=Basic->Time;

				// Not necessary right now, we dont calculate magnetic bearings
				// CalculateMagneticVariation();

				gotValidFix=true;
			}
		}
		// else we should consider SIMMODE and PAN repositions , here! TODO
	}

	// watchout for replay files
	if (LastSearchBestTime > Basic->Time ) {
		LastSearchBestTime = Basic->Time - (BESTALTERNATEINTERVAL + 10);
	}

	if (Basic->Time > (LastSearchBestTime + BESTALTERNATEINTERVAL)) {
		LastSearchBestTime = Basic->Time;
		if (SearchBestAlternate(Basic, Calculated)) {
			AutomaticRadioStation({Basic->Latitude, Basic->Longitude});
		}
	}
}
