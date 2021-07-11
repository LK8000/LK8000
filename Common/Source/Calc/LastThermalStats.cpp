/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "ThermalLocator.h"

#define THERMAL_TIME_MIN 45.0
ThermalLocator thermallocator;


static void ThermalSources(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  double ground_longitude;
  double ground_latitude;
  double ground_altitude;
  thermallocator.
    EstimateThermalBase(
			Calculated->ThermalEstimate_Longitude,
			Calculated->ThermalEstimate_Latitude,
			Calculated->NavAltitude,
			Calculated->LastThermalAverage,
			Calculated->WindSpeed, 
			Calculated->WindBearing,
			&ground_longitude,
			&ground_latitude,
			&ground_altitude
			);
  
  if (ground_altitude>0) {
    double tbest=0;
    int ibest=0;

    for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
      if (Calculated->ThermalSources[i].LiftRate<0.0) {
	ibest = i;
	break;
      }
      double dt = Basic->Time - Calculated->ThermalSources[i].Time;
      if (dt> tbest) {
	tbest = dt;
	ibest = i;
      }
    }
    Calculated->ThermalSources[ibest].LiftRate = 
      Calculated->LastThermalAverage;
    Calculated->ThermalSources[ibest].Latitude = ground_latitude;
    Calculated->ThermalSources[ibest].Longitude = ground_longitude;
    Calculated->ThermalSources[ibest].GroundHeight = ground_altitude;
    Calculated->ThermalSources[ibest].Time = Basic->Time;
  }
}




void LastThermalStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static bool LastCircling = false;

  if((!Calculated->Circling) && (LastCircling)
     && (Calculated->ClimbStartTime>=0))
    {
      double ThermalTime = Calculated->CruiseStartTime 
        - Calculated->ClimbStartTime;
                                      
      if(ThermalTime >0)
        {
          double ThermalGain = Calculated->CruiseStartAlt + Calculated->EnergyHeight
            - Calculated->ClimbStartAlt;

          if (ThermalGain>0) {
            if (ThermalTime>THERMAL_TIME_MIN) {

	      Calculated->LastThermalAverage = ThermalGain/ThermalTime;
	      Calculated->LastThermalGain = ThermalGain;
	      Calculated->LastThermalTime = ThermalTime;

              flightstats.ThermalAverage.
                least_squares_update(Calculated->LastThermalAverage);

#ifdef DEBUG_STATS
              DebugStore("%f %f # thermal stats\n",
                      flightstats.ThermalAverage.m,
                      flightstats.ThermalAverage.b
                      );
#endif
              if (EnableThermalLocator) {
                ThermalSources(Basic, Calculated);
              }
            }
	  }
	}
    }
  LastCircling = Calculated->Circling;
}
