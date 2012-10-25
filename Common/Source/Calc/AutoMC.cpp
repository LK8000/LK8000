/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

double	PirkerAnalysis(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_bearing, const double GlideSlope);
bool	ActiveIsFinalWaypoint();


void DoAutoMacCready(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

  if (!Calculated->AutoMacCready) return;

  bool is_final_glide = false;
  //  LockFlightData();
  LockTaskData();

  double mc_new = MACCREADY;
  static bool first_mc = true;

  if ( AutoMcMode==amcEquivalent ) {
	if ( Calculated->Circling != TRUE && Calculated->OnGround != TRUE) {
		if (Calculated->EqMc>=0) {
			// MACCREADY = LowPassFilter(MACCREADY,Calculated->EqMc,0.8); 
			MACCREADY = Calculated->EqMc;
		} else {
			// -1.0 is used as an invalid flag. Normally flying at -1 MC means almost flying
			// at stall speed, which is pretty unusual. Maybe in wave conditions?
			if (Calculated->EqMc >-1) {
				MACCREADY=Calculated->EqMc*-1;
			}
		}
	}
	UnlockTaskData();
	return;
  }

  // otherwise, if AutoMc for finalglide or "both", return if no goto
  if (!ValidTaskPoint(ActiveWayPoint)) {
	UnlockTaskData();
	return;
  }

  if (Calculated->FinalGlide && ActiveIsFinalWaypoint()) {  
    is_final_glide = true;
  } else {
    first_mc = true;
  }

  double av_thermal = -1;
  if (flightstats.ThermalAverage.y_ave>0) {
    if (Calculated->Circling && (Calculated->AverageThermal>0)) {
      av_thermal = (flightstats.ThermalAverage.y_ave
		*flightstats.ThermalAverage.sum_n 
		+ Calculated->AverageThermal)/
	(flightstats.ThermalAverage.sum_n+1);
    } else {
      av_thermal = flightstats.ThermalAverage.y_ave;
    }
  } else if (Calculated->Circling && (Calculated->AverageThermal>0)) {
    // insufficient stats, so use this/last thermal's average
    av_thermal = Calculated->AverageThermal;
  }

  if (!ValidTaskPoint(ActiveWayPoint)) {
    if (av_thermal>0) {
      mc_new = av_thermal;
    }
  } else if ( ((AutoMcMode==amcFinalGlide)||(AutoMcMode==amcFinalAndClimb)) && is_final_glide) {

      if (Calculated->TaskAltitudeDifference0>0) {
	
      // only change if above final glide with zero Mc
      // otherwise when we are well below, it will wind Mc back to
      // zero
      
      double slope = 
	(Calculated->NavAltitude + Calculated->EnergyHeight
	 - FAIFinishHeight(Basic, Calculated, ActiveWayPoint))/
	(Calculated->WaypointDistance+1);
      
      double mc_pirker = PirkerAnalysis(Basic, Calculated,
					Calculated->WaypointBearing,
					slope);
      mc_pirker = max(0.0, mc_pirker);
      if (first_mc) {
	// don't allow Mc to wind down to zero when first achieving
	// final glide; but do allow it to wind down after that
	if (mc_pirker >= mc_new) {
	  mc_new = mc_pirker;
	  first_mc = false;
	} else if (AutoMcMode==amcFinalAndClimb) {
	  // revert to averager based auto Mc
	  if (av_thermal>0) {
	    mc_new = av_thermal;
	  }
	}
      } else {
	mc_new = mc_pirker;
      }
    } else { // below final glide at zero Mc, never achieved final glide
      if (first_mc && (AutoMcMode==amcFinalAndClimb)) {
	// revert to averager based auto Mc
	if (av_thermal>0) {
	  mc_new = av_thermal;
	}
      }
    }
  } else if ( (AutoMcMode==amcAverageClimb) || ((AutoMcMode==amcFinalAndClimb)&& !is_final_glide) ) {
    if (av_thermal>0) {
      mc_new = av_thermal;
    }
  }

  MACCREADY = LowPassFilter(MACCREADY,mc_new,0.15);

  UnlockTaskData();
  //  UnlockFlightData();

}

