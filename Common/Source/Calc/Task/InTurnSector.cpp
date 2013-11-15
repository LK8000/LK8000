/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"



bool InTurnSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const int the_turnpoint)
{
  double AircraftBearing;
  bool bisectorOverpassed;


  if (!ValidTaskPoint(the_turnpoint)) return false;

  if(SectorType==CIRCLE)
    {
      if(Calculated->WaypointDistance < SectorRadius)
        {
          return true;
        }
    }
  else
    {
	  if(SectorType==LINE) {
		//First check if we simply passed the WayPoint
		LockTaskData();
		if(Calculated->LegDistanceToGo<Task[the_turnpoint].Leg && Calculated->LegDistanceCovered>=Task[the_turnpoint].Leg) {
			UnlockTaskData();
			return true;
		}
		//Then check if we passed the bisector
		if(AngleLimit360(Task[the_turnpoint].InBound-Task[the_turnpoint].Bisector) < 180)
			bisectorOverpassed = AngleInRange(Reciprocal(Task[the_turnpoint].Bisector),Task[the_turnpoint].Bisector,Calculated->WaypointBearing,true);
		else
			bisectorOverpassed = AngleInRange(Task[the_turnpoint].Bisector,Reciprocal(Task[the_turnpoint].Bisector),Calculated->WaypointBearing,true);
		UnlockTaskData();
		if(bisectorOverpassed) return true;
	  }

      if (SectorType==DAe) {
        // JMW added german rules
        if (Calculated->WaypointDistance<500) {
          return true;
        }
      }

      LockTaskData();
      DistanceBearing(WayPointList[Task[the_turnpoint].Index].Latitude,
                      WayPointList[Task[the_turnpoint].Index].Longitude,
                      Basic->Latitude , 
                      Basic->Longitude,
                      NULL, &AircraftBearing);
      AircraftBearing = AircraftBearing - Task[the_turnpoint].Bisector;
      UnlockTaskData();
      
      while (AircraftBearing<-180) {
        AircraftBearing+= 360;
      }
      while (AircraftBearing>180) {
        AircraftBearing-= 360;
      }
      
      if( (AircraftBearing >= -45) && (AircraftBearing <= 45))
        {
          if (SectorType==SECTOR) {
            if(Calculated->WaypointDistance < SectorRadius)
              {
                return true;
              }
          } else {
            // JMW added german rules
            if(Calculated->WaypointDistance < 10000)
              {
                return true;
              }
          }
        }
    }       
  return false;
}
