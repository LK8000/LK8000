/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: DoNearest.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "NavFunctions.h"


// Fill Calculated values for waypoint, assuming that DistanceBearing has already been performed!
// Assumes that waypoint IS VALID
// Currently called by DoNearest

// This is the windows in degrees, +-60, where we accept waypoints when sorting by current direction
#define DIRECTIONRANGE	60

void DoNearestAlternate(NMEA_INFO *Basic, DERIVED_INFO *Calculated, int AltWaypoint) { 

  BUGSTOP_LKASSERT(ValidWayPoint(AltWaypoint));

  if (!ValidWayPoint(AltWaypoint)) return;

  double *altwp_gr	= &WayPointCalc[AltWaypoint].GR;
  double *altwp_arrival	= &WayPointCalc[AltWaypoint].AltArriv[AltArrivMode];

  *altwp_gr = CalculateGlideRatio( WayPointCalc[AltWaypoint].Distance,
	Calculated->NavAltitude - WayPointList[AltWaypoint].Altitude - GetSafetyAltitude(AltWaypoint));

  *altwp_arrival = CalculateWaypointArrivalAltitude(Basic, Calculated, AltWaypoint);

  WayPointCalc[AltWaypoint].VGR = GetVisualGlideRatio(*altwp_arrival, *altwp_gr);
}



// Partially rewritten on december 2010 to make use of unsorted data from Range
// CAREFUL> SortedLandablexxx sized MAXNEAREST!!
// 101218 RangeLandables &C. are UNSORTED, careful
// This function is called with no map painted, and we have plenty of CPU time to spend!
//
// 130120 added VisualGlide support
// #define DEBUG_DONEAREST	1
void DoNearest(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i,k,l;
   int wp_index, inserted;
   double wp_bearing, wp_distance, wp_value;
   double sortedValue[MAXNEAREST+1];
   int *p_sortedIndex;
   int *p_rangeIndex;
   int sortedRangeIndex[MAXNEAREST+1];

   // careful, consider MapSpaceMode could change while running!
   short curmapspace=MapSpaceMode;

   static double LastRunTime=0;

   // Consider replay mode...
   if (  LastRunTime > Basic->Time ) LastRunTime=Basic->Time;
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoNearest) {
	return;
   }

   // LastDoNearest is used to delay recalculations while we are selecting items on the nearest pages.
   // It is not used by VisualGlide
   if (  LastDoNearest > Basic->Time ) LastDoNearest=Basic->Time;
   if ( Basic->Time < (LastDoNearest+PAGINGTIMEOUT)) {
	return;
   }
   LastDoNearest = 0;
   LKForceDoNearest=false;

   if (WayPointList.empty()) return;
   // No need to check airports, cannot be better because Airports are landables
   if ( RangeLandableNumber==0 && RangeTurnpointNumber==0) {
	return;
   }

   // This is done also at startup from InitModeTable
   for (i=0; i<MAXNEAREST;i++) {
	SortedAirportIndex[i]=-1;
	SortedLandableIndex[i]=-1;
        SortedTurnpointIndex[i]=-1;
	sortedRangeIndex[i]=-1;
	sortedValue[i]=99999; 
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
	case MSM_VISUALGLIDE:
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

   // If we are sorting by direction, we must pick initially only wp ahead of us, += 90 degrees max!
   // Otherwise we are going to miss valid waypoints ahead, skipped because there are closer wps
   // at our back..
   bool directsort=(SortedMode[curmapspace]==2);

   // This can be a problem, careful: we use MAXRANGELANDABLE but we may be using MAXRANGETURNPOINT
   // if in the future we want to make them different (currently both 500, so ok).
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

	if (directsort) {
		double brgdiff = WayPointCalc[wp_index].Bearing -  Basic->TrackBearing;
		if (brgdiff < -180.0) {
			brgdiff += 360.0;
		} else {
			if (brgdiff > 180.0) brgdiff -= 360.0;
		}
		if ((brgdiff<-DIRECTIONRANGE)|| (brgdiff>DIRECTIONRANGE)) continue;
	}

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
			if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
				wp_value=WayPointCalc[wp_index].Bearing;
				break;
			}
			wp_value = WayPointCalc[wp_index].Bearing -  Basic->TrackBearing;
			if (wp_value < -180.0) wp_value += 360.0;
			else
				if (wp_value > 180.0) wp_value -= 360.0;
			if (wp_value<0) wp_value*=-1;

			// Sort by distance with better bearing accuracy
			// We should not consider anything above 60, but never mind.
			if (wp_value>=60) {
				wp_value=WayPointCalc[wp_index].Distance+3000000;
			} else {
				if (wp_value>=45)
					wp_value=WayPointCalc[wp_index].Distance+2000000;
				else {
					if (wp_value>=22.5) {
						wp_value=WayPointCalc[wp_index].Distance+1000000;
					} else {
						wp_value=WayPointCalc[wp_index].Distance;
					}
				}
			}
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
					if ( _tcscmp( WayPointList[wp_index].Name, WayPointList[p_sortedIndex[k]].Name) >0) continue;
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

