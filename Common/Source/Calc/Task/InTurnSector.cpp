/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"


bool InTurnSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const int the_turnpoint) {
	if(!ValidTaskPoint(the_turnpoint)) return false;
	switch(SectorType) {
		case CIRCLE:
			if(Calculated->WaypointDistance < SectorRadius) return true;
			break;
		case SECTOR:
		case DAe: {
			if(SectorType==DAe) { // JMW added german rules
				if (Calculated->WaypointDistance<500) return true;
			}
			double AircraftBearing;
			LockTaskData();
			DistanceBearing(WayPointList[Task[the_turnpoint].Index].Latitude,
					WayPointList[Task[the_turnpoint].Index].Longitude,
					Basic->Latitude ,
					Basic->Longitude,
					NULL, &AircraftBearing);
			AircraftBearing = AircraftBearing - Task[the_turnpoint].Bisector;
			UnlockTaskData();
			while(AircraftBearing<-180) AircraftBearing+= 360;
			while(AircraftBearing>180) AircraftBearing-= 360;
			if(AircraftBearing>=-45 && AircraftBearing<=45) {
				if(SectorType==SECTOR) {
					if(Calculated->WaypointDistance < SectorRadius) return true;
				} else { //It's a DAe
					if(Calculated->WaypointDistance < 10000) return true; // JMW added german rules
				}
			}
		}   break;
		case LINE: {
			//First check if we simply passed the WayPoint
			LockTaskData();
			if(Calculated->LegDistanceToGo<Task[the_turnpoint].Leg && Calculated->LegDistanceCovered>=Task[the_turnpoint].Leg) {
				UnlockTaskData();
				return true;
			}
			//Then check if we passed the bisector
			bool bisectorOverpassed;
			if(AngleLimit360(Task[the_turnpoint].InBound-Task[the_turnpoint].Bisector) < 180)
				bisectorOverpassed = AngleInRange(Reciprocal(Task[the_turnpoint].Bisector),Task[the_turnpoint].Bisector,Calculated->WaypointBearing,true);
			else
				bisectorOverpassed = AngleInRange(Task[the_turnpoint].Bisector,Reciprocal(Task[the_turnpoint].Bisector),Calculated->WaypointBearing,true);
			UnlockTaskData();
			if(bisectorOverpassed) return true;
		}   break;
		default: //Unknown sector type
			break;
	}
	return false;
}
