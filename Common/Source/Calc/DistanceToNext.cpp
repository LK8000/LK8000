/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"
#include "AATDistance.h"

extern double AATCloseBearing(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

void DistanceToNext(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
  //  LockFlightData();
  LockTaskData();

  if(ValidTaskPoint(ActiveTaskPoint))
    {
      double w1lat, w1lon;
      double w0lat, w0lon;

      if(DoOptimizeRoute()) {
        w0lat = Task[ActiveTaskPoint].AATTargetLat;
        w0lon = Task[ActiveTaskPoint].AATTargetLon;
      } else {
        w0lat = WayPointList[TASKINDEX].Latitude;
        w0lon = WayPointList[TASKINDEX].Longitude;
      }

      DistanceBearing(Basic->Latitude, Basic->Longitude,
                      w0lat, w0lon,
                      &Calculated->WaypointDistance,
                      &Calculated->WaypointBearing);

      Calculated->ZoomDistance = Calculated->WaypointDistance;

      if (UseAATTarget()
	  && (ActiveTaskPoint>0) && 
          ValidTaskPoint(ActiveTaskPoint+1)) {

        w1lat = Task[ActiveTaskPoint].AATTargetLat;
        w1lon = Task[ActiveTaskPoint].AATTargetLon;

        DistanceBearing(Basic->Latitude, Basic->Longitude,
                        w1lat, w1lon,
                        &Calculated->WaypointDistance,
                        &Calculated->WaypointBearing);

        if (Calculated->WaypointDistance>AATCloseDistance()*3.0) {
          Calculated->ZoomDistance = max(Calculated->WaypointDistance,
                                         Calculated->ZoomDistance);
        } else {
	  Calculated->WaypointBearing = AATCloseBearing(Basic, Calculated);
        }

      } else if ((ActiveTaskPoint==0) && (ValidTaskPoint(ActiveTaskPoint+1))
                 && (Calculated->IsInSector) ) {

        // JMW set waypoint bearing to start direction if in start sector

        if (UseAATTarget()) {
          w1lat = Task[ActiveTaskPoint+1].AATTargetLat;
          w1lon = Task[ActiveTaskPoint+1].AATTargetLon;
        } else {
          w1lat = WayPointList[Task[ActiveTaskPoint+1].Index].Latitude; 
          w1lon = WayPointList[Task[ActiveTaskPoint+1].Index].Longitude;
        }

        DistanceBearing(Basic->Latitude, Basic->Longitude,
                        w1lat, w1lon,
                        NULL,
                        &Calculated->WaypointBearing);
      }
    }
  else
    {
      Calculated->ZoomDistance = 0;
      Calculated->WaypointDistance = 0;
      Calculated->WaypointBearing = 0;
    }
  UnlockTaskData();
  //  UnlockFlightData();
}
