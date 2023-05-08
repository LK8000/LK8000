/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "NavFunctions.h"

static
bool InDefaultTurnSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int the_turnpoint) {
	if(!ValidTaskPoint(the_turnpoint)) return false;
	switch(SectorType) {
		case sector_type_t::CIRCLE:
			if(Calculated->WaypointDistance < SectorRadius) return true;
			break;
		case sector_type_t::DAe:
			// JMW added german rules
			if (Calculated->WaypointDistance < 500) {
				return true;
			}
		case sector_type_t::SECTOR: {
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
				if(SectorType == sector_type_t::SECTOR) {
					if(Calculated->WaypointDistance < SectorRadius) return true;
				} else { //It's a DAe
					if(Calculated->WaypointDistance < 10000) return true; // JMW added german rules
				}
			}
		}   break;
		case sector_type_t::LINE: {
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
		default:
			assert(false);
			break;
	}
	return false;
}


bool InTurnSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int tp) {
	if (UseAATTarget()) {
		return InAATTurnSector(Basic->Longitude, Basic->Latitude, tp, Basic->Altitude);
	} else {
		return InDefaultTurnSector(Basic, Calculated, tp);
	}
}
