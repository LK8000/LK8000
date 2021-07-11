/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"



void CalculateAATIsoLines(void) {
  int i;
  int awp = ActiveTaskPoint;
  double stepsize = 25.0;

  if(gTaskType==TSK_AAT)
    return;

  LockTaskData();

  for(i=1;i<MAXTASKPOINTS;i++) {

    if(ValidTaskPoint(i)) {
      if (!ValidTaskPoint(i+1)) {
        // This must be the final waypoint, so it's not an AAT OZ
        continue;
      }
      // JMWAAT: if locked, don't move it
      if (i<awp) {
        // only update targets for current/later waypoints
        continue;
      }

      int j;
      for (j=0; j<MAXISOLINES; j++) {
        TaskStats[i].IsoLine_valid[j] = false;
      }

      double latitude = Task[i].AATTargetLat;
      double longitude = Task[i].AATTargetLon;
      double dist_0, dist_north, dist_east;
      bool in_sector = true;

      double max_distance, delta;
      if(Task[i].AATType == SECTOR) {
        max_distance = Task[i].AATSectorRadius;
      } else {
        max_distance = Task[i].AATCircleRadius;
      }
      delta = max_distance*2.4 / (MAXISOLINES);
      bool left = false;

      /*
      double distance_glider=0;
      if ((i==ActiveTaskPoint) && (CALCULATED_INFO.IsInSector)) {
        distance_glider = DoubleLegDistance(i, GPS_INFO.Longitude, GPS_INFO.Latitude);
      }
      */

      // fill
      j=0;
      // insert start point
      TaskStats[i].IsoLine_Latitude[j] = latitude;
      TaskStats[i].IsoLine_Longitude[j] = longitude;
      TaskStats[i].IsoLine_valid[j] = true;
      j++;

      do {
        dist_0 = DoubleLegDistance(i, longitude, latitude);

        double latitude_north, longitude_north;
        FindLatitudeLongitude(latitude, longitude,
                              0, stepsize,
                              &latitude_north,
                              &longitude_north);
        dist_north = DoubleLegDistance(i, longitude_north, latitude_north);

        double latitude_east, longitude_east;
        FindLatitudeLongitude(latitude, longitude,
                              90, stepsize,
                              &latitude_east,
                              &longitude_east);
        dist_east = DoubleLegDistance(i, longitude_east, latitude_east);

        double angle = AngleLimit360(RAD_TO_DEG*atan2(dist_east-dist_0, dist_north-dist_0)+90);
        if (left) {
          angle += 180;
        }

        FindLatitudeLongitude(latitude, longitude,
                              angle, delta,
                              &latitude,
                              &longitude);

        in_sector = InAATTurnSector(longitude, latitude, i, 0);
        /*
        if (dist_0 < distance_glider) {
          in_sector = false;
        }
        */
        if (in_sector) {
          TaskStats[i].IsoLine_Latitude[j] = latitude;
          TaskStats[i].IsoLine_Longitude[j] = longitude;
          TaskStats[i].IsoLine_valid[j] = true;
          j++;
        } else {
          j++;
          if (!left && (j<MAXISOLINES-2))  {
            left = true;
            latitude = Task[i].AATTargetLat;
            longitude = Task[i].AATTargetLon;
            in_sector = true; // cheat to prevent early exit

            // insert start point (again)
            TaskStats[i].IsoLine_Latitude[j] = latitude;
            TaskStats[i].IsoLine_Longitude[j] = longitude;
            TaskStats[i].IsoLine_valid[j] = true;
            j++;
          }
        }
      } while (in_sector && (j<MAXISOLINES));

    }
  }
  UnlockTaskData();
}
