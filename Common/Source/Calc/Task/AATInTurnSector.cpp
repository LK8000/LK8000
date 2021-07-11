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
  int Type; double Radius;
  GetTaskSectorParameter(the_turnpoint, &Type, &Radius);
  switch(Type) {
      case ESS_CIRCLE:
      case CIRCLE:
          retval = (distance < Radius);
          break;
      case SECTOR:
          retval = (AngleInRange(Task[the_turnpoint].AATStartRadial,
                                    Task[the_turnpoint].AATFinishRadial,
                                    AngleLimit360(AircraftBearing), true));
          break;
      case CONE:
          retval = (distance < PGConeTaskPt::ConeRadius(Altitude, Task[the_turnpoint].PGConeBase, Task[the_turnpoint].PGConeSlope, Task[the_turnpoint].PGConeBaseRadius));
          break;
  }


  UnlockTaskData();
  return retval;
}
