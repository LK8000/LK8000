/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "CalcTask.h"
#include "AATDistance.h"
#include "McReady.h"
#include "NavFunctions.h"

extern double CRUISE_EFFICIENCY;

static 
void ResetTaskStatistics(NMEA_INFO* Basic, DERIVED_INFO* Calculated, const double this_maccready) {
    Calculated->LegDistanceToGo = 0;
    Calculated->LegDistanceCovered = 0;
    Calculated->LegTimeToGo = 0;

    if (gTaskType != task_type_t::AAT) {
      Calculated->AATTimeToGo = 0;
    }

    //    Calculated->TaskSpeed = 0;

    Calculated->TaskDistanceToGo = 0;
    Calculated->TaskDistanceCovered = 0;
    Calculated->TaskTimeToGo = 0;
    Calculated->LKTaskETE = 0;

    Calculated->TaskAltitudeRequired = 0;
    Calculated->TaskAltitudeDifference = 0;
    Calculated->TaskAltitudeDifference0 = 0;

    Calculated->TaskAltitudeArrival = 0;

    Calculated->TerrainWarningLatitude = 0.0;
    Calculated->TerrainWarningLongitude = 0.0;

    Calculated->GRFinish = INVALID_GR;

    Calculated->FinalGlide = false;
    CheckGlideThroughTerrain(Basic, Calculated);  // BUGFIX 091123

    // no task selected, so work things out at current heading

    GlidePolar::MacCreadyAltitude(this_maccready, 100.0, Basic->TrackBearing, Calculated->WindSpeed,
                                  Calculated->WindBearing, &(Calculated->BestCruiseTrack), &(Calculated->VMacCready),
                                  Calculated->FinalGlide, NULL, 1.0e6, CRUISE_EFFICIENCY);

}


