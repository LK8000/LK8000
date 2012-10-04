/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "CalcTask.h"

extern AATDistance aatdistance;


void CheckInSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  if (ActiveWayPoint>0) {
    AddAATPoint(Basic, Calculated, ActiveWayPoint-1);
  }
  AddAATPoint(Basic, Calculated, ActiveWayPoint);

  if(DoOptimizeRoute()) {
	  if(ValidTaskPoint(ActiveWayPoint+1) && Task[ActiveWayPoint].OutCircle) {
		  if (aatdistance.HasEntered(ActiveWayPoint)) {

			  double CenterDist = 0.0;
			  DistanceBearing( WayPointList[TASKINDEX].Latitude, 
								WayPointList[TASKINDEX].Longitude,
								Basic->Latitude,
								Basic->Longitude,
								&CenterDist, NULL );

			  if(CenterDist > Task[ActiveWayPoint].AATCircleRadius) {
				  if (ReadyToAdvance(Calculated, true, false)) {
					  AnnounceWayPointSwitch(Calculated, true);
					  Calculated->LegStartTime = Basic->Time;
					  flightstats.LegStartTime[ActiveWayPoint] = Basic->Time;
				  }
			  }
		  }
		  return;
	  }
  }
  
  // JMW Start bug XXX

  if (aatdistance.HasEntered(ActiveWayPoint)) {
    if (ReadyToAdvance(Calculated, true, false)) {
      AnnounceWayPointSwitch(Calculated, true);
      Calculated->LegStartTime = Basic->Time;
      flightstats.LegStartTime[ActiveWayPoint] = Basic->Time;
    }
    if (Calculated->Flying) {
      Calculated->ValidFinish = false;
    }
  }
}

