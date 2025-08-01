/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "InputEvents.h"
#include "windanalyser.h"
#include "ThermalLocator.h"
#include "LDRotaryBuffer.h"
#include "Calc/ThermalHistory.h"
#include "Util/Clamp.hpp"

void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
void ThermalBand(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

#define MinTurnRate  4

extern WindAnalyser *windanalyser;
extern ThermalLocator thermallocator;

namespace {

// #define DEBUGTURN 1
template<typename... Args>
static void TurningLog(Args... args) {
#ifdef DEBUGTURN
  DebugLog(args...);
#endif
}

enum class FlightState {
  Unknown, // this is the state when not flying;
  CRUISE,
  WAITCLIMB,
  CLIMB,
  WAITCRUISE
};

struct StartStateT {
  StartStateT() = default;

  StartStateT(const NMEA_INFO *Basic, const DERIVED_INFO *Calculated) {
    Time = Basic->Time;
    Position = {{Basic->Latitude, Basic->Longitude}, Calculated->NavAltitude};
    EnergyHeight = Calculated->EnergyHeight;
  }

  double Elapsed(const NMEA_INFO *Basic) {
    return Basic->Time - Time;
  }

  double Time = 0;
  AGeoPoint Position = {{0., 0.}, 0.};
  double EnergyHeight = 0.;
};

void SwitchZoomClimb(NMEA_INFO *Basic, DERIVED_INFO *Calculated, bool isclimb, bool left) {
  if ( (AutoWindMode == D_AUTOWIND_CIRCLING) || (AutoWindMode==D_AUTOWIND_BOTHCIRCZAG) ) {
    ScopeLock lock(CritSec_FlightData);
    windanalyser->slot_newFlightMode();
  }
}

void PercentCircling(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double Rate) {

  if (Calculated->Circling && (Rate>MinTurnRate)) {
    Calculated->timeCircling+= 1.0;
    Calculated->TotalHeightClimb += Calculated->GPSVario;
    if (ThermalBar) {
      ThermalBand(Basic, Calculated);
    }
  } else {
    Calculated->timeCruising += 1.0;
  }

  if ((Calculated->timeCruising + Calculated->timeCircling) > 1) {
    Calculated->PercentCircling =
      100.0 * Calculated->timeCircling / (Calculated->timeCruising + Calculated->timeCircling);
  } else {
    Calculated->PercentCircling = 0.0;
  }
}

void SetClimbStart(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const StartStateT& Start, bool TurnLeft) {
  Calculated->Circling = true;
  Calculated->ClimbStartLat = Start.Position.latitude;
  Calculated->ClimbStartLong = Start.Position.longitude;
  Calculated->ClimbStartAlt = Start.Position.altitude + Start.EnergyHeight;
  Calculated->ClimbStartTime = Start.Time;

  if (flightstats.Altitude_Ceiling.sum_n > 0) {
    // only update base if have already climbed, otherwise
    // we will catch the takeoff height as the base.
    flightstats.Altitude_Base.least_squares_update(
        max(0.0, Calculated->ClimbStartTime - Calculated->TakeOffTime) / 3600.0, Start.Position.altitude);
  }

  SwitchZoomClimb(Basic, Calculated, true, TurnLeft);
  InputEvents::processGlideComputer(GCE_FLIGHTMODE_CLIMB);
}

void SetCruiseStart(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const StartStateT& Start, bool TurnLeft) {
  Calculated->Circling = false;
  Calculated->CruiseStartTime = Start.Time;
  Calculated->CruiseStartLat = Start.Position.latitude;
  Calculated->CruiseStartLong = Start.Position.longitude;
  Calculated->CruiseStartAlt = Start.Position.altitude;

  // Here we assign automatically this last thermal to the L> multitarget
  if (Calculated->ThermalGain > 100) {

    // Force immediate calculation of average thermal, it would be made
    // during next cycle, but we need it here immediately
    AverageThermal(Basic, Calculated);

    if (EnableThermalLocator) {
      InsertThermalHistory(Calculated->ClimbStartTime,
                            {Calculated->ThermalEstimate_Latitude, Calculated->ThermalEstimate_Longitude},
                            Calculated->ClimbStartAlt, Calculated->NavAltitude, Calculated->AverageThermal);
    } else {
      InsertThermalHistory(Calculated->ClimbStartTime,
                            {Calculated->ClimbStartLat, Calculated->ClimbStartLong},
                            Calculated->ClimbStartAlt, Calculated->NavAltitude, Calculated->AverageThermal);
    }
  }

  rotaryLD.Init();
  InitWindRotary(&rotaryWind);

  flightstats.Altitude_Ceiling.least_squares_update(
      max(0.0, Calculated->CruiseStartTime - Calculated->TakeOffTime) / 3600.0, Calculated->CruiseStartAlt);

  SwitchZoomClimb(Basic, Calculated, false, TurnLeft);
  InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
}

double LastTrack = 0;
double LastTime = 0;
double LastRate = 0;

StartStateT Start;

FlightState MODE = FlightState::CRUISE;

void Reset(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  LastTrack = 0;
  LastTrack = 0;
  LastTime = 0;
  Start = {};

  MODE = FlightState::CRUISE;

  // Finally do the transition to cruise
  Calculated->Circling = false;
  SwitchZoomClimb(Basic, Calculated, false, false);
  InputEvents::processGlideComputer(GCE_FLIGHTMODE_CRUISE);
}

} // namespace

void Turning(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  if (!Calculated->Flying) {
    MODE = FlightState::Unknown;
    return;
  }

  if (MODE == FlightState::Unknown) {
    if (Calculated->Flying) {
      TestLog(_T(".... Start flying, Reset turning state !"));
      Reset(Basic, Calculated);
    }
    return;
  }

  // Back in time in IGC replay mode ?
  if (Basic->Time < LastTime) {
    TestLog(_T("...... Turning check is back in time. Now=%f Last=%f: reset %s"),
                  Basic->Time, LastTime, WhatTimeIsIt());
    Reset(Basic, Calculated);
    return;
  }

  double dT = Basic->Time - LastTime;
  if (dT == 0) {
    return;
  }

  Calculated->TurnRate = AngleLimit180(Basic->TrackBearing - LastTrack) / dT;

  if (dT < 2.0 && dT != 0) {
    // time step ok

    // calculate acceleration
    const double dRate = (Calculated->TurnRate - LastRate) / dT;

    double dtlead = 0.3;
    // integrate assuming constant acceleration, for one second
    Calculated->NextTrackBearing =
            Basic->TrackBearing + dtlead * (Calculated->TurnRate + 0.5 * dtlead * dRate);
    // s = u.t+ 0.5*a*t*t

    Calculated->NextTrackBearing =
            AngleLimit360(Calculated->NextTrackBearing);

  } else {
    // time step too big, so just take it at last measurement
    Calculated->NextTrackBearing = Basic->TrackBearing;
  }

  // JMW limit rate to 50 deg per second otherwise a big spike
  // will cause spurious lock on circling for a long time
  double Rate = Clamp(Calculated->TurnRate, -50., 50.);

  if (MODE == FlightState::CLIMB || MODE == FlightState::WAITCRUISE) {
    Rate = LowPassFilter(LastRate, Rate, 0.9);
  } 
  else {
    Rate = LowPassFilter(LastRate, Rate, 0.3);
  }

  TurningLog(_T("...Turning : Rate = %+6.1f° / %+6.1f° / %6.2fs" ), Calculated->TurnRate, Rate, dT);

  if ((AutoWindMode == D_AUTOWIND_CIRCLING) || (AutoWindMode == D_AUTOWIND_BOTHCIRCZAG)) {
    ScopeLock lock(CritSec_FlightData);
    windanalyser->slot_newSample(Basic, Calculated);
  }

  LastRate = Rate;
  LastTime = Basic->Time;
  LastTrack = Basic->TrackBearing;

  bool TurnLeft = (Rate < 0);
  Rate = std::abs(Rate);

  PercentCircling(Basic, Calculated, Rate);

  // USED by LXV7 device Driver only
  bool forcecruise = false;
  bool forcecircling = false;

  if (EnableExternalTriggerCruise) {
    if (ExternalTriggerCruise && ExternalTriggerCircling) {
      // this should never happen
      ExternalTriggerCircling = false;
    }
    forcecruise = ExternalTriggerCruise;
    forcecircling = ExternalTriggerCircling;
  }

  // turn rate to switch to "wait climb" 
  const double cruise_turnthreshold = (ISPARAGLIDER)? 5 : 4; 
  
  // turn rate and timer to switch to "climb"
  const double waitclimb_turnthreshold = (ISPARAGLIDER)? 5 : 4; 
  const double cruiseclimbswitch = 15; // this is ok for gliders, should be finetuned for PGs
  
  // turn rate to switch to "wait cruise" 
  const double climb_turnthreshold = (ISPARAGLIDER)? 10 : 4;
  
  // turn rate and timer to switch to "cruise"
  const double waitcruise_turnthreshold = (ISPARAGLIDER)? 10 : 4;
  const double climbcruiseswitch = (ISPARAGLIDER)? 15 : 9; // ok for gliders
  
  switch (MODE) {
    case FlightState::CRUISE:

      if (forcecircling || (Rate >= cruise_turnthreshold)) {
        // This is initialising the potential thermalling start
        // We still dont know if we are really circling for thermal
        Start = { Basic, Calculated };
        MODE = FlightState::WAITCLIMB;

        TurningLog(_T("..... CRUISE -> WAITCLIMB"));
      }

      /* 
       * continue to wait climb, to speedup switch
       * otherwise, switch is done on next GPS fix ( 1 second later )
       */
      if (!forcecircling) {
        break;
      }
    case FlightState::WAITCLIMB:
      if (forcecruise) {
        MODE = FlightState::CRUISE;
        break;
      }

      if (Start.Time == 0.) {
        // Probably a replay flight, with fast forward with no cruise init
        MODE = FlightState::CRUISE;
        break;
      }

      if (forcecircling || (Rate >= waitclimb_turnthreshold)) {
        // WE CANNOT do this, because we also may need Circling mode to detect FF!!
        // if( (Calculated->FreeFlying && ((Basic->Time  - StartTime) > cruiseclimbswitch))|| forcecircling) {
        if ((!ISCAR && !ISGAAIRCRAFT && (Start.Elapsed(Basic) > cruiseclimbswitch)) || forcecircling) {

#ifdef TOW_CRUISE
          // If free flight (FF) hasn�t yet been detected, then we may
          // still be on tow.  The following prevents climb mode from
          // engaging due to normal on-aerotow turns.

          if (!Calculated->FreeFlying && (fabs(Calculated->TurnRate) < 12))
            break;
#endif

          // JMW Transition to climb
          MODE = FlightState::CLIMB;
          TurningLog(_T("..... WAITCLIMB -> CLIMB"));
          SetClimbStart(Basic, Calculated, Start, TurnLeft);
        }
      } else {
        // nope, not turning, so go back to cruise
        MODE = FlightState::CRUISE;
        TurningLog(_T("..... WAITCLIMB -> CRUISE"));
      }
      break;
    case FlightState::CLIMB:

      if ((Rate < climb_turnthreshold) || (forcecruise)) {
        Start = {Basic, Calculated};
        // JMW Transition to cruise, due to not properly turning
        MODE = FlightState::WAITCRUISE;
        TurningLog(_T("..... CLIMB -> WAITCRUISE"));
      }

      /* 
       * continue to WAITCRUISE, to speedup switch
       * otherwise, switch is done on next GPS fix ( 1 second later )
       */
      if (!forcecruise) {
        break;
      }
    case FlightState::WAITCRUISE:

      if (forcecircling) {
        MODE = FlightState::CLIMB;
        break;
      }

      //
      // Exiting climb mode?
      //
      if (forcecruise || (Rate < waitcruise_turnthreshold)) {
        if (forcecruise || (Start.Elapsed(Basic) > climbcruiseswitch)) {

          // We are no more in climb mode
          if (Start.Time == 0) {
            Start = { Basic, Calculated };
          }

          SetCruiseStart(Basic, Calculated, Start, TurnLeft);

          // Finally do the transition to cruise
          MODE = FlightState::CRUISE;
          TurningLog(_T("... WAITCRUISE -> CRUISE"));
        }  // climbcruiseswitch time in range

      } else {  // Rate>Minturnrate, back to climb, turning again
        MODE = FlightState::CLIMB;
        TurningLog(_T("..... WAITCRUISE -> CLIMB"));
      }
      break;
    case FlightState::Unknown:
      assert(Calculated->Flying);
      break;
  }

  // generate new wind vector if altitude changes or a new
  // estimate is available
  if (AutoWindMode > D_AUTOWIND_MANUAL && AutoWindMode < D_AUTOWIND_EXTERNAL) {
    ScopeLock lock(CritSec_FlightData);
    windanalyser->slot_Altitude(Basic, Calculated);
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
}
