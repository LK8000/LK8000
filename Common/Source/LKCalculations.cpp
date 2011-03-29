/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKCalculations.cpp,v 1.26 2010/12/22 01:07:44 root Exp root $
*/

#include "StdAfx.h"
#include "Defines.h" 
#include "LKUtils.h"
#include "Cpustats.h"
#include "Calculations.h"
#include "compatibility.h"
#ifdef OLDPPC
#include "XCSoarProcess.h"
#else
#include "Process.h"
#endif
#include "Utils.h"
#include "Utils2.h"
#include "externs.h"
#include "McReady.h"
#include "MapWindow.h"
#include "RasterTerrain.h"
#include <math.h>
#include <tchar.h>
#include "Calculations2.h"
#include "Message.h"
#include "Logger.h"

#include "utils/heapcheck.h"
#if defined(LKAIRSPACE) || defined(NEW_OLC)
using std::min;
using std::max;
#endif

extern void LatLon2Flat(double lon, double lat, int *scx, int *scy);
extern int CalculateWaypointApproxDistance(int scx_aircraft, int scy_aircraft, int i);

extern void InsertCommonList(int newwp);
extern void InsertRecentList(int newwp);
extern void RemoveRecentList(int newwp);

#define TASKINDEX	Task[ActiveWayPoint].Index
#define ISPARAGLIDER (AircraftCategory == (AircraftCategory_t)umParaglider)


/*
 * Used by Alternates and BestAlternate
 * Colors VGR are disabled, but available
 */

void DoAlternates(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint) {

  // handle virtual wps as alternates
  if (AltWaypoint<=RESWP_END) {
	if (!ValidResWayPoint(AltWaypoint)) return;
  } else {
	if (!ValidWayPoint(AltWaypoint)) return;
  }
  double w1lat = WayPointList[AltWaypoint].Latitude;
  double w1lon = WayPointList[AltWaypoint].Longitude;
  double w0lat = Basic->Latitude;
  double w0lon = Basic->Longitude;
  double *altwp_dist = &WayPointCalc[AltWaypoint].Distance;
  double *altwp_gr   = &WayPointCalc[AltWaypoint].GR;
  double *altwp_arrival = &WayPointCalc[AltWaypoint].AltArriv[AltArrivMode];
  short  *altwp_vgr  = &WayPointCalc[AltWaypoint].VGR;
  double GRsafecalc;

  DistanceBearing(w1lat, w1lon,
                  w0lat, w0lon,
                  altwp_dist, NULL);

  if (SafetyAltitudeMode==0 && !WayPointCalc[AltWaypoint].IsLandable)
	GRsafecalc = Calculated->NavAltitude - WayPointList[AltWaypoint].Altitude;
  else 
	GRsafecalc = Calculated->NavAltitude - WayPointList[AltWaypoint].Altitude - SAFETYALTITUDEARRIVAL;

  if (GRsafecalc <=0) *altwp_gr = INVALID_GR;
  else {
	*altwp_gr = *altwp_dist / GRsafecalc;
	if ( *altwp_gr >ALTERNATE_MAXVALIDGR || *altwp_gr <0 ) *altwp_gr = INVALID_GR;
	else if ( *altwp_gr <1 ) *altwp_gr = 1;
  }


  // We need to calculate arrival also for BestAlternate, since the last "reachable" could be
  // even 60 seconds old and things may have changed drastically

  *altwp_arrival = CalculateWaypointArrivalAltitude(Basic, Calculated, AltWaypoint);
  if ( (*altwp_arrival - ALTERNATE_OVERSAFETY) >0 ) {
  	if ( *altwp_gr <= (GlidePolar::bestld *SAFELD_FACTOR) ) *altwp_vgr = 1; // full green vgr
  	else 
  		if ( *altwp_gr <= GlidePolar::bestld ) *altwp_vgr = 2; // yellow vgr
		else *altwp_vgr =3; // RED vgr
  } else 
  {
	*altwp_vgr = 3; // full red
  }
}

// Fill Calculated values for waypoint, assuming that DistanceBearing has already been performed!
// Assumes that waypoint IS VALID
// Currently called by DoNearest

void DoNearestAlternate(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint) { 

  double *altwp_dist = &WayPointCalc[AltWaypoint].Distance;
  double *altwp_gr   = &WayPointCalc[AltWaypoint].GR;
  double *altwp_arrival = &WayPointCalc[AltWaypoint].AltArriv[AltArrivMode];
  short  *altwp_vgr  = &WayPointCalc[AltWaypoint].VGR;
  double GRsafecalc;

  if (SafetyAltitudeMode==0 && !WayPointCalc[AltWaypoint].IsLandable)
	GRsafecalc = Calculated->NavAltitude - WayPointList[AltWaypoint].Altitude;
  else
	GRsafecalc = Calculated->NavAltitude - WayPointList[AltWaypoint].Altitude - SAFETYALTITUDEARRIVAL;

  if (GRsafecalc <=0) *altwp_gr = INVALID_GR;
  else {
	*altwp_gr = *altwp_dist / GRsafecalc;
	if ( *altwp_gr >ALTERNATE_MAXVALIDGR || *altwp_gr <0 ) *altwp_gr = INVALID_GR;
	else if ( *altwp_gr <1 ) *altwp_gr = 1;
  }

  *altwp_arrival = CalculateWaypointArrivalAltitude(Basic, Calculated, AltWaypoint);
  if ( (*altwp_arrival - ALTERNATE_OVERSAFETY) >0 ) {
  	if ( *altwp_gr <= (GlidePolar::bestld *SAFELD_FACTOR) ) *altwp_vgr = 1; // full green vgr
  	else 
  		if ( *altwp_gr <= GlidePolar::bestld ) *altwp_vgr = 2; // yellow vgr
		else *altwp_vgr =3; // RED vgr
  } else 
  {
	*altwp_vgr = 3; // full red
  }
}

