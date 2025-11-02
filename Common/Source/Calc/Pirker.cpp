/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Pirker.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "CalcTask.h"
#include "AATDistance.h"

extern double CRUISE_EFFICIENCY;

double ComputeTaskRequiredAltitude(NMEA_INFO* Basic, DERIVED_INFO* Calculated,
                                   double this_maccready) {
  double LegToGo = 0;
  double LegBearing = 0;

  const GeoPoint cur_pos = GetCurrentPosition(*Basic);
  const GeoPoint next_pos = GetTurnpointTarget(ActiveTaskPoint);
  cur_pos.Reverse(next_pos, LegBearing, LegToGo);

  if (ActiveTaskPoint < 1) {
    if (ValidTaskPoint(ActiveTaskPoint + 1)) {  // BUGFIX 091221
      LegToGo = 0;
    }
  }

  double TaskAltitudeRequired = 0;

  // Calculate Final Glide To Finish
  int FinalWayPoint = getFinalWaypoint();

  double final_height =
      FAIFinishHeight(Basic, Calculated, -1);
  double total_energy_height =
      Calculated->NavAltitude + Calculated->EnergyHeight;
  double height_above_finish = total_energy_height - final_height;

  TaskAltitudeRequired = final_height;

  // Now add it for remaining waypoints
  int task_index = FinalWayPoint;

  while ((task_index > ActiveTaskPoint) && (ValidTaskPoint(task_index))) {
    bool this_is_final = (task_index == FinalWayPoint) || ForceFinalGlide;

    GeoPoint p0 = GetTurnpointTarget(task_index - 1);
    GeoPoint p1 = GetTurnpointTarget(task_index);

    double NextLegDistance, NextLegBearing;
    p0.Reverse(p1, NextLegBearing, NextLegDistance);

    double this_LegTimeToGo;
    double LegAltitude = GlidePolar::MacCreadyAltitude(
        this_maccready, NextLegDistance, NextLegBearing, Calculated->WindSpeed,
        Calculated->WindBearing, 0, 0, this_is_final, &this_LegTimeToGo,
        height_above_finish, CRUISE_EFFICIENCY);

    TaskAltitudeRequired += LegAltitude;

    // if required altitude is less than previous turpoint altitude,
    //   use previous turn point altitude
    double w0Alt = FAIFinishHeight(
      Basic, Calculated,
      task_index - 1);

    if (TaskAltitudeRequired < w0Alt) {
      TaskAltitudeRequired = w0Alt;
    }

    height_above_finish -= LegAltitude;

    task_index--;
  }  // WHILE

  // current waypoint, do this last!
  if (UseAATTarget() && (ActiveTaskPoint > 0) &&
      ValidTaskPoint(ActiveTaskPoint + 1) && Calculated->IsInSector) {
    if (Calculated->WaypointDistance < AATCloseDistance() * 3.0) {
      LegBearing = AATCloseBearing(Basic, Calculated);
    }
  }

  double LegAltitude = GlidePolar::MacCreadyAltitude(
      this_maccready, LegToGo, LegBearing, Calculated->WindSpeed,
      Calculated->WindBearing,
#ifdef BCT_ALT_FIX
      0,
#else
      &(Calculated->BestCruiseTrack),
#endif
      &(Calculated->VMacCready),
      true,  // JMW CHECK FGAMT
      &(Calculated->LegTimeToGo), height_above_finish, CRUISE_EFFICIENCY);

  TaskAltitudeRequired += LegAltitude;

  return TaskAltitudeRequired;
}

