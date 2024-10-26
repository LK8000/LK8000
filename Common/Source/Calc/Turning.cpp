/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "windanalyser.h"
#include "Atmosphere.h"
#include "ThermalLocator.h"
#include "LDRotaryBuffer.h"


extern void PercentCircling(NMEA_INFO *Basic, DERIVED_INFO *Calculated,const double Rate);
extern void SwitchZoomClimb(NMEA_INFO *Basic, DERIVED_INFO *Calculated, bool isclimb, bool left);
extern void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void ThermalBand(NMEA_INFO *Basic, DERIVED_INFO *Calculated);


#define MinTurnRate  4
#define CRUISE 0
#define WAITCLIMB 1
#define CLIMB 2
#define WAITCRUISE 3


extern WindAnalyser *windanalyser;
extern ThermalLocator thermallocator;

// #define DEBUGTURN 1

void Turning(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  static double LastTrack = 0;
  static double StartTime  = 0;
  static double StartLong = 0;
  static double StartLat = 0;
  static double StartAlt = 0;
  static double StartEnergyHeight = 0;
  static double LastTime = 0;
  static int MODE = CRUISE;
  static bool LEFT = FALSE;
  double Rate;
  static double LastRate=0;
  double dRate;
  double dT;

  if (!Calculated->Flying) {
	if (MODE!=CRUISE) {
		TestLog(_T(".... Not flying, still circling -> Cruise forced!"));
		goto _forcereset;
	}
	return;
  }

  // Back in time in IGC replay mode?
  if(Basic->Time <= LastTime) {
    TestLog(_T("...... Turning check is back in time. Now=%f Last=%f: reset %s"),Basic->Time, LastTime,WhatTimeIsIt());
_forcereset:
    LastTime = Basic->Time; // 101216 PV not sure of this.. 
    LastTrack = 0;
    StartTime  = 0;
    StartLong = 0;
    StartLat = 0;
    StartAlt = 0;
    StartEnergyHeight = 0;
    LastTime = 0;
    LEFT = FALSE;
    if (MODE!=CRUISE) {
	MODE = CRUISE;
        // Finally do the transition to cruise
        Calculated->Circling = false;
        SwitchZoomClimb(Basic, Calculated, false, LEFT);
        InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
    }
    return;
  }
  dT = Basic->Time - LastTime;
  LastTime = Basic->Time;

  BUGSTOP_LKASSERT(dT!=0);
  if (dT==0) dT=1;

  Rate = AngleLimit180(Basic->TrackBearing-LastTrack)/dT;

  #if DEBUGTURN
  StartupStore(_T("... Rate=%f  in time=%f\n"),Rate,dT);
  #endif

  if (dT<2.0 && dT!=0) {
    // time step ok

    // calculate acceleration
    dRate = (Rate-LastRate)/dT;

    double dtlead=0.3;
    // integrate assuming constant acceleration, for one second
    Calculated->NextTrackBearing = Basic->TrackBearing
      + dtlead*(Rate+0.5*dtlead*dRate);
    // s = u.t+ 0.5*a*t*t

    Calculated->NextTrackBearing = 
      AngleLimit360(Calculated->NextTrackBearing);
    
  } else {
    // time step too big, so just take it at last measurement
    Calculated->NextTrackBearing = Basic->TrackBearing;
  }

  Calculated->TurnRate = Rate;

  // JMW limit rate to 50 deg per second otherwise a big spike
  // will cause spurious lock on circling for a long time
  if (Rate>50) {
    Rate = 50;
  } 
  if (Rate<-50) {
    Rate = -50;
  }

  if (MODE==CLIMB||MODE==WAITCRUISE)
	Rate = LowPassFilter(LastRate,Rate,0.9);
  else
	Rate = LowPassFilter(LastRate,Rate,0.3);

  LastRate = Rate;

  if(Rate <0)
    {
      if (LEFT) {
        // OK, already going left
      } else {
        LEFT = true;
      }
      Rate *= -1;
    } else {
    if (!LEFT) {
      // OK, already going right
    } else {
      LEFT = false;
    }
  }

  PercentCircling(Basic, Calculated, Rate);

  LastTrack = Basic->TrackBearing;

  bool forcecruise = false;
  bool forcecircling = false;

  #if 1 // UNUSED, EnableExternalTriggerCruise not configurable, set to false since ever
  if (EnableExternalTriggerCruise ) {
    if (ExternalTriggerCruise && ExternalTriggerCircling) {
      // this should never happen
      ExternalTriggerCircling = false;
    }
    forcecruise = ExternalTriggerCruise;
    forcecircling = ExternalTriggerCircling;
  }
  #endif

  switch(MODE) {
  case CRUISE:

    double cruise_turnthreshold;
    if (ISPARAGLIDER)
	cruise_turnthreshold=5;
    else
	cruise_turnthreshold=4;

    if((Rate >= cruise_turnthreshold)||(forcecircling)) {
      // This is initialising the potential thermalling start
      // We still dont know if we are really circling for thermal
      StartTime = Basic->Time;
      StartLong = Basic->Longitude;
      StartLat  = Basic->Latitude;
      StartAlt  = Calculated->NavAltitude;
      StartEnergyHeight  = Calculated->EnergyHeight;
      #if DEBUGTURN
      StartupStore(_T("... CRUISE -> WAITCLIMB\n"));
      #endif
      MODE = WAITCLIMB;
    }
    if (forcecircling) {
      MODE = WAITCLIMB;
    } else {
      break;
    }
  case WAITCLIMB:
    if (forcecruise) {
      MODE = CRUISE;
      break;
    }

    double waitclimb_turnthreshold;
    double cruiseclimbswitch;
    if (ISPARAGLIDER) {
	waitclimb_turnthreshold=5;
        cruiseclimbswitch=15; // this should be finetuned for PGs
    } else {
	waitclimb_turnthreshold=4;
        cruiseclimbswitch=15; // this is ok for gliders
    }

    if((Rate >= waitclimb_turnthreshold)||(forcecircling)) {
      // WE CANNOT do this, because we also may need Circling mode to detect FF!!
      // if( (Calculated->FreeFlying && ((Basic->Time  - StartTime) > cruiseclimbswitch))|| forcecircling) {
       if( (!ISCAR && !ISGAAIRCRAFT && ((Basic->Time  - StartTime) > cruiseclimbswitch))|| forcecircling) { 

         #ifdef TOW_CRUISE
         // If free flight (FF) hasn�t yet been detected, then we may
         // still be on tow.  The following prevents climb mode from
         // engaging due to normal on-aerotow turns.

         if (!Calculated->FreeFlying && (fabs(Calculated->TurnRate) < 12))
           break;
         #endif

         #if DEBUGTURN
         StartupStore(_T("... WAITCLIMB -> CLIMB\n"));
         #endif
       Calculated->Circling = true;
        // JMW Transition to climb
        MODE = CLIMB;
	// Probably a replay flight, with fast forward with no cruise init
	if (StartTime==0) {
	      StartTime = Basic->Time;
	      StartLong = Basic->Longitude;
	      StartLat  = Basic->Latitude;
	      StartAlt  = Calculated->NavAltitude;
	      StartEnergyHeight  = Calculated->EnergyHeight;
	}
        Calculated->ClimbStartLat = StartLat;
        Calculated->ClimbStartLong = StartLong;
        Calculated->ClimbStartAlt = StartAlt+StartEnergyHeight;
        Calculated->ClimbStartTime = StartTime;
        
        if (flightstats.Altitude_Ceiling.sum_n>0) {
          // only update base if have already climbed, otherwise
          // we will catch the takeoff height as the base.

          flightstats.Altitude_Base.
            least_squares_update(max(0.0, Calculated->ClimbStartTime
                                     - Calculated->TakeOffTime)/3600.0,
                                 StartAlt);
        }
        
        SwitchZoomClimb(Basic, Calculated, true, LEFT);
        InputEvents::processGlideComputer(GCE_FLIGHTMODE_CLIMB);
      }
    } else {
      // nope, not turning, so go back to cruise
      #if DEBUGTURN
      StartupStore(_T("... WAITCLIMB -> CRUISE\n"));
      #endif
      MODE = CRUISE;
    }
    break;
  case CLIMB:
    if ( (AutoWindMode == D_AUTOWIND_CIRCLING) || (AutoWindMode==D_AUTOWIND_BOTHCIRCZAG) ) {
      LockFlightData();
      windanalyser->slot_newSample(Basic, Calculated);
      UnlockFlightData();
    }

    double climb_turnthreshold;
    if (ISPARAGLIDER)
	climb_turnthreshold=10;
    else
	climb_turnthreshold=4;
    
    if((Rate < climb_turnthreshold)||(forcecruise)) {
      StartTime = Basic->Time;
      StartLong = Basic->Longitude;
      StartLat  = Basic->Latitude;
      StartAlt  = Calculated->NavAltitude;
      StartEnergyHeight  = Calculated->EnergyHeight;
      // JMW Transition to cruise, due to not properly turning
      MODE = WAITCRUISE;
      #if DEBUGTURN
      StartupStore(_T("... CLIMB -> WAITCRUISE\n"));
      #endif
    }
    if (forcecruise) {
      MODE = WAITCRUISE;
    } else {
      break;
    }
  case WAITCRUISE:

    if (forcecircling) {
      MODE = CLIMB;
      break;
    }

    double waitcruise_turnthreshold;
    double climbcruiseswitch;
    if (ISPARAGLIDER) {
	waitcruise_turnthreshold=10;
        climbcruiseswitch=15;
    } else {
	waitcruise_turnthreshold=4;
        climbcruiseswitch=9; // ok for gliders
    }
    //
    // Exiting climb mode?
    //
    if((Rate < waitcruise_turnthreshold) || forcecruise) {

      if( ((Basic->Time  - StartTime) > climbcruiseswitch) || forcecruise) {

	// We are no more in climb mode

        if (StartTime==0) {
          StartTime = Basic->Time;
          StartLong = Basic->Longitude;
          StartLat  = Basic->Latitude;
          StartAlt  = Calculated->NavAltitude;
          StartEnergyHeight  = Calculated->EnergyHeight;
        }
        Calculated->CruiseStartLat  = StartLat;
        Calculated->CruiseStartLong = StartLong;
        Calculated->CruiseStartAlt  = StartAlt;
        Calculated->CruiseStartTime = StartTime;

	// Here we assign automatically this last thermal to the L> multitarget
	if (Calculated->ThermalGain >100) {

		// Force immediate calculation of average thermal, it would be made
		// during next cycle, but we need it here immediately
		AverageThermal(Basic,Calculated);

		if (EnableThermalLocator) {
			InsertThermalHistory(Calculated->ClimbStartTime, 
				Calculated->ThermalEstimate_Latitude, Calculated->ThermalEstimate_Longitude,
				Calculated->ClimbStartAlt, Calculated->NavAltitude, Calculated->AverageThermal);
		} else {
			InsertThermalHistory(Calculated->ClimbStartTime, 
				Calculated->ClimbStartLat, Calculated->ClimbStartLong, 
				Calculated->ClimbStartAlt, Calculated->NavAltitude, Calculated->AverageThermal);
		}

	}

	rotaryLD.Init();
	InitWindRotary(&rotaryWind);
        
        flightstats.Altitude_Ceiling.
          least_squares_update(max(0.0, Calculated->CruiseStartTime
                                   - Calculated->TakeOffTime)/3600.0,
                               Calculated->CruiseStartAlt);
        
        // Finally do the transition to cruise
        Calculated->Circling = false;
        MODE = CRUISE;
        #if DEBUGTURN
        StartupStore(_T("... WAITCRUISE -> CRUISE\n"));
        #endif
        SwitchZoomClimb(Basic, Calculated, false, LEFT);
        InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);

      } // climbcruiseswitch time in range

    } else { // Rate>Minturnrate, back to climb, turning again
      #if DEBUGTURN
      StartupStore(_T("... WAITCRUISE -> CLIMB\n"));
      #endif
      MODE = CLIMB;
    }
    break;
  default:
    // error, go to cruise
    MODE = CRUISE;
  }
  // generate new wind vector if altitude changes or a new
  // estimate is available
  if (AutoWindMode>D_AUTOWIND_MANUAL && AutoWindMode <D_AUTOWIND_EXTERNAL) {
    LockFlightData();
    windanalyser->slot_Altitude(Basic, Calculated);
    UnlockFlightData();
  }

  if (EnableThermalLocator) {
    if (Calculated->Circling) {
      thermallocator.AddPoint(Basic->Time, Basic->Longitude, Basic->Latitude,
			      Calculated->NettoVario);
      thermallocator.Update(Basic->Time, Basic->Longitude, Basic->Latitude,
			    Calculated->WindSpeed, Calculated->WindBearing,
			    Basic->TrackBearing,
			    &Calculated->ThermalEstimate_Longitude,
			    &Calculated->ThermalEstimate_Latitude,
			    &Calculated->ThermalEstimate_W,
			    &Calculated->ThermalEstimate_R);
    } else {
      Calculated->ThermalEstimate_W = 0;
      Calculated->ThermalEstimate_R = -1;
      thermallocator.Reset();
    }
  }

  // update atmospheric model
  CuSonde::updateMeasurements(Basic, Calculated);

}



void PercentCircling(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double Rate) {

  if (Calculated->Circling && (Rate>MinTurnRate)) {
    Calculated->timeCircling+= 1.0;
    Calculated->TotalHeightClimb += Calculated->GPSVario;
    if (ThermalBar)
        ThermalBand(Basic, Calculated);
  } else {
    Calculated->timeCruising+= 1.0;
  }

  if (Calculated->timeCruising+Calculated->timeCircling>1) {
    Calculated->PercentCircling =
      100.0*(Calculated->timeCircling)/(Calculated->timeCruising+ Calculated->timeCircling);
  } else {
    Calculated->PercentCircling = 0.0;
  }
}



void SwitchZoomClimb(NMEA_INFO *Basic, DERIVED_INFO *Calculated, bool isclimb, bool left) {

  if ( (AutoWindMode == D_AUTOWIND_CIRCLING) || (AutoWindMode==D_AUTOWIND_BOTHCIRCZAG) ) {
    LockFlightData();
    windanalyser->slot_newFlightMode(Basic, Calculated, left, 0);
    UnlockFlightData();
  }
}