// Partially rewritten on december 2010 to make use of unsorted data from Range
// CAREFUL> SortedLandablexxx sized MAXNEAREST!!
// 101218 RangeLandables &C. are UNSORTED, careful
// This function is called with no map painted, and we have plenty of CPU time to spend!
// #define DEBUG_DONEAREST	1
void DoNearest(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i,k,l;
   int wp_index, inserted;
   double wp_bearing, wp_distance, wp_value;
   double sortedValue[MAXNEAREST+1];
   int *p_sortedIndex;
   int *p_rangeIndex;
   int dstSortedIndex[MAXNEAREST+1];
   int sortedRangeIndex[MAXNEAREST+1];

   // careful, consider MapSpaceMode could change while running!
   short curmapspace=MapSpaceMode;

   static double LastRunTime=0;

   // Consider replay mode...
   if (  LastRunTime > Basic->Time ) LastRunTime=Basic->Time;
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoNearest) {
	return;
   }
   if (  LastDoNearest > Basic->Time ) LastDoNearest=Basic->Time;
   if ( Basic->Time < (LastDoNearest+PAGINGTIMEOUT)) {
	return;
   }
   LastDoNearest = 0;
   LKForceDoNearest=false;

   if (!WayPointList) return;
   // No need to check airports, cannot be better
   if ( RangeLandableNumber==0) {
	return;
   }

   // This is done also at startup from InitModeTable
   for (i=0; i<MAXNEAREST;i++) {
	SortedAirportIndex[i]=-1;
	SortedLandableIndex[i]=-1;
        SortedTurnpointIndex[i]=-1;
	sortedRangeIndex[i]=-1;
	sortedValue[i]=99999; 
	dstSortedIndex[i]=-1;
   }

   // what are we looking at? we shall sort only that data
   switch (curmapspace) {
	case MSM_LANDABLE:
		p_rangeIndex=RangeLandableIndex;
   		p_sortedIndex=SortedLandableIndex;
		break;
	case MSM_AIRPORTS:
		p_rangeIndex=RangeAirportIndex;
   		p_sortedIndex=SortedAirportIndex;
		break;
	case MSM_NEARTPS:
		p_rangeIndex=RangeTurnpointIndex;
   		p_sortedIndex=SortedTurnpointIndex;
		break;
	default:
		p_rangeIndex=RangeLandableIndex;
   		p_sortedIndex=SortedLandableIndex;
		break;
   }

   // Range index is unsorted, we need to sort it by distance, because these are called "nearest"
   // we may have up to 500 (maxrangelandables) items to sort, but it is very unlikely.
   // normally we have only a few dozen airports in range 150km, and maybe 300 tps in range 75km

   #if DEBUG_DONEAREST
   StartupStore(_T(".. SORTING nearest\n"));
   #endif
   for (i=0, inserted=0; i<MAXRANGELANDABLE; i++) { 

	wp_index=*(p_rangeIndex+i);
	if (wp_index <0) {
		continue;
	}
   	#if DEBUG_DONEAREST
	StartupStore(_T("wp_index=%d  <%s>\n"),wp_index, WayPointList[wp_index].Name);
	#endif

	DistanceBearing(Basic->Latitude , Basic->Longitude , WayPointList[wp_index].Latitude,
		WayPointList[wp_index].Longitude, &wp_distance, &wp_bearing);

	// since we have them calculated, lets save these values 
	WayPointCalc[wp_index].Distance = wp_distance;
	WayPointCalc[wp_index].Bearing  = wp_bearing;

	wp_value=wp_distance;

	for (k=0; k< MAXNEAREST; k++)  {
		// if wp is closer than this one or this one isn't filled and not replacing with same
		if ( ((wp_value < sortedValue[k])	
		|| (sortedRangeIndex[k]== -1))
		&& (sortedRangeIndex[k] != wp_index) )
		{
			// ok, got new closer waypoint, put it into the slot.
			for (l=MAXNEAREST-1; l>k; l--) {
				if (l>0) {
					sortedValue[l] = sortedValue[l-1];
					sortedRangeIndex[l] = sortedRangeIndex[l-1];
				}
			}
			sortedValue[k] = wp_value;
			sortedRangeIndex[k] = wp_index;

			k=MAXNEAREST;
			inserted++;
			break;
		}
	} // for k
   } // for i 

   // All MAXNEAREST inserted values are updated
   #if DEBUG_DONEAREST
   StartupStore(_T("Sorted nearest are %d:\n"), inserted);
   #endif
   for (i=0; i<MAXNEAREST; i++) {
	sortedValue[i]=99999;  // we need to reset it  to use it again!
	wp_index=sortedRangeIndex[i];
	if (wp_index<0) continue; // just for safety
  	#if DEBUG_DONEAREST
	StartupStore(_T("(%d) wp n.%d <%s>\n"),i,wp_index,WayPointList[wp_index].Name);
   	#endif
	DoNearestAlternate(Basic,Calculated,wp_index);
   }

   // Range[..]Index is now sorted inside sortedRangeIndex 
   if (SortedMode[curmapspace] == 0 ) goto go_alfasort;
   if (SortedMode[curmapspace] == 1 ) goto go_directsort;

   // now the list is already sorted by distance and all values are updated 
   for (i=0; i<MAXNEAREST; i++) { 

	wp_index=sortedRangeIndex[i];
	if (wp_index <0) continue; // break is also ok

	// we calculate DoNearestAlternate only if really needed here, since we do it for MAXNEAREST*2
	// which is TWICE what we really need. 
	switch (SortedMode[curmapspace]) {
		case 2:
#ifndef MAP_ZOOM
			if (DisplayMode == dmCircling) {
#else /* MAP_ZOOM */
			if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
#endif /* MAP_ZOOM */
				wp_value=WayPointCalc[wp_index].Bearing;
				break;
			}
			wp_value = WayPointCalc[wp_index].Bearing -  GPS_INFO.TrackBearing;
			if (wp_value < -180.0) wp_value += 360.0;
			else
				if (wp_value > 180.0) wp_value -= 360.0;
			if (wp_value<0) wp_value*=-1;
			break;
		case 3:
			wp_value=WayPointCalc[wp_index].GR;
			break;
		case 4:
			wp_value=WayPointCalc[wp_index].AltArriv[AltArrivMode];
			// we sort lowest is better, so in this case we invert sign since higher is better
			wp_value*=-1;
			break;
		default:
			wp_value=WayPointCalc[wp_index].Distance;
			break;
	}
  	#if DEBUG_DONEAREST
	StartupStore(_T("... sort item %d <%d = %s>  by value=%f\n"),i,wp_index,WayPointList[wp_index].Name, wp_value);
   	#endif

	// Now sort by chosen field. Data is already sorted by distance ..
	// Yes I know, we dont need to sort by distance again. 
	for (k=0; k< MAXNEAREST; k++)  {
		// if wp is closer than this one or this one isn't filled and not replacing with same
		if ( ((wp_value < sortedValue[k])	
		|| (p_sortedIndex[k]== -1))
		&& (p_sortedIndex[k] != wp_index) )
		{
			// ok, got new closer waypoint, put it into the slot.
			for (l=MAXNEAREST-1; l>k; l--) {
				if (l>0) {
					sortedValue[l] = sortedValue[l-1];
					p_sortedIndex[l] = p_sortedIndex[l-1];
				}
			}
			sortedValue[k] = wp_value;
			p_sortedIndex[k] = wp_index;

			k=MAXNEAREST*2; 

			break;
		}
	} // for k

   } // for i

   go_directsort:
   if (SortedMode[curmapspace] == 1 ) {
	for (i=0; i<MAXNEAREST; i++) {
		p_sortedIndex[i] = sortedRangeIndex[i];
	}
   }

   go_alfasort:
   // Out of sorting mode, we still need to sort by name for alfa mode. We have sorted by distance for alfa, already.
   // So we shall be sorting nearest items, by alfa.
   if (SortedMode[curmapspace] == 0 ) {

	for (i=0; i<MAXNEAREST; i++) {
		wp_index=sortedRangeIndex[i];
		if (wp_index<0) {
			// There are less than 25 or whatever items available for sorting, no problems
			break; 
		}
		for (k=0; k< MAXNEAREST; k++)  {
			// if unfilled position or filled with not the same item already
			if ( (p_sortedIndex[k] < 0 ) || (p_sortedIndex[k] != wp_index) ) {
				if ( p_sortedIndex[k]>=0 ) 
					if ( wcscmp( WayPointList[wp_index].Name, WayPointList[p_sortedIndex[k]].Name) >0) continue;
				for (l=MAXNEAREST-1; l>k; l--) {
					if (l>0) {
						p_sortedIndex[l] = p_sortedIndex[l-1];
					}
				}
				p_sortedIndex[k] = wp_index;
				k=MAXNEAREST; 
			}
		} // for k
	} // for i

   }

   // Set flag dataready for LK8000. Reading functions will clear it!
   NearestDataReady=true;
   LastRunTime=Basic->Time;
   // update items found so far, in public variable. We have looked for MAXNEAREST items from a list of MAXNEAREST*1.5
   SortedNumber= inserted>MAXNEAREST ? MAXNEAREST : inserted;
   return;
}

// This was updated in december 2010
// REDUCE WAYPOINTLIST TO THOSE IN RANGE, ROUGHLY SORTED BY DISTANCE
// Keep an updated list of in-range landable waypoints.
// Attention: since this list is sorted only periodically, calling functions must sort data themselves.
// DISTANCE IS ASSUMED ON A FLAT SURFACE, ONLY APPROXIMATED!

// #define DEBUG_DORANGE	1

