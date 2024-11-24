/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Calculations2.h"
#include "McReady.h"
#include "Sideview.h"
#include "Multimap.h"
#include "Comm/ExternalWind.h"
#include "LDRotaryBuffer.h"
#include "LD.h"
#include "Atmosphere.h"


extern void Heading(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void Flaps(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void Average30s(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void LastThermalStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void ThermalGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void MaxHeightGain(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void DistanceToNext(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void EnergyHeightNavAltitude(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void TaskSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_maccready);
extern void AltitudeRequired(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_maccready);
extern void TaskStatistics(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_maccready);
extern void InSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void PredictNextPosition(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void AATStats(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void DoAutoMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void TerrainHeight(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void Turning(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void ConditionMonitorsUpdate(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void DistanceToHome(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern bool FlightTimes(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void AverageClimbRate(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern bool TargetDialogOpen;



bool DoCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

  // first thing: assign navaltitude!
  EnergyHeightNavAltitude(Basic, Calculated);

  // second thing: if available, get external wind precalculated!
  if ((ExternalWindAvailable(*Basic)) && (AutoWindMode == D_AUTOWIND_EXTERNAL)) {
    if (Basic->ExternalWindSpeed > 0 && Basic->ExternalWindSpeed < 35) {
      Calculated->WindSpeed = Basic->ExternalWindSpeed;
      Calculated->WindBearing = Basic->ExternalWindDirection;
    }
  }

  UpdateFlarmTarget(*Basic);

  Heading(Basic, Calculated);
  DistanceToNext(Basic, Calculated);
  DistanceToHome(Basic, Calculated);
  DetectFreeFlying(Basic,Calculated);	// check ongoing powerless flight
  DoLogging(Basic, Calculated);
  TerrainHeight(Basic, Calculated);
  AltitudeRequired(Basic, Calculated, MACCREADY);

  if(ValidTaskPoint(ActiveTaskPoint)) {
    DoAlternates(Basic,Calculated,TASKINDEX);
  }
  if (IsMultiMapShared()) {
    DoAlternates(Basic,Calculated,RESWP_LASTTHERMAL);
    DoAlternates(Basic,Calculated,RESWP_TEAMMATE);
    DoAlternates(Basic,Calculated,RESWP_FLARMTARGET);
    DoAlternates(Basic,Calculated,HomeWaypoint);

    if ( OvertargetMode == OVT_XC ) {
      DoAlternates(Basic, Calculated, RESWP_FAIOPTIMIZED);    // In Contest mode the Triangle closing point is our Goal.
    }

    if (DoOptimizeRoute() || ACTIVE_WP_IS_AAT_AREA) {
      DoAlternates(Basic, Calculated, RESWP_OPTIMIZED);
    }

    for (int i=RESWP_FIRST_MARKER; i<=RESWP_LAST_MARKER; i++) {
      if (WayPointList[i].Latitude==RESWP_INVALIDNUMBER) continue;
      DoAlternates(Basic,Calculated,i);
    }
  } else {
    // The following is needed only for the next-WP glide terrain line,
    // not for the main/primary glide terrain line.

    if ((FinalGlideTerrain > 2) && (DoOptimizeRoute() || ACTIVE_WP_IS_AAT_AREA))
      DoAlternates(Basic, Calculated, RESWP_OPTIMIZED);
  }

  if (!TargetDialogOpen) {
    // don't calculate these if optimise function being invoked or
    // target is being adjusted
    TaskStatistics(Basic, Calculated, MACCREADY);
    AATStats(Basic, Calculated);  
    TaskSpeed(Basic, Calculated, MACCREADY);
  }

  if (!FlightTimes(Basic, Calculated)) {
    // time hasn't advanced, so don't do calculations requiring an advance
    // or movement
    return false;
  }

  Turning(Basic, Calculated);
  // update atmospheric model
  CuSonde::updateMeasurements(Basic, Calculated);
  LD(Basic,Calculated);
  CruiseLD(Basic,Calculated);

  // We calculate flaps settings only if the polar is extended.
  // We do assume that GA planes will NOT use extended polars
  if (GlidePolar::FlapsPosCount >0) Flaps(Basic,Calculated);

  rotaryLD.Calculate(*Basic, *Calculated); 
  Average30s(Basic,Calculated);
  AverageThermal(Basic,Calculated);
  AverageClimbRate(Basic,Calculated);
  ThermalGain(Basic,Calculated);
  LastThermalStats(Basic, Calculated);
  //  ThermalBand(Basic, Calculated); moved to % circling function
  MaxHeightGain(Basic,Calculated);

  PredictNextPosition(Basic, Calculated);

  if (Orbiter) CalculateOrbiter(Basic,Calculated);

  CalculateOwnTeamCode(Basic, Calculated);
  CalculateTeammateBearingRange(Basic, Calculated);

  BallastDump();

  CalculateOptimizedTargetPos(Basic,Calculated);
  InSector(Basic, Calculated);

  DoAutoMacCready(Basic, Calculated);

  DoAlternates(Basic, Calculated,Alternate1); 
  DoAlternates(Basic, Calculated,Alternate2); 
  DoAlternates(Basic, Calculated,BestAlternate); 

  // Calculate nearest waypoints when needed
  if ( !MapWindow::mode.AnyPan() && DrawBottom && (MapSpaceMode>MSM_MAP) ) {
	switch(MapSpaceMode) {
		case MSM_LANDABLE:
		case MSM_AIRPORTS:
		case MSM_NEARTPS: // 101222
			DoNearest(Basic,Calculated);
			break;
		case MSM_COMMON:
			DoCommon(Basic,Calculated);
			break;
		case MSM_RECENT:
			DoRecent(Basic,Calculated);
			break;
		case MSM_VISUALGLIDE:
			// Only if visualglide is really painted: topview is not fullscreen.
			if (Current_Multimap_SizeY<SIZE4) {
				DoNearest(Basic,Calculated);
			}
			break;
	}
  }

  CalculateHeadWind(Basic,Calculated);

  ConditionMonitorsUpdate(Basic, Calculated);

  return true;
}
