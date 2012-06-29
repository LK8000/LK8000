/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "windanalyser.h"
#include "Atmosphere.h"
#include "ThermalLocator.h"

#include "utils/heapcheck.h"

using std::min;
using std::max;

extern void PercentCircling(NMEA_INFO *Basic, DERIVED_INFO *Calculated,const double Rate);
extern double LowPassFilter(double y_last, double x_in, double fact);
extern void SwitchZoomClimb(NMEA_INFO *Basic, DERIVED_INFO *Calculated, bool isclimb, bool left);
extern void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void ThermalBand(NMEA_INFO *Basic, DERIVED_INFO *Calculated);


#define MinTurnRate  4
#define CruiseClimbSwitch 15
#define ClimbCruiseSwitch 10
#define CRUISE 0
#define WAITCLIMB 1
#define CLIMB 2
#define WAITCRUISE 3


extern WindAnalyser *windanalyser;
extern ThermalLocator thermallocator;



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
		#if TESTBENCH
		StartupStore(_T(".... Not flying, still circling -> Cruise forced!\n"));
		#endif
		goto _forcereset;
	}
	return;
  }

  // Back in time in IGC replay mode?
  if(Basic->Time <= LastTime) {
    #if TESTBENCH
    StartupStore(_T("...... Turning check is back in time: reset\n"));
    #endif
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
        Calculated->Circling = FALSE;
        SwitchZoomClimb(Basic, Calculated, false, LEFT);
        InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
    }
    return;
  }
  dT = Basic->Time - LastTime;
  LastTime = Basic->Time;

  Rate = AngleLimit180(Basic->TrackBearing-LastTrack)/dT;

  if (dT<2.0) {
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

  // average rate, to detect essing
  static double rate_history[60];
  double rate_ave=0;
  for (int i=59; i>0; i--) {
    rate_history[i] = rate_history[i-1];
    rate_ave += rate_history[i];
  }
  rate_history[0] = Rate;
  rate_ave /= 60;
  
  Calculated->Essing = fabs(rate_ave)*100/MinTurnRate;

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
    if((Rate >= MinTurnRate)||(forcecircling)) {
      // This is initialising the potential thermalling start
      // We still dont know if we are really circling for thermal
      StartTime = Basic->Time;
      StartLong = Basic->Longitude;
      StartLat  = Basic->Latitude;
      StartAlt  = Calculated->NavAltitude;
      StartEnergyHeight  = Calculated->EnergyHeight;
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
    if((Rate >= MinTurnRate)||(forcecircling)) {
      // WE CANNOT do this, because we also may need Circling mode to detect FF!!
      // if( (Calculated->FreeFlying && ((Basic->Time  - StartTime) > CruiseClimbSwitch))|| forcecircling) {
       if( (!ISCAR && !ISGAAIRCRAFT && ((Basic->Time  - StartTime) > CruiseClimbSwitch))|| forcecircling) { 

         #ifdef TOW_CRUISE
         // If free flight (FF) hasn’t yet been detected, then we may
         // still be on tow.  The following prevents climb mode from
         // engaging due to normal on-aerotow turns.

         if (!Calculated->FreeFlying && (fabs(Calculated->TurnRate) < 12))
           break;
         #endif

       Calculated->Circling = TRUE;
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
      MODE = CRUISE;
    }
    break;
  case CLIMB:
    if ( (AutoWindMode == D_AUTOWIND_CIRCLING) || (AutoWindMode==D_AUTOWIND_BOTHCIRCZAG) ) {
      LockFlightData();
      windanalyser->slot_newSample(Basic, Calculated);
      UnlockFlightData();
    }
    
    if((Rate < MinTurnRate)||(forcecruise)) {
      StartTime = Basic->Time;
      StartLong = Basic->Longitude;
      StartLat  = Basic->Latitude;
      StartAlt  = Calculated->NavAltitude;
      StartEnergyHeight  = Calculated->EnergyHeight;
      // JMW Transition to cruise, due to not properly turning
      MODE = WAITCRUISE;
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

    //
    // Exiting climb mode?
    //
    if((Rate < MinTurnRate) || forcecruise) {

      if( ((Basic->Time  - StartTime) > ClimbCruiseSwitch) || forcecruise) {

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

		InsertThermalHistory(Calculated->ClimbStartTime, 
			// Calculated->ClimbStartLat, Calculated->ClimbStartLong, 
			Calculated->ThermalEstimate_Latitude, Calculated->ThermalEstimate_Longitude,
			Calculated->ClimbStartAlt, Calculated->NavAltitude, Calculated->AverageThermal);

	}

	InitLDRotary(&rotaryLD);
	InitWindRotary(&rotaryWind);
        
        flightstats.Altitude_Ceiling.
          least_squares_update(max(0.0, Calculated->CruiseStartTime
                                   - Calculated->TakeOffTime)/3600.0,
                               Calculated->CruiseStartAlt);
        
        // Finally do the transition to cruise
        Calculated->Circling = FALSE;
        MODE = CRUISE;
        SwitchZoomClimb(Basic, Calculated, false, LEFT);
        InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);

      } // climbcruiseswitch time in range

    } else { // Rate>Minturnrate, back to climb, turning again
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



