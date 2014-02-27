/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"



// For PGs we are using cylinders, so: 
// If we are inside cylinder return true  else false
bool InStartSector_Internal(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                           int Index, 
                           double OutBound, 
                           bool &LastInSector)
{
  (void)Calculated;
  if (!ValidWayPoint(Index)) return false;

  // No Task Loaded

  double AircraftBearing;
  double FirstPointDistance;

  // distance from aircraft to start point
  DistanceBearing(Basic->Latitude,
                  Basic->Longitude,
                  WayPointList[Index].Latitude, 
                  WayPointList[Index].Longitude,
                  &FirstPointDistance,
                  &AircraftBearing);

  bool inrange = false;
  inrange = (FirstPointDistance<StartRadius);

  if(StartLine==0) { 
	// Start Circle 
	return inrange;
  }
        
  // Start Line
  AircraftBearing = AngleLimit180(AircraftBearing - OutBound);

  // JMW bugfix, was Bisector, which is invalid

  bool approaching;
  if(StartLine==1) { // Start line 
	approaching = ((AircraftBearing >= -90) && (AircraftBearing <= 90));
  } else {
	// FAI 90 degree
	approaching = ((AircraftBearing >= -45) && (AircraftBearing <= 45));
  }

  if (inrange) {
	return approaching;
  } else {
	// cheat fail of last because exited from side
	LastInSector = false;
  }

  return false;
}


bool InStartSector(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int &index, BOOL *CrossedStart)
{
  static bool LastInSector = false;
  static int EntryStartSector = index;

  if(ISGAAIRCRAFT) { //Detect start for GA aircraft
  	if(!ValidTaskPoint(ActiveWayPoint) || !ValidTaskPoint(0)) return false;
  	if(ActiveWayPoint==0) { //if the next WP is the departure
  		LockTaskData();
  		double departureRadius=1000; //1 Km default departure radius
  		if(WayPointList[Task[0].Index].RunwayLen>0) departureRadius=WayPointList[Task[0].Index].RunwayLen/2; // if we have runaway length available
  		if(Calculated->WaypointDistance<=departureRadius) *CrossedStart=true; //consider in departure as reached
  		else *CrossedStart=false;
  		UnlockTaskData();
  		if(*CrossedStart) return true;
  	} else return false;
  }

  bool isInSector= false;
  bool retval=false;

  if (!Calculated->Flying || !ValidTaskPoint(ActiveWayPoint) || !ValidTaskPoint(0))
	return false;

  LockTaskData();

  bool in_height = true;

  if ((ActiveWayPoint>0) && !ValidTaskPoint(ActiveWayPoint+1)) {
	// don't detect start if finish is selected
	retval = false;
	goto OnExit;
  }

  in_height = InsideStartHeight(Basic, Calculated, StartMaxHeightMargin);

  // if waypoint is not the task 0 start wp, and it is valid, then make it the entrystartsector. ?? why ??
  if ((Task[0].Index != EntryStartSector) && (EntryStartSector>=0)) {
	LastInSector = false;
	EntryStartSector = Task[0].Index;
  }

  // are we inside the start sector?
  isInSector = InStartSector_Internal(Basic, Calculated, 
                                      Task[0].Index, Task[0].OutBound,
                                      LastInSector);
  // and within height limits?
  isInSector &= in_height;

  // PGStartOut only Valid if Start is Cylindre.
  if (ISPARAGLIDER && StartLine==0 && PGStartOut) { // 100509
  	// we crossed the start if we were outside sector and now we are in.
	*CrossedStart = !LastInSector && isInSector;
  } else {
  	// we crossed the start if we were in sector and now we are not.
	*CrossedStart = LastInSector && !isInSector;
  }

  LastInSector = isInSector;
  if (*CrossedStart) {
	goto OnExit;
  }
  
  if (EnableMultipleStartPoints) {
    for (int i=0; i<MAXSTARTPOINTS; i++) {
      if (StartPoints[i].Active && (StartPoints[i].Index>=0)
          && (StartPoints[i].Index != Task[0].Index)) {
        
        retval = InStartSector_Internal(Basic, Calculated, 
                                        StartPoints[i].Index, 
                                        StartPoints[i].OutBound,
                                        StartPoints[i].InSector);
	retval &= in_height;
        isInSector |= retval;

        index = StartPoints[i].Index;
        *CrossedStart = StartPoints[i].InSector && !retval;
        StartPoints[i].InSector = retval;
        if (*CrossedStart) {
          if (Task[0].Index != index) {
            Task[0].Index = index;
            LastInSector = false;
            EntryStartSector = index;
            RefreshTask();
          }
          goto OnExit;
        }

      }
    }
  }

 OnExit:

  UnlockTaskData();
  return isInSector;
}
