/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: ThermalHistory.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "LKProcess.h"
#include "McReady.h"
#include "Waypointparser.h"
#include "DoInits.h"
#include "NavFunctions.h"


// 
// Thermal History functions
//
// #define DEBUG_THISTORY 1

// This is holding the thermal selected for multitarget 
static int ThermalMultitarget=-1;



void InitThermalHistory(void) {
  int i;
  for (i=0; i<MAX_THERMAL_HISTORY; i++) {
	ThermalHistory[i].Valid=false;
  }
  ThermalMultitarget=-1;
  #if DEBUG_THISTORY
  StartupStore(_T("... Init Thermal History done\n"));
  #endif
}


// This function is called upon exiting a thermal, just before input event to cruise mode is issued
// Functions using the ThermalHistory array should always check for Valid flag at each step,
// and in any case never use the array while Circling. Because anytime we may change a value, in case
// (very rare) the 100 limit is passed and we are reusing a value.
//
// When this function is called, the L> thermal is automatically selected as multitarget L
//
void InsertThermalHistory(double ThTime,  double ThLat, double ThLon, double ThBase,double ThTop, double ThAvg) {

  int i;

  for (i=0; i<MAX_THERMAL_HISTORY; i++) {
	if (ThermalHistory[i].Valid == false) break;
  }

  if (i==MAX_THERMAL_HISTORY) {
	#if DEBUG_THISTORY
	StartupStore(_T("... no more space in thermal history\n"));
	#endif
	int oldest=0; // lets assume the 0 is the oldest, to begin
  	for (i=1; i<MAX_THERMAL_HISTORY; i++) {
		if ( ThermalHistory[i].Time < ThermalHistory[oldest].Time ) {
			oldest=i;
			continue;
		}
	}
	#if DEBUG_THISTORY
	StartupStore(_T(".... oldest thermal in history is n.%d\n"),oldest);
	#endif
	i=oldest;
  }

  if (i<0 || i>=MAX_THERMAL_HISTORY) {
	#if DEBUG_THISTORY
	StartupStore(_T("..... Insert thermal history out of range: <%d>!\n"),i);
	#endif
	return;
  }

  ThermalHistory[i].Valid=false;

  TCHAR tstring[10];
  Units::TimeToTextSimple(tstring,LocalTime(ThTime));
  
  _stprintf(ThermalHistory[i].Name,_T("th%s"),tstring);
  ThermalHistory[i].Time = ThTime;
  ThermalHistory[i].Latitude = ThLat;
  ThermalHistory[i].Longitude = ThLon;
  ThermalHistory[i].HBase = ThBase;
  ThermalHistory[i].HTop = ThTop;
  ThermalHistory[i].Lift = ThAvg;
  ThermalHistory[i].Valid=true;

  int j=FindNearestFarVisibleWayPoint(ThLon,ThLat,15000,WPT_UNKNOWN);
  if (j>0) {
	#if DEBUG_THISTORY
	StartupStore(_T("..... Thermal is near wp.%d <%s>\n"),j,WayPointList[j].Name);
	#endif
	TCHAR wpnear[NAME_SIZE+1];
	_tcscpy(wpnear,WayPointList[j].Name);
	wpnear[19]='\0'; // sized 20 chars
	_tcscpy(ThermalHistory[i].Near,wpnear);
  } else {
	_tcscpy(ThermalHistory[i].Near,_T(""));
  }

  LockTaskData();
  _tcscpy(WayPointList[RESWP_LASTTHERMAL].Name , ThermalHistory[i].Name);
  WayPointList[RESWP_LASTTHERMAL].Latitude  = CALCULATED_INFO.ClimbStartLat;
  WayPointList[RESWP_LASTTHERMAL].Longitude = CALCULATED_INFO.ClimbStartLong;
  WayPointList[RESWP_LASTTHERMAL].Altitude  = CALCULATED_INFO.ClimbStartAlt;
  if (j>0)
    SetWaypointComment(WayPointList[RESWP_LASTTHERMAL],ThermalHistory[i].Name);
  else
    SetWaypointComment(WayPointList[RESWP_LASTTHERMAL], MsgToken(1320)); // last good thermal

  UnlockTaskData();

  // Update holder selected L> 
  SetThermalMultitarget(i);

  #if DEBUG_THISTORY
  StartupStore(_T("... Insert new thermal in history[%d]: <%s> Base=%.0f Top=%.0f Lift=%.1f\n"),
	i, ThermalHistory[i].Name, ThBase, ThTop, ThAvg);
  #endif

}


