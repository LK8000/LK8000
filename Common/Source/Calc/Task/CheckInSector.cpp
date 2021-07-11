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


void CheckInSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  if (ActiveTaskPoint>0) {
    AddAATPoint(Basic, Calculated, ActiveTaskPoint-1);
  }
  AddAATPoint(Basic, Calculated, ActiveTaskPoint);

  if(DoOptimizeRoute()) {
	  if(ValidTaskPoint(ActiveTaskPoint+1) && Task[ActiveTaskPoint].OutCircle) {
		  if (aatdistance.HasEntered(ActiveTaskPoint)) {

			  double CenterDist = 0.0;
			  DistanceBearing( WayPointList[TASKINDEX].Latitude, 
								WayPointList[TASKINDEX].Longitude,
								Basic->Latitude,
								Basic->Longitude,
								&CenterDist, NULL );

			  if(CenterDist > Task[ActiveTaskPoint].AATCircleRadius) {
				  if (ReadyToAdvance(Calculated, true, false)) {
					  AnnounceWayPointSwitch(Calculated, true);
					  Calculated->LegStartTime = Basic->Time;
					  flightstats.LegStartTime[ActiveTaskPoint] = Basic->Time;
				  }
			  }
		  }
		  return;
	  }
  }
  
  // JMW Start bug XXX

  if (aatdistance.HasEntered(ActiveTaskPoint)) {
    if (ReadyToAdvance(Calculated, true, false)) {
      AnnounceWayPointSwitch(Calculated, true);
      Calculated->LegStartTime = Basic->Time;
      flightstats.LegStartTime[ActiveTaskPoint] = Basic->Time;
    }
    if (Calculated->Flying) {
      Calculated->ValidFinish = false;
    }
  }
}