void TaskStatistics(NMEA_INFO* Basic, DERIVED_INFO* Calculated, const double this_maccready) {
  if (Calculated->ValidFinish) {  // don't update statistics after task finished
    return;
  }

  if (!ValidTaskPoint(ActiveTaskPoint) || ((ActiveTaskPoint > 0) && !ValidTaskPoint(ActiveTaskPoint - 1))) {
    ResetTaskStatistics(Basic, Calculated, this_maccready);
    return;
  }

  ScopeLock lock(CritSec_TaskData);

  // Calculate Task Distances
  // First calculate distances for this waypoint

  double LegCovered, LegToGo = 0, LegXTD = 0, LegCurrentCourse;
  double LegBearing = 0;

  const GeoPoint cur_pos = GetCurrentPosition(*Basic);
  const GeoPoint next_pos = GetTurnpointTarget(ActiveTaskPoint);
  cur_pos.Reverse(next_pos, LegBearing, LegToGo);

  if (ActiveTaskPoint < 1) {
    LegCovered = 0;
    LegCurrentCourse = LegBearing;
    if (ValidTaskPoint(ActiveTaskPoint + 1)) {  // BUGFIX 091221
      LegToGo = 0;
    }
  } else {
    GeoPoint prev_pos = GetTurnpointTarget(ActiveTaskPoint - 1);

    LegCovered =
        ProjectedDistance(prev_pos, next_pos, cur_pos, &LegXTD, &LegCurrentCourse);

    if ((StartLine == sector_type_t::CIRCLE) && (ActiveTaskPoint == 1)) {
      // Correct speed calculations for radius
      // JMW TODO accuracy: legcovered replace this with more accurate version
      // LegDistance -= StartRadius;
      LegCovered = max(0.0, LegCovered - StartRadius);
    }
  }

  Calculated->LegDistanceToGo = LegToGo;
  Calculated->LegDistanceCovered = LegCovered;
  Calculated->LegCrossTrackError = LegXTD;
  Calculated->LegActualTrueCourse = LegCurrentCourse;
  Calculated->TaskDistanceCovered = LegCovered;

  if (Basic->Time > Calculated->LegStartTime) {
    if (flightstats.LegStartTime[ActiveTaskPoint] < 0) {
      flightstats.LegStartTime[ActiveTaskPoint] = Basic->Time;
    }
  }

  // Now add distances from start to previous waypoint
  if (gTaskType == task_type_t::AAT) {
    if (ActiveTaskPoint > 0) {
      // JMW added correction for distance covered
      Calculated->TaskDistanceCovered = aatdistance.DistanceCovered(cur_pos);
    }
  } else {
    if (ValidTaskPoint(0)) {
      GeoPoint p0 = GetTurnpointTarget(0);
      for (int i = 1; i < ActiveTaskPoint && ValidTaskPoint(i); i++) {
        GeoPoint p1 = GetTurnpointTarget(i);

        Calculated->TaskDistanceCovered += p0.Distance(p1);

        p0 = p1;
      }
    }
  }

  // If it is not a glider, or if it is a glider and it is freeflying with take off since 5 minutes
  if (!(ISGLIDER || ISPARAGLIDER) || (Calculated->FreeFlying && Calculated->FlightTime > (60 * 5))) {
    CheckTransitionFinalGlide(Basic, Calculated);
  }

  // accumulators
  double TaskAltitudeRequired = 0;
  double TaskAltitudeRequired0 = 0;
  Calculated->TaskDistanceToGo = 0;
  Calculated->TaskTimeToGo = 0;
  Calculated->LKTaskETE = 0;
  Calculated->TaskAltitudeArrival = 0;

  double LegTime0;

  // Calculate Final Glide To Finish

  int FinalWayPoint = getFinalWaypoint();

  double final_height = FAIFinishHeight(Basic, Calculated, -1);
  double total_energy_height = Calculated->NavAltitude + Calculated->EnergyHeight;
  double height_above_finish = total_energy_height - final_height;

  if (ISPARAGLIDER) {
    TaskAltitudeRequired = final_height;
    TaskAltitudeRequired0 = final_height;
  }

  // Now add it for remaining waypoints
  int task_index = FinalWayPoint;

  double StartBestCruiseTrack = -1;

  while ((task_index > ActiveTaskPoint) && (ValidTaskPoint(task_index))) {
    bool this_is_final = (task_index == FinalWayPoint) || ForceFinalGlide;

    GeoPoint p0 = GetTurnpointTarget(task_index -1);
    GeoPoint p1 = GetTurnpointTarget(task_index);

    double NextLegDistance, NextLegBearing;
    p0.Reverse(p1, NextLegBearing, NextLegDistance);

    double this_LegTimeToGo;
    double LegAltitude = GlidePolar::MacCreadyAltitude(
        this_maccready, NextLegDistance, NextLegBearing, Calculated->WindSpeed, Calculated->WindBearing, 0, 0,
        this_is_final, &this_LegTimeToGo, height_above_finish, CRUISE_EFFICIENCY);

    double LegAltitude0 =
        GlidePolar::MacCreadyAltitude(0, NextLegDistance, NextLegBearing, Calculated->WindSpeed,
                                      Calculated->WindBearing, 0, 0, true, &LegTime0, 1.0e6, CRUISE_EFFICIENCY);

    if (LegTime0 >= 0.9 * ERROR_TIME) {
      // can't make it, so assume flying at current mc
      LegAltitude0 = LegAltitude;
    }

    TaskAltitudeRequired += LegAltitude;
    TaskAltitudeRequired0 += LegAltitude0;

    if (ISPARAGLIDER) {
      // if required altitude is less than previous turpoint altitude,
      //   use previous turn point altitude
      double w0Alt = FAIFinishHeight(Basic, Calculated, task_index - 1);
      if (TaskAltitudeRequired < w0Alt) {
        Calculated->TaskAltitudeArrival += w0Alt - TaskAltitudeRequired;

        TaskAltitudeRequired = w0Alt;
      }
      if (TaskAltitudeRequired0 < w0Alt) {
        TaskAltitudeRequired0 = w0Alt;
      }
    }

    Calculated->TaskDistanceToGo += NextLegDistance;
    Calculated->TaskTimeToGo += this_LegTimeToGo;

    if (task_index == 1) {
      StartBestCruiseTrack = NextLegBearing;
    }

    height_above_finish -= LegAltitude;

    task_index--;
  }

  // current waypoint, do this last!

  if (UseAATTarget() && (ActiveTaskPoint > 0) && ValidTaskPoint(ActiveTaskPoint + 1) && Calculated->IsInSector) {
    if (Calculated->WaypointDistance < AATCloseDistance() * 3.0) {
      LegBearing = AATCloseBearing(Basic, Calculated);
    }
  }

#ifdef BCT_ALT_FIX
  // Don't calculate BCT yet.  LegAltitude will be used to calculate
  // task altitude difference, which will then be used to calculate BCT.
#endif

  double LegAltitude =
      GlidePolar::MacCreadyAltitude(this_maccready, LegToGo, LegBearing, Calculated->WindSpeed, Calculated->WindBearing,
#ifdef BCT_ALT_FIX
                                    0,
#else
                                    &(Calculated->BestCruiseTrack),
#endif
                                    &(Calculated->VMacCready),

                                    // (Calculated->FinalGlide==1),
                                    true,  // JMW CHECK FGAMT

                                    &(Calculated->LegTimeToGo), height_above_finish, CRUISE_EFFICIENCY);

  double LegAltitude0 =
      GlidePolar::MacCreadyAltitude(0, LegToGo, LegBearing, Calculated->WindSpeed, Calculated->WindBearing, 0, 0, true,
                                    &LegTime0, 1.0e6, CRUISE_EFFICIENCY);

#ifndef BCT_ALT_FIX
  // fix problem of blue arrow wrong in task sector
  if (StartBestCruiseTrack >= 0)  // use it only if assigned, workaround
    if (Calculated->IsInSector && (ActiveTaskPoint == 0)) {
      // set best cruise track to first leg bearing when in start sector
      Calculated->BestCruiseTrack = StartBestCruiseTrack;
    }
#endif

  // JMW TODO accuracy: Use safetymc where appropriate

  if (LegTime0 >= 0.9 * ERROR_TIME) {
    // can't make it, so assume flying at current mc
    LegAltitude0 = LegAltitude;
  }

  TaskAltitudeRequired += LegAltitude;
  TaskAltitudeRequired0 += LegAltitude0;
  Calculated->TaskDistanceToGo += LegToGo;
  Calculated->TaskTimeToGo += Calculated->LegTimeToGo;

#ifndef BCT_ALT_FIX
  height_above_finish -= LegAltitude;
#endif

  if (ISPARAGLIDER) {
    Calculated->TaskAltitudeRequired = TaskAltitudeRequired;
  } else {
    Calculated->TaskAltitudeRequired = TaskAltitudeRequired + final_height;

    TaskAltitudeRequired0 += final_height;
  }

  Calculated->TaskAltitudeDifference = total_energy_height - Calculated->TaskAltitudeRequired;
  Calculated->TaskAltitudeDifference0 = total_energy_height - TaskAltitudeRequired0;
  Calculated->NextAltitudeDifference0 = total_energy_height - Calculated->NextAltitudeRequired0;
  if (Calculated->TaskStartTime > 0)
    Calculated->TaskElapsedTime = Basic->Time - Calculated->TaskStartTime;
  else
    Calculated->TaskElapsedTime = 0;
  Calculated->TaskAltitudeArrival += Calculated->TaskAltitudeDifference;

  Calculated->GRFinish = CalculateGlideRatio(Calculated->TaskDistanceToGo, Calculated->NavAltitude - final_height);

  if (Calculated->TaskSpeedAchieved > 0)
    Calculated->LKTaskETE = Calculated->TaskDistanceToGo / Calculated->TaskSpeedAchieved;
  else
    Calculated->LKTaskETE = 0;

  // recalculate for powered people
  if (ISCAR || ISGAAIRCRAFT) {
    if (Basic->Speed > 0)
      Calculated->LKTaskETE = Calculated->TaskDistanceToGo / Basic->Speed;
  }

#ifdef BCT_ALT_FIX
  // This MCA call's only purpose is to update BestCruiseTrack (BCT).
  // It must occur after TaskAltitudeDifference (TAD) is updated,
  // since BCT depends on TAD.

  GlidePolar::MacCreadyAltitude(this_maccready, LegToGo, LegBearing, Calculated->WindSpeed, Calculated->WindBearing,
                                &(Calculated->BestCruiseTrack), 0, true, 0, height_above_finish, CRUISE_EFFICIENCY,
                                Calculated->TaskAltitudeDifference);

  // fix problem of blue arrow wrong in task sector
  if (StartBestCruiseTrack >= 0)  // use it only if assigned, workaround
    if (Calculated->IsInSector && (ActiveTaskPoint == 0)) {
      // set best cruise track to first leg bearing when in start sector
      Calculated->BestCruiseTrack = StartBestCruiseTrack;
    }

  height_above_finish -= LegAltitude;
#endif

  CheckGlideThroughTerrain(Basic, Calculated);

  CheckForceFinalGlide(Basic, Calculated);
}
