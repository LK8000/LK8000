/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Calculations2.h"
#include "McReady.h"


extern void Vario(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void LD(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void Heading(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void CruiseLD(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
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
extern double CalculateLDRotary(ldrotary_s *buf, DERIVED_INFO *Calculated);
extern void AverageThermal(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void Turning(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void ConditionMonitorsUpdate(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
#ifndef GTL2
extern void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint); // VENTA3
#endif
extern void DistanceToHome(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern bool FlightTimes(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void AverageClimbRate(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern void CalculateHeadWind(NMEA_INFO *Basic, DERIVED_INFO *Calculated);
extern bool TargetDialogOpen;



BOOL DoCalculations(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

  // first thing: assign navaltitude!
  EnergyHeightNavAltitude(Basic, Calculated);

  // second thing: if available, get external wind precalculated!
  if ( (Basic->ExternalWindAvailable==TRUE) && (AutoWindMode==D_AUTOWIND_EXTERNAL)) {
	if (Basic->ExternalWindSpeed>0 && Basic->ExternalWindSpeed<35) {
		Calculated->WindSpeed = Basic->ExternalWindSpeed;
		Calculated->WindBearing = Basic->ExternalWindDirection;
	}
  }

  Heading(Basic, Calculated);
  DistanceToNext(Basic, Calculated);
  DistanceToHome(Basic, Calculated);
  DetectFreeFlying(Basic,Calculated);	// check ongoing powerless flight
  DoLogging(Basic, Calculated);
  Vario(Basic,Calculated);
  TerrainHeight(Basic, Calculated);
  AltitudeRequired(Basic, Calculated, MACCREADY);
  DoAlternates(Basic,Calculated,TASKINDEX); 
  if (MAPMODE8000) {
	DoAlternates(Basic,Calculated,RESWP_LASTTHERMAL);
	DoAlternates(Basic,Calculated,RESWP_TEAMMATE);
	DoAlternates(Basic,Calculated,RESWP_FLARMTARGET);
	DoAlternates(Basic,Calculated,HomeWaypoint); 	
	#ifdef GTL2
	if (DoOptimizeRoute() || ACTIVE_WP_IS_AAT_AREA)
		DoAlternates(Basic, Calculated, RESWP_OPTIMIZED);
	#else
	if (DoOptimizeRoute()) DoAlternates(Basic,Calculated,RESWP_OPTIMIZED); 	
	#endif
	for (int i=RESWP_FIRST_MARKER; i<=RESWP_LAST_MARKER; i++) {
		if (WayPointList[i].Latitude==RESWP_INVALIDNUMBER) continue;
		DoAlternates(Basic,Calculated,i);
	}
  #ifndef GTL2
  }
  #else
  } else {
    // The following is needed only for the next-WP glide terrain line,
    // not for the main/primary glide terrain line.

    if ((FinalGlideTerrain > 2) && (DoOptimizeRoute() || ACTIVE_WP_IS_AAT_AREA))
      DoAlternates(Basic, Calculated, RESWP_OPTIMIZED);
  }
  #endif

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
    return FALSE;
  }

  Turning(Basic, Calculated);
  LD(Basic,Calculated);
  CruiseLD(Basic,Calculated);

  // We calculate flaps settings only if the polar is extended.
  // We do assume that GA planes will NOT use extended polars
  if (GlidePolar::FlapsPosCount >0) Flaps(Basic,Calculated);

  Calculated->AverageLD=CalculateLDRotary(&rotaryLD,Calculated); 
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

  // reminder: Paragliders have AAT always on
  CalculateOptimizedTargetPos(Basic,Calculated);

  if (ValidTaskPoint(ActiveWayPoint)) {
	// only if running a real task
	if (ValidTaskPoint(1)) InSector(Basic, Calculated);
	DoAutoMacCready(Basic, Calculated);
	#ifdef DEBUGTASKSPEED
	DebugTaskCalculations(Basic, Calculated);
	#endif
  } else { // 101002
	DoAutoMacCready(Basic, Calculated); // will set only EqMC 
  }

  DoAlternates(Basic, Calculated,Alternate1); 
  DoAlternates(Basic, Calculated,Alternate2); 
  DoAlternates(Basic, Calculated,BestAlternate); 

  // Calculate nearest landing when needed
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
	}
  }

  CalculateHeadWind(Basic,Calculated);

  ConditionMonitorsUpdate(Basic, Calculated);

  return TRUE;
}
