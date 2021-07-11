/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"

extern double CRUISE_EFFICIENCY;

// Current Waypoint calculations for task (no safety?) called only once at beginning
// of DoCalculations, using MACCREADY
void AltitudeRequired(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                      const double this_maccready)
{
  //  LockFlightData();
  (void)Basic;
  LockTaskData();
  if(ValidTaskPoint(ActiveTaskPoint))
    {
 	int index;
      double wp_alt = FAIFinishHeight(Basic, Calculated, ActiveTaskPoint); 
      double height_above_wp = Calculated->NavAltitude + Calculated->EnergyHeight - wp_alt;

      Calculated->NextAltitudeRequired = GlidePolar::MacCreadyAltitude(this_maccready,
                        Calculated->WaypointDistance,
                        Calculated->WaypointBearing, 
                        Calculated->WindSpeed, Calculated->WindBearing, 
                        0, 0, 
			true,
			NULL, height_above_wp, CRUISE_EFFICIENCY
                        );

	if (this_maccready==0 ) Calculated->NextAltitudeRequired0=Calculated->NextAltitudeRequired;
        else
	      Calculated->NextAltitudeRequired0 = GlidePolar::MacCreadyAltitude(0,
				Calculated->WaypointDistance,
				Calculated->WaypointBearing, 
				Calculated->WindSpeed, Calculated->WindBearing, 
				0, 0, 
				true,
				NULL, height_above_wp, CRUISE_EFFICIENCY
				);


      Calculated->NextAltitudeRequired += wp_alt;
      Calculated->NextAltitudeRequired0 += wp_alt; // VENTA6

      Calculated->NextAltitudeDifference = Calculated->NavAltitude + Calculated->EnergyHeight - Calculated->NextAltitudeRequired;
      Calculated->NextAltitudeDifference0 = Calculated->NavAltitude + Calculated->EnergyHeight - Calculated->NextAltitudeRequired0;

	// We set values only for current destination active waypoint.
	index=TASKINDEX;
	WayPointCalc[index].AltArriv[ALTA_MC]=1111.0;
	WayPointCalc[index].AltArriv[ALTA_SMC]=2222.0; // FIX 091012
	WayPointCalc[index].AltArriv[ALTA_MC0]=3333.0;
	WayPointCalc[index].AltArriv[ALTA_AVEFF]=1234.0;

    }
  else
    {
      Calculated->NextAltitudeRequired = 0;
      Calculated->NextAltitudeDifference = 0;
      Calculated->NextAltitudeDifference0 = 0; // VENTA6 
    }
  UnlockTaskData();
  //  UnlockFlightData();
}