bool DoRangeWaypointList(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

   int rangeLandableDistance[MAXRANGELANDABLE];
   int rangeAirportDistance[MAXRANGELANDABLE];
   int rangeTurnpointDistance[MAXRANGETURNPOINT];
   #if UNSORTEDRANGE
   int i, kl, kt, ka;
   #else
   int i, k, l;
   bool inserted;
   #endif
   //double arrival_altitude;
   static bool DoInit=true;

   #if DEBUG_DORANGE
   StartupStore(_T("... DoRangeWaypointList is running\n"));
   #endif

   if (!WayPointList) {
	return false;
   }

   // Do init and RETURN, caution!
   // We need a locked GPS position to proceed!

   // TODO FIX LOCK DATA IN DRAWNEAREST when updating this list!
   if (DoInit) {
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
	DoInit=false;
	#if DEBUG_DORANGE
	StartupStore(_T("... DoRangeWaypointList INIT done, return.\n"));
	#endif
	return false;
   }

   LockTaskData();

   int scx_aircraft, scy_aircraft;
   LatLon2Flat(Basic->Longitude, Basic->Latitude, &scx_aircraft, &scy_aircraft);

  for (i=0; i<MAXRANGELANDABLE; i++) {
	RangeLandableNumber=0;
	RangeLandableIndex[i]= -1;
	rangeLandableDistance[i] = 0;
	RangeAirportNumber=0;
	RangeAirportIndex[i]= -1;
	rangeAirportDistance[i] = 0;
  }
  for (i=0; i<MAXRANGETURNPOINT; i++) {
	RangeTurnpointNumber=0;
	RangeTurnpointIndex[i]= -1;
	rangeTurnpointDistance[i] = 0;
  }

  #if UNSORTEDRANGE
  for (i=0, kt=0, kl=0, ka=0; i<(int)NumberOfWayPoints; i++) {
  #else
  for (i=0; i<(int)NumberOfWayPoints; i++) {
  #endif

	int approx_distance = CalculateWaypointApproxDistance(scx_aircraft, scy_aircraft, i);

	// Size a reasonable distance, wide enough 
	if ( approx_distance > DSTRANGETURNPOINT ) goto LabelLandables;

	// Get only non landables
	if (
		( (TpFilter==(TpFilter_t)TfNoLandables) && (!WayPointCalc[i].IsLandable ) ) ||
		( (TpFilter==(TpFilter_t)TfAll) ) ||
		( (TpFilter==(TpFilter_t)TfTps) && ((WayPointList[i].Flags & TURNPOINT) == TURNPOINT) ) 
	 ) {
		#if UNSORTEDRANGE
		if (kt+1<MAXRANGETURNPOINT) { // never mind if we use maxrange-2
			RangeTurnpointIndex[kt++]=i;
			RangeTurnpointNumber++;
		}
		#if DEBUG_DORANGE
		else {
			StartupStore(_T("... OVERFLOW RangeTurnpoint cannot insert <%s> in list\n"),WayPointList[i].Name);
		}
		#endif
		#else
		for (k=0, inserted=false; k< MAXRANGETURNPOINT; k++)  {
			if (((approx_distance < rangeTurnpointDistance[k]) 
			|| (RangeTurnpointIndex[k]== -1))
			&& (RangeTurnpointIndex[k]!= i))
			{
				// ok, got new biggest, put it into the slot.
				for (l=MAXRANGETURNPOINT-1; l>k; l--) {
					if (l>0) {
						rangeTurnpointDistance[l] = rangeTurnpointDistance[l-1];
						RangeTurnpointIndex[l] = RangeTurnpointIndex[l-1];
					}
				}
				rangeTurnpointDistance[k] = approx_distance;
				RangeTurnpointIndex[k] = i;
				k=MAXRANGETURNPOINT;
				inserted=true;
			}
		} // for k
		if (inserted) RangeTurnpointNumber++;
		#endif
	}


LabelLandables:

	if ( approx_distance > DSTRANGELANDABLE ) continue;

	// Skip non landable waypoints that are between DSTRANGETURNPOINT and DSTRANGELANDABLE
	if (!WayPointCalc[i].IsLandable )
		continue; 

	#if UNSORTEDRANGE
	if (kl+1<MAXRANGELANDABLE) { // never mind if we use maxrange-2
		RangeLandableIndex[kl++]=i;
		RangeLandableNumber++;
		// StartupStore(_T(".. insert landable <%s>\n"),WayPointList[i].Name); 
	}
	#if DEBUG_DORANGE
	else {
		StartupStore(_T("... OVERFLOW RangeLandable cannot insert <%s> in list\n"),WayPointList[i].Name);
	}
	#endif

	#else // not unsortedrange in use
	for (k=0, inserted=false; k< MAXRANGELANDABLE; k++)  {
		if (((approx_distance < rangeLandableDistance[k]) 
		|| (RangeLandableIndex[k]== -1))
        	&& (RangeLandableIndex[k]!= i))
        	{
          		// ok, got new biggest, put it into the slot.
          		for (l=MAXRANGELANDABLE-1; l>k; l--) {
            			if (l>0) {
              				rangeLandableDistance[l] = rangeLandableDistance[l-1];
             				RangeLandableIndex[l] = RangeLandableIndex[l-1];
            			}
          		}
         		rangeLandableDistance[k] = approx_distance;
          		RangeLandableIndex[k] = i;
          		k=MAXRANGELANDABLE;
			inserted=true;
        	}
	} // for k
	if (inserted) RangeLandableNumber++;
	#endif


	// If it's an Airport then we also take it into account separately
	if ( WayPointCalc[i].IsAirport )
	{
		#if UNSORTEDRANGE
		if (ka+1<MAXRANGELANDABLE) { // never mind if we use maxrange-2
			RangeAirportIndex[ka++]=i;
			RangeAirportNumber++;
		}
		#if DEBUG_DORANGE
		else {
			StartupStore(_T("... OVERFLOW RangeAirport cannot insert <%s> in list\n"),WayPointList[i].Name);
		}
		#endif
		#else
		for (k=0, inserted=false; k< MAXRANGELANDABLE; k++)  {
			if (((approx_distance < rangeAirportDistance[k]) 
			|| (RangeAirportIndex[k]== -1))
			&& (RangeAirportIndex[k]!= i))
			{
				// ok, got new biggest, put it into the slot.
				for (l=MAXRANGELANDABLE-1; l>k; l--) {
					if (l>0) {
						rangeAirportDistance[l] = rangeAirportDistance[l-1];
						RangeAirportIndex[l] = RangeAirportIndex[l-1];
					}
				}
				rangeAirportDistance[k] = approx_distance;
				RangeAirportIndex[k] = i;
				k=MAXRANGELANDABLE;
				inserted=true;
			}
		} // for k
		if (inserted) RangeAirportNumber++;
		#endif
	}


   } // for i

	// We have filled the list... which was too small 
	// this cannot happen with UNSORTED RANGE
	if (RangeTurnpointNumber>MAXRANGETURNPOINT) RangeTurnpointNumber=MAXRANGETURNPOINT;
	if (RangeAirportNumber>MAXRANGELANDABLE) RangeAirportNumber=MAXRANGELANDABLE;
	if (RangeLandableNumber>MAXRANGELANDABLE) RangeLandableNumber=MAXRANGELANDABLE;

   UnlockTaskData();
   return true;
}

bool DoCommonList(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

   int i;


   CommonNumber=0;
   for (i=0; i<MAXCOMMON; i++) {
	CommonIndex[i]= -1;
   }

   if (!WayPointList) {
        return false;
   }

   LockTaskData(); 

   // first we look for home and place it in position 0
   if (ValidWayPoint(HomeWaypoint)) {
	CommonIndex[CommonNumber++]=HomeWaypoint;
   }

   // Then we update the alternates in the common list
   InsertCommonList(BestAlternate);
   InsertCommonList(Alternate1);
   InsertCommonList(Alternate2);

   // Then we insert active destination and task points
   for (i=0; i< MAXTASKPOINTS; i++) {
	if (ValidTaskPoint(i))
		InsertCommonList(Task[i].Index);
   }

   UnlockTaskData();
   return true;
}

// Only insert if not already existing in the list
// and valid waypoint
void InsertCommonList(int newwp) {
    int i;

    if ((CommonNumber+1) >=MAXCOMMON) return;
    if ( !ValidWayPoint(newwp) ) return;

    for (i=0; i<CommonNumber; i++) {
	if ( CommonIndex[i] == newwp) return;
    }
    CommonIndex[CommonNumber++]=newwp;
}

// Load and save the recent list
bool LoadRecentList() {

   TCHAR buffer[100];
   FILE *fp;
   char st[100];
//   int TmpIndex[MAXCOMMON+1];
   int i=0, nwp;
   unsigned int csum;

#if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )
  LocalPath(buffer,TEXT(LKD_CONF));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(LKF_RECENTS)); // 091101

#else
  SHGetSpecialFolderPath(hWndMainWindow, buffer, CSIDL_PERSONAL, false);
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,TEXT(XCSDATADIR));
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,TEXT(LKD_CONF)); // 091101
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,_T(LKF_RECENTS)); // 091101
#endif

   RecentNumber=0;
   for (i=0; i<MAXCOMMON; i++) {
	RecentIndex[i]= -1;
	RecentChecksum[i]=0;
   }

   if (!WayPointList) {
   	StartupStore(_T("... Load RecentList: No Waypoint List available%s"),NEWLINE);
        return false;
   }

   if ( (fp=_tfopen(buffer, _T("r"))) == NULL ) {
	StartupStore(_T("... Cannot load recent wp history%s"),NEWLINE);
	return false;
   }

   i=0;
   while ( (fgets(st, 80, fp))!=NULL) {
	if (st[0]=='#') continue; // skip comments
	nwp=atoi(st);
	if (!ValidWayPoint(nwp)) {
		wsprintf(buffer,_T("---- Loading history. Found an invalid wp: <%d>%s"),nwp,NEWLINE); // BUGFIX 091122
		StartupStore(buffer);
		wsprintf(buffer,_T("---- History file could be corrupted or wp file changed abruptly. Discarded!%s"),NEWLINE);
		StartupStore(buffer);
		break;
	}
	if (i >=(MAXCOMMON)) {
		StartupStore(_T("---- Too many history waypoints to load!%s"),NEWLINE);
		break;
	}
	if ( (fgets(st,80,fp))==NULL) {
		StartupStore(_T("---- Incomplete or broken history file%s"),NEWLINE);
		break;
	}
	csum=atoi(st);
	// takeoff recent is different from others and can be forced on
	if (nwp!=RESWP_TAKEOFF) {
		if ( GetWpChecksum(nwp) != csum ) {
			StartupStore(_T("---- Loading history. Found an invalid checksum, aborting.%s"),NEWLINE);
			break;
		}
	}
	RecentIndex[i]=nwp;
	RecentChecksum[i++]=csum;
   }
   fclose(fp);
   RecentNumber=i;
   wsprintf(buffer,_T(". LoadRecentList: loaded %d recent waypoints%s"),i,NEWLINE);
   StartupStore(buffer);

   return true;
}

bool SaveRecentList() {

   TCHAR buffer[100];
   FILE *fp;
   int i;

#if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )
  LocalPath(buffer,TEXT(LKD_CONF));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(LKF_RECENTS)); // 091101
