/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "NavFunctions.h"

extern double CRUISE_EFFICIENCY;

bool TaskAltitudeRequired(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                                 double this_maccready, double *Vfinal,
                                 double *TotalTime, double *TotalDistance,
                                 int *ifinal)
{
  int i;
  double w1lat;
  double w1lon;
  double w0lat;
  double w0lon;
  double LegTime, LegDistance, LegBearing, LegAltitude;
  bool retval = false;

  // Calculate altitude required from start of task

  bool isfinal=true;
  LegAltitude = 0;
  double TotalAltitude = 0;
  *TotalTime = 0; *TotalDistance = 0;
  *ifinal = 0;

  LockTaskData();

  double heightFinal = FAIFinishHeight(Basic, Calculated, -1);
  double height_above_finish = FAIFinishHeight(Basic, Calculated, 0) - heightFinal;

  TotalAltitude = heightFinal; // start from final height

  for(i=MAXTASKPOINTS-2;i>=0;i--) {


    if (!ValidTaskPoint(i) || !ValidTaskPoint(i+1)) continue;
    
    w1lat = WayPointList[Task[i].Index].Latitude;
    w1lon = WayPointList[Task[i].Index].Longitude;
    w0lat = WayPointList[Task[i+1].Index].Latitude;
    w0lon = WayPointList[Task[i+1].Index].Longitude;
    
    if (UseAATTarget()) {
      w1lat = Task[i].AATTargetLat;
      w1lon = Task[i].AATTargetLon;
      
      // also use optimized finish point for PG optimized task.
      if (!isfinal || DoOptimizeRoute()) {
        w0lat = Task[i+1].AATTargetLat;
        w0lon = Task[i+1].AATTargetLon;
      }
    }
    
    DistanceBearing(w1lat, w1lon,
                    w0lat, w0lon,
                    &LegDistance, &LegBearing);

    *TotalDistance += LegDistance;
    
    LegAltitude = 
      GlidePolar::MacCreadyAltitude(this_maccready, 
                                    LegDistance, 
                                    LegBearing, 
                                    Calculated->WindSpeed, 
                                    Calculated->WindBearing,
                                    0,
                                    0,
                                    true,
                                    &LegTime,
				    height_above_finish, 
				    CRUISE_EFFICIENCY
                                    );

    // JMW CHECK FGAMT
    height_above_finish-= LegAltitude;

    TotalAltitude += LegAltitude; // Add leg requiered height

    if( ISPARAGLIDER ) {
    	double w1Alt = FAIFinishHeight(Basic, Calculated, i);
    	if( TotalAltitude < w1Alt ) {
            // if required altitude is less this turpoint altitude,
	    	//   use previous this point altitude ( takes safety arrival alt into account )
	        TotalAltitude = w1Alt;
    	}
    }

    if (LegTime<0) {
		retval = false;
		goto OnExit;
    } else {
      *TotalTime += LegTime;
    }
    if (isfinal) {
      *ifinal = i+1;
      if (LegTime>0) {
        *Vfinal = LegDistance/LegTime;
      }
    }
    isfinal = false;
  }

  if (*ifinal==0) {
    retval = false;
    goto OnExit;
  }

  if (!ValidTaskPoint(*ifinal)) {
    Calculated->TaskAltitudeRequiredFromStart = TotalAltitude;
    retval = false;
  } else {
    Calculated->TaskAltitudeRequiredFromStart = TotalAltitude;
    retval = true;
  }
 OnExit:
  UnlockTaskData();
  return retval;
}

