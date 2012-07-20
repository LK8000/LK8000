/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


bool TaskModified = false;
bool TargetModified = false;
TCHAR LastTaskFileName[MAX_PATH]= TEXT("\0");



void ResetTaskWaypoint(int j) {
  Task[j].Index = -1;
  if (DoOptimizeRoute())
  	Task[j].AATTargetOffsetRadius = -100.0;
  else
  	Task[j].AATTargetOffsetRadius = 0.0;
  Task[j].AATTargetOffsetRadial = 0.0;
  Task[j].AATTargetLocked = false;
  Task[j].AATSectorRadius = SectorRadius;
  Task[j].AATCircleRadius = SectorRadius;
  Task[j].AATStartRadial = 0;
  Task[j].AATFinishRadial = 360;
}



void RefreshTaskWaypoint(int i) {
  if(i==0)
    { 
      Task[i].Leg = 0;
      Task[i].InBound = 0;
    }
  else
    {
      DistanceBearing(WayPointList[Task[i].Index].Latitude,
                      WayPointList[Task[i].Index].Longitude,
                      WayPointList[Task[i-1].Index].Latitude,
                      WayPointList[Task[i-1].Index].Longitude,
                      &Task[i].Leg,
                      &Task[i].InBound);
      Task[i].InBound += 180;
      if (Task[i].InBound >= 360)
        Task[i].InBound -= 360;

      Task[i-1].OutBound = Task[i].InBound;
      Task[i-1].Bisector = BiSector(Task[i-1].InBound,Task[i-1].OutBound);
      if (i==1) {
        if (EnableMultipleStartPoints) {
          for (int j=0; j<MAXSTARTPOINTS; j++) {
            if ((StartPoints[j].Index != -1)&&(StartPoints[j].Active)) {
              DistanceBearing(WayPointList[StartPoints[j].Index].Latitude,   
                              WayPointList[StartPoints[j].Index].Longitude,
                              WayPointList[Task[i].Index].Latitude, 
                              WayPointList[Task[i].Index].Longitude,
                              NULL, &StartPoints[j].OutBound);
            }
          }
        }
      }
    }
}



double FindInsideAATSectorDistance_old(double latitude,
                                       double longitude,
                                       int taskwaypoint, 
                                       double course_bearing,
                                       double p_found) {
  bool t_in_sector;
  double delta;
  double max_distance;
  if(Task[taskwaypoint].AATType == SECTOR) {
    max_distance = Task[taskwaypoint].AATSectorRadius*2;
  } else {
    max_distance = Task[taskwaypoint].AATCircleRadius*2;
  }
  delta = max(250.0, max_distance/40.0);

  double t_distance = p_found;
  double t_distance_inside;

  do {
    double t_lat, t_lon;
    t_distance_inside = t_distance;
    t_distance += delta;

    FindLatitudeLongitude(latitude, longitude, 
                          course_bearing, t_distance,
                          &t_lat,
                          &t_lon);
    
    t_in_sector = InAATTurnSector(t_lon,
                                  t_lat,
                                  taskwaypoint);

  } while (t_in_sector);

  return t_distance_inside;
}


double FindInsideAATSectorDistance(double latitude,
                                   double longitude,
                                   int taskwaypoint, 
                                   double course_bearing,
                                   double p_found) {

  double max_distance;
  if(Task[taskwaypoint].AATType == SECTOR) {
    max_distance = Task[taskwaypoint].AATSectorRadius;
  } else {
    max_distance = Task[taskwaypoint].AATCircleRadius;
  }

  // Do binary bounds search for longest distance within sector

  double delta = max_distance;
  double t_distance_lower = p_found;
  double t_distance = p_found+delta*2;
  int steps = 0;
  do {

    double t_lat, t_lon;
    FindLatitudeLongitude(latitude, longitude, 
                          course_bearing, t_distance,
                          &t_lat, &t_lon);

    if (InAATTurnSector(t_lon, t_lat, taskwaypoint)) {
      t_distance_lower = t_distance;
      // ok, can go further
      t_distance += delta;
    } else {
      t_distance -= delta;
    }
    delta /= 2.0;
  } while ((delta>5.0)&&(steps++<20));

  return t_distance_lower;
}


double FindInsideAATSectorRange(double latitude,
                                double longitude,
                                int taskwaypoint, 
                                double course_bearing,
                                double p_found) {

  double t_distance = FindInsideAATSectorDistance(latitude, longitude, taskwaypoint,
                                                  course_bearing, p_found);
  return (p_found / 
          max(1.0,t_distance))*2-1;
}


double DoubleLegDistance(int taskwaypoint,
                         double longitude,
                         double latitude) {

  if (taskwaypoint>0) {
    return DoubleDistance(Task[taskwaypoint-1].AATTargetLat,
			  Task[taskwaypoint-1].AATTargetLon,
			  latitude,
			  longitude,
			  Task[taskwaypoint+1].AATTargetLat,
			  Task[taskwaypoint+1].AATTargetLon);    
  } else {
    double d1;
    DistanceBearing(latitude,
		    longitude,
		    Task[taskwaypoint+1].AATTargetLat,
		    Task[taskwaypoint+1].AATTargetLon,
		    &d1, NULL);
    return d1;
  }

}