#else
  SHGetSpecialFolderPath(hWndMainWindow, buffer, CSIDL_PERSONAL, false);
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,TEXT(XCSDATADIR));
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,TEXT(LKD_CONF)); // 091101
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,_T(LKF_RECENTS)); // 091101
#endif

   StartupStore(_T(". Save history to <%s>%s"),buffer,NEWLINE);  // 091122
   if (!WayPointList) {
   	StartupStore(_T(". No Waypoint List is available, cannot save history%s"),NEWLINE);
        return false;
   }
   if (RecentNumber <0 || RecentNumber >(MAXCOMMON+1)) {
	StartupStore(_T("---- Save history: RecentNumber=%d exceeds limit, something is wrong. Aborting%s"),RecentNumber,NEWLINE);
	return false;
   }
   if ( (fp=_tfopen(buffer, _T("w"))) == NULL ) {
	StartupStore(_T("---- SaveRecentList: OPEN FILE FAILED. Cannot save recent wp history%s"),NEWLINE);
	return false;
   }

   fprintf(fp,"### LK8000 History of Goto Waypoints - DO NOT MODIFY THIS FILE! ###\r\n");
   fprintf(fp,"### WPRECENT FORMAT 01T \r\n");
   for (i=0; i<RecentNumber; i++)  {
	if ( !ValidNotResWayPoint(RecentIndex[i])) {
		StartupStore(_T("---- SaveHistory: invalid wp, maybe file has changed. Aborting.%s"),NEWLINE);
		break;
	}
	if ( WayPointList[RecentIndex[i]].FileNum == -1) continue; // 100219
	if ( GetWpChecksum(RecentIndex[i]) != RecentChecksum[i] ) {
		StartupStore(_T("---- SaveHistory: invalid checksum for wp=%d checksum=%d oldchecksum=%d, maybe file has changed. Aborting.%s"),
		i,GetWpChecksum(RecentIndex[i]), RecentChecksum[i],NEWLINE);
		break;
	}

	fprintf(fp,"%d\n%d\n",RecentIndex[i], RecentChecksum[i]);
   }
   fclose(fp);

   wsprintf(buffer,_T(". SaveRecentList: saved %d recent waypoints%s"),RecentNumber,NEWLINE);
   StartupStore(buffer);
   return true;
}

// Only insert if not already existing in the list
// Insert a wp into the list: always put it on top. If already existing in the list, 
// remove old existing one from the old position.
void InsertRecentList(int newwp) {
    int i,j;
    int TmpIndex[MAXCOMMON+1];
    unsigned int TmpChecksum[MAXCOMMON+1];

    // TCHAR buffer[100];
    // wsprintf(buffer,_T(". Insert WP=%d into recent waypoints%s"),newwp,NEWLINE);
    // StartupStore(buffer);

    // j holding number of valid recents, excluded the new one to be put in position 0
    for (i=0,j=0; i<RecentNumber; i++) {
	if ( j >=(MAXCOMMON-2) ) break;
	if (RecentIndex[i]==newwp) continue;
	TmpIndex[++j]=RecentIndex[i];
	TmpChecksum[j]=RecentChecksum[i];
    }
    RecentIndex[0]=newwp;
    
    RecentChecksum[0]= GetWpChecksum(newwp);
    for (i=1; i<=j; i++) {
	RecentIndex[i]=TmpIndex[i];
	RecentChecksum[i]=TmpChecksum[i];
    }
    RecentNumber=j+1;
}

void RemoveRecentList(int newwp) {
    int i,j;
    int TmpIndex[MAXCOMMON+1];
    unsigned int TmpChecksum[MAXCOMMON+1];

    for (i=0,j=0;i<RecentNumber; i++) {
	if (RecentIndex[i]==newwp) continue;
	TmpIndex[j]=RecentIndex[i];
	TmpChecksum[j++]=RecentChecksum[i];
    }
    RecentNumber=j;
    for (i=0; i<=j; i++) {
	RecentIndex[i]=TmpIndex[i];
	RecentChecksum[i]=TmpChecksum[i];
    }
}

void ResetRecentList() {
   int i;

   RecentNumber=0;
   for (i=0; i<MAXCOMMON; i++) {
	RecentIndex[i]= -1;
	RecentChecksum[i]= 0;
   }
   StartupStore(_T(". ResetRecentList%s"),NEWLINE);

}

// Recent informations: called from Calculation task
void DoRecent(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i;
   static bool DoInit=true;
   static double LastRunTime=0;

   // Safe initialisation, passthrough mode
   if (DoInit) {
        //for (i=0; i<MAXCOMMON; i++) RecentIndex[i]=-1;
	//RecentNumber=0;
	//LoadRecentList();
	// load recent file
        DoInit=false;
   }

   if (!WayPointList) return;

   // Consider replay mode...
   if (  LastRunTime > Basic->Time ) LastRunTime=Basic->Time;
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoRecent) {
        return;
   }
   LKForceDoRecent=false;
reload:
   //DoRecentList(Basic,Calculated);  NO NEED TO RELOAD...
   if (RecentNumber==0) return;

   for (i=0; i<RecentNumber; i++) {
	if (!ValidWayPoint(RecentIndex[i])) {
		RemoveRecentList(RecentIndex[i]);
		goto reload;
	}
	else
		DoAlternates(Basic, Calculated, RecentIndex[i]);
   }

   LastRunTime=Basic->Time;
   RecentDataReady=true;

}

// Common informations: home, best, alt1, alt2 and recent waypoints
void DoCommon(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{

   int i;
   static bool doinit=true;
   static double LastRunTime=0;

   // Safe initialisation, passthrough mode
   if (doinit) {
        for (i=0; i<MAXCOMMON; i++) CommonIndex[i]=-1;
	CommonNumber=0;
	DoCommonList(Basic,Calculated);
        doinit=false;
   }

   if (!WayPointList) return;

   // Consider replay mode...
   if (  LastRunTime > Basic->Time ) LastRunTime=Basic->Time;
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoCommon) {
        return;
   }
   if (  LastDoCommon > Basic->Time ) LastDoCommon=Basic->Time;
   if ( Basic->Time < (LastDoCommon+PAGINGTIMEOUT)) {
	return;
   }
   LastDoCommon = 0;
   LKForceDoCommon=false;

   DoCommonList(Basic,Calculated);
   if (CommonNumber==0) return;

   for (i=0; i<CommonNumber; i++) {
	DoAlternates(Basic, Calculated, CommonIndex[i]);
   }

   LastRunTime=Basic->Time;
   CommonDataReady=true;
} 



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

static bool CheckLandableReachableTerrainNew(NMEA_INFO *Basic,
                                          DERIVED_INFO *Calculated,
                                          double LegToGo,
                                          double LegBearing) {
  double lat, lon;
  bool out_of_range;
  double distance_soarable =
    FinalGlideThroughTerrain(LegBearing,
                             Basic, Calculated,
                             &lat,
                             &lon,
                             LegToGo, &out_of_range, NULL);

  if ((out_of_range)||(distance_soarable> LegToGo)) {
    return true;
  } else {
    return false;
  }
}

