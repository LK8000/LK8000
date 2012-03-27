/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "CalcTask.h"
#include "utils/heapcheck.h"

extern AATDistance aatdistance;


void AddAATPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int taskwaypoint) {
  bool insector = false;
  if (taskwaypoint>0) {
    if (AATEnabled) {
      insector = InAATTurnSector(Basic->Longitude,
                                 Basic->Latitude, taskwaypoint);
    } else {
      insector = InTurnSector(Basic, Calculated, taskwaypoint);
    }
    if(insector) {
      if (taskwaypoint == ActiveWayPoint) {
        Calculated->IsInSector = true;
      }
      aatdistance.AddPoint(Basic->Longitude,
                           Basic->Latitude,
                           taskwaypoint);
    }
  }
}

double AATCloseBearing(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // ensure waypoint goes in direction of track if very close
  double course_bearing;
  DistanceBearing(Task[ActiveWayPoint-1].AATTargetLat,
		  Task[ActiveWayPoint-1].AATTargetLon,
		  Basic->Latitude,
		  Basic->Longitude,
		  NULL, &course_bearing);
  
  course_bearing = AngleLimit360(course_bearing+
				 Task[ActiveWayPoint].AATTargetOffsetRadial);
  return course_bearing;
}