double MultiLegPirkerAnalysis(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  double available_height =
      (Calculated->NavAltitude + Calculated->EnergyHeight);
  double pirker_mc = 0.0;
  double h;
  double dh = 1.0;
  double last_pirker_mc = 5.0;
  double last_dh = -1.0;
  double pirker_mc_zero = 0.0;

  short retry = 1;
  double max_mc = 10.0;
  if (ISPARAGLIDER) {
    retry = 20;
    max_mc = 20.0;
  }

  while (pirker_mc < max_mc) {
    h = ComputeTaskRequiredAltitude(Basic, Calculated, pirker_mc);
    dh = available_height - h;

    if ((--retry < 1) && (dh == last_dh)) {
      // same height, must have hit max speed.
      if (dh > 0) {
        return last_pirker_mc;
      }
      else {
        return 0.0;
      }
    }

    if (ISPARAGLIDER) {
      if (dh < 0) {
        return last_pirker_mc;
      }
    }

    if ((dh <= 0) && (last_dh > 0)) {
      if (dh - last_dh < 0) {
        double f = (-last_dh) / (dh - last_dh);
        pirker_mc_zero = last_pirker_mc * (1.0 - f) + f * pirker_mc;
      }
      else {
        pirker_mc_zero = pirker_mc;
      }
      return pirker_mc_zero;
    }
    last_dh = dh;
    last_pirker_mc = pirker_mc;

    if (ISPARAGLIDER) {
      pirker_mc += 1.0;
    }
    else {
      pirker_mc += 0.5;
    }
  }  // while

  if (dh >= 0) {
    return pirker_mc;
  }
  return -1.0;  // no solution found, unreachable without further climb
}

//
// Herbert Pirker calculation
//
double PirkerAnalysis(NMEA_INFO* Basic, DERIVED_INFO* Calculated,
                      const double this_bearing, const double GlideSlope) {
  double pirker_mc = 0.0;
  double h_target = GlideSlope;
  double h;
  double dh = 1.0;
  double last_pirker_mc = 5.0;
  double last_dh = -1.0;
  double pirker_mc_zero = 0.0;

  short retry = ISPARAGLIDER ? 20 : 1;
  double max_mc = ISPARAGLIDER ? 20.0 : 10.0;

  bool is_final_glide = Calculated->FinalGlide;

  // Calculate total task distance and altitude difference for multi-leg tasks
  double total_distance = Calculated->WaypointDistance;
  double total_alt_diff =
      Calculated->NavAltitude + Calculated->EnergyHeight -
      FAIFinishHeight(Basic, Calculated, getFinalWaypoint());

  int FinalWayPoint = getFinalWaypoint();
  if (is_final_glide && ActiveTaskPoint != FinalWayPoint) {
    // Aggregate distance and altitude for all remaining task points
    LockTaskData();
    total_distance = Calculated->TaskDistanceToGo;
    int final_wp = getFinalWaypoint();

    // Use final waypoint altitude as target
    total_alt_diff = Calculated->NavAltitude + Calculated->EnergyHeight -
                     FAIFinishHeight(Basic, Calculated, final_wp);
    UnlockTaskData();

    // Adjust glide slope for total task
    h_target =
        total_alt_diff / (total_distance + 1.0);  // Avoid division by zero
  }

  while (pirker_mc < max_mc) {
    // Compute required altitude for unit distance, scaled for total distance
    h = GlidePolar::MacCreadyAltitude(
        pirker_mc,
        1.0,  // Unit distance for glide calculation
        this_bearing, Calculated->WindSpeed, Calculated->WindBearing, 0, 0,
        true, 0);

    dh = (h_target - h) *
         total_distance;  // Scale height difference by total distance

    if ((--retry < 1) && (dh == last_dh)) {
      if (dh > 0) {
        return last_pirker_mc;
      }
      else {
        return 0.0;
      }
    }

    if (ISPARAGLIDER) {
      if (dh < 0) {
        return last_pirker_mc;
      }
    }

    if ((dh <= 0) && (last_dh > 0)) {
      if (dh - last_dh < 0) {
        double f = (-last_dh) / (dh - last_dh);
        pirker_mc_zero = last_pirker_mc * (1.0 - f) + f * pirker_mc;
      }
      else {
        pirker_mc_zero = pirker_mc;
      }
      return pirker_mc_zero;
    }
    last_dh = dh;
    last_pirker_mc = pirker_mc;

    pirker_mc += ISPARAGLIDER ? 1.0 : 0.5;
  }

  if (dh >= 0) {
    return pirker_mc;
  }
  return -1.0;  // No solution found
}
