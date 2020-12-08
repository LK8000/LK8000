/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "NavFunctions.h"


extern bool TargetDialogOpen;



void CalculateAATTaskSectors()
{
  int awp = ActiveTaskPoint;

  if(!UseAATTarget())
    return;

  double latitude = GPS_INFO.Latitude;
  double longitude = GPS_INFO.Longitude;
  double altitude = GPS_INFO.Altitude;

  LockTaskData();

  Task[0].AATTargetOffsetRadius = 0.0;
  Task[0].AATTargetOffsetRadial = 0.0;
  if (Task[0].Index>=0) {
    Task[0].AATTargetLat = WayPointList[Task[0].Index].Latitude;
    Task[0].AATTargetLon = WayPointList[Task[0].Index].Longitude;
  }

  for(int i=1;i<MAXTASKPOINTS;i++) {
    if(ValidTaskPoint(i)) {
      if (!ValidTaskPoint(i+1)) {
        // This must be the final waypoint, so it's not an AAT OZ
        Task[i].AATTargetLat = WayPointList[Task[i].Index].Latitude;
        Task[i].AATTargetLon = WayPointList[Task[i].Index].Longitude;
        continue;
      }

      // JMWAAT: if locked, don't move it
      if (i<awp) {
        // only update targets for current/later waypoints
        continue;
      }

      Task[i].AATTargetOffsetRadius =
        min(1.0, max(Task[i].AATTargetOffsetRadius,-1.0));

      Task[i].AATTargetOffsetRadial =
        min(90.0, max(-90.0, Task[i].AATTargetOffsetRadial));

      double targetbearing;
      double targetrange;

      targetbearing = AngleLimit360(Task[i].Bisector+Task[i].AATTargetOffsetRadial);

      if(Task[i].AATType == SECTOR) {

        //AATStartRadial
        //AATFinishRadial

        targetrange = ((Task[i].AATTargetOffsetRadius+1.0)/2.0);

        double aatbisector = HalfAngle(Task[i].AATStartRadial,
                                       Task[i].AATFinishRadial);

        if (fabs(AngleLimit180(aatbisector-targetbearing))>90) {
          // bisector is going away from sector
          targetbearing = Reciprocal(targetbearing);
          targetrange = 1.0-targetrange;
        }
        if (!AngleInRange(Task[i].AATStartRadial,
                          Task[i].AATFinishRadial,
                          targetbearing,true)) {

          // Bisector is not within AAT sector, so
          // choose the closest radial as the target line

          if (fabs(AngleLimit180(Task[i].AATStartRadial-targetbearing))
              <fabs(AngleLimit180(Task[i].AATFinishRadial-targetbearing))) {
            targetbearing = Task[i].AATStartRadial;
          } else {
            targetbearing = Task[i].AATFinishRadial;
          }
        }

        targetrange*= Task[i].AATSectorRadius;

      } else {
        targetrange = Task[i].AATTargetOffsetRadius
          *Task[i].AATCircleRadius;
      }

      // TODO accuracy: if i=awp and in sector, range parameter needs to
      // go from current aircraft position to projection of target
      // out to the edge of the sector

      if (InAATTurnSector(longitude, latitude, i, altitude) && (awp==i) &&
          !Task[i].AATTargetLocked) {

        // special case, currently in AAT sector/cylinder

        double dist;
        double qdist;
        double bearing;

        // find bearing from last target through current aircraft position with offset
        DistanceBearing(Task[i-1].AATTargetLat,
                        Task[i-1].AATTargetLon,
                        latitude,
                        longitude,
                        &qdist, &bearing);

        bearing = AngleLimit360(bearing+Task[i].AATTargetOffsetRadial);

        dist = ((Task[i].AATTargetOffsetRadius+1)/2.0)*
          FindInsideAATSectorDistance(latitude, longitude, i, bearing);

        // if (dist+qdist>aatdistance.LegDistanceAchieved(awp)) {
        // JMW: don't prevent target from being closer to the aircraft
        // than the best achieved, so can properly plan arrival time

        FindLatitudeLongitude (latitude,
                               longitude,
                               bearing,
                               dist,
                               &Task[i].AATTargetLat,
                               &Task[i].AATTargetLon);

        UpdateTargetAltitude(Task[i]);

        TargetModified = true;

        // }

      } else {

        FindLatitudeLongitude (WayPointList[Task[i].Index].Latitude,
                               WayPointList[Task[i].Index].Longitude,
                               targetbearing,
                               targetrange,
                               &Task[i].AATTargetLat,
                               &Task[i].AATTargetLon);

        UpdateTargetAltitude(Task[i]);
        TargetModified = true;

      }
    }
  }

  CalculateAATIsoLines();
  if (!TargetDialogOpen) {
    TargetModified = false;
    // allow target dialog to detect externally changed targets
  }

  UnlockTaskData();
}
