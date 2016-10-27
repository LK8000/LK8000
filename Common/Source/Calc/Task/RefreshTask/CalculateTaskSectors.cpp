/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


void CalculateTaskSectors(void)
{
  int i;
  double SectorAngle, SectorSize, SectorBearing;

  LockTaskData();

  if (EnableMultipleStartPoints) {
    for(i=0;i<MAXSTARTPOINTS-1;i++) {
      if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
	if (StartLine==2) {
          SectorAngle = 45+90;
        } else {
          SectorAngle = 90;
        }
        SectorSize = StartRadius;
        SectorBearing = StartPoints[i].OutBound;

        FindLatitudeLongitude(WayPointList[StartPoints[i].Index].Latitude,
                              WayPointList[StartPoints[i].Index].Longitude,
                              SectorBearing + SectorAngle, SectorSize,
                              &StartPoints[i].SectorStartLat,
                              &StartPoints[i].SectorStartLon);

        FindLatitudeLongitude(WayPointList[StartPoints[i].Index].Latitude,
                              WayPointList[StartPoints[i].Index].Longitude,
                              SectorBearing - SectorAngle, SectorSize,
                              &StartPoints[i].SectorEndLat,
                              &StartPoints[i].SectorEndLon);
      }
    }
  }

  for(i=0;i<=MAXTASKPOINTS-1;i++)
    {
      if((Task[i].Index >=0))
	{
	  if ((Task[i+1].Index >=0)||(i==MAXTASKPOINTS-1)) {

	    if(i == 0)
	      {
		// start line
		if (StartLine==2) {
		  SectorAngle = 45+90;
		} else {
		  SectorAngle = 90;
		}
		SectorSize = StartRadius;
		SectorBearing = Task[i].OutBound;
	      }
	    else
	      {
		// normal turnpoint sector
                if(SectorType==LINE) {
                   SectorAngle = 90;
                } else {
                   SectorAngle = 45;
                }
		if (SectorType == DAe) {
		  SectorSize = 10000; // German DAe 0.5/10
		} else {
		  SectorSize = SectorRadius;  // FAI sector
		}
		SectorBearing = Task[i].Bisector;
                if(SectorType==LINE) {
                    SectorBearing += 90;
                }
	      }
	  } else {
	    // finish line
	    if (FinishLine==2) {
	      SectorAngle = 45;
	    } else {
	      SectorAngle = 90;
	    }
	    SectorSize = FinishRadius;
	    SectorBearing = Task[i].InBound;

	  }

          FindLatitudeLongitude(WayPointList[Task[i].Index].Latitude,
                                WayPointList[Task[i].Index].Longitude,
                                SectorBearing + SectorAngle, SectorSize,
                                &Task[i].SectorStartLat,
                                &Task[i].SectorStartLon);

          FindLatitudeLongitude(WayPointList[Task[i].Index].Latitude,
                                WayPointList[Task[i].Index].Longitude,
                                SectorBearing - SectorAngle, SectorSize,
                                &Task[i].SectorEndLat,
                                &Task[i].SectorEndLon);

          if (!AATEnabled)
          {
            Task[i].AATStartRadial  =
              AngleLimit360(SectorBearing - SectorAngle);
            Task[i].AATFinishRadial =
              AngleLimit360(SectorBearing + SectorAngle);
          }

	}
    }
  UnlockTaskData();
}