// -------------------- start old version ----------------------------------
#ifndef MULTICALC
// Warning: this is called from mapwindow Draw task , not from calculations!!
// this is calling CheckLandableReachableTerrainNew
void MapWindow::CalculateWaypointReachableNew(void)
{
  unsigned int i;
  /*
  #if UNSORTEDRANGE
  int j;
  #endif
   */
  double waypointDistance, waypointBearing,altitudeRequired,altitudeDifference;
  double dtmp;

  // LandableReachable is used only by the thermal bar indicator in MapWindow2, after here
  // apparently, is used to tell you if you are below final glide but in range for a landable wp
  LandableReachable = false;

  if (!WayPointList) return;

  LockTaskData();

  /*
  101218 We should include in this list also task points, at least.
  For 2.0 we still use all waypoints, since the check for Visible is fast. Pity.
  #if UNSORTEDRANGE
  for(j=0;j<RangeLandableNumber;j++)
  {
	i=RangeLandableIndex[j];
  #else
  */
  for(i=0;i<NumberOfWayPoints;i++)
  {
  // #endif
    if ( ( ((WayPointCalc[i].AltArriv >=0)||(WayPointList[i].Visible)) && (WayPointCalc[i].IsLandable)) // 100307
	|| WaypointInTask(i) ) {

	DistanceBearing(DrawInfo.Latitude, DrawInfo.Longitude, WayPointList[i].Latitude, WayPointList[i].Longitude, &waypointDistance, &waypointBearing);

	WayPointCalc[i].Distance=waypointDistance; 
	WayPointCalc[i].Bearing=waypointBearing;

	if (SafetyAltitudeMode==0 && !WayPointCalc[i].IsLandable)
		dtmp=DerivedDrawInfo.NavAltitude - WayPointList[i].Altitude;
	else
		dtmp=DerivedDrawInfo.NavAltitude - SAFETYALTITUDEARRIVAL - WayPointList[i].Altitude;

	if (dtmp>0) {
		WayPointCalc[i].GR = waypointDistance / dtmp;
		if (WayPointCalc[i].GR > INVALID_GR) WayPointCalc[i].GR=INVALID_GR; else
		if (WayPointCalc[i].GR <1) WayPointCalc[i].GR=1;
	} else
		WayPointCalc[i].GR = INVALID_GR;

	altitudeRequired = GlidePolar::MacCreadyAltitude (GetMacCready(i,0), waypointDistance, waypointBearing,  // 091221
						DerivedDrawInfo.WindSpeed, DerivedDrawInfo.WindBearing, 0,0,true,0);

	if (SafetyAltitudeMode==0 && !WayPointCalc[i].IsLandable)
		altitudeRequired = altitudeRequired + WayPointList[i].Altitude ;
	else
		altitudeRequired = altitudeRequired + SAFETYALTITUDEARRIVAL + WayPointList[i].Altitude ;

	WayPointCalc[i].AltReqd[AltArrivMode] = altitudeRequired; 

	altitudeDifference = DerivedDrawInfo.NavAltitude - altitudeRequired; 
	WayPointList[i].AltArivalAGL = altitudeDifference;
      
	if(altitudeDifference >=0){

		WayPointList[i].Reachable = TRUE;

	  	if (CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo, waypointDistance, waypointBearing)) {
			if ((signed)i!=TASKINDEX) { 
		  		LandableReachable = true;
			}
	  	} else {
			WayPointList[i].Reachable = FALSE;
		}
	} else {
		WayPointList[i].Reachable = FALSE;
	}

    } // if landable or in task
  } // for all waypoints

  if (!LandableReachable) // 091203
  /* 101218 same as above, bugfix
  #if UNSORTEDRANGE
  for(j=0;j<RangeLandableNumber;j++) {
	i = RangeLandableIndex[j];
  #else
  */
  for(i=0;i<NumberOfWayPoints;i++) {
  // #endif
    if(!WayPointList[i].Visible && WayPointList[i].FarVisible)  {
	// visible but only at a distance (limit this to 100km radius)

	if(  WayPointCalc[i].IsLandable ) {

		DistanceBearing(DrawInfo.Latitude, 
                                DrawInfo.Longitude, 
                                WayPointList[i].Latitude, 
                                WayPointList[i].Longitude,
                                &waypointDistance,
                                &waypointBearing);
               
		WayPointCalc[i].Distance=waypointDistance;  // VENTA6
		WayPointCalc[i].Bearing=waypointBearing;

		if (waypointDistance<100000.0) {

			altitudeRequired = GlidePolar::MacCreadyAltitude (GetMacCready(i,0), waypointDistance, waypointBearing,  // 091221
					DerivedDrawInfo.WindSpeed, DerivedDrawInfo.WindBearing, 0,0,true,0);
                  
			if (SafetyAltitudeMode==0 && !WayPointCalc[i].IsLandable)
                		altitudeRequired = altitudeRequired + WayPointList[i].Altitude ;
			else
                		altitudeRequired = altitudeRequired + SAFETYALTITUDEARRIVAL + WayPointList[i].Altitude ;

               		altitudeDifference = DerivedDrawInfo.NavAltitude - altitudeRequired;                                      
                	WayPointList[i].AltArivalAGL = altitudeDifference;

			WayPointCalc[i].AltReqd[AltArrivMode] = altitudeRequired; // VENTA6

                	if(altitudeDifference >=0){

                	    	WayPointList[i].Reachable = TRUE;

                	    	if (CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo, waypointDistance, waypointBearing)) {
                    	 		LandableReachable = true;
                     		} else
                    			WayPointList[i].Reachable = FALSE;
                    	} 
			else { 	
                    		WayPointList[i].Reachable = FALSE;
			}
		} else {
			WayPointList[i].Reachable = FALSE;
		} // <100000

	} // landable wp
     } // visible or far visible
   } // for all waypoint

  UnlockTaskData(); 
}
#endif
// -------------------------------- end of old version -------------------

double FarFinalGlideThroughTerrain(const double this_bearing, 
				NMEA_INFO *Basic, 
                                DERIVED_INFO *Calculated,
                                double *retlat, double *retlon,
                                const double max_range,
				bool *out_of_range,
				double myaltitude,
				double *TerrainBase)
{
  double irange = GlidePolar::MacCreadyAltitude(MACCREADY, 
						1.0, this_bearing, 
						Calculated->WindSpeed, 
						Calculated->WindBearing, 
						0, 0, true, 0);
  const double start_lat = Basic->Latitude;
  const double start_lon = Basic->Longitude;
  if (retlat && retlon) {
    *retlat = start_lat;
    *retlon = start_lon;
  }
  *out_of_range = false;

  if ((irange<=0.0)||(myaltitude<=0)) {
    // can't make progress in this direction at the current windspeed/mc
    return 0;
  }

  const double glide_max_range = myaltitude/irange;

  // returns distance one would arrive at altitude in straight glide
  // first estimate max range at this altitude
  double lat, lon;
  double last_lat, last_lon;
  double h=0.0, dh=0.0;
//  int imax=0;
  double last_dh=0;
  double altitude;
 
  RasterTerrain::Lock();
  double retval = 0;
  int i=0;
  bool start_under = false;

  // calculate terrain rounding factor

  FindLatitudeLongitude(start_lat, start_lon, 0, 
                        glide_max_range/NUMFINALGLIDETERRAIN, &lat, &lon);

  double Xrounding = fabs(lon-start_lon)/2;
  double Yrounding = fabs(lat-start_lat)/2;
  RasterTerrain::SetTerrainRounding(Xrounding, Yrounding);

  lat = last_lat = start_lat;
  lon = last_lon = start_lon;

  altitude = myaltitude;
  h =  max(0, (int)RasterTerrain::GetTerrainHeight(lat, lon)); 
  if (h==TERRAIN_INVALID) h=0; //@ 101027 FIX
  dh = altitude - h - SAFETYALTITUDETERRAIN;
  last_dh = dh;
  if (dh<0) {
    start_under = true;
    // already below safety terrain height
    //    retval = 0;
    //    goto OnExit;
  }

  // find grid
  double dlat, dlon;

  FindLatitudeLongitude(lat, lon, this_bearing, glide_max_range, &dlat, &dlon);
  dlat -= start_lat;
  dlon -= start_lon;

  double f_scale = 1.0/NUMFINALGLIDETERRAIN;
  if ((max_range>0) && (max_range<glide_max_range)) {
    f_scale *= max_range/glide_max_range;
  }

  double delta_alt = -f_scale*myaltitude;

  dlat *= f_scale;
  dlon *= f_scale;

  for (i=1; i<=NUMFINALGLIDETERRAIN; i++) {
    double f;
    bool solution_found = false;
    double fi = i*f_scale;
    // fraction of glide_max_range

    if ((max_range>0)&&(fi>=1.0)) {
      // early exit
      *out_of_range = true;
      retval = max_range;
      goto OnExit;
    }

    if (start_under) {
      altitude += 2.0*delta_alt;
    } else {
      altitude += delta_alt;
    }

    // find lat, lon of point of interest

    lat += dlat;
    lon += dlon;

    // find height over terrain
    h =  max(0,(int)RasterTerrain::GetTerrainHeight(lat, lon)); 
    if (h==TERRAIN_INVALID) h=0;

    dh = altitude - h - SAFETYALTITUDETERRAIN;

    if (TerrainBase && (dh>0) && (h>0)) {
      *TerrainBase = min(*TerrainBase, h);
    }

    if (start_under) {
      if (dh>last_dh) {
	// better solution found, ok to continue...
	if (dh>0) {
	  // we've now found a terrain point above safety altitude,
	  // so consider rest of track to search for safety altitude
	  start_under = false;
	}
      } else {
	f= 0.0;
	solution_found = true;
      }
    } else if (dh<=0) {
      if ((dh<last_dh) && (last_dh>0)) {
	if ( dh-last_dh==0 ) { // 091213
		StartupStore(_T("++++++++ FarFinalGlide recovered from division by zero!%s"),NEWLINE); // 091213
		f = 0.0;
	} else
        f = max(0.0,min(1.0,(-last_dh)/(dh-last_dh)));
      } else {
	f = 0.0;
      }
      solution_found = true;
    }
    if (solution_found) {
      double distance;
      lat = last_lat*(1.0-f)+lat*f;
      lon = last_lon*(1.0-f)+lon*f;
      if (retlat && retlon) {
        *retlat = lat;
        *retlon = lon;
      }
      DistanceBearing(start_lat, start_lon, lat, lon, &distance, NULL);
      retval = distance;
      goto OnExit;
    }
    last_dh = dh;
    last_lat = lat;
    last_lon = lon;
  }

  *out_of_range = true;
  retval = glide_max_range;

 OnExit:
  RasterTerrain::Unlock();
  return retval;
}

void ResetTask() {

  CALCULATED_INFO.ValidFinish = false;
  CALCULATED_INFO.ValidStart = false;
  CALCULATED_INFO.TaskStartTime = 0;
  CALCULATED_INFO.TaskStartSpeed = 0;
  CALCULATED_INFO.TaskStartAltitude = 0;
  CALCULATED_INFO.LegStartTime = 0;
  CALCULATED_INFO.MinAltitude = 0;
  CALCULATED_INFO.MaxHeightGain = 0;

// TaskDistanceCovered
// TaskDistanceToGo
// TaskSpeedAchieved

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
  DoStatusMessage(gettext(TEXT("_@M676_")));

}


