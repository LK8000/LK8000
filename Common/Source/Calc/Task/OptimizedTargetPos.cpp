/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "AATDistance.h"
#include "Waypointparser.h"
#include "NavFunctions.h"
#include "PGTask/PGTaskMgr.h"

PGTaskMgr gPGTask;

void CalculateOptimizedTargetPos(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

	if (!DoOptimizeRoute()) 
		return;

	gPGTask.Optimize(Basic);

	LockTaskData();

	for(size_t i=0; i<gPGTask.Count(); ++i) {
		gPGTask.getOptimized(i, Task[i].AATTargetLat, Task[i].AATTargetLon);
	}
		
	int stdwp=Task[ActiveWayPoint].Index;

	WayPointList[RESWP_OPTIMIZED].Latitude = Task[ActiveWayPoint].AATTargetLat;
	WayPointList[RESWP_OPTIMIZED].Longitude = Task[ActiveWayPoint].AATTargetLon;
	WayPointList[RESWP_OPTIMIZED].Altitude = WayPointList[stdwp].Altitude;
	WaypointAltitudeFromTerrain(&WayPointList[RESWP_OPTIMIZED]);

	wsprintf(WayPointList[RESWP_OPTIMIZED].Name, _T("!%s"),WayPointList[stdwp].Name);

	UnlockTaskData();
}

// Clear PG 
void ClearOptimizedTargetPos() {

	if (!DoOptimizeRoute())
		return;

	LockTaskData();

	WayPointList[RESWP_OPTIMIZED].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
	// name will be assigned by function dynamically
	_tcscpy(WayPointList[RESWP_OPTIMIZED].Name, _T("OPTIMIZED") );

	for(int i = 0; ValidWayPoint(Task[i].Index); ++i) {
		Task[i].AATTargetLat = WayPointList[Task[i].Index].Latitude;
		Task[i].AATTargetLon = WayPointList[Task[i].Index].Longitude;
		Task[i].AATTargetLocked = false;
	}

	UnlockTaskData();

	gPGTask.Initialize();
}