//
// Running every n seconds ONLY when the thermal history page is active and we are not drawing map.
// Returns true if did calculations, false if ok to use old values.
// Warning, this function is run by Draw thread.
bool DoThermalHistory(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i,k,l;
   double bearing, distance, sortvalue;
   double sortedValue[MAX_THERMAL_HISTORY+1];

   if (DoInit[MDI_DOTHERMALHISTORY]) {
	#ifdef DEBUG_THISTORY
	StartupStore(_T("... DoTHistory Init memset CopyTHistory\n"));
	#endif
	memset(CopyThermalHistory, 0, sizeof(CopyThermalHistory));
	LKNumThermals=0;
	#ifdef DEBUG_THISTORY
	StartupStore(_T("... DoTHistory Init memset LKSortedThermals\n"));
	#endif
	memset(LKSortedThermals, -1, sizeof(LKSortedThermals));
	DoInit[MDI_DOTHERMALHISTORY]=false;
	return true;
   }

   // Wait for n seconds before updating again, to avoid data change too often
   // distracting the pilot.
   static double lastRunTime=0;
   if (  lastRunTime > Basic->Time ) lastRunTime=Basic->Time;
   if (  (Basic->Time < (lastRunTime+NEARESTUPDATETIME)) && (LastDoThermalH>0)) {
        return false;
   }

   // Consider replay mode...
   if (  LastDoThermalH > Basic->Time ) LastDoThermalH=Basic->Time;
   // Dont recalculate while user is using the virtual keys
   if ( Basic->Time < (LastDoThermalH+PAGINGTIMEOUT) ) { 
	return false;
   }

   LastDoThermalH=Basic->Time;
   lastRunTime=Basic->Time;

   // Do not copy the struct while circling, we are awaiting for new thermal being inserted.
   // We should copy only once every new thermal has been updated.
   // However, since we should also consider igc replay, flight resets, etc.etc.
   // lets copy and make it short.
   if (!Calculated->Circling) {
     #ifdef DEBUG_THISTORY
     StartupStore(_T("... DoTHistory Copy CopyThermalHistory and reset LKSortedThermals\n"));
     #endif
     LockFlightData(); // No need to lock really
     memcpy(CopyThermalHistory, ThermalHistory, sizeof(ThermalHistory));
     UnlockFlightData();
   }

   memset(LKSortedThermals, -1, sizeof(LKSortedThermals));
   memset(sortedValue, -1, sizeof(sortedValue));
   LKNumThermals=0;

   for (i=0; i<MAX_THERMAL_HISTORY; i++) {
	if ( CopyThermalHistory[i].Valid != true ) continue;
	LKNumThermals++;
	DistanceBearing(Basic->Latitude, Basic->Longitude, 
		CopyThermalHistory[i].Latitude, CopyThermalHistory[i].Longitude,
		&distance, &bearing);

	double altReqd = GlidePolar::MacCreadyAltitude (MACCREADY,
                distance, bearing,
                Calculated->WindSpeed, Calculated->WindBearing,
                0, 0, true, NULL);

	// These values are available only in the mapwindow thread, i.e. in the Copy.
	CopyThermalHistory[i].Arrival= Calculated->NavAltitude + Calculated->EnergyHeight - altReqd - CopyThermalHistory[i].HBase;
	CopyThermalHistory[i].Distance=distance;
	CopyThermalHistory[i].Bearing=bearing;
   }
   if (LKNumThermals<1) return true;

   // We know there is at least one thermal
   for (i=0; i<MAX_THERMAL_HISTORY; i++) {

	if ( CopyThermalHistory[i].Valid != true ) continue;

	switch (SortedMode[MSM_THERMALS]) {
		case 0:	
			sortvalue=CopyThermalHistory[i].Time;
			// sort by highest: the highest the closer to now
			sortvalue*=-1;
			break;
		case 1:
			sortvalue=CopyThermalHistory[i].Distance;
			break;
		case 2:
			if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
				sortvalue=CopyThermalHistory[i].Bearing;
				break;
			}
			sortvalue=CopyThermalHistory[i].Bearing - Basic->TrackBearing;
			if (sortvalue < -180.0) sortvalue += 360.0;
			else
				if (sortvalue > 180.0) sortvalue -= 360.0;
			if (sortvalue<0) sortvalue*=-1;
			break;
		case 3:
			sortvalue=CopyThermalHistory[i].Lift;
			// sort by highest
			sortvalue*=-1;
			break;
		case 4:
			sortvalue=CopyThermalHistory[i].Arrival;
			// sort by highest
			sortvalue*=-1;
			break;
		default:
			sortvalue=CopyThermalHistory[i].Distance;
			break;
	}

	for (k=0; k< MAXTHISTORY; k++)  {

		// if new value is lower or index position is empty,  AND index position is not itself
		if ( ((sortvalue < sortedValue[k]) || (LKSortedThermals[k]== -1))
		&& (LKSortedThermals[k] != i) )
		{
			// ok, got new lower value, put it into slot
			for (l=MAXTHISTORY-1; l>k; l--) {
				if (l>0) {
					sortedValue[l] = sortedValue[l-1];
					LKSortedThermals[l] = LKSortedThermals[l-1];
				}
			}
			sortedValue[k] = sortvalue;
			LKSortedThermals[k] = i;
			//inserted++;
			break;
		}
	} // for k
	continue;

   } // for i
   #ifdef DEBUG_THISTORY
   StartupStore(_T("... DoTHistory Sorted, LKNumThermals=%d :\n"),LKNumThermals);
   for (i=0; i<MAXTHISTORY; i++) {
	if (LKSortedThermals[i]>=0)
		StartupStore(_T("... DoTHistory LKSortedThermals[%d]=CopyThermalHistory[%d] Valid=%d Name=<%s> Lift=%.1f\n"), i, 
			LKSortedThermals[i],
			CopyThermalHistory[LKSortedThermals[i]].Valid,
			CopyThermalHistory[LKSortedThermals[i]].Name,
			CopyThermalHistory[LKSortedThermals[i]].Lift);
   }
   #endif

   return true;
}

bool IsThermalMultitarget(int idx) {

  if (idx<0||idx>MAX_THERMAL_HISTORY)
	return false;

  return (ThermalMultitarget==idx);
}

void SetThermalMultitarget(int idx) {

  if (idx<0||idx>=MAX_THERMAL_HISTORY)
	return;

  ThermalMultitarget=idx;
  return;
}

int GetThermalMultitarget(void) {
  if (ThermalMultitarget<0||ThermalMultitarget>=MAX_THERMAL_HISTORY)
	return -1;

  return ThermalMultitarget;
}


