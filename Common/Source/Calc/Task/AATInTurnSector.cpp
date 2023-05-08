/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "PGTask/PGConeTaskPt.h"
#include "NavFunctions.h"



bool InAATTurnSector(const double longitude, const double latitude,
                    const int the_turnpoint, const double Altitude)
{
  double AircraftBearing;
  bool retval = false;

  if (!ValidTaskPoint(the_turnpoint)) {
    return false;
  }

  double distance;
  LockTaskData();
  DistanceBearing(WayPointList[Task[the_turnpoint].Index].Latitude,
                  WayPointList[Task[the_turnpoint].Index].Longitude,
                  latitude,
                  longitude,
                  &distance, &AircraftBearing);
  sector_type_t Type;
  double Radius;
  GetTaskSectorParameter(the_turnpoint, &Type, &Radius);
  switch(Type) {
      case sector_type_t::ESS_CIRCLE:
      case sector_type_t::CIRCLE:
          retval = (distance < Radius);
          break;
      case sector_type_t::SECTOR:
          retval = (AngleInRange(Task[the_turnpoint].AATStartRadial,
                                    Task[the_turnpoint].AATFinishRadial,
                                    AngleLimit360(AircraftBearing), true));
          break;
      case sector_type_t::CONE:
          retval = (distance < PGConeTaskPt::ConeRadius(Altitude, Task[the_turnpoint].PGConeBase, Task[the_turnpoint].PGConeSlope, Task[the_turnpoint].PGConeBaseRadius));
          break;
      default:
          assert(false);
          break;
  }


  UnlockTaskData();
  return retval;
}
