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
extern AATDistance aatdistance;


void TaskStatistics(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
                    const double this_maccready)
{
  if( Calculated->ValidFinish) { // don't update statistics after task finished
    return;
  }
  if (!ValidTaskPoint(ActiveTaskPoint) || 
      ((ActiveTaskPoint>0) && !ValidTaskPoint(ActiveTaskPoint-1))) {


    Calculated->LegSpeed = 0;
    Calculated->LegDistanceToGo = 0;
    Calculated->LegDistanceCovered = 0;
    Calculated->LegTimeToGo = 0;

    if (gTaskType!=TSK_AAT) {
      Calculated->AATTimeToGo = 0;
    }

    //    Calculated->TaskSpeed = 0;

    Calculated->TaskDistanceToGo = 0;
    Calculated->TaskDistanceCovered = 0;
    Calculated->TaskTimeToGo = 0;
    Calculated->LKTaskETE = 0; 
    Calculated->TaskTimeToGoTurningNow = -1;

    Calculated->TaskAltitudeRequired = 0;
    Calculated->TaskAltitudeDifference = 0;
    Calculated->TaskAltitudeDifference0 = 0;

    Calculated->TaskAltitudeArrival = 0;

    Calculated->TerrainWarningLatitude = 0.0;
    Calculated->TerrainWarningLongitude = 0.0;

    Calculated->GRFinish = INVALID_GR;
   

    Calculated->FinalGlide = false;
    CheckGlideThroughTerrain(Basic, Calculated); // BUGFIX 091123
    
    // no task selected, so work things out at current heading

    GlidePolar::MacCreadyAltitude(this_maccready, 100.0, 
                                  Basic->TrackBearing, 
                                  Calculated->WindSpeed, 
                                  Calculated->WindBearing, 
                                  &(Calculated->BestCruiseTrack),
                                  &(Calculated->VMacCready),
                                  (Calculated->FinalGlide==true),
                                  NULL, 1.0e6, CRUISE_EFFICIENCY);
    return;
  }

  //  LockFlightData();
  LockTaskData();


  // Calculate Task Distances
  // First calculate distances for this waypoint

  double LegCovered, LegToGo=0, LegXTD=0, LegCurrentCourse;
  double LegDistance, LegBearing=0;
  bool calc_turning_now;

  double w1lat;
  double w1lon;
  double w0lat;
  double w0lon;
  
  if (UseAATTarget() && (ActiveTaskPoint>0) && (ValidTaskPoint(ActiveTaskPoint))) {
    w1lat = Task[ActiveTaskPoint].AATTargetLat;
    w1lon = Task[ActiveTaskPoint].AATTargetLon;
  } else {
    LKASSERT(ValidTaskPoint(ActiveTaskPoint));
    if (!ValidTaskPoint(ActiveTaskPoint)) {
        UnlockTaskData(); return;
    }
    w1lat = WayPointList[TASKINDEX].Latitude;
    w1lon = WayPointList[TASKINDEX].Longitude;
  }
  
  DistanceBearing(Basic->Latitude, 
                  Basic->Longitude, 
                  w1lat, 
                  w1lon, 
                  &LegToGo, &LegBearing);

  if (UseAATTarget() && (ActiveTaskPoint>0) && ValidTaskPoint(ActiveTaskPoint+1)
      && Calculated->IsInSector && (this_maccready>0.1) ) {
    calc_turning_now = true;
  } else {
    calc_turning_now = false;
  }

  if (ActiveTaskPoint<1) {
    LegCovered = 0;
    LegCurrentCourse=LegBearing;
    if (ValidTaskPoint(ActiveTaskPoint+1)) {  // BUGFIX 091221
      LegToGo=0;
    }
   } else {
    if (UseAATTarget()) {
      LKASSERT((ActiveTaskPoint-1)>=0);
      // TODO accuracy: Get best range point to here...
      w0lat = Task[ActiveTaskPoint-1].AATTargetLat;
      w0lon = Task[ActiveTaskPoint-1].AATTargetLon;
    } else {
      LKASSERT((ActiveTaskPoint-1)>=0);
      LKASSERT(ValidTaskPoint(ActiveTaskPoint-1));
      w0lat = WayPointList[Task[ActiveTaskPoint-1].Index].Latitude;
      w0lon = WayPointList[Task[ActiveTaskPoint-1].Index].Longitude;
    }
    
    DistanceBearing(w1lat, 
                    w1lon,
                    w0lat, 
                    w0lon,
                    &LegDistance, NULL);
    
    LegCovered = ProjectedDistance(w0lon, w0lat,
                                   w1lon, w1lat,
                                   Basic->Longitude,
                                   Basic->Latitude,
                                   &LegXTD, &LegCurrentCourse);

    if ((StartLine == sector_type_t::CIRCLE) && (ActiveTaskPoint == 1)) {
      // Correct speed calculations for radius
      // JMW TODO accuracy: legcovered replace this with more accurate version
      // LegDistance -= StartRadius;
      LegCovered = max(0.0, LegCovered-StartRadius);
    }
  }
  
  Calculated->LegDistanceToGo = LegToGo;
  Calculated->LegDistanceCovered = LegCovered;
  Calculated->LegCrossTrackError = LegXTD;
  Calculated->LegActualTrueCourse = LegCurrentCourse;
  Calculated->TaskDistanceCovered = LegCovered;
  
  if (Basic->Time > Calculated->LegStartTime) {
    if (flightstats.LegStartTime[ActiveTaskPoint]<0) {
      flightstats.LegStartTime[ActiveTaskPoint] = Basic->Time;
    }
    Calculated->LegSpeed = Calculated->LegDistanceCovered
      / (Basic->Time - Calculated->LegStartTime); 
  }

  // Now add distances for start to previous waypoint
 
    if (!UseAATTarget()) {
      for(int i=0;i< ActiveTaskPoint-1; i++)
        {
          if (!ValidTaskPoint(i) || !ValidTaskPoint(i+1)) continue;
          
          w1lat = WayPointList[Task[i].Index].Latitude;
          w1lon = WayPointList[Task[i].Index].Longitude;
          w0lat = WayPointList[Task[i+1].Index].Latitude;
          w0lon = WayPointList[Task[i+1].Index].Longitude;
          
          DistanceBearing(w1lat, 
                          w1lon,
                          w0lat, 
                          w0lon,
                          &LegDistance, NULL);                      
          Calculated->TaskDistanceCovered += LegDistance;
        }
    } else if (ActiveTaskPoint>0) {
      // JMW added correction for distance covered
      Calculated->TaskDistanceCovered = 
        aatdistance.DistanceCovered(Basic->Longitude,
                                    Basic->Latitude,
                                    ActiveTaskPoint);
    }

  // If it is not a glider, or if it is a glider and it is freeflying with take off since 5 minutes
  if (!(ISGLIDER||ISPARAGLIDER) || (Calculated->FreeFlying && Calculated->FlightTime>(60*5)))
     CheckTransitionFinalGlide(Basic, Calculated);

  // accumulators
  double TaskAltitudeRequired = 0;
  double TaskAltitudeRequired0 = 0;
  Calculated->TaskDistanceToGo = 0;
  Calculated->TaskTimeToGo = 0;
  Calculated->LKTaskETE = 0;
  Calculated->TaskTimeToGoTurningNow = 0;
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
  int task_index= FinalWayPoint;

  double StartBestCruiseTrack = -1; 

    while ((task_index>ActiveTaskPoint) && (ValidTaskPoint(task_index))) {
      double this_LegTimeToGo;
      bool this_is_final = (task_index==FinalWayPoint)
	|| ForceFinalGlide;

      this_is_final = true; // JMW CHECK FGAMT
      
      if (UseAATTarget()) {
	w1lat = Task[task_index].AATTargetLat;
	w1lon = Task[task_index].AATTargetLon;
	w0lat = Task[task_index-1].AATTargetLat;
	w0lon = Task[task_index-1].AATTargetLon;
      } else {
	w1lat = WayPointList[Task[task_index].Index].Latitude;
	w1lon = WayPointList[Task[task_index].Index].Longitude;
	w0lat = WayPointList[Task[task_index-1].Index].Latitude;
	w0lon = WayPointList[Task[task_index-1].Index].Longitude;
      }
      
      double NextLegDistance, NextLegBearing;
      
      DistanceBearing(w0lat, 
		      w0lon,
		      w1lat, 
		      w1lon,
		      &NextLegDistance, &NextLegBearing);
      
      double LegAltitude = GlidePolar::
	MacCreadyAltitude(this_maccready, 
			  NextLegDistance, NextLegBearing, 
			  Calculated->WindSpeed, 
			  Calculated->WindBearing, 
			  0, 0,
			  this_is_final,
			  &this_LegTimeToGo,
			  height_above_finish, CRUISE_EFFICIENCY);

      double LegAltitude0 = GlidePolar::
	MacCreadyAltitude(0, 
			  NextLegDistance, NextLegBearing, 
			  Calculated->WindSpeed, 
			  Calculated->WindBearing, 
			  0, 0,
			  true,
			  &LegTime0, 1.0e6, CRUISE_EFFICIENCY
			  );
      
      if (LegTime0>=0.9*ERROR_TIME) {
	// can't make it, so assume flying at current mc
	LegAltitude0 = LegAltitude;
      }          

      TaskAltitudeRequired += LegAltitude;
      TaskAltitudeRequired0 += LegAltitude0;

      if(ISPARAGLIDER) {
      	// if required altitude is less than previous turpoint altitude,
      	//   use previous turn point altitude
      	double w0Alt = FAIFinishHeight(Basic, Calculated, task_index-1);
      	if(TaskAltitudeRequired < w0Alt) {
            Calculated->TaskAltitudeArrival += w0Alt - TaskAltitudeRequired;

            TaskAltitudeRequired = w0Alt;
      	}
      	if(TaskAltitudeRequired0 < w0Alt) {
        	  TaskAltitudeRequired0 = w0Alt;
      	}
      }
      
      Calculated->TaskDistanceToGo += NextLegDistance;
      Calculated->TaskTimeToGo += this_LegTimeToGo;      

	if (task_index==1) {
		StartBestCruiseTrack = NextLegBearing;
	}

      if (calc_turning_now) {
	if (task_index == ActiveTaskPoint+1) {
	  
	  double NextLegDistanceTurningNow, NextLegBearingTurningNow;
	  double this_LegTimeToGo_turningnow=0;
	  
	  DistanceBearing(Basic->Latitude, 
			  Basic->Longitude,
			  w1lat, 
			  w1lon,
			  &NextLegDistanceTurningNow, 
			  &NextLegBearingTurningNow);
	  
	  GlidePolar::
	    MacCreadyAltitude(this_maccready, 
			      NextLegDistanceTurningNow, 
			      NextLegBearingTurningNow, 
			      Calculated->WindSpeed, 
			      Calculated->WindBearing, 
			      0, 0,
			      this_is_final,
			      &this_LegTimeToGo_turningnow,
			      height_above_finish, CRUISE_EFFICIENCY); 
	  Calculated->TaskTimeToGoTurningNow += this_LegTimeToGo_turningnow;
	} else {
	  Calculated->TaskTimeToGoTurningNow += this_LegTimeToGo;
	}
      }
      
      height_above_finish-= LegAltitude;
      
      task_index--;
    }


  // current waypoint, do this last!

  if (UseAATTarget() && (ActiveTaskPoint>0) && ValidTaskPoint(ActiveTaskPoint+1) && Calculated->IsInSector) {
	if (Calculated->WaypointDistance<AATCloseDistance()*3.0) {
		LegBearing = AATCloseBearing(Basic, Calculated);
	}
  }
  
#ifdef BCT_ALT_FIX
  // Don't calculate BCT yet.  LegAltitude will be used to calculate
  // task altitude difference, which will then be used to calculate BCT.
#endif

  double LegAltitude = 
    GlidePolar::MacCreadyAltitude(this_maccready, 
                                  LegToGo, 
                                  LegBearing, 
                                  Calculated->WindSpeed, 
                                  Calculated->WindBearing,
                                #ifdef BCT_ALT_FIX
                                  0,
                                #else
                                  &(Calculated->BestCruiseTrack),
                                #endif
                                  &(Calculated->VMacCready),

				  // (Calculated->FinalGlide==1),
				  true,  // JMW CHECK FGAMT

                                  &(Calculated->LegTimeToGo),
                                  height_above_finish, CRUISE_EFFICIENCY);
  
  double LegAltitude0 = 
    GlidePolar::MacCreadyAltitude(0, 
                                  LegToGo, 
                                  LegBearing, 
                                  Calculated->WindSpeed, 
                                  Calculated->WindBearing,
                                  0,
                                  0,
                                  true,
                                  &LegTime0, 1.0e6, CRUISE_EFFICIENCY
                                  );

#ifndef BCT_ALT_FIX
  // fix problem of blue arrow wrong in task sector
  if (StartBestCruiseTrack>=0)  // use it only if assigned, workaround
	if (Calculated->IsInSector && (ActiveTaskPoint==0)) {
		// set best cruise track to first leg bearing when in start sector
		Calculated->BestCruiseTrack = StartBestCruiseTrack;
	} 
#endif

  // JMW TODO accuracy: Use safetymc where appropriate

  if (LegTime0>= 0.9*ERROR_TIME) {
    // can't make it, so assume flying at current mc
    LegAltitude0 = LegAltitude;
  }

  TaskAltitudeRequired += LegAltitude;
  TaskAltitudeRequired0 += LegAltitude0;
  Calculated->TaskDistanceToGo += LegToGo;
  Calculated->TaskTimeToGo += Calculated->LegTimeToGo;

#ifndef BCT_ALT_FIX
  height_above_finish-= LegAltitude;
#endif

  if (calc_turning_now) {
    Calculated->TaskTimeToGoTurningNow += 
      Basic->Time-Calculated->TaskStartTime;
  } else {
    Calculated->TaskTimeToGoTurningNow = -1;
  }


  if (ISPARAGLIDER) {
  Calculated->TaskAltitudeRequired = TaskAltitudeRequired;
  } else {
  Calculated->TaskAltitudeRequired = TaskAltitudeRequired + final_height;
 
  TaskAltitudeRequired0 += final_height;
  }
  
  Calculated->TaskAltitudeDifference = total_energy_height - Calculated->TaskAltitudeRequired; 
  Calculated->TaskAltitudeDifference0 = total_energy_height - TaskAltitudeRequired0;
  Calculated->NextAltitudeDifference0 = total_energy_height - Calculated->NextAltitudeRequired0;
  if( Calculated->TaskStartTime > 0)
    Calculated->TaskElapsedTime =  Basic->Time-  Calculated->TaskStartTime;
  else
    Calculated->TaskElapsedTime =0;
  Calculated->TaskAltitudeArrival += Calculated->TaskAltitudeDifference;

  Calculated->GRFinish= CalculateGlideRatio(Calculated->TaskDistanceToGo, Calculated->NavAltitude - final_height);

  if (Calculated->TaskSpeedAchieved >0)
	Calculated->LKTaskETE = Calculated->TaskDistanceToGo/Calculated->TaskSpeedAchieved;
  else
	Calculated->LKTaskETE=0;

  // recalculate for powered people
  if (ISCAR  || ISGAAIRCRAFT) {
        if (Basic->Speed>0) Calculated->LKTaskETE = Calculated->TaskDistanceToGo/Basic->Speed;
  }

#ifdef BCT_ALT_FIX
  // This MCA call's only purpose is to update BestCruiseTrack (BCT).
  // It must occur after TaskAltitudeDifference (TAD) is updated,
  // since BCT depends on TAD.

  GlidePolar::MacCreadyAltitude(this_maccready,
                                LegToGo,
                                LegBearing,
                                Calculated->WindSpeed,
                                Calculated->WindBearing,
                                &(Calculated->BestCruiseTrack),
                                0,
                                true,
                                0,
                                height_above_finish,
                                CRUISE_EFFICIENCY,
                                Calculated->TaskAltitudeDifference);

  // fix problem of blue arrow wrong in task sector
  if (StartBestCruiseTrack>=0)  // use it only if assigned, workaround
    if (Calculated->IsInSector && (ActiveTaskPoint==0)) {
      // set best cruise track to first leg bearing when in start sector
      Calculated->BestCruiseTrack = StartBestCruiseTrack;
    } 

  height_above_finish-= LegAltitude;
#endif

  CheckGlideThroughTerrain(Basic, Calculated);
  
  CheckForceFinalGlide(Basic, Calculated);
  
  UnlockTaskData();

}

