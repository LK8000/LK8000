/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: DoRangeWaypointList.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "DoInits.h"
#include "NavFunctions.h"

extern int CalculateWaypointApproxDistance(int scx_aircraft, int scy_aircraft, int i);


// This was introduced in december 2010, updated in october 2011
// REDUCE WAYPOINTLIST TO THOSE IN RANGE, UNSORTED
// Keep an updated list of in-range waypoints
// The nearest pages are using this list. 
// Even if we have thousands of waypoints, nearest pages are using these.
// So we keep a linear response time, practically not depending on the
// number of waypoints. Creating the nearest list is fast because unsorted.

// #define DEBUG_DORANGE	1

bool DoRangeWaypointList(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

   size_t i;
   int kl, kt, ka;
   //double arrival_altitude;

   #if DEBUG_DORANGE
   StartupStore(_T(".... >> DoRangeWaypointList is running! <<\n"));
   #endif

   ScopeLock lock(CritSec_TaskData);
   
   if (WayPointList.empty()) {
	return false;
   }

  // Initialise turnpoint and landable range distance to default 
  // These are reset to defaults at DoInit time, also.
  static int dstrangeturnpoint=DSTRANGETURNPOINT;
  static int dstrangelandable=DSTRANGELANDABLE;
  // Number of attempts to recalculate distances to reduce range, for huge cup files
  // This will reduce number of waypoints in range, and fit them.
  #define MAXRETUNEDST	6
  // Number of attempts to retry the list to grow up again.
  // This will only happen after a minimum granted interval, so this is really only
  // safety limit to avoid unusual overloads (100x10m = 1000m = several hours)
  #define MAXRETRYRETUNEDST 100
  // The minimum interval after which is worth considering retuning distances.
  // At 120kmh in 10m we do 20km. Taken.
  #define MAXRETRYRETUNEINT 600	// 10m, minimum interval in seconds
  static short retunecount=0;
  static short retryretunecount=0;
  static double lastRetryRetuneTime=0;

   // Do init and RETURN, caution!
   // We need a locked GPS position to proceed!

   // TODO FIX LOCK DATA IN DRAWNEAREST when updating this list!
   if (DoInit[MDI_DORANGEWAYPOINTLIST]) {
	for (i=0; i<MAXRANGELANDABLE; i++) {
		RangeLandableIndex[i]= -1;
		RangeAirportIndex[i]= -1;
	}
	for (i=0; i<MAXRANGETURNPOINT; i++) {
		RangeTurnpointIndex[i]= -1;
	}
	RangeLandableNumber=0;
	RangeAirportNumber=0;
	RangeTurnpointNumber=0;

	// Reset all statics for Master DoInits actions
	dstrangeturnpoint=DSTRANGETURNPOINT;
	dstrangelandable=DSTRANGELANDABLE;
	retunecount=0;
	retryretunecount=0;
	lastRetryRetuneTime=0;

	DoInit[MDI_DORANGEWAYPOINTLIST]=false;
	#if DEBUG_DORANGE
	StartupStore(_T(".... >> DoRangeWaypointList INIT done, return <<\n"));
	#endif
	return false;
   }

   #if 0
   DoStatusMessage(_T("WAIT RECALCULATING WAYPOINTS"));
   #endif

   int scx_aircraft, scy_aircraft;
   LatLon2Flat(Basic->Longitude, Basic->Latitude, &scx_aircraft, &scy_aircraft);

  bool retunedst_tps;
  bool retunedst_lnd;

_retunedst:

  retunedst_tps=false;
  retunedst_lnd=false;

  for (i=0; i<MAXRANGELANDABLE; i++) {
	RangeLandableNumber=0;
	RangeLandableIndex[i]= -1;
	RangeAirportNumber=0;
	RangeAirportIndex[i]= -1;
  }

  for (i=0; i<MAXRANGETURNPOINT; i++) {
	RangeTurnpointNumber=0;
	RangeTurnpointIndex[i]= -1;
  }

  #if DEBUG_DORANGE
  StartupStore(_T(".... dstrangeturnpoint=%d  dstrangelandable=%d\n"),dstrangeturnpoint,dstrangelandable);
  #endif

  for (i=0, kt=0, kl=0, ka=0; i<WayPointList.size(); i++) {

	int approx_distance = CalculateWaypointApproxDistance(scx_aircraft, scy_aircraft, i);

	// Size a reasonable distance, wide enough 
	if ( approx_distance > dstrangeturnpoint ) goto LabelLandables;

	// Get only non landables
	if (
		( (TpFilter==(TpFilter_t)TfNoLandables) && (!WayPointCalc[i].IsLandable ) ) ||
		( (TpFilter==(TpFilter_t)TfAll) ) ||
		( (TpFilter==(TpFilter_t)TfTps) && ((WayPointList[i].Flags & TURNPOINT) == TURNPOINT) ) 
	 ) {
		if (kt+1<MAXRANGETURNPOINT) { // never mind if we use maxrange-2
			RangeTurnpointIndex[kt++]=i;
			RangeTurnpointNumber++;
			#if DEBUG_DORANGE
			StartupStore(_T(".. insert turnpoint <%s>\n"),WayPointList[i].Name); 
			#endif
		} else {
			#if DEBUG_DORANGE
			StartupStore(_T("... OVERFLOW RangeTurnpoint cannot insert <%s> in list\n"),WayPointList[i].Name);
			#endif
			// Attempt to reduce range since we have a huge concentration of waypoints
			// This can only happen "count" times, and it is not reversible: it wont grow up anymore.
			if (retunecount <MAXRETUNEDST) {
				#if DEBUG_DORANGE
				StartupStore(_T("... Retunedst attempt %d of %d\n"),retunecount+1,MAXRETUNEDST);
				#endif
				retunedst_tps=true;
				retunecount++;
				break;
			}
		}
	}


LabelLandables:

	if ( approx_distance > dstrangelandable ) continue;

	// Skip non landable waypoints that are between DSTRANGETURNPOINT and DSTRANGELANDABLE
	if (!WayPointCalc[i].IsLandable )
		continue; 

	if (kl+1<MAXRANGELANDABLE) { // never mind if we use maxrange-2
		RangeLandableIndex[kl++]=i;
		RangeLandableNumber++;
		#if DEBUG_DORANGE
		StartupStore(_T(".. insert landable <%s>\n"),WayPointList[i].Name); 
		#endif
	}
	else {
		#if DEBUG_DORANGE
		StartupStore(_T("... OVERFLOW RangeLandable cannot insert <%s> in list\n"),WayPointList[i].Name);
		#endif
		if (retunecount <MAXRETUNEDST) {
			#if DEBUG_DORANGE
			StartupStore(_T("... Retunedst attempt %d of %d\n"),retunecount+1,MAXRETUNEDST);
			#endif
			retunedst_lnd=true;
			retunecount++;
			break;
		}
	}

	// If it's an Airport then we also take it into account separately
	if ( WayPointCalc[i].IsAirport )
	{
		if (ka+1<MAXRANGELANDABLE) { // never mind if we use maxrange-2
			RangeAirportIndex[ka++]=i;
			RangeAirportNumber++;
		}
		else {
			#if DEBUG_DORANGE
			StartupStore(_T("... OVERFLOW RangeAirport cannot insert <%s> in list\n"),WayPointList[i].Name);
			#endif
			if (retunecount <MAXRETUNEDST) {
				#if DEBUG_DORANGE
				StartupStore(_T("... Retunedst attempt %d of %d\n"),retunecount+1,MAXRETUNEDST);
				#endif
				retunedst_lnd=true;
				retunecount++;
				break;
			}
		}
	}


   } // for i

   // We lower full value, we higher by halved values
   #define TPTRANGEDELTA 12
   #define LNDRANGEDELTA 15

   if (retunedst_tps) {
	dstrangeturnpoint-=TPTRANGEDELTA; // 100km 90 80 70 60 50
	if (dstrangeturnpoint<30) dstrangeturnpoint=30; // min 30km radius
	#if (DEBUG_DORANGE || TESTBENCH)
	StartupStore(_T("... Retuning dstrangeturnpoint to %d km \n"),dstrangeturnpoint);
	#endif
	goto _retunedst;
  }
   if (retunedst_lnd) {
	dstrangelandable-=LNDRANGEDELTA; // 150km - 135 - 120 - 105 - 90 - 75
	if (dstrangelandable<50) dstrangelandable=50; // min 50km radius
	#if (DEBUG_DORANGE || TESTBENCH)
	StartupStore(_T("... Retuning dstrangelandable to %d km \n"),dstrangelandable);
	#endif
	goto _retunedst;
  }

  // This should only happen in simulation, logger replay mode
  // But no problems to do a spare more check during flight, caring about
  // faulty gps firmware.
  if (Basic->Time < (lastRetryRetuneTime-10) ) {
	#if (DEBUG_DORANGE || TESTBENCH)
	StartupStore(_T("... lastRetryRetuneTime back in time, reset.\n"));
	#endif
	lastRetryRetuneTime=Basic->Time; 
  }
  // Set it for the first time only
  if (lastRetryRetuneTime==0) lastRetryRetuneTime=Basic->Time; 

  #if TESTBENCH
  StartupStore(_T(".... RangeTurnepoint=%d/%d (TPrange=%d) RangeLandable=%d/%d (LNrange=%d) retunecount=%d retrycount=%d sinceretry=%0.0fs \n"),
	RangeTurnpointNumber, MAXRANGETURNPOINT, dstrangeturnpoint,
	RangeLandableNumber, MAXRANGELANDABLE, dstrangelandable,
	retunecount, retryretunecount,
	Basic->Time-lastRetryRetuneTime);
  #endif

  // If we are not over the counter limit, and enough time has passed, check for recalibration
  if (retryretunecount<MAXRETRYRETUNEDST && ((Basic->Time - lastRetryRetuneTime)>MAXRETRYRETUNEINT) ) {
	bool retryretune=false;
	// If we have space for 20% more points, lets retry to use them in the next run
	if ( (dstrangeturnpoint<DSTRANGETURNPOINT) && ((MAXRANGETURNPOINT - RangeTurnpointNumber) > (MAXRANGETURNPOINT/5)) ) {
		#if (DEBUG_DORANGE || TESTBENCH)
		StartupStore(_T(".... (TPT) Retryretune=%d, RangeTurnepoint=%d too low, range grow from %d to %d\n"),
			retryretunecount,RangeTurnpointNumber, dstrangeturnpoint, dstrangeturnpoint+(TPTRANGEDELTA/2));
		#endif
		dstrangeturnpoint+=(TPTRANGEDELTA/2);
		retryretune=true;
	}
	if ( (dstrangelandable<DSTRANGELANDABLE) && ((MAXRANGELANDABLE - RangeLandableNumber) > (MAXRANGELANDABLE/5)) ) {
		#if (DEBUG_DORANGE || TESTBENCH)
		StartupStore(_T(".... (LND) Retryretune=%d, RangeLandable=%d too low, range grow from %d to %d\n"),
			retryretunecount,RangeLandableNumber, dstrangelandable, dstrangelandable+(LNDRANGEDELTA/2));
		#endif
		dstrangelandable+=(LNDRANGEDELTA/2);
		retryretune=true;
	}

	if (retryretune) {
		if (dstrangeturnpoint>DSTRANGETURNPOINT) dstrangeturnpoint=DSTRANGETURNPOINT;
		if (dstrangelandable>DSTRANGELANDABLE) dstrangelandable=DSTRANGELANDABLE;

		// Notice that we are storing the time we flagged the reequest, which will happen later on!
		lastRetryRetuneTime=Basic->Time;
		retryretunecount++;
		retunecount=0;
		#if DEBUG_DORANGE
		StartupStore(_T(".... (Retry retune flagged for next time dorange)\n"));
		#endif
	}

  }

  // We have filled the list... which was too small 
  // this cannot happen with UNSORTED RANGE
  if (RangeTurnpointNumber>MAXRANGETURNPOINT) RangeTurnpointNumber=MAXRANGETURNPOINT;
  if (RangeAirportNumber>MAXRANGELANDABLE) RangeAirportNumber=MAXRANGELANDABLE;
  if (RangeLandableNumber>MAXRANGELANDABLE) RangeLandableNumber=MAXRANGELANDABLE;

  return true;
}


