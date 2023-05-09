/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "CalcTask.h"
#include "NavFunctions.h"

extern AATDistance aatdistance;


void AddAATPoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int taskwaypoint) {
  if (taskwaypoint>0) {
    bool insector = InTurnSector(Basic, taskwaypoint);

    if (taskwaypoint == ActiveTaskPoint) {
      Calculated->IsInSector = insector;
    }

    if(insector) {
      aatdistance.AddPoint(Basic->Longitude, Basic->Latitude, taskwaypoint);
    }
  }
}

double AATCloseBearing(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  // ensure waypoint goes in direction of track if very close
  double course_bearing;
  DistanceBearing(Task[ActiveTaskPoint-1].AATTargetLat,
		  Task[ActiveTaskPoint-1].AATTargetLon,
		  Basic->Latitude,
		  Basic->Longitude,
		  NULL, &course_bearing);
  
  course_bearing = AngleLimit360(course_bearing+
				 Task[ActiveTaskPoint].AATTargetOffsetRadial);
  return course_bearing;
}
