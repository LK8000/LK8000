/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKCalculations.cpp,v 1.25 2010/12/11 13:56:45 root Exp root $
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

/*
 * Search for the best landing option
 */
#ifdef DEBUG
#define DEBUG_BESTALTERNATE
#endif
#define MAXBEST 10      // max number of reachable landing points searched for, 
			// among a preliminar list of MAXBEST * 2 - CPU HOGGING ALERT!

void SearchBestAlternate(NMEA_INFO *Basic, 
			 DERIVED_INFO *Calculated)
{
  int sortedLandableIndex[MAXBEST];
  double sortedArrivalAltitude[MAXBEST];
  int sortApproxDistance[MAXBEST*2];
  int sortApproxIndex[MAXBEST*2];
  int i, k, l;
  #if UNSORTEDRANGE
  int j;
  #endif
  double arrival_altitude;
  int active_bestalternate_on_entry=-1;
  int bestalternate=-1;

  #ifdef DEBUG_BESTALTERNATE
  TCHAR ventabuffer[200];
  #endif

  if (!WayPointList) return;

  double searchrange=(Calculated->NavAltitude-SAFETYALTITUDEARRIVAL)* GlidePolar::bestld /1000;
  if (searchrange <= 0) 
	searchrange=2; // lock to home airport at once
  if (searchrange > ALTERNATE_MAXRANGE) 
	searchrange=ALTERNATE_MAXRANGE;

  LockTaskData();
  active_bestalternate_on_entry = BestAlternate;

  // Do preliminary fast search
  int scx_aircraft, scy_aircraft;
  LatLon2Flat(Basic->Longitude, Basic->Latitude, &scx_aircraft, &scy_aircraft);

  // Clear search lists
  for (i=0; i<MAXBEST*2; i++) {
	sortApproxIndex[i]= -1;
	sortApproxDistance[i] = 0;
  }
  #if UNSORTEDRANGE
  for (j=0; j<RangeLandableNumber; j++) {
	i=RangeLandableIndex[j];
  #else
  for (i=0; i<(int)NumberOfWayPoints; i++) {

	#ifdef USEISLANDABLE
	if (!WayPointCalc[i].IsLandable) {
	#else
	if (!(((WayPointList[i].Flags & AIRPORT) == AIRPORT) || ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT))) {
	#endif
		continue; // ignore non-landable fields
	}
   #endif

	int approx_distance = CalculateWaypointApproxDistance(scx_aircraft, scy_aircraft, i);

	// Size a reasonable distance, wide enough 
	if ( approx_distance > searchrange ) continue;

	// see if this fits into slot
	for (k=0; k< MAXBEST*2; k++)  {
      
		if (((approx_distance < sortApproxDistance[k]) 
			// wp is closer than this one
			|| (sortApproxIndex[k]== -1))   // or this one isn't filled
			&& (sortApproxIndex[k]!= i))    // and not replacing with same
		{
			// ok, got new biggest, put it into the slot.
			for (l=MAXBEST*2-1; l>k; l--) {
				if (l>0) {
					sortApproxDistance[l] = sortApproxDistance[l-1];
					sortApproxIndex[l] = sortApproxIndex[l-1];
				}
			}

			sortApproxDistance[k] = approx_distance;
			sortApproxIndex[k] = i;
			k=MAXBEST*2;
		}
	} // for k
  } // for all waypoints, or the reduced list by Range

  #ifdef DEBUG_BESTALTERNATE
  FILE *fp;
  if ( (fp=_tfopen(_T("DEBUG.TXT"),_T("a"))) != NULL )  {
	wsprintf(ventabuffer,TEXT("==================\n"));
	fprintf(fp,"%S",ventabuffer);
	wsprintf(ventabuffer,TEXT("[GPSTIME=%02d:%02d:%02d] Altitude=%dm searchrange=%dKm Curr.Best=%d\n\n"),
	     GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second,
	     (int)Calculated->NavAltitude, (int)searchrange, BestAlternate);
	fprintf(fp,"%S",ventabuffer);
	for ( int dbug=0; dbug<MAXBEST*2; dbug++) {
		if ( sortApproxIndex[dbug] <0 ) wsprintf(ventabuffer,_T("%d=empty\n"), dbug);
		else
			wsprintf(ventabuffer,TEXT("%d=%s(%d)\n"), dbug, 
			WayPointList[sortApproxIndex[dbug]].Name, sortApproxDistance[dbug] );
		fprintf(fp,"%S",ventabuffer);
	}
	fclose(fp);
  } else
	DoStatusMessage(_T("CANNOT OPEN DEBUG FILE"));
  #endif


  // Now do detailed search
  for (i=0; i<MAXBEST; i++) {
	sortedLandableIndex[i]= -1;
	sortedArrivalAltitude[i] = 0;
  }

  bool found_reachable_airport = false;

  for (int scan_airports_slot=0; scan_airports_slot<2; scan_airports_slot++) {

	if (found_reachable_airport ) { 
		continue; // don't bother filling the rest of the list
	}

	for (i=0; i<MAXBEST*2; i++) {
		if (sortApproxIndex[i]<0) { // ignore invalid points
			continue;
		}

		if ((scan_airports_slot==0) && 
			#ifdef USEISLANDABLE
			(!WayPointCalc[sortApproxIndex[i]].IsAirport))
			#else
			((WayPointList[sortApproxIndex[i]].Flags & AIRPORT) != AIRPORT))
			#endif
		{
			// we are in the first scan, looking for airports only
			continue;
		}

		arrival_altitude = CalculateWaypointArrivalAltitude(Basic, Calculated, sortApproxIndex[i]);

		WayPointCalc[sortApproxIndex[i]].AltArriv[AltArrivMode] = arrival_altitude; 
		// This is holding the real arrival value

		/* 
	 	 * We can't use degraded polar here, but we can't accept an
		 * arrival 1m over safety.  That is 2m away from being
		 * unreachable! So we higher this value to 100m.
		 */
		arrival_altitude -= ALTERNATE_OVERSAFETY; 

		if (scan_airports_slot==0) {
			if (arrival_altitude<0) {
				// in first scan, this airport is unreachable, so ignore it.
				continue;
			} else 
				// this airport is reachable
				found_reachable_airport = true;
		}

		// see if this fits into slot
		for (k=0; k< MAXBEST; k++) {
			if (((arrival_altitude > sortedArrivalAltitude[k]) 
				// closer than this one
				||(sortedLandableIndex[k]== -1))
				// or this one isn't filled
				&&(sortedLandableIndex[k]!= i))  // and not replacing with same
			{
				double wp_distance, wp_bearing;
				DistanceBearing(Basic->Latitude , Basic->Longitude ,
					WayPointList[sortApproxIndex[i]].Latitude,
					WayPointList[sortApproxIndex[i]].Longitude,
					&wp_distance, &wp_bearing);

				WayPointCalc[sortApproxIndex[i]].Distance = wp_distance;
				WayPointCalc[sortApproxIndex[i]].Bearing = wp_bearing;
            
				bool out_of_range;
				double distance_soarable = FinalGlideThroughTerrain(wp_bearing, Basic, Calculated,
					NULL, NULL, wp_distance, &out_of_range, NULL);
            
				if ((distance_soarable>= wp_distance)||(arrival_altitude<0)) {
					// only put this in the index if it is reachable
					// and doesn't go through terrain, OR, if it is unreachable
					// it doesn't matter if it goes through terrain because
					// pilot has to climb first anyway
              
					// ok, got new biggest, put it into the slot.
					for (l=MAXBEST-1; l>k; l--) {
						if (l>0) {
							sortedArrivalAltitude[l] = sortedArrivalAltitude[l-1];
							sortedLandableIndex[l] = sortedLandableIndex[l-1];
						}
					}

					sortedArrivalAltitude[k] = arrival_altitude;
					sortedLandableIndex[k] = sortApproxIndex[i];
					k=MAXBEST;
				} 
			} // if (((arrival_altitude > sortedArrivalAltitude[k]) ...
		} // for (k=0; k< MAXBEST; k++) {
	} // for i
  }

  #ifdef DEBUG_BESTALTERNATE
  if ( (fp=_tfopen(_T("DEBUG.TXT"),_T("a"))) != NULL )  {
	wsprintf(ventabuffer,_T("\nLandable:\n"));
	fprintf(fp,"%S",ventabuffer);
	for ( int dbug=0; dbug<MAXBEST; dbug++) {
		if ( sortedLandableIndex[dbug] <0 ) {
			wsprintf(ventabuffer,_T("%d=empty\n"), dbug);
			fprintf(fp,"%S",ventabuffer);
		} else {
			wsprintf(ventabuffer,_T("%d=%s (%dm)"), dbug, 
			WayPointList[sortedLandableIndex[dbug]].Name, (int)sortedArrivalAltitude[dbug] );
			fprintf(fp,"%S",ventabuffer);
			if ( sortedLandableIndex[dbug] == HomeWaypoint )
				wsprintf(ventabuffer,_T(":HOME") );
			else
				if ( WayPointCalc[sortedLandableIndex[dbug]].Preferred == TRUE )
					wsprintf(ventabuffer,_T(":PREF") );
				else
					wsprintf(ventabuffer,_T("") );
				fprintf(fp,"%S\n",ventabuffer);
		}
				
	}
	fclose(fp);
  } else
	DoStatusMessage(_T("CANNOT OPEN DEBUG FILE"));
  #endif

  bestalternate=-1;  // reset the good choice
  double safecalc = Calculated->NavAltitude - SAFETYALTITUDEARRIVAL;
  static double grpolar = GlidePolar::bestld *SAFELD_FACTOR; 
  int curwp, curbestairport=-1, curbestoutlanding=-1;
  double curgr=0, curbestgr=INVALID_GR;
  if ( safecalc <= 0 ) {
	/*
	* We're under the absolute safety altitude at MSL, can't be any better elsewhere!
	* Use the closer, hopefully you are landing on your airport
	*/
	#ifdef DEBUG_BESTALTERNATE
	wsprintf(ventabuffer,TEXT("Under safety at MSL, using closer"));
	if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL){;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
	// DoStatusMessage(ventabuffer);
	#endif

  } else
	for (k=0;  k< MAXBEST; k++) {
		curwp = sortedLandableIndex[k];

		if ( !ValidWayPoint(curwp) ) {
			//#ifdef DEBUG_BESTALTERNATE
			//wsprintf(ventabuffer,TEXT("k=%d skip invalid waypoint curwp=%d"), k, curwp );
			//if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL){;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			//#endif
			continue;
			// break;  // that list is unsorted !
		}

		// At the first unsafe landing, stop searching down the list and use the best found or the first
		double grsafe=safecalc - WayPointList[curwp].Altitude;
		if ( grsafe <= 0 ) {
			// We're under the safety altitude for this waypoint. 
			break;  
			//continue; 
		}

		WayPointCalc[curwp].GR = WayPointCalc[curwp].Distance / grsafe; grsafe = WayPointCalc[curwp].GR;
		curgr=WayPointCalc[curwp].GR;

		if ( grsafe > grpolar ) {
			// Over degraded polar GR for this waypoint
			#ifdef DEBUG_BESTALTERNATE
			wsprintf(ventabuffer,TEXT("k=%d %s grsafe=%d > grpolar=%d, skipping. "), 
			 k, WayPointList[curwp].Name, (int)grsafe, (int)grpolar );
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			  {;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			#endif

			continue; 
			// break; 
		}

		// Anything now is within degraded glide ratio, so if our homewaypoint is safely reachable then 
		// attempt to lock it even if we already have a valid best, even if it is preferred and even
		// if it has a better GR

		if ( (HomeWaypoint >= 0) && (curwp == HomeWaypoint) ) {
			#ifdef DEBUG_BESTALTERNATE
			wsprintf(ventabuffer,TEXT("k=%d locking Home"), k);
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			#endif
			bestalternate = curwp;
			break;
		}

		// If we already found a preferred, stop searching for anything but home

		if ( bestalternate >= 0 && WayPointCalc[bestalternate].Preferred) {
			#ifdef DEBUG_BESTALTERNATE
			wsprintf(ventabuffer,TEXT("Ignoring:[k=%d]%s because current best <%s> is a PREF"), k, 
			WayPointList[curwp].Name, WayPointList[bestalternate].Name);
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			#endif
			continue;
		}

		// VENTA5 TODO: extend search on other preferred, choosing the closer one

		// Preferred list has priority, first found is taken (could be smarted)

		if ( WayPointCalc[ curwp ].Preferred ) {
			bestalternate=curwp;
			#ifdef DEBUG_BESTALTERNATE
			wsprintf(ventabuffer,TEXT("k=%d PREFERRED bestalternate=%d,%s"), k,curwp,
			WayPointList[curwp].Name );
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			// DoStatusMessage(ventabuffer);
			#endif
			continue;
		}

		// else we remember the best landable GR found so far. We shall use this in case
		// at the end of the search no home and no preferred were found.

		if ( curgr < curbestgr ) {
			#ifdef USEISLANDABLE
			if ( WayPointCalc[curwp].IsAirport) {
			#else
			if ( ( WayPointList[curwp].Flags & AIRPORT) == AIRPORT) {
			#endif
				curbestairport=curwp;
				curbestgr=curgr; // ONLY FOR AIRPORT! NOT FOR OUTLANDINGS!!
				#ifdef DEBUG_BESTALTERNATE
				wsprintf(ventabuffer,TEXT("[k=%d]<%s> (curgr=%d < curbestgr=%d) set as bestairport"), 
				k, WayPointList[curwp].Name, (int)curgr, (int)curbestgr );
				if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
				{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
				#endif
			} else {
				curbestoutlanding=curwp;
				#ifdef DEBUG_BESTALTERNATE
				wsprintf(ventabuffer,TEXT("[k=%d]<%s> (curgr=%d < curbestgr=%d) set as bestoutlanding"), 
				k, WayPointList[curwp].Name, (int)curgr, (int)curbestgr );
				if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
				{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
				#endif
			}
		}
		continue;
	} // for

  if ( bestalternate <0 ) {

	if ( curbestairport >= 0 ) {
		#ifdef DEBUG_BESTALTERNATE
		wsprintf(ventabuffer,TEXT("--> no bestalternate, choosing airport <%s> with gr=%d"), 
		WayPointList[curbestairport].Name, (int)curbestgr );
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		// DoStatusMessage(ventabuffer);
		#endif
		bestalternate=curbestairport;
	} else {
		if ( curbestoutlanding >= 0 ) {
			#ifdef DEBUG_BESTALTERNATE
			wsprintf(ventabuffer,TEXT("--> no bestalternate, choosing outlanding <%s> with gr=%d"), 
			WayPointList[curbestoutlanding].Name, (int)curbestgr );
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			// DoStatusMessage(ventabuffer);
			#endif
			bestalternate=curbestoutlanding;
		} else {
			/* 
			 * Here we are in troubles, nothing really reachable, but we
			 * might still be lucky to be within the "yellow" glide
			 * path. In any case we select the best arrival altitude place
			 * available, even if it is "red".
			 */
			if ( ValidWayPoint(sortedLandableIndex[0]) ) {
				bestalternate=sortedLandableIndex[0];
				#ifdef DEBUG_BESTALTERNATE
				wsprintf(ventabuffer,TEXT("--> No bestalternate was found, and no good airport or outlanding!\n    Setting first available: <%s>"),
				WayPointList[bestalternate].Name);
				if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
				{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
				// DoStatusMessage(ventabuffer);
				#endif
			} else {
				/*
			 	 * Else the Landable list is EMPTY, although we might be
				 * near to a landable point but the terrain obstacles look
				 * too high (or the DEM resolution is not high enough to
				 * show a passage).
				 * 
				 * Still the old BestAlternate could simply be out of range,
				 * but reachable...  These values have certainly just been
				 * calculated by DoAlternates() , so they are usable.
				 */
				// Attempt to use the old best, but check there's one.. it
				// might be empty for the first run
				if ( ValidWayPoint(active_bestalternate_on_entry) ) {
					bestalternate=active_bestalternate_on_entry;
					if ( WayPointCalc[bestalternate].AltArriv[AltArrivMode] <0 ) {
						#ifdef DEBUG_BESTALTERNATE
						wsprintf(ventabuffer,TEXT("Landable list is empty and old bestalternate <%s> has Arrival=%d <0, NO good."),
						WayPointList[bestalternate].Name, (int) WayPointCalc[bestalternate].AltArriv[AltArrivMode]);
						if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
						{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
						#endif
						// Pick up the closest!
						if ( ValidWayPoint( sortApproxIndex[0]) ) {
							bestalternate=sortApproxIndex[0];
							#ifdef DEBUG_BESTALTERNATE
							wsprintf(ventabuffer,
							TEXT(".. using the closer point found: <%s> distance=~%d Km, you need to climb!"),
							WayPointList[bestalternate].Name, sortApproxDistance[0]);
							if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
							{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
							#endif
						} else {
							// CRITIC POINT
							// Otherwise .. 
							// nothing, either keep the old best or set it empty
							// Put here "PULL-UP! PULL-UP! Boeing cockpit voice sound and possibly shake the stick.
							#ifdef DEBUG_BESTALTERNATE
							wsprintf(ventabuffer,TEXT("Out of ideas..good luck"));
							if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
							{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
							#endif
						}
					} else {
						// MapWindow2 is checking for reachables separately,
						// se let's see if this closest is reachable
						if ( ValidWayPoint( sortApproxIndex[0] )) {
							if ( WayPointList[sortApproxIndex[0]].Reachable ) {
								bestalternate = sortApproxIndex[0];
								#ifdef DEBUG_BESTALTERNATE
								wsprintf(ventabuffer,
								TEXT("Closer point found: <%s> distance=~%d Km, Reachable with arrival at %d!"),
								WayPointList[bestalternate].Name, sortApproxDistance[0], 
								(int) WayPointList[bestalternate].AltArivalAGL);
								if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
								{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
								#endif
							} else {
								#ifdef DEBUG_BESTALTERNATE
								wsprintf(ventabuffer,
								TEXT("Closer point found: <%s> distance=~%d Km, UNReachable"),
								WayPointList[bestalternate].Name, sortApproxDistance[0]);
								if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
								{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
								#endif
							}
						} else {
							#ifdef DEBUG_BESTALTERNATE
							wsprintf(ventabuffer, _T("Landable list is empty, no Closer Approx, but old best %s is still reachable (arrival=%d)"),
							WayPointList[bestalternate].Name, (int)WayPointCalc[bestalternate].AltArriv[AltArrivMode]);
							if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
							{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
							#endif
						}
					}
				} else {
					// CRITIC POINT
					#ifdef DEBUG_BESTALTERNATE
					wsprintf(ventabuffer,TEXT("Landable list is empty, and NO valid old bestalternate"));
					if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
					{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
					#endif
				}
			}
			/*
			 * Don't make any sound at low altitudes, pilot is either taking off
			 * or landing, or searching for an immediate outlanding.  Do not disturb.
			 * If safetyaltitude is 300m, then below 500m be quiet.
			 * If there was no active alternate on entry, and nothing was found, then we
			 * better be quiet since probably the user had already been alerted previously
			 * and now he is low..
			 */
			if ( bestalternate >0 && ((safecalc-WayPointList[bestalternate].Altitude) >ALTERNATE_QUIETMARGIN)) {
				if ( WayPointList[bestalternate].AltArivalAGL <100 )
					AlertBestAlternate(2);
			}
		}
	}
  }

  /* 
   * If still invalid, it should mean we are just taking off
   * in this case no problems, we set the very first bestalternate of the day as the home
   * trusting the user to be home really!
   */

  if ( bestalternate < 0 ) {
	if ( HomeWaypoint >= 0 ) {
		#ifdef DEBUG_BESTALTERNATE
		wsprintf(ventabuffer,TEXT("BESTALTERNATE HOME (%s)"), WayPointList[HomeWaypoint].Name );
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		//DoStatusMessage(ventabuffer);
		#endif
		bestalternate=HomeWaypoint;
	} 
  } else {
	// If still invalid, i.e. not -1, then there's a big problem
	if ( !ValidWayPoint(bestalternate) ) {
		//if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_RED"));
		AlertBestAlternate(2);
		#ifdef DEBUG_BESTALTERNATE
		wsprintf(ventabuffer,TEXT("WARNING ERROR INVALID BEST=%d"),bestalternate);
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		#endif
		DoStatusMessage(_T("ERR-051 invalid bestalternate"));
		FailStore(_T("------ ERR-051 invalid bestalternate <%d>%s"),bestalternate,NEWLINE); // 091122
		// todo: immediate disable function  
	} 
  }

  if (active_bestalternate_on_entry != bestalternate) {
	BestAlternate = bestalternate;
	if ( bestalternate >0 && ((safecalc-WayPointList[bestalternate].Altitude) >ALTERNATE_QUIETMARGIN))
		AlertBestAlternate(1);
  }

  UnlockTaskData();
} // end of search for the holy grail



/*
 * Do not disturb too much. Play alert sound only once every x minutes, not more.
 */
void AlertBestAlternate(short soundmode) {
#ifdef DEBUG_BESTALTERNATE
  TCHAR ventabuffer[100];
  FILE *fp;
#endif
  TCHAR mbuf[150];

  static double LastAlertTime=0;

  if (!BestWarning || !CALCULATED_INFO.Flying) return; // 091125

  if ( GPS_INFO.Time > LastAlertTime + 120.0 ) { 
	if (EnableSoundModes) {
		LastAlertTime = GPS_INFO.Time; 
		switch (soundmode) {
			case 0:
				break;
			case 1:
				#ifndef DISABLEAUDIO
				PlayResource(TEXT("IDR_WAV_GREEN"));
				#endif
				_stprintf(mbuf,_T("BestAlternate: %s  @%.0f%s"), WayPointList[BestAlternate].Name,
				DISTANCEMODIFY*WayPointCalc[BestAlternate].Distance,
				(Units::GetDistanceName()) );
				Message::Lock(); // 091211
				Message::AddMessage(5000, 3, mbuf);
				Message::Unlock();
				break;
			case 2: 
				#ifndef DISABLEAUDIO
				PlayResource(TEXT("IDR_WAV_RED"));
				#endif
				wsprintf(mbuf,_T("BestAlternate: WARNING, NO LANDINGS"));
				// Do NOT disturb the pilot for 5 minutes with useless further messages
				LastAlertTime += 180.0;
				Message::Lock(); // 091211
				Message::AddMessage(10000, 3, mbuf);
				Message::Unlock();
	
				break;
			case 11:
				#ifndef DISABLEAUDIO
				PlayResource(TEXT("IDR_WAV_GREEN"));
				PlayResource(TEXT("IDR_WAV_GREEN"));
				#endif
				break;
			default:
				break;
		}
		#ifdef DEBUG_BESTALTERNATE
		wsprintf(ventabuffer,TEXT("(PLAYED ALERT SOUND, soundmode=%d)"), soundmode);
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		#endif
	} 
  } else {
	#ifdef DEBUG_BESTALTERNATE
	wsprintf(ventabuffer,TEXT("(QUIET, NO PLAY ALERT SOUND, soundmode=%d)"), soundmode);
	if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
	{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
	#endif
  }
}

// CAREFUL> SortedLandablexxx sized MAXNEAREST!!
void DoNearest(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i,k,l;
   int wp_index, inserted;
   double wp_bearing, wp_distance, wp_value;
   double sortedValue[MAXNEAREST+1];
   int *p_sortedIndex;
   int dstSortedIndex[MAXNEAREST+1];

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

   for (i=0; i<MAXNEAREST;i++) {
	SortedLandableIndex[i]=-1;
	SortedAirportIndex[i]=-1;
	sortedValue[i]=99999; 
	dstSortedIndex[i]=-1;
   }
  
   // Sort by distance in SortedLandableIndex
   // We sample twice as much as needed values. 
   // MAXNEAREST should really be the real number of displayed values and not a constant
   for (i=0, inserted=0; i<MAXNEAREST*2; i++) { // was 1.5 FIX 090925

	switch (curmapspace) {
		case MSM_LANDABLE:
			p_sortedIndex=SortedLandableIndex;
			wp_index=RangeLandableIndex[i];
			break;
		case MSM_AIRPORTS:
			p_sortedIndex=SortedAirportIndex;
			wp_index=RangeAirportIndex[i];
			break;
		default:
			p_sortedIndex=SortedLandableIndex;
			wp_index=RangeLandableIndex[i];
			break;
	}


	if (wp_index <0) {
		// we should be at the end and could break out, however we do a failsafe forward
		continue;
	}

	DistanceBearing(Basic->Latitude , Basic->Longitude , WayPointList[wp_index].Latitude,
		WayPointList[wp_index].Longitude, &wp_distance, &wp_bearing);

	// DISTANCE AND BEARING CALCULATED FOR WAYPOINT
	WayPointCalc[wp_index].Distance = wp_distance;
	WayPointCalc[wp_index].Bearing  = wp_bearing;

	if (SortedMode[curmapspace] == 0) goto AlfaSortNearest;


	// we calculate DoNearestAlternate only if really needed here, since we do it for MAXNEAREST*2
	// which is TWICE what we really need. 
	switch (SortedMode[curmapspace]) {
		// wp name, should not happen
		case 0:
		case 1:
			wp_value=wp_distance;
			// requires later DoNearestAlternate!
			break;
		case 2:
			if (DisplayMode == dmCircling) {
				wp_value=wp_bearing; // 100328
				break;
			}
			wp_value = wp_bearing -  GPS_INFO.TrackBearing;
			if (wp_value < -180.0) wp_value += 360.0;
			else
				if (wp_value > 180.0) wp_value -= 360.0;
			if (wp_value<0) wp_value*=-1;
			// requires later DoNearestAlternate!
			break;
		case 3:
			DoNearestAlternate(Basic,Calculated,wp_index); 
			wp_value=WayPointCalc[wp_index].GR;
			break;
		case 4:
			DoNearestAlternate(Basic,Calculated,wp_index); 
			wp_value=WayPointCalc[wp_index].AltArriv[AltArrivMode];
			// we sort lowest is better, so in this case we invert sign since higher is better
			wp_value*=-1;
			break;
		default:
			wp_value=wp_distance;
			break;
	}

	// insert sorted value for Landfields and Airports
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

			// Distance and eventually name sorted still need DoNearest to be done
			if (SortedMode[curmapspace]<3) DoNearestAlternate(Basic,Calculated,wp_index);

			k=MAXNEAREST*2; // should not be needed

			// update number of items inserted so far 
			inserted++;
			break;
		}
	} // for k

	continue;

AlfaSortNearest:

	// First we get the distance sorting
	wp_value=wp_distance;
	for (k=0; k< MAXNEAREST; k++)  {
		if ( ( (wp_value < sortedValue[k]) || (dstSortedIndex[k]== -1) )
			&& (dstSortedIndex[k] != wp_index) )
		{
			// ok, got new closer waypoint, put it into the slot.
			for (l=MAXNEAREST-1; l>k; l--) {
				if (l>0) {
					sortedValue[l] = sortedValue[l-1];
					dstSortedIndex[l] = dstSortedIndex[l-1];
				}
			}
			sortedValue[k] = wp_value;
			dstSortedIndex[k] = wp_index;

			k=MAXNEAREST*2; // should not be needed

			// update number of items inserted so far
			inserted++;
			break;
		}
	} // for k


   } // for i

   if (SortedMode[curmapspace] == 0 ) {

	for (i=0; i<MAXNEAREST; i++) {
		wp_index=dstSortedIndex[i];
		if (wp_index<0) {
			// There are less than 25 or whatever items available for sorting, no problems
			// DoStatusMessage(_T("DBG-198 sorting nearest"));
			break; // should be impossible
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
				// Distance and eventually name sorted still need DoNearest to be done
				if (SortedMode[curmapspace]<3) DoNearestAlternate(Basic,Calculated,wp_index);
				k=MAXNEAREST*2; // should not be needed
				// update number of items inserted so far 
				inserted++;
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

// REDUCE WAYPOINTLIST TO THOSE IN RANGE, ROUGHLY SORTED BY DISTANCE
// Keep an updated list of in-range landable waypoints.
// Attention: since this list is sorted only periodically, calling functions must sort data themselves.
// DISTANCE IS ASSUMED ON A FLAT SURFACE, ONLY APPROXIMATED!

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
	#ifdef USEISLANDABLE
	if (
		( (TpFilter==(TpFilter_t)TfNoLandables) && (!WayPointCalc[i].IsLandable ) ) ||
		( (TpFilter==(TpFilter_t)TfAll) ) ||
		( (TpFilter==(TpFilter_t)TfTps) && ((WayPointList[i].Flags & TURNPOINT) == TURNPOINT) ) 
	 ) {
	#else
	if (! (((WayPointList[i].Flags & AIRPORT) == AIRPORT) || ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT))) {
	#endif
		#if UNSORTEDRANGE
		if (kt+1<MAXRANGETURNPOINT) { // never mind if we use maxrange-2
			RangeTurnpointIndex[kt++]=i;
			RangeTurnpointNumber++;
		}
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
	#ifdef USEISLANDABLE
	if (!WayPointCalc[i].IsLandable )
	#else
	if (!(((WayPointList[i].Flags & AIRPORT) == AIRPORT) || ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT))) 
	#endif
		continue; 

	#if UNSORTEDRANGE
	if (kl+1<MAXRANGELANDABLE) { // never mind if we use maxrange-2
		RangeLandableIndex[kl++]=i;
		RangeLandableNumber++;
	}
	#else
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
	#ifdef USEISLANDABLE
	if ( WayPointCalc[i].IsAirport )
	#else
	if ( ((WayPointList[i].Flags & AIRPORT) == AIRPORT) )
	#endif
	{
		#if UNSORTEDRANGE
		if (ka+1<MAXRANGELANDABLE) { // never mind if we use maxrange-2
			RangeAirportIndex[ka++]=i;
			RangeAirportNumber++;
		}
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

   // Then we insert the alternates, if existing
   if (OnBestAlternate) InsertCommonList(BestAlternate);
   if (OnAlternate1) InsertCommonList(Alternate1);
   if (OnAlternate2) InsertCommonList(Alternate2);

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
	#ifdef NEWTASKWP
	if ( !ValidNotResWayPoint(RecentIndex[i])) {
		StartupStore(_T("---- SaveHistory: invalid wp, maybe file has changed. Aborting.%s"),NEWLINE);
		break;
	}
	if ( WayPointList[RecentIndex[i]].FileNum == -1) continue; // 100219
	#endif
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
    TCHAR buffer[100];

    wsprintf(buffer,_T(". Insert WP=%d into recent waypoints%s"),newwp,NEWLINE);
    StartupStore(buffer);

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


// Nearest turnpoints
// Running every n seconds ONLY when the nearest page is active and we are not drawing map.
// This is measured to take 10ms cpu on a HP314
// 120kmh = 33.3m/s     5s = 167m  consider altitude loss, assuming -5m/s * 5 = -25m QNH
// So we calculate new values every 5 seconds.
void DoNearestTurnpoint(NMEA_INFO *Basic, DERIVED_INFO *Calculated)
{
   int i,k,l;
   int wp_index, inserted;
   double wp_bearing, wp_distance, wp_value=-1;

   double sortedValue[MAXNEARTURNPOINT+1];
   int *p_sortedIndex;
   int dstSortedIndex[MAXNEARTURNPOINT+1];

   static double LastRunTime=0;

/*
   static bool DoInit=true;
   if (DoInit) {
	DoInit=false;
   }
*/

   if (  LastRunTime > Basic->Time ) LastRunTime=Basic->Time;
   if (  (Basic->Time < (LastRunTime+NEARESTUPDATETIME)) && !LKForceDoNearestTurnpoint) {
	return;
   }
   if (  LastDoNearestTp > Basic->Time ) LastDoNearestTp=Basic->Time;
   if ( Basic->Time < (LastDoNearestTp+PAGINGTIMEOUT)) {
	return;
   }
   LastDoNearestTp = 0;

   LKForceDoNearestTurnpoint=false;

   if (!WayPointList) return;
   if ( RangeTurnpointNumber==0) {
	return;
   }

   // This is done also at startup from InitModeTable
   for (i=0; i<MAXNEARTURNPOINT;i++) {
	SortedTurnpointIndex[i]=-1;
	sortedValue[i]=99999; 
	dstSortedIndex[i]=-1;
   }
  
   // We sample twice as much as needed values. 
   for (i=0, inserted=0; i<MAXNEARTURNPOINT*2; i++) {

	p_sortedIndex=SortedTurnpointIndex;
	wp_index=RangeTurnpointIndex[i];

	if (wp_index <0) {
		continue;
	}

	DistanceBearing(Basic->Latitude , Basic->Longitude , WayPointList[wp_index].Latitude,
		WayPointList[wp_index].Longitude, &wp_distance, &wp_bearing);


	WayPointCalc[wp_index].Distance = wp_distance;
	WayPointCalc[wp_index].Bearing  = wp_bearing;

	if (SortedMode[MSM_NEARTPS] == 0 ) goto AlfaSort; 

	switch (SortedMode[MSM_NEARTPS]) {
		case 0:
		case 1:
			wp_value=wp_distance;
			break;
		case 2:
			if (DisplayMode == dmCircling) {
				wp_value=wp_bearing; // 100328
				break;
			}
			wp_value = wp_bearing -  GPS_INFO.TrackBearing;
			if (wp_value < -180.0) wp_value += 360.0;
			else
				if (wp_value > 180.0) wp_value -= 360.0;
			if (wp_value<0) wp_value*=-1;
			break;
		case 3:
			DoNearestAlternate(Basic,Calculated,wp_index); 
			wp_value=WayPointCalc[wp_index].GR;
			break;
		case 4:
			DoNearestAlternate(Basic,Calculated,wp_index); 
			wp_value=WayPointCalc[wp_index].AltArriv[AltArrivMode];
			wp_value*=-1;
			break;
		default:
			wp_value=wp_distance;
			break;
	}

	for (k=0; k< MAXNEARTURNPOINT; k++)  {
		if ( ((wp_value < sortedValue[k])	
		|| (p_sortedIndex[k]== -1))
		&& (p_sortedIndex[k] != wp_index) )
		{
			for (l=MAXNEARTURNPOINT-1; l>k; l--) {
				if (l>0) {
					sortedValue[l] = sortedValue[l-1];
					p_sortedIndex[l] = p_sortedIndex[l-1];
				}
			}
			sortedValue[k] = wp_value;
			p_sortedIndex[k] = wp_index;

			if (SortedMode[MapSpaceMode]<3) DoNearestAlternate(Basic,Calculated,wp_index);

			k=MAXNEARTURNPOINT*2; // should not be needed

			inserted++;
			break;
		}
	} // for k
	continue;


AlfaSort:
	wp_value=wp_distance;
	for (k=0; k< MAXNEARTURNPOINT; k++)  {
		if ( ((wp_value < sortedValue[k])	
		|| (dstSortedIndex[k]== -1))
		&& (dstSortedIndex[k] != wp_index) )
		{
			for (l=MAXNEARTURNPOINT-1; l>k; l--) {
				if (l>0) {
					sortedValue[l] = sortedValue[l-1];
					dstSortedIndex[l] = dstSortedIndex[l-1];
				}
			}
			sortedValue[k] = wp_value;
			dstSortedIndex[k] = wp_index;

			k=MAXNEARTURNPOINT*2; // should not be needed

			inserted++;
			break;
		}
	} // for k


   } // for i

   if (SortedMode[MSM_NEARTPS] == 0 ) {

   	p_sortedIndex=SortedTurnpointIndex;

	for (i=0; i< MAXNEARTURNPOINT; i++) {

		wp_index=dstSortedIndex[i];
		if (wp_index<0) {
			break;  // should be impossible
		}

		for (k=0; k< MAXNEARTURNPOINT; k++)  {
			if ( (p_sortedIndex[k] < 0 ) || (p_sortedIndex[k] != wp_index) ) {
				if ( p_sortedIndex[k]>=0 ) 
					if ( wcscmp( WayPointList[wp_index].Name, WayPointList[p_sortedIndex[k]].Name) >0) continue;
				for (l=MAXNEARTURNPOINT-1; l>k; l--) {
					if (l>0) {
						p_sortedIndex[l] = p_sortedIndex[l-1];
					}
				}
				p_sortedIndex[k] = wp_index;
				if (SortedMode[MapSpaceMode]<3) DoNearestAlternate(Basic,Calculated,wp_index);
				k=MAXNEARTURNPOINT*2; // should not be needed
			}
		} // for k
	} // for i
   }

   NearestTurnpointDataReady=true;
   LastRunTime=Basic->Time;
   SortedTurnpointNumber= inserted>MAXNEARTURNPOINT ? MAXNEARTURNPOINT : inserted;

   return;
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

// Warning: this is called from mapwindow Draw task , not from calculations!!
// this is calling CheckLandableReachableTerrainNew
void MapWindow::CalculateWaypointReachableNew(void)
{
  unsigned int i;
  #if UNSORTEDRANGE
  int j;
  #endif
  double waypointDistance, waypointBearing,altitudeRequired,altitudeDifference;
  double dtmp;

  // LandableReachable is used only by the thermal bar indicator in MapWindow2, after here
  // apparently, is used to tell you if you are below final glide but in range for a landable wp
  LandableReachable = false;

  if (!WayPointList) return;

  LockTaskData();

  #if UNSORTEDRANGE
  for(j=0;j<RangeLandableNumber;j++)
  {
	i=RangeLandableIndex[j];
  #else
  for(i=0;i<NumberOfWayPoints;i++)
  {
  #endif
    #ifdef USEISLANDABLE
    if ( ( ((WayPointCalc[i].AltArriv >=0)||(WayPointList[i].Visible)) && (WayPointCalc[i].IsLandable)) // 100307
    #else
    if ((WayPointList[i].Visible && ( ((WayPointList[i].Flags & AIRPORT) == AIRPORT) || ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT) ))
    #endif
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

    } // for all landables or in task
  } // for all waypoints

  if (!LandableReachable) // 091203
  #if UNSORTEDRANGE
  for(j=0;j<RangeLandableNumber;j++) {
	i = RangeLandableIndex[j];
  #else
  for(i=0;i<NumberOfWayPoints;i++) {
  #endif
    if(!WayPointList[i].Visible && WayPointList[i].FarVisible)  {
	// visible but only at a distance (limit this to 100km radius)

	#ifdef USEISLANDABLE
	if(  WayPointCalc[i].IsLandable ) {
	#else
	if(  ((WayPointList[i].Flags & AIRPORT) == AIRPORT) || ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT) ) {
	#endif

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
  h =  max(0, RasterTerrain::GetTerrainHeight(lat, lon)); 
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
    h =  max(0,RasterTerrain::GetTerrainHeight(lat, lon)); 
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
        f = max(0,min(1,(-last_dh)/(dh-last_dh)));
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
			if (DisplayMode == dmCircling) {
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
	#if NOSIM
	if (SIMMODE) {
		// for those who test the sim mode in the evening..
		if (hours>21) hours-=12;
		if (hours<7) hours+=12;
	}
	#else
	#ifdef _SIM_
	// for those who test the sim mode in the evening..
	if (hours>21) hours-=12;
	if (hours<7) hours+=12;
	#endif
	#endif
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