//
// Running every n seconds ONLY when the nearest page is active and we are not drawing map.
// We DONT use FLARM_Traffic after we copy inside LKTraffic!!
// Returns true if did calculations, false if ok to use old values
bool DoTraffic(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i,k,l;
   double bearing, distance, sortvalue;
   double sortedValue[MAXTRAFFIC+1];

   static bool doinit=true;

   if (doinit) {
	#ifdef DEBUG_LKT
	StartupStore(_T("... DoTraffic Init memset LKTraffic\n"));
	#endif
	memset(LKTraffic, 0, sizeof(LKTraffic));
	LKNumTraffic=0;
	#ifdef DEBUG_LKT
	StartupStore(_T("... DoTraffic Init memset LKSortedTraffic\n"));
	#endif
	memset(LKSortedTraffic, -1, sizeof(LKSortedTraffic));
	doinit=false;
	return true;
   }


   // DoTraffic is called from MapWindow, in real time. We have enough CPU power there now
   // Consider replay mode...
   if (  LastDoTraffic > Basic->Time ) LastDoTraffic=Basic->Time;
   if ( Basic->Time < (LastDoTraffic+PAGINGTIMEOUT) ) { 
	return false;
   }
   LastDoTraffic=Basic->Time;

   #ifdef DEBUG_LKT
   StartupStore(_T("... DoTraffic Copy LKTraffic and reset LKSortedTraffic\n"));
   #endif
   LockFlightData();
   memcpy(LKTraffic, Basic->FLARM_Traffic, sizeof(LKTraffic));
   UnlockFlightData();

   memset(LKSortedTraffic, -1, sizeof(LKSortedTraffic));
   memset(sortedValue, -1, sizeof(sortedValue));

   LKNumTraffic=0;
   for (i=0; i<FLARM_MAX_TRAFFIC; i++) {
	if ( LKTraffic[i].ID <=0 ) continue;
	LKNumTraffic++;
	DistanceBearing(Basic->Latitude, Basic->Longitude, 
		LKTraffic[i].Latitude, LKTraffic[i].Longitude,
		&distance, &bearing);
	LKTraffic[i].Distance=distance;
	LKTraffic[i].Bearing=bearing;
   }
   if (LKNumTraffic<1) return true;

   // We know there is at least one traffic..
   for (i=0; i<FLARM_MAX_TRAFFIC; i++) {

	if ( LKTraffic[i].ID <=0 ) continue;

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
#ifndef MAP_ZOOM
			if (DisplayMode == dmCircling) {
#else /* MAP_ZOOM */
			if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
#endif /* MAP_ZOOM */
				sortvalue=LKTraffic[i].Bearing;
				break;
			}
			sortvalue=LKTraffic[i].Bearing - GPS_INFO.TrackBearing;
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


// Running every n seconds ONLY when the Target Infopage is active ( we are not drawing map).
// We DONT use FLARM_Traffic after we copy inside LKTraffic!!
// Returns true if did calculations, false if ok to use old values
#define LKTARG LKTraffic[LKTargetIndex]
bool DoTarget(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   double x0, y0, etas, ttgo=0;
   double bearing, distance;

   if (LKTargetIndex<0 || LKTargetIndex>=MAXTRAFFIC) return false;

   // DoTarget is called from MapWindow, in real time. We have enough CPU power there now

   #if 0
   if (  LastDoTarget > Basic->Time ) LastDoTarget=Basic->Time;
   // We calculate in real time, because PFLAA sentences are calculated too in real time
   if ( Basic->Time < (LastDoTarget+3.0) ) { 
	return false;
   }
   LastDoTarget=Basic->Time;
   #endif

   #ifdef DEBUG_LKT
   StartupStore(_T("... DoTarget Copy LKTraffic\n"));
   #endif
   LockFlightData();
   memcpy(LKTraffic, Basic->FLARM_Traffic, sizeof(LKTraffic));
   UnlockFlightData();

   if (LKTARG.ID <=0) return false;

   DistanceBearing(Basic->Latitude, Basic->Longitude, 
		LKTARG.Latitude, LKTARG.Longitude,
		&distance, &bearing);
   LKTARG.Distance=distance;
   LKTARG.Bearing=bearing;

   if (LKTARG.Speed>1) {
	x0 = fastsine(LKTARG.TrackBearing)*LKTARG.Speed;
	y0 = fastcosine(LKTARG.TrackBearing)*LKTARG.Speed;
	x0 += fastsine(Calculated->WindBearing)*Calculated->WindSpeed;
	y0 += fastcosine(Calculated->WindBearing)*Calculated->WindSpeed;
	// LKTARG.Heading = AngleLimit360(atan2(x0,y0)*RAD_TLK_DEG); // 101210 check
	etas = isqrt4((unsigned long)(x0*x0*100+y0*y0*100))/10.0;
	LKTARG.EIAS = etas/AirDensityRatio(LKTARG.Altitude);
   } else {
	LKTARG.EIAS=0;
   }


   //double height_above_target = Calculated->NavAltitude - LKTARG.Altitude;

   LKTARG.AltArriv = Calculated->NavAltitude - GlidePolar::MacCreadyAltitude(MACCREADY,
	distance,
	bearing,
	Calculated->WindSpeed, Calculated->WindBearing,
	0, 0,
	// final glide, use wind
	true, 
	&ttgo) - LKTARG.Altitude;
	
   // We CANNOT use RelativeAltitude because when ghost or zombie, it wont be updated in real time in respect
   // to OUR real position!! Lets use the last target altitude known.
   double relalt=Calculated->NavAltitude - LKTARG.Altitude;
   if (relalt==0)
	LKTARG.GR=999;
   else {
	// we need thus to invert the relative altitude
	LKTARG.GR=distance/(relalt);
   }


   return true;
}


double GetAzimuth() {
  double sunazimuth=0;
  if (!Shading) return 0;
  if (CALCULATED_INFO.WindSpeed<1.7) { // below 6kmh the sun will prevail, for a guess.

	#if 0
	if (GPS_INFO.Latitude>=0)
		// NORTHERN EMISPHERE
		sunazimuth = MapWindow::GetDisplayAngle() + 135; //@  -45+180=135
	else
		// SOUTHERN EMISPHERE
		sunazimuth = MapWindow::GetDisplayAngle() + 45.0;
	#else
	int sunoffset=0; // 0 for southern emisphere, 180 for northern. 
	if (GPS_INFO.Latitude>=0) sunoffset=180; 

	int dd = abs(DetectCurrentTime()) % (3600*24);
	int hours = (dd/3600);
	int mins = (dd/60-hours*60);
	hours = hours % 24;
	if (SIMMODE) {
		// for those who test the sim mode in the evening..
		if (hours>21) hours-=12;
		if (hours<7) hours+=12;
	}
	double h=(12-(double)hours)-((double)mins/60.0);

	if (h>=0) {
		if (h>5) h=5;
		sunazimuth = MapWindow::GetDisplayAngle() +sunoffset + (h*12); //@ 60 deg from east, max
	} else {
		h*=-1;
		if (h>6) h=6;
		sunazimuth = MapWindow::GetDisplayAngle() +sunoffset - (h*10);
	}
	#endif

  } else {
	// else we use wind direction for shading. 
	sunazimuth = MapWindow::GetDisplayAngle()-CALCULATED_INFO.WindBearing;
  }
  return sunazimuth;
}

unsigned int GetWpChecksum(unsigned int index) { //@ 101018

  int clon, clat, csum;
 
  if (index<NUMRESWP || index > NumberOfWayPoints) {
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



#define LKTH_LAT		Calculated->ThermalEstimate_Latitude
#define LKTH_LON		Calculated->ThermalEstimate_Longitude
#define LKTH_R			Calculated->ThermalEstimate_R
#define LKTH_W			Calculated->ThermalEstimate_W
#define LK_TURNRATE		Calculated->TurnRate
#define LK_HEADING		Calculated->Heading
#define LK_BANKING		Calculated->BankAngle
#define LK_MYTRACK		GPS_INFO.TrackBearing
#define LK_MYLAT		GPS_INFO.Latitude
#define LK_MYLON		GPS_INFO.Longitude

//#define DEBUG_ORBITER	1

// Thermal orbiter by Paolo Ventafridda, November 2010
void CalculateOrbiter(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  static unsigned int timepassed=0;
  static bool alreadywarned=false;

  if (!Calculated->Circling || !EnableSoundModes) return;
  timepassed++;
  if (LKTH_R <1) {
	return;  // no thermal center available
  }

  double thtime = Basic->Time - Calculated->ClimbStartTime;
  // we need a valid thermal center to work on. Assuming 2 minutes is enough to sample the air around.
  if (thtime<120) return;
  // after 1500ft thermal gain, autodisable
  if (Calculated->ThermalGain>500) return;

  // StartupStore(_T("*** Tlat=%f Tlon=%f R=%f W=%f  TurnRate=%f \n"), LKTH_LAT, LKTH_LON, LKTH_R, LKTH_W, LK_TURNRATE);
  // StartupStore(_T("*** CalcHeading=%f Track=%f TurnRate=%f Bank=%f \n"), LK_HEADING, LK_MYTRACK,  LK_TURNRATE, LK_BANKING);

  double th_center_distance, th_center_bearing;	// thermal center
  double orbital_distance, orbital_bearing;	// orbital tangent 
  double orbital_brgdiff;			// current difference between track and orbital_bearing
  double orbital_warning_angle;			// warning angle difference

  double alpha;					// alpha angle to the tangent relative to th_center_bearing
  double circlesense=0;				// 1 for CW, -1 for CCW

  // these parameters should be dynamically calculated for each thermal in the future
  // th_angle should be correlated to IAS

  double th_radius;				// thermal radius
  double th_minoverradius, th_maxoverradius;	// radius+overradius is the minimal and max approach distance
  double th_angle;				// deg/s default turning in the thermal. Pilot should turn with this 
						// angolar speed.
  double reactiontime;			// how many seconds pass before the pilot start turning
  double turningcapacitypersecond;	// how many degrees can I change in my turnrate in a second?

  // TODO: TUNE THEM IN REAL TIME!
  if (ISPARAGLIDER) {
	th_radius=50;
	th_minoverradius=1;
	th_maxoverradius=100;
	th_angle=18;
	turningcapacitypersecond=10;
	reactiontime=1.0;
  } else { 
	th_radius=90;
	th_minoverradius=5;
	th_maxoverradius=160;
	th_angle=16.5;
	turningcapacitypersecond=10;
	reactiontime=1.0;
  }

  circlesense=LK_TURNRATE>0 ? 1 : -1;	//@ CCW: -1   CW: 1
  if (circlesense==0 || (fabs(LK_TURNRATE)<8)) { // 8 deg/sec are 45 seconds per turn
	alreadywarned=false;
	return;
  }

  DistanceBearing(LK_MYLAT, LK_MYLON,LKTH_LAT,LKTH_LON,&th_center_distance,&th_center_bearing);

  if (th_center_distance< (th_radius+th_minoverradius)) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** Too near:  dist=%.0f < %.0f\n"), th_center_distance, th_radius+th_minoverradius);
	#endif
	return;
  }
  if (th_center_distance> (th_radius+th_maxoverradius)) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** Too far:  dist=%.0f >  %.0f\n"), th_center_distance, th_radius+th_maxoverradius);
	#endif
	return;
  }
  // StartupStore(_T("*** distance to orbit %.0f (ratio:%0.f)\n"), 
  // th_center_distance- th_radius , (th_center_distance - th_radius)/th_radius);

  alpha= atan2(th_radius, th_center_distance)*RAD_TO_DEG;

  orbital_bearing  = th_center_bearing - (alpha*circlesense); //@ add for CCW
  if (orbital_bearing>359) orbital_bearing-=360;
  orbital_distance = th_radius / sin(alpha);

  // StartupStore(_T("*** tc_dist=%f th_center_bearing=%f  orbital_distance=%f orbital_bearing=%f  alpha=%f  mydir=%f\n"),
  // th_center_distance, th_center_bearing, orbital_distance, orbital_bearing, alpha, LK_HEADING );

  if (circlesense==1) 
	orbital_brgdiff = orbital_bearing-LK_HEADING; // CW
  else
	orbital_brgdiff = LK_HEADING - orbital_bearing; // CCW

  if (orbital_brgdiff<0) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** passed target orbit direction\n"));
	#endif
	return;
  }
  if (orbital_brgdiff>90) {
	#if DEBUG_ORBITER
	StartupStore(_T("*** wrong direction\n"));
	#endif
	return;
  }

  static int lasttimewarned=0;
  double actionadvancetime;		// how many seconds are needed in advance, given the current turnrate?

  // assuming circling with the same radius of the perfect thermal: 18deg/s, 20s turn rate approx.
  actionadvancetime=(circlesense*(LK_TURNRATE/turningcapacitypersecond))+reactiontime;

  // When circling with a radius greater than the default, we have a problem.
  // We must correct the predicted angle threshold
  orbital_warning_angle=actionadvancetime*(LK_TURNRATE*LK_TURNRATE)/th_angle;
  #if DEBUG_ORBITER
  StartupStore(_T("... LK_TURNRATE=%.0f  advancetime=%.2f  angle=%.1f \n"), LK_TURNRATE, actionadvancetime,orbital_warning_angle);
  #endif

  if (orbital_brgdiff<orbital_warning_angle) {
	if (!alreadywarned && ((timepassed-lasttimewarned)>12) ) {

		if (th_center_distance<(th_radius+(th_maxoverradius/2))) {
			#if DEBUG_ORBITER
			StartupStore(_T("****** GO STRAIGHT NOW FOR SHORT TIME ******\n"));
			#endif
			LKSound(_T("LK_ORBITER1.WAV"));
		} else {
			#if DEBUG_ORBITER
			StartupStore(_T("****** GO STRAIGHT NOW FOR LONGER TIME ******\n"));
			#endif
			LKSound(_T("LK_ORBITER2.WAV"));
		}
		alreadywarned=true;
		lasttimewarned=timepassed;
	} else {
		#if DEBUG_ORBITER
		StartupStore(_T("****** GO STRAIGHT, already warned\n"));
		#endif
	}
  } else {
	alreadywarned=false;
  }


}

// -------- new version ---------------------------
#if MULTICALC

// Warning: this is called from mapwindow Draw task , not from calculations!!
// this is calling CheckLandableReachableTerrainNew
// 
void MapWindow::LKCalculateWaypointReachable(short multicalc_slot, short numslots)
{
  unsigned int i;
  /*
  #if UNSORTEDRANGE
  int j;
  #endif
   */
  double waypointDistance, waypointBearing,altitudeRequired,altitudeDifference;
  double dtmp;

  // LandableReachable is used only by the thermal bar indicator in MapWindow2, after here
  // apparently, is used to tell you if you are below final glide but in range for a landable wp
  LandableReachable = false;

  if (!WayPointList) return;

  unsigned int scanstart;
  unsigned int scanend;

  LockTaskData();

  if (multicalc_slot==0) {
	scanstart=0; // including this
	scanend=NumberOfWayPoints; // will be used -1, so up to this excluded value

	//StartupStore(_T("... wps=%d multicalc_slot=0 ignored numslot=%d, full scan %d < %d%s"),NumberOfWayPoints,
	//	numslots,scanstart,scanend,NEWLINE);
  } else {
	scanstart=(NumberOfWayPoints/numslots)*(multicalc_slot-1); 
	if (multicalc_slot==numslots)
		scanend=NumberOfWayPoints;
	else
		scanend=scanstart+(NumberOfWayPoints/numslots);

	//StartupStore(_T("... wps=%d multicalc_slot=%d of %d, scan %d < %d%s"),NumberOfWayPoints,
	//	multicalc_slot, numslots,scanstart,scanend,NEWLINE);
  }

  /*
  // 101218 We should include in this list also task points, at least.
  // For 2.0 we still use all waypoints, since the check for Visible is fast. Pity.
  #if UNSORTEDRANGE
  for(j=0;j<RangeLandableNumber;j++)
  {
	i=RangeLandableIndex[j];
	...
  #endif
  */

  for(i=scanstart;i<scanend;i++) {
    if ( ( ((WayPointCalc[i].AltArriv >=0)||(WayPointList[i].Visible)) && (WayPointCalc[i].IsLandable)) 
	|| WaypointInTask(i) ) {

	DistanceBearing(DrawInfo.Latitude, DrawInfo.Longitude, WayPointList[i].Latitude, WayPointList[i].Longitude, 
		&waypointDistance, &waypointBearing);

	WayPointCalc[i].Distance=waypointDistance; 
	WayPointCalc[i].Bearing=waypointBearing;

	if (SafetyAltitudeMode==0 && !WayPointCalc[i].IsLandable)
		dtmp=DerivedDrawInfo.NavAltitude - WayPointList[i].Altitude;
	else
		dtmp=DerivedDrawInfo.NavAltitude - SAFETYALTITUDEARRIVAL - WayPointList[i].Altitude;

	if (dtmp>0) {
		WayPointCalc[i].GR = waypointDistance / dtmp;
		if (WayPointCalc[i].GR > INVALID_GR) WayPointCalc[i].GR=INVALID_GR; else
		if (WayPointCalc[i].GR <1) WayPointCalc[i].GR=1;
	} else
		WayPointCalc[i].GR = INVALID_GR;

	altitudeRequired = GlidePolar::MacCreadyAltitude (GetMacCready(i,0), waypointDistance, waypointBearing,  // 091221
						DerivedDrawInfo.WindSpeed, DerivedDrawInfo.WindBearing, 0,0,true,0);

	if (SafetyAltitudeMode==0 && !WayPointCalc[i].IsLandable)
		altitudeRequired = altitudeRequired + WayPointList[i].Altitude ;
	else
		altitudeRequired = altitudeRequired + SAFETYALTITUDEARRIVAL + WayPointList[i].Altitude ;

	WayPointCalc[i].AltReqd[AltArrivMode] = altitudeRequired; 

	altitudeDifference = DerivedDrawInfo.NavAltitude - altitudeRequired; 
	WayPointList[i].AltArivalAGL = altitudeDifference;
      
	if(altitudeDifference >=0){

		WayPointList[i].Reachable = TRUE;

	  	if (CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo, waypointDistance, waypointBearing)) {
			if ((signed)i!=TASKINDEX) { 
		  		LandableReachable = true;
			}
	  	} else {
			WayPointList[i].Reachable = FALSE;
		}
	} else {
		WayPointList[i].Reachable = FALSE;
	}

    } // if landable or in task
  } // for all waypoints

  if (!LandableReachable) // indentation wrong here

  /* 101218 same as above, bugfix
  #if UNSORTEDRANGE
  for(j=0;j<RangeLandableNumber;j++) {
	i = RangeLandableIndex[j];
	... 
  #endif
  */
  for(i=scanstart;i<scanend;i++) {
    if(!WayPointList[i].Visible && WayPointList[i].FarVisible)  {
	// visible but only at a distance (limit this to 100km radius)

	if(  WayPointCalc[i].IsLandable ) {

		DistanceBearing(DrawInfo.Latitude, 
                                DrawInfo.Longitude, 
                                WayPointList[i].Latitude, 
                                WayPointList[i].Longitude,
                                &waypointDistance,
                                &waypointBearing);
               
		WayPointCalc[i].Distance=waypointDistance;  // VENTA6
		WayPointCalc[i].Bearing=waypointBearing;

		if (waypointDistance<100000.0) {

			altitudeRequired = GlidePolar::MacCreadyAltitude (GetMacCready(i,0), waypointDistance, waypointBearing,  // 091221
					DerivedDrawInfo.WindSpeed, DerivedDrawInfo.WindBearing, 0,0,true,0);
                  
			if (SafetyAltitudeMode==0 && !WayPointCalc[i].IsLandable)
                		altitudeRequired = altitudeRequired + WayPointList[i].Altitude ;
			else
                		altitudeRequired = altitudeRequired + SAFETYALTITUDEARRIVAL + WayPointList[i].Altitude ;

               		altitudeDifference = DerivedDrawInfo.NavAltitude - altitudeRequired;                                      
                	WayPointList[i].AltArivalAGL = altitudeDifference;

			WayPointCalc[i].AltReqd[AltArrivMode] = altitudeRequired; // VENTA6

                	if(altitudeDifference >=0){

                	    	WayPointList[i].Reachable = TRUE;

                	    	if (CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo, waypointDistance, waypointBearing)) {
                    	 		LandableReachable = true;
                     		} else
                    			WayPointList[i].Reachable = FALSE;
                    	} 
			else { 	
                    		WayPointList[i].Reachable = FALSE;
			}
		} else {
			WayPointList[i].Reachable = FALSE;
		} // <100000

	} // landable wp
     } // visible or far visible
   } // for all waypoint

  UnlockTaskData(); 
}
#endif
// ---------- new version ----------------


