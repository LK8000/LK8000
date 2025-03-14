/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: BestAlternate.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "LKProcess.h"
#include "McReady.h"
#include "Calculations2.h"
#include "Message.h"
#include "Sound/Sound.h"
#include "NavFunctions.h"
#include "Radio.h"

extern int CalculateWaypointApproxDistance(int scx_aircraft, int scy_aircraft, int i);

extern void InsertCommonList(int newwp);

// #define LOGBEST 1 // DEBUG inside Runtime log. 

#ifdef LOGBEST
#define STS StartupStore(_T
#endif

/*
 * Search for the best landing option
 */

// #define DEBUG_BESTALTERNATE // DO NOT USE ON LINUX, %S PROBLEM

#define MAXBEST 10      // max number of reachable landing points searched for, 
			// among a preliminar list of MAXBEST * 2 - CPU HOGGING ALERT!

/**
 * @return true if BestAlternate change
 */
bool SearchBestAlternate(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {

  ScopeLock Lock(CritSec_TaskData);

  if (WayPointList.empty()) {
	return false;
  }

  int sortedLandableIndex[MAXBEST];
  double sortedArrivalAltitude[MAXBEST];
  int sortApproxDistance[MAXBEST*2];
  int sortApproxIndex[MAXBEST*2];
  int i, k, l;
  int j;
  double arrival_altitude;
  int active_bestalternate_on_entry=-1;
  int bestalternate=-1;

  #ifdef DEBUG_BESTALTERNATE
  TCHAR ventabuffer[200];
  #endif
  if( DisableBestAlternate ) {
      if ( HomeWaypoint >= 0 )
          BestAlternate=HomeWaypoint;
      else
          BestAlternate = 0; // RESWP_TAKEOFF

      return false;
  }

  // We are not considering total energy here, forbidden for safety reasons
  // V5: double searchrange=(Calculated->NavAltitude-(SAFETYALTITUDEARRIVAL/10))* GlidePolar::bestld /1000;
  // V6: we enlarge preliminar search range because of possible tail wind for some destinations
  double searchrange=(Calculated->NavAltitude)* 1.2 * GlidePolar::bestld /1000;
  if (searchrange <= 0) 
	searchrange=2; // lock to home airport at once
  if (searchrange > ALTERNATE_MAXRANGE) 
	searchrange=ALTERNATE_MAXRANGE;

  active_bestalternate_on_entry = BestAlternate;

  // Do preliminary fast search
  int scx_aircraft, scy_aircraft;
  LatLon2Flat(Basic->Longitude, Basic->Latitude, &scx_aircraft, &scy_aircraft);

  // Clear search lists
  for (i=0; i<MAXBEST*2; i++) {
	sortApproxIndex[i]= -1;
	sortApproxDistance[i] = 0;
  }
  #ifdef LOGBEST
  STS("\n\nNEW SEARCH\n\n"));
  #endif
  for (j=0; j<RangeLandableNumber; j++) {
	i=RangeLandableIndex[j];

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
                        #ifdef LOGBEST
                        STS("INSERT FROM RANGELANDABLE: [%d] %d %s\n"),k,sortApproxIndex[k],WayPointList[sortApproxIndex[k]].Name);
                        #endif
			k=MAXBEST*2;
		}
	} // for k
  #ifdef LOGBEST
  STS("------------\n"));
  #endif
  } // for all waypoints, or the reduced list by Range

  #ifdef DEBUG_BESTALTERNATE
  FILE *fp;
  if ( (fp=_tfopen(_T("DEBUG.TXT"),_T("a"))) != NULL )  {
	_stprintf(ventabuffer,TEXT("==================\n"));
	fprintf(fp,"%S",ventabuffer);
	_stprintf(ventabuffer,TEXT("[GPSTIME=%02d:%02d:%02d] Altitude=%dm searchrange=%dKm Curr.Best=%d\n\n"),
	     GPS_INFO.Hour, GPS_INFO.Minute, GPS_INFO.Second,
	     (int)Calculated->NavAltitude, (int)searchrange, BestAlternate);
	fprintf(fp,"%S",ventabuffer);
	for ( int dbug=0; dbug<MAXBEST*2; dbug++) {
		if ( sortApproxIndex[dbug] <0 ) _stprintf(ventabuffer,_T("%d=empty\n"), dbug);
		else
			_stprintf(ventabuffer,TEXT("%d=%s(%d)\n"), dbug, 
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


  for (int scan_airports_slot=0; scan_airports_slot<2; scan_airports_slot++) {
  #ifdef LOGBEST
  STS("SCAN SLOT= %d\n"),scan_airports_slot);
  #endif
	for (i=0; i<MAXBEST*2; i++) {
		if (sortApproxIndex[i]<0) { // ignore invalid points
			continue;
		}
                #ifdef LOGBEST
                STS("Examine: [%d] %d %s\n"),i,sortApproxIndex[i],WayPointList[sortApproxIndex[i]].Name);
                #endif

		if ((scan_airports_slot==0) && 
			(!WayPointCalc[sortApproxIndex[i]].IsAirport))
		{
			// we are in the first scan, looking for airports only
                        // In second scan we accept again airports and also outlandings
                        #ifdef LOGBEST
                        STS("... scanning only for airports, this one is not an airport\n"));
                        #endif
			continue;
		}

		arrival_altitude = CalculateWaypointArrivalAltitude(Basic, Calculated, sortApproxIndex[i]);
                #ifdef LOGBEST
                STS("...... arrival altitude is %f\n"),arrival_altitude);
                #endif

		WayPointCalc[sortApproxIndex[i]].AltArriv[AltArrivMode] = arrival_altitude; 
		// This is holding the real arrival value

		if (scan_airports_slot==0) {
			if (arrival_altitude<0) {
                                #ifdef LOGBEST
                                STS("... scanning only for airports, and this is unreachable (%f), continue\n"),arrival_altitude);
                                #endif
				// in first scan, this airport is unreachable, so ignore it.
				continue;
			} 
		}

		// see if this fits into slot
		for (k=0; k< MAXBEST; k++) {
			if (((arrival_altitude > sortedArrivalAltitude[k]) 
				// closer than this one
				||(sortedLandableIndex[k]== -1))
				// or this one isn't filled
				&&(sortedLandableIndex[k]!= sortApproxIndex[i]))  // and not replacing with same
			{
				double wp_distance, wp_bearing;
				DistanceBearing(Basic->Latitude , Basic->Longitude ,
					WayPointList[sortApproxIndex[i]].Latitude,
					WayPointList[sortApproxIndex[i]].Longitude,
					&wp_distance, &wp_bearing);

				WayPointCalc[sortApproxIndex[i]].Distance = wp_distance;
				WayPointCalc[sortApproxIndex[i]].Bearing = wp_bearing;
            
				bool out_of_range;
				double distance_soarable = FinalGlideThroughTerrain(wp_bearing, Basic->Latitude,
					Basic->Longitude, Calculated->NavAltitude, Calculated,
					NULL, NULL, wp_distance, &out_of_range, NULL);
            
				// V5 bug: if ((distance_soarable>= wp_distance)||(arrival_altitude<0)) {
				if ((distance_soarable>= wp_distance)) {
					// only put this in the index if it is reachable
					// and doesn't go through terrain, OR, if it is unreachable
					// it doesn't matter if it goes through terrain because
					// pilot has to climb first anyway
                                        // 160407: THE ^^ ABOVE WAS A BUG. Because we were inserting a waypoint
                                        // if it had no obstacles, OR if it had an obstacle but with a NEGATIVE arrival
                                        // altitude. We were thus including negative arrival altitudes wp.
              
					// ok, got new biggest, put it into the slot.
					for (l=MAXBEST-1; l>k; l--) {
						if (l>0) {
							sortedArrivalAltitude[l] = sortedArrivalAltitude[l-1];
							sortedLandableIndex[l] = sortedLandableIndex[l-1];
						}
					}

					sortedArrivalAltitude[k] = arrival_altitude;
					sortedLandableIndex[k] = sortApproxIndex[i];
                                        #ifdef LOGBEST
                                        STS("INSERT INTO LANDABLES: [%d] %d %s\n"),k,sortedLandableIndex[k],WayPointList[sortedLandableIndex[k]].Name);
                                        #endif
					k=MAXBEST;
				} 
			} // if (((arrival_altitude > sortedArrivalAltitude[k]) ...
		} // for (k=0; k< MAXBEST; k++) {
	} // for i
  }
  #ifdef LOGBEST
  STS("------------\n"));
  #endif

  #ifdef DEBUG_BESTALTERNATE
  if ( (fp=_tfopen(_T("DEBUG.TXT"),_T("a"))) != NULL )  {
	_stprintf(ventabuffer,_T("\nLandable:\n"));
	fprintf(fp,"%S",ventabuffer);
	for ( int dbug=0; dbug<MAXBEST; dbug++) {
		if ( sortedLandableIndex[dbug] <0 ) {
			_stprintf(ventabuffer,_T("%d=empty\n"), dbug);
			fprintf(fp,"%S",ventabuffer);
		} else {
			_stprintf(ventabuffer,_T("%d=%s (%dm)"), dbug, 
			WayPointList[sortedLandableIndex[dbug]].Name, (int)sortedArrivalAltitude[dbug] );
			fprintf(fp,"%S",ventabuffer);
			if ( sortedLandableIndex[dbug] == HomeWaypoint )
				_stprintf(ventabuffer,_T(":HOME") );
			else
				if ( WayPointCalc[sortedLandableIndex[dbug]].Preferred == TRUE )
					_stprintf(ventabuffer,_T(":PREF") );
				else
					_stprintf(ventabuffer,_T("") );
				fprintf(fp,"%S\n",ventabuffer);
		}
				
	}
	fclose(fp);
  } else
	DoStatusMessage(_T("CANNOT OPEN DEBUG FILE"));
  #endif

  bestalternate=-1;  // reset the good choice
  double safecalc = Calculated->NavAltitude - (SAFETYALTITUDEARRIVAL/10);
  double grpolar = GlidePolar::bestld *SAFELD_FACTOR; // use updated polar bestld for current ballast
  int curwp, curbestairport=-1, curbestoutlanding=-1;
  double curgr=0, curbestgr=INVALID_GR, curbestgr_outlanding=INVALID_GR, curbestgr_preferred=INVALID_GR;;
  if ( safecalc <= 0 ) {
	/*
	* We're under the absolute safety altitude at MSL, can't be any better elsewhere!
	* Use the closer, hopefully you are landing on your airport
	*/
	#ifdef DEBUG_BESTALTERNATE
	_stprintf(ventabuffer,TEXT("Under safety at MSL, using closer"));
	if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL){;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
	// DoStatusMessage(ventabuffer);
	#endif

  } else
	for (k=0;  k< MAXBEST; k++) {
		curwp = sortedLandableIndex[k];

		if ( !ValidWayPoint(curwp) ) {
			//#ifdef DEBUG_BESTALTERNATE
			//_stprintf(ventabuffer,TEXT("k=%d skip invalid waypoint curwp=%d"), k, curwp );
			//if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL){;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			//#endif
			continue;
			// break;  // that list is unsorted !
		}

		// grsafe is the altitude we can spend in a glide
		double grsafe=safecalc - WayPointList[curwp].Altitude;
		if ( grsafe <= 0 ) {
			// We're under the safety altitude for this waypoint. 
			//break;   BUG
			continue; 
		}

		WayPointCalc[curwp].GR = WayPointCalc[curwp].Distance / grsafe; grsafe = WayPointCalc[curwp].GR;
		curgr=WayPointCalc[curwp].GR;

		if ( grsafe > grpolar ) {
			// Over degraded polar GR for this waypoint
			#ifdef DEBUG_BESTALTERNATE
			_stprintf(ventabuffer,TEXT("k=%d %s grsafe=%d > grpolar=%d, skipping. "), 
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
			_stprintf(ventabuffer,TEXT("k=%d locking HOME!"), k);
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			#endif
			bestalternate = curwp;
			break;
		}

		// This preferred landable is reachable..
		if ( WayPointCalc[ curwp ].Preferred ) {
			// if we already have home selected, forget it
			if ( bestalternate >=NUMRESWP && bestalternate==HomeWaypoint ) continue;
			// if we have another better preferred, we dont use this one
			if (curgr >= curbestgr_preferred) continue;

			// else this preferred is elected best!
			bestalternate=curwp;
			curbestgr_preferred=curgr;
			
			#ifdef DEBUG_BESTALTERNATE
			_stprintf(ventabuffer,TEXT("k=%d PREFERRED bestalternate=%d,%s"), k,curwp,
			WayPointList[curwp].Name );
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			// DoStatusMessage(ventabuffer);
			#endif
			continue;
		}

		// If we already found a preferred, stop searching for anything but home
		if ( bestalternate >= 0 && WayPointCalc[bestalternate].Preferred) {
			#ifdef DEBUG_BESTALTERNATE
			_stprintf(ventabuffer,TEXT("Ignoring:[k=%d]%s because current best <%s> is a PREF"), k, 
			WayPointList[curwp].Name, WayPointList[bestalternate].Name);
			if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
			{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
			#endif
			continue;
		}

		// else we remember the best landable GR found so far. We shall use this in case
		// at the end of the search no home and no preferred were found.

		if ( curgr < curbestgr ) {
			if ( WayPointCalc[curwp].IsAirport) {
				curbestairport=curwp;
				curbestgr=curgr; // ONLY FOR AIRPORT! NOT FOR OUTLANDINGS!!
				#ifdef DEBUG_BESTALTERNATE
				_stprintf(ventabuffer,TEXT("[k=%d]<%s> (curgr=%d < curbestgr=%d) set as bestairport"), 
				k, WayPointList[curwp].Name, (int)curgr, (int)curbestgr );
				if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
				{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
				#endif
			} else {
				// if not better than old outlanding, dont select it
				if (curgr >= curbestgr_outlanding) continue;

				curbestgr_outlanding=curgr;
				curbestoutlanding=curwp;
				#ifdef DEBUG_BESTALTERNATE
				_stprintf(ventabuffer,TEXT("[k=%d]<%s> (curgr=%d < curbestgr=%d and <curbestgr_outland=%d) set as bestoutlanding"), 
				k, WayPointList[curwp].Name, (int)curgr, (int)curbestgr, (int)curbestgr_outlanding );
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
		_stprintf(ventabuffer,TEXT("--> no bestalternate, choosing airport <%s> with gr=%d"), 
		WayPointList[curbestairport].Name, (int)curbestgr );
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		// DoStatusMessage(ventabuffer);
		#endif
		bestalternate=curbestairport;
	} else {
		if ( curbestoutlanding >= 0 ) {
			#ifdef DEBUG_BESTALTERNATE
			_stprintf(ventabuffer,TEXT("--> no bestalternate, choosing outlanding <%s> with gr=%d"), 
			WayPointList[curbestoutlanding].Name, 
			(int)WayPointCalc[curbestoutlanding].GR);
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
				_stprintf(ventabuffer,TEXT("--> No bestalternate was found, and no good airport or outlanding!\n    Setting first available: <%s>"),
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
						_stprintf(ventabuffer,TEXT("Landable list is empty and old bestalternate <%s> has Arrival=%d <0, NO good."),
						WayPointList[bestalternate].Name, (int) WayPointCalc[bestalternate].AltArriv[AltArrivMode]);
						if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
						{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
						#endif
						// Pick up the closest!
						if ( ValidWayPoint( sortApproxIndex[0]) ) {
							bestalternate=sortApproxIndex[0];
							#ifdef DEBUG_BESTALTERNATE
							_stprintf(ventabuffer,
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
							_stprintf(ventabuffer,TEXT("Out of ideas..good luck"));
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
								_stprintf(ventabuffer,
								TEXT("Closer point found: <%s> distance=~%d Km, Reachable with arrival at %d!"),
								WayPointList[bestalternate].Name, sortApproxDistance[0], 
								(int) WayPointList[bestalternate].AltArivalAGL);
								if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
								{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
								#endif
							} else {
								#ifdef DEBUG_BESTALTERNATE
								_stprintf(ventabuffer,
								TEXT("Closer point found: <%s> distance=~%d Km, UNReachable"),
								WayPointList[bestalternate].Name, sortApproxDistance[0]);
								if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
								{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
								#endif
							}
						} else {
							#ifdef DEBUG_BESTALTERNATE
							_stprintf(ventabuffer, _T("Landable list is empty, no Closer Approx, but old best %s is still reachable (arrival=%d)"),
							WayPointList[bestalternate].Name, (int)WayPointCalc[bestalternate].AltArriv[AltArrivMode]);
							if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
							{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
							#endif
						}
					}
				} else {
					// CRITIC POINT
					#ifdef DEBUG_BESTALTERNATE
					_stprintf(ventabuffer,TEXT("Landable list is empty, and NO valid old bestalternate"));
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
			 * and now he is low on terrain..
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
		_stprintf(ventabuffer,TEXT("BESTALTERNATE HOME (%s)"), WayPointList[HomeWaypoint].Name );
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		//DoStatusMessage(ventabuffer);
		#endif
		bestalternate=HomeWaypoint;
	} 
  } else {
	// If still invalid, i.e. not -1, then there's a big problem
	if ( !ValidWayPoint(bestalternate) ) {
		AlertBestAlternate(2);
		#ifdef DEBUG_BESTALTERNATE
		_stprintf(ventabuffer,TEXT("WARNING ERROR INVALID BEST=%d"),bestalternate);
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		#endif
		DoStatusMessage(_T("ERR-051 invalid bestalternate"));
		StartupStore(_T("------ ERR-051 invalid bestalternate <%d>%s"),bestalternate,NEWLINE); // 091122
		// todo: immediate disable function  
	} 
  }

  if (active_bestalternate_on_entry != bestalternate) {
	BestAlternate = bestalternate;
	if ( bestalternate >0 && ((safecalc-WayPointList[bestalternate].Altitude) >ALTERNATE_QUIETMARGIN)) {
		AlertBestAlternate(1);
	}
	return true;
  }
  return false;
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

  if (GPS_INFO.Time < (LastAlertTime-60) ) {
	TestLog(_T("... AlertBestAlternate back in time: now=%f last=%f, reset %s"), GPS_INFO.Time, LastAlertTime, WhatTimeIsIt());
	LastAlertTime=GPS_INFO.Time;
  }

  if ( GPS_INFO.Time > LastAlertTime + 120.0 ) { 
		LastAlertTime = GPS_INFO.Time; 
		switch (soundmode) {
			case 0:
				break;
			case 1:
				LKSound(_T("LK_GREEN.WAV"));

				_stprintf(mbuf,_T("%s %s  @%.0f%s"), MsgToken<1840>(), WayPointList[BestAlternate].Name,
				Units::ToDistance(WayPointCalc[BestAlternate].Distance),
				Units::GetDistanceName());
				Message::AddMessage(2000, 3, mbuf);
				break;
			case 2: 
				LKSound(_T("LK_RED.WAV"));
				_stprintf(mbuf,_T("%s %s"), MsgToken<1840>(), MsgToken<916>()); // WARNING, NO LANDINGS
				// Do NOT disturb the pilot for 5 minutes with useless further messages
				LastAlertTime += 180.0;
				Message::AddMessage(2000, 3, mbuf);
	
				break;
			case 11:
				LKSound(_T("LK_GREEN.WAV"));
				LKSound(_T("LK_GREEN.WAV"));
				break;
			default:
				break;
		}
		#ifdef DEBUG_BESTALTERNATE
		_stprintf(ventabuffer,TEXT("(PLAYED ALERT SOUND, soundmode=%d)"), soundmode);
		if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
		{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
		#endif
  } else {
	#ifdef DEBUG_BESTALTERNATE
	_stprintf(ventabuffer,TEXT("(QUIET, NO PLAY ALERT SOUND, soundmode=%d)"), soundmode);
	if ((fp=_tfopen(_T("DEBUG.TXT"),_T("a")))!= NULL)
	{;fprintf(fp,"%S\n",ventabuffer);fclose(fp);}
	#endif
  }
}


