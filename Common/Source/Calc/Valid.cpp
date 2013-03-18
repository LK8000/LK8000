/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"



bool ValidWayPoint(int i) {
  bool retval = true;
  LockTaskData();
  if ((!WayPointList)||(i<0)||(i>=(int)NumberOfWayPoints)) {
    retval = false;
  }
  UnlockTaskData();
  return retval;
}

// A waypoint is valid here only if a list exists, it is within range and it is not reserved
bool ValidNotResWayPoint(int i) { // 091213
  bool retval = true;
  LockTaskData();
  if ((!WayPointList)||(i<=RESWP_END)||(i>=(int)NumberOfWayPoints)) {
    retval = false;
  }
  UnlockTaskData();
  return retval;
}

// 100929 A waypoint is valid here only if it is virtual, and with a valid content
bool ValidResWayPoint(int i) { // 091213
  bool retval = true;
  LockTaskData();
  if ( (i<0) || (i>RESWP_END) )
	retval = false;
  else {
	if (WayPointList[i].Latitude == RESWP_INVALIDNUMBER) retval=false;
  }
  UnlockTaskData();
  return retval;
}

bool ValidTaskPoint(int i) {
  bool retval = true;
  LockTaskData();
  if ((i<0) || (i>= MAXTASKPOINTS)) 
    retval = false;
  else if (!ValidWayPoint(Task[i].Index)) 
    retval = false;
  UnlockTaskData();
  return retval;
}

bool ValidStartPoint(size_t i) {
    bool retVal=false;
    LockTaskData();
    if(i<MAXSTARTPOINTS) {
        retVal = ValidWayPoint(StartPoints[i].Index);
    } 
    UnlockTaskData();
    return retVal;
}



bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, DWORD Margin) {
  bool valid = true;
  if (StartMaxSpeed!=0) {
    if (Basic->AirspeedAvailable) {
      if ((Basic->IndicatedAirspeed*1000)>(StartMaxSpeed+Margin))
        valid = false;
    } else {
	// StartMaxSpeed is in millimeters per second, and so is Margin
	if ((Basic->Speed*1000)>(StartMaxSpeed+Margin))  { //@ 101014 FIX
		valid = false;
	}
    }
  }
  return valid;
}


bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
  return ValidStartSpeed(Basic, Calculated, 0);
}


bool ValidFinish(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
 (void)Basic;
  if ( ((FinishMinHeight/1000)>0) && (Calculated->TerrainValid) && (Calculated->AltitudeAGL < (FinishMinHeight/1000))) {
	return false;
  } else {
	return true;
  }
  
}

