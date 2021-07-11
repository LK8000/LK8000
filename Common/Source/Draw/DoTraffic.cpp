/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: DoTraffic.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "DoInits.h"
#include "NavFunctions.h"


//
// Running every n seconds ONLY when the nearest page is active and we are not drawing map.
// We DONT use FLARM_Traffic after we copy inside LKTraffic!!
// Returns true if did calculations, false if ok to use old values
//
// 121213 I realised that all calculations made in local copy, so we dont need to use LKTraffic at all.
// However, no time for v4 right now to fix this waste of memory, not too much indeed.
// But at least we dont need to lock!

bool DoTraffic(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i,k,l;
   double bearing, distance, sortvalue;
   double sortedValue[MAXTRAFFIC+1];

   static double lastRunTime=0;

   if (DoInit[MDI_DOTRAFFIC]) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... DoTraffic Init memset LKTraffic\n"));
	#endif
	memset(LKTraffic, 0, sizeof(LKTraffic));
	LKNumTraffic=0;
	#ifdef DEBUG_LKT
	StartupStore(_T("... DoTraffic Init memset LKSortedTraffic\n"));
	#endif
	memset(LKSortedTraffic, -1, sizeof(LKSortedTraffic));
	lastRunTime=0;
	DoInit[MDI_DOTRAFFIC]=false;
	return true;
   }

   // Wait for n seconds before updating again, to avoid data change too often
   // distracting the pilot.
   if (  lastRunTime > Basic->Time ) lastRunTime=Basic->Time;
   if (  (Basic->Time < (lastRunTime+NEARESTUPDATETIME)) && (LastDoTraffic>0)) {
        return false;
   }

   // DoTraffic is called from MapWindow, in real time. We have enough CPU power there now
   // Consider replay mode...
   if (  LastDoTraffic > Basic->Time ) LastDoTraffic=Basic->Time;
   if ( Basic->Time < (LastDoTraffic+PAGINGTIMEOUT) ) { 
	return false;
   }
   LastDoTraffic=Basic->Time;
   lastRunTime=Basic->Time;

   #ifdef DEBUG_LKT
   StartupStore(_T("... DoTraffic Copy LKTraffic and reset LKSortedTraffic\n"));
   #endif
   // No Need to lock, we are copying the drawinfo..copy of the real Basic.
   //LockFlightData();

   static_assert(sizeof(LKTraffic) == sizeof(Basic->FLARM_Traffic), "invalid sizeof");
   memcpy(LKTraffic, Basic->FLARM_Traffic, sizeof(LKTraffic));


   //UnlockFlightData();


   LKNumTraffic=0;
   for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if (LKTraffic[i].RadioId <= 0) continue;
	LKNumTraffic++;
	DistanceBearing(Basic->Latitude, Basic->Longitude, 
		LKTraffic[i].Latitude, LKTraffic[i].Longitude,
		&distance, &bearing);
	LKTraffic[i].Distance=distance;
	LKTraffic[i].Bearing=bearing;
   }
   if (LKNumTraffic<1) return true;

   //
   // In RADAR multimap there is no traffic sorting
   //
   if (MapSpaceMode==MSM_MAPRADAR) return true;

   memset(LKSortedTraffic, -1, sizeof(LKSortedTraffic));
   memset(sortedValue, -1, sizeof(sortedValue));

   // We know there is at least one traffic..
   for (i=0; i<FLARM_MAX_TRAFFIC; i++) {

	if ( LKTraffic[i].RadioId <= 0 ) continue;

	switch (SortedMode[MSM_TRAFFIC]) {
		case 0:	
			sortvalue=LKTraffic[i].Time_Fix;
			// sort by highest: the highest the closer to now
			sortvalue*=-1;
			break;
		case 1:
			sortvalue=LKTraffic[i].Distance;
			break;
		case 2:
			if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
				sortvalue=LKTraffic[i].Bearing;
				break;
			}
			sortvalue=LKTraffic[i].Bearing - Basic->TrackBearing;
			if (sortvalue < -180.0) sortvalue += 360.0;
			else
				if (sortvalue > 180.0) sortvalue -= 360.0;
			if (sortvalue<0) sortvalue*=-1;
			break;
		case 3:
			sortvalue=LKTraffic[i].Average30s;
			// sort by highest
			sortvalue*=-1;
			break;
		case 4:
			sortvalue=LKTraffic[i].Altitude;
			// sort by highest
			sortvalue*=-1;
			break;
		default:
			sortvalue=LKTraffic[i].Distance;
			break;
	}

	// MAXTRAFFIC is simply FLARM_MAX_TRAFFIC 
	for (k=0; k< MAXTRAFFIC; k++)  {

		// if new value is lower or index position is empty,  AND index position is not itself
		// (just for safety, since it is not possible with traffic)
		if ( ((sortvalue < sortedValue[k]) || (LKSortedTraffic[k]== -1))
		&& (LKSortedTraffic[k] != i) )
		{
			// ok, got new lower value, put it into slot
			for (l=MAXTRAFFIC-1; l>k; l--) {
				if (l>0) {
					sortedValue[l] = sortedValue[l-1];
					LKSortedTraffic[l] = LKSortedTraffic[l-1];
				}
			}
			sortedValue[k] = sortvalue;
			LKSortedTraffic[k] = i;
			//inserted++;
			break;
		}
	} // for k
	continue;

   } // for i
   #ifdef DEBUG_LKT
   StartupStore(_T("... DoTraffic Sorted, LKNumTraffic=%d :\n"),LKNumTraffic);
   for (i=0; i<MAXTRAFFIC; i++) {
	if (LKSortedTraffic[i]>=0)
		StartupStore(_T("... DoTraffic LKSortedTraffic[%d]=LKTraffic[%d] Id=%lx Name=<%s> Cn=<%s> Status=%d\n"), i, 
			LKSortedTraffic[i],
			LKTraffic[LKSortedTraffic[i]].ID,
			LKTraffic[LKSortedTraffic[i]].Name,
			LKTraffic[LKSortedTraffic[i]].Cn,
			LKTraffic[LKSortedTraffic[i]].Status);
   }
   #endif

   return true;
}