/* 
 * Detect start of free flight (FF) for both towing and winching.
 * Start of FF may be marked 3 seconds later, max. Small error.
 * This combined approach is much more accurate than seeyou!
 * WARNING> the internal IGC interpolator is buggy and will cause problems
 * while replaying IGC file logged at more than 2s interval.
 * During testing, all 1s interval logs (which are equivalent to real flight)
 * worked fine, while we got false positives using >2s interval logs.
 * We dont care much, since we need real flight FF detection, and LK
 * is logging always at 1s. So this might actually happen only replaying 
 * an external log file on LK. 
 *
 * Called by Calculations thread, each second.
 *
 * No message will be given to the user concerning start of free flight.
 * We should not disturb pilots during critical phases of flight.
 */
#define FF_TOWING_ALTLOSS	25		// meters loss for towing
#define FF_WINCHIN_ALTLOSS	10		// meters loss for winching, careful about GPS H errors.
#define FF_MAXTOWTIME	1200			// 20 minutes

#define DEBUG_DFF	1		// REMOVE BEFORE FLIGHT!

bool DetectFreeFlying(DERIVED_INFO *Calculated) {

  static bool   ffDetected=false;
  static int    lastMaxAltitude=-1000;
  static double gndAltitude=0;
  static bool   doinit=true;
  static double vario[8];
  static bool   winchdetected=false;
  static short  wlaunch=0;
  static int    altLoss=0;

  if (doinit) {
    for (int i=0; i<8; i++) vario[i]=0;
    doinit=false;
  }

  // reset on ground
  if (Calculated->Flying == FALSE) {
    Calculated->FreeFlying=false;
    ffDetected=false;
    lastMaxAltitude=-1000;
    gndAltitude=GPS_INFO.Altitude;
    winchdetected=false;
    wlaunch=0;
    altLoss=FF_TOWING_ALTLOSS;
    return false;
  }

  if (ISPARAGLIDER) {
    Calculated->FreeFlying=true;
    ffDetected=true;
    return true; 
  }

  // do something special for other situations
  if (SIMMODE && !ReplayLogger::IsEnabled()) {
    Calculated->FreeFlying=true;
    ffDetected=true;
    return true; 
  }

  // If flying, and start was already detected, assume valid freeflight.
  // Put here in the future the Engine Noise Detection for motorplanes
  if (ffDetected)
    return true;

  // Here we are flying, and the start of free flight must still be detected

  // In any case, after this time, force a start. This is to avoid that for any reason, no FF is ever detected.
  if ((int)GPS_INFO.Time > ( Calculated->TakeOffTime + FF_MAXTOWTIME)) {
    #if DEBUG_DFF
    DoStatusMessage(_T("DFF:  TIMEOUT"));
    #endif
    goto backtrue;	// unconditionally force start FF
  }

 
  // A loss of altitude will trigger FF 
  lastMaxAltitude = std::max(lastMaxAltitude, (int)GPS_INFO.Altitude);
  if ((int)GPS_INFO.Altitude <= ( lastMaxAltitude - altLoss)) {
    #if DEBUG_DFF
    if ( (winchdetected) || ((GPS_INFO.Altitude - gndAltitude)>=400) 
      && ((GPS_INFO.Time - Calculated->TakeOffTime) >= 150))
      DoStatusMessage(_T("DFF:  ALTITUDE LOSS"));
    #endif
    goto lastcheck;
  }

  // If circling we assume that we are in free flight already, using the start of circling time and position.
  // But we must be sure that we are not circling.. while towed. A 12 deg/sec turn rate will make it quite sure.
  // Of course nobody can circle while winchlaunched, so in this case FF is immediately detected.
  if (Calculated->Circling && ( winchdetected || ( fabs(Calculated->TurnRate) >=12 ) ) ) {

    CContestMgr::Instance().Add(new CPointGPS(static_cast<unsigned>(Calculated->ClimbStartTime),
	Calculated->ClimbStartLat, Calculated->ClimbStartLong,
	static_cast<unsigned>(Calculated->ClimbStartAlt)));

    #if DEBUG_DFF
    DoStatusMessage(_T("DFF:  THERMALLING"));
    #endif
    goto backtrue;
  }

  vario[7]=vario[6];
  vario[6]=vario[5];
  vario[5]=vario[4];
  vario[4]=vario[3];
  vario[3]=vario[2];
  vario[2]=vario[1];
  vario[1]=Calculated->Vario;

  double newavervario;
  double oldavervario;

  newavervario=(vario[1]+vario[2]+vario[3])/3;

  if (newavervario>=10) {
    wlaunch++;
  } else {
    wlaunch=0;
  }
  // After (6+2) 8 seconds of consecutive fast climbing (AFTER TAKEOFF DETECTED!), winch launch is detected
  if (wlaunch==6) {
    #if DEBUG_DFF
    DoStatusMessage(_T("DFF:  WINCH LAUNCH"));
    #endif
    altLoss=FF_WINCHIN_ALTLOSS;
    winchdetected=true;
  }
    
  if (newavervario>0.3)
    return false;

  if (newavervario<=0) {
    #if DEBUG_DFF
    if ( (winchdetected) || ((GPS_INFO.Altitude - gndAltitude)>=400) 
      && ((GPS_INFO.Time - Calculated->TakeOffTime) >= 150))
      DoStatusMessage(_T("DFF:  VARIO SINK"));
    #endif
    goto lastcheck;
  }

  oldavervario=(vario[4]+vario[5]+vario[6]+vario[7])/4;
  // current avervario between 0.0 and 0.3: uncertain situation,  we check what we had in the previous time
  // Windy situations during towing may lead to false positives if delta is too low. This is the most
  // difficult part which could lead to false positives or negatives.
  if ( oldavervario >=4.5 ) {
    #if DEBUG_DFF
    StartupStore(_T("..... oldavervario=%.1f newavervario=%.1f current=%.1f\n"),oldavervario,newavervario,Calculated->Vario);
    if ( (winchdetected) || ((GPS_INFO.Altitude - gndAltitude)>=400) 
      && ((GPS_INFO.Time - Calculated->TakeOffTime) >= 150))
      DoStatusMessage(_T("DFF:  DELTA VARIO"));
    #endif
    goto lastcheck;
  }

  // No free flight detected
  return false;

  lastcheck: 

  // Unless under a winch launch, we shall not consider anything below 400m gain, 
  // and before 2.5minutes have passed since takeoff
  // Anybody releasing tow before 3 minutes will be below 500m QFE, and won't go much around
  // until a thermal is found. So no problems. 

  if (!winchdetected) {
    if ( (GPS_INFO.Altitude - gndAltitude)<400)
      return false;
    if ( (GPS_INFO.Time - Calculated->TakeOffTime) < 150)
      return false;
  }

  backtrue:
  #if DEBUG_DFF
  DoStatusMessage(gettext(TEXT("_@M1452_")));  // LKTOKEN  _@M1452_ = "Free flight detected"
  #endif
  ffDetected=true;
  Calculated->FreeFlying=true;
  return true;

}
