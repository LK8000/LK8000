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

      double error_factor=0;

      if((Task[i].Index >=0))
	{
	  if ((Task[i+1].Index >=0)||(i==MAXTASKPOINTS-1)) {

	    if(i == 0)
	      {
		// start line
		if (StartLine==2) {
		  SectorAngle = 45+90;
		} else {
		  error_factor=Task[i+1].Leg/60000.0;
		  SectorAngle = 90;
		}
		SectorSize = StartRadius - (error_factor*(StartRadius/100));
		SectorBearing = Task[i].OutBound;

		#if 0
		if (error_factor>0) {
			StartupStore(_T("..... START Leg=%f ef=%f  Radius=%d correction=%f\n"),Task[i+1].Leg,error_factor,(int)StartRadius, error_factor*(StartRadius/100));
		}
		#endif

	      }
	    else
	      {
		// normal turnpoint sector
		SectorAngle = 45;
		if (SectorType == 2) {
		  SectorSize = 10000; // German DAe 0.5/10
		} else {
		  SectorSize = SectorRadius;  // FAI sector
		}
		SectorBearing = Task[i].Bisector;
	      }
	  } else {
	    // finish line
	    if (FinishLine==2) {
	      SectorAngle = 45;
	    } else {
	      error_factor=Task[i].Leg/60000.0;
	      SectorAngle = 90;
	    }
	    SectorSize = FinishRadius - (error_factor*(FinishRadius/100));
	    SectorBearing = Task[i].InBound;

		#if 0
		if (error_factor>0) {
			StartupStore(_T("..... FINISH Leg=%f ef=%f  Radius=%d correction=%f\n"),Task[i].Leg,error_factor,(int)FinishRadius, error_factor*(FinishRadius/100));
		}
		#endif
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

          if (!AATEnabled) {
            Task[i].AATStartRadial  = 
              AngleLimit360(SectorBearing - SectorAngle);
            Task[i].AATFinishRadial = 
              AngleLimit360(SectorBearing + SectorAngle);
          }

	}
    }
  UnlockTaskData();
}

