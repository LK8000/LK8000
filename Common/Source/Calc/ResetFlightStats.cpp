/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "ThermalHistory.h"



extern double CRUISE_EFFICIENCY;

//
// This is called only by calculations thread, at init, at restart of a replay flight, and also on takeoff
// IT SHOULD NEVER HAPPEN DURING REAL FLIGHT, AFTER TAKEOFF!
// PLEASE NOTICE THAT TAKEOFF IS NOT NECESSARILY THE START OF FREEFLIGHT
// We reset some values at start of free flight in LKCalculations
// 
void ResetFlightStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  int i;
  (void)Basic;

    TestLog(_T(". Reset flight statistics"));

    // It is better to reset it even if UseContestEngine() if false, because we might
    // change aircraft type during runtime. We never know.
    CContestMgr::Instance().Reset(Handicap);
    flightstats.Reset();
    aatdistance.Reset();
    CRUISE_EFFICIENCY = 1.0;

    Calculated->FlightTime = 0;
    Calculated->TakeOffTime = 0;
    Calculated->FreeFlightStartTime = 0;
    Calculated->FreeFlightStartQNH = 0;
    Calculated->FreeFlightStartQFE = 0;
    Calculated->FreeFlying=false;
    Calculated->Flying=false;	// Takeoff will immediately set it TRUE again
    Calculated->OnGround=true;	// we must init this value
    Calculated->Circling = false;
    Calculated->FinalGlide = false;
    Calculated->timeCruising = 0;
    Calculated->timeCircling = 0;

    Calculated->TotalHeightClimb = 0;
    Calculated->CruiseStartTime = -1;
    Calculated->CruiseStartAlt = 0;
    Calculated->CruiseStartLong = 0;
    Calculated->CruiseStartLat = 0;

    Calculated->ClimbStartTime = -1;
    Calculated->ClimbStartAlt = 0;
    Calculated->ClimbStartLong = 0;
    Calculated->ClimbStartLat = 0;

    Calculated->AverageThermal = 0;
    Calculated->Average30s = 0;
    Calculated->ThermalGain = 0;
    Calculated->LastThermalAverage = 0;
    Calculated->LastThermalGain = 0;
    Calculated->LastThermalTime = 0;

    Calculated->GRFinish = INVALID_GR;
    Calculated->CruiseLD = INVALID_GR;
    Calculated->AverageLD = INVALID_GR;
    Calculated->LD = INVALID_GR;
    Calculated->LDvario = INVALID_GR;
    Calculated->Odometer = 0; // 091228
    lk::strcpy(Calculated->Flaps,_T("???"));

    for (i=0; i<MAXAVERAGECLIMBRATESIZE; i++) {
      Calculated->AverageClimbRate[i]= 0;
      Calculated->AverageClimbRateN[i]= 0;
    }

    Calculated->GlideFootPrint_valid = false;
    Calculated->GlideFootPrint2_valid = false;
    
    std::fill(std::begin(Calculated->GlideFootPrint), std::end(Calculated->GlideFootPrint), GeoPoint{0.,0.});
    std::fill(std::begin(Calculated->GlideFootPrint2), std::end(Calculated->GlideFootPrint2), GeoPoint{0.,0.});


    Calculated->TerrainWarningLatitude = 0.0;
    Calculated->TerrainWarningLongitude = 0.0;

    // Task system reset
    Calculated->ValidFinish = false;
    Calculated->ValidStart = false;
    Calculated->TaskStartTime = 0;
    Calculated->TaskElapsedTime =0;
    Calculated->TaskStartSpeed = 0;
    Calculated->TaskStartAltitude = 0;
    Calculated->LegStartTime = 0;

    // Min and Max Altitude are then updated correctly after FF detection
    // The MaxHeightGain function wait for FF in flight and will update 
    // considering 0 as a no-altitude-set-yet .
    Calculated->MinAltitude = 0;
    Calculated->MaxAltitude = 0;
    Calculated->MaxHeightGain = 0;

    Calculated->HeadWind=-999; // invalid values for LKProcess

    Calculated->MaxThermalHeight = 0;
    for (i=0; i<NUMTHERMALBUCKETS; i++) {
      Calculated->ThermalProfileN[i]=0;
      Calculated->ThermalProfileW[i]=0;
    }
    // clear thermal sources for first time.
    for (i=0; i<MAX_THERMAL_SOURCES; i++) {
      Calculated->ThermalSources[i].LiftRate= -1.0;
    }

    // Reset Thermal History
    InitThermalHistory();

}


