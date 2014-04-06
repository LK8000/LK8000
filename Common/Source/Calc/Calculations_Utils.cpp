/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Calculations_Utils.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"


double CRUISE_EFFICIENCY = 1.0;

// AverageLD return 0 if invalid, or 999 if too high...
// This function will return bestld if too high, and last valid average if invalid because on ground or circling
double GetCurrentEfficiency(DERIVED_INFO *Calculated, short effmode) {

   static double lastValidValue=GlidePolar::bestld;
   double cruise=Calculated->AverageLD;

   if ( cruise == INVALID_GR )		return GlidePolar::bestld;
   if ( cruise == 0 )			return lastValidValue;
   if ( cruise > 0 && cruise <1)	return 1.0;
   if ( cruise < 0 || cruise >GlidePolar::bestld ) cruise = GlidePolar::bestld; 

   lastValidValue=cruise;
   return cruise;
}



void ResetTask(bool showConfirmMsg) {

  CALCULATED_INFO.ValidFinish = false;
  CALCULATED_INFO.ValidStart = false;
  CALCULATED_INFO.TaskStartTime = 0;
  CALCULATED_INFO.TaskStartSpeed = 0;
  CALCULATED_INFO.TaskStartAltitude = 0;
  CALCULATED_INFO.LegStartTime = 0;


  ActiveWayPoint=0;
  if (UseGates()) {
	int i;
	i=NextGate();
	if (i<0 || i>PGNumberOfGates) { // just for safety we check also numbgates
		i=RunningGate();
		if (i<0) i=PGNumberOfGates-1;
	}
	ActiveGate=i;
  }

	// LKTOKEN  _@M676_ = "TASK RESTART RESET" 
  if(showConfirmMsg) DoStatusMessage(gettext(TEXT("_@M676_")));

}




unsigned int GetWpChecksum(unsigned int index) { //@ 101018

  int clon, clat, csum;

  if (index<NUMRESWP || index > NumberOfWayPoints) {
        // it is ok to insert a reserved wp in the history, but not to save it. 
        // So we get this error, which is not an error for reswp..
        if (index>=NUMRESWP)
	    StartupStore(_T("...... Impossible waypoint number=%d for Checksum%s"),index,NEWLINE);

	return 0;
  }

  clon=(int)WayPointList[index].Longitude;
  clat=(int)WayPointList[index].Latitude;
  if (clon<0) clon*=-1;
  if (clat<0) clat*=-1;

  csum= WayPointList[index].Name[0] + ((int)WayPointList[index].Altitude*100) + (clat*1000*clon);

  return csum;
}



//
// Notice: GR should not consider total energy. This is a mere geometric value.
// 
double CalculateGlideRatio(const double grdistance, const double havailable) {
  double ratio=0;
  if (havailable <=0) {
	ratio=INVALID_GR;
  } else {
	ratio= grdistance / havailable;

	if ( ratio >ALTERNATE_MAXVALIDGR || ratio <0 )
		ratio=INVALID_GR;
	else
		if ( ratio <1 )
			ratio=1;
  }
  return ratio;
}


//
// Assumes that wpindex IS already checked for existance!
// SafetyAltitudeMode:  0=landables only,  1=landables and turnpoints
//
bool CheckSafetyAltitudeApplies(const int wpindex) {

  LKASSERT(wpindex>=0);
  if (!ValidWayPoint(wpindex)) return false;
  //
  if (SafetyAltitudeMode==0 && !WayPointCalc[wpindex].IsLandable)
	return false;
  else
	return true;
}

double GetSafetyAltitude(const int wpindex) {

  if (CheckSafetyAltitudeApplies(wpindex))
	return SAFETYALTITUDEARRIVAL/10;
  else
	return 0;

}

// Returns a Green, Yellow or Red condition for the glide
short GetVisualGlideRatio(const double arrival, const double gr) {
  // Greeen requires 100m more height
  if ( (arrival - ALTERNATE_OVERSAFETY) >0 ) {
  	if ( gr <= (GlidePolar::bestld *SAFELD_FACTOR) )
		return 1; // full green vgr
  	else 
  		if ( gr <= GlidePolar::bestld )
			return 2; // yellow vgr
  } 
  return 3; // full red
}

//
// Like CheckSafetyAltitudeApplies, but we check for valid wpindex AND we also
// check that there is a valid Safetyaltitude!
// This is used by overlay in LKDrawLook
bool IsSafetyAltitudeInUse(const int wpindex) {

  // Virtual wps are not landables, correct? Hope so!
  if (!ValidWayPoint(wpindex))
	return false;
  if (!CheckSafetyAltitudeApplies(wpindex))
	return false;
  if (SAFETYALTITUDEARRIVAL<500) // SAFETY is *10, so we are checking <50
	return false;

  return true;
}



bool IsSafetyMacCreadyInUse(const int wpindex) {

  // Virtual wps are not landables, correct? Hope so!
  if (!ValidWayPoint(wpindex))
	return false;

  if (!WayPointCalc[wpindex].IsLandable)
	return false;

  if (MACCREADY>GlidePolar::SafetyMacCready)
	return false;

  return true;
}


// If no task, it is returning -1. Always to be checked!
int getFinalWaypoint() {
  int i;
  i=max(-1,min(MAXTASKPOINTS,ActiveWayPoint));

  i++;
  LockTaskData();
  while((i<MAXTASKPOINTS) && (Task[i].Index != -1))
    {
      i++;
    }
  UnlockTaskData();
  return i-1;
}

// Attention: if no task, this is true!
bool ActiveIsFinalWaypoint() {
  return (ActiveWayPoint == getFinalWaypoint());
}


bool IsFinalWaypoint(void) {
  bool retval;
  LockTaskData();
  if (ValidTaskPoint(ActiveWayPoint) && (Task[ActiveWayPoint+1].Index >= 0)) {
    retval = false;
  } else {
    retval = true;
  }
  UnlockTaskData();
  return retval;
}

