/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

bool ValidWayPoint(int i) {
  bool retval = true;
  LockTaskData();
  retval = ValidWayPointFast(i);
  UnlockTaskData();
  return retval;
}

// A waypoint is valid here only if a list exists, it is within range and it is not reserved
bool ValidNotResWayPoint(int i) { // 091213
  bool retval = true;
  LockTaskData();
  if ((i<=RESWP_END)||(i>=(int)WayPointList.size())) {
    retval = false;
  }
  UnlockTaskData();
  return retval;
}

// 100929 A waypoint is valid here only if it is virtual, and with a valid content
bool ValidResWayPointFast(int i) { // 091213
  if (i < 0) {
    return false;
  }
  if (i>RESWP_END) {
	  return false;
  }
	if (WayPointList[i].Latitude == RESWP_INVALIDNUMBER) {
    return false;
  }
  return true;
}

bool ValidResWayPoint(int i) { // 091213
  ScopeLock lock(CritSec_TaskData);
  return ValidResWayPointFast(i);
}

bool ValidTaskPoint(int i) {
  bool retval = false;
  LockTaskData();
  retval = ValidTaskPointFast(i);
  UnlockTaskData();
  return retval;
}

bool ValidStartPoint(size_t i) {
    bool retVal=false;
    LockTaskData();
    if(i<MAXSTARTPOINTS) {
        retVal = ValidWayPointFast(StartPoints[i].Index);
    } 
    UnlockTaskData();
    return retVal;
}



bool ValidStartSpeed(NMEA_INFO *Basic, DERIVED_INFO *Calculated, unsigned Margin) {
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

