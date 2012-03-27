/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "CalcTask.h"
#include "AATDistance.h"
#include "utils/heapcheck.h"

extern AATDistance aatdistance;



void StartTask(NMEA_INFO *Basic, DERIVED_INFO *Calculated, 
	       const bool do_advance,
               const bool do_announce) {
  Calculated->ValidFinish = false;
  Calculated->TaskStartTime = Basic->Time ;
  Calculated->TaskStartSpeed = Basic->Speed;
  Calculated->TaskStartAltitude = Calculated->NavAltitude;
  Calculated->LegStartTime = Basic->Time;
  flightstats.LegStartTime[0] = Basic->Time;
  flightstats.LegStartTime[1] = Basic->Time;

  Calculated->CruiseStartLat = Basic->Latitude;
  Calculated->CruiseStartLong = Basic->Longitude;
  Calculated->CruiseStartAlt = Calculated->NavAltitude;
  Calculated->CruiseStartTime = Basic->Time;

  aatdistance.Reset();

  // JMW TODO accuracy: Get time from aatdistance module since this is
  // more accurate

  // JMW clear thermal climb average on task start
  flightstats.ThermalAverage.Reset();
  flightstats.Task_Speed.Reset();
  Calculated->AverageThermal = 0;
  Calculated->WaypointBearing=0;

  // JMW reset time cruising/time circling stats on task start
  Calculated->timeCircling = 0;
  Calculated->timeCruising = 0;
  Calculated->TotalHeightClimb = 0;

//  // reset max height gain stuff on task start REMOVE REMOVE we dont reset anymore on task start or restart
//  Calculated->MaxHeightGain = 0; // REMOVE
//  Calculated->MinAltitude = 0;   // REMOVE
//  Calculated->MaxAltitude = 0;   // REMOVE

  if (do_announce) {
    AnnounceWayPointSwitch(Calculated, do_advance);
  } else {
    if (do_advance) {
      ActiveWayPoint=1;
      SelectedWaypoint = TASKINDEX; 
    }
  }
}

