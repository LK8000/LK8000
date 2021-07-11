/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: CalculateWaypointReachable.cpp,v 1.1 2011/12/21 10:28:45 root Exp root $
*/

#include "externs.h"
#include "McReady.h"
#include "LKInterface.h"
#include "LKStyle.h"
#include "NavFunctions.h"
// #define DEBUGCW 1

bool CheckLandableReachableTerrainNew(NMEA_INFO *Basic, DERIVED_INFO *Calculated,
                                          double LegToGo, double LegBearing) {

  double lat, lon;
  bool out_of_range;

  double distance_soarable = FinalGlideThroughTerrain(LegBearing, Basic->Latitude,
                                                      Basic->Longitude, Calculated->NavAltitude,
                                                      Calculated, &lat, &lon,
                                                      LegToGo, &out_of_range, NULL);

  if ((out_of_range)||(distance_soarable> LegToGo)) {
    return true;
  } else {
    return false;
  }
}



// Warning: this is called from mapwindow Draw task , not from calculations!!
// this is calling CheckLandableReachableTerrainNew
// Use multicalc approach, splitting calculation inside MapWindow
// thread into multiple instances, 0.5 or 0.33 Hz recommended
// 
//
// Calculations are taking time and slow down painting of map, beware
//
  #define MULTICALC_MINROBIN     5      // minimum split
  #define MULTICALC_MAXROBIN    20      // max split

void MapWindow::LKCalculateWaypointReachable(const bool forced)
{
  if (!forced) ONEHZLIMITER;

  static short multicalc_slot=0; // -1 (which becomes immediately 0) will force full loading on startup, but this is not good
                                 // because currently we are not waiting for ProgramStarted=psNormalOp
                                 // and the first scan is made while still initializing other things

  short numslots=1; // TODO assign numslots with a function, based also on available CPU time
  if (WayPointList.size()>200) {
	numslots=WayPointList.size()/400;
	// keep numslots optimal
	if (numslots<MULTICALC_MINROBIN) numslots=MULTICALC_MINROBIN; // seconds for full scan, as this is executed at 1Hz
	if (numslots>MULTICALC_MAXROBIN) numslots=MULTICALC_MAXROBIN;

	// When waypointnumber has changed, we wont be using an exceeded multicalc_slot, which would crash the sw
	// In this case, we shall probably continue for the first round to calculate without going from the beginning
	// but this is not a problem, we are round-robin all the time here.
	if (++multicalc_slot>numslots) multicalc_slot=1;
  } else {
	multicalc_slot=0; // forcing full scan
  }


  unsigned int i;
  double waypointDistance, waypointBearing,altitudeRequired,altitudeDifference;

  // LandableReachable is used only by the thermal bar indicator in MapWindow2, after here
  // apparently, is used to tell you if you are below final glide but in range for a landable wp
  // Since nov 2011 we dont user LandableReachable in FinalGlide anymore. 
  // However it is still to be understood what drawbacks we might have by changing calculations here.
  LandableReachable = false;

  if (WayPointList.empty()) return;

  unsigned int scanstart;
  unsigned int scanend;

  #if DEBUGCW
  unsigned int numwpscanned=0;
  #endif

  LockTaskData();

  if (multicalc_slot==0) {
	scanstart=0; // including this
	scanend=WayPointList.size(); // will be used -1, so up to this excluded value

	#if DEBUGCW
	StartupStore(_T("... wps=%d multicalc_slot=0 ignored numslot=%d, full scan %d < %d%s"),WayPointList.size(),numslots,scanstart,scanend,NEWLINE);
	#endif
  } else {
	scanstart=(WayPointList.size()/numslots)*(multicalc_slot-1);
	if (multicalc_slot==numslots)
		scanend=WayPointList.size();
	else
		scanend=scanstart+(WayPointList.size()/numslots);

	#if DEBUGCW
	StartupStore(_T("... wps=%d multicalc_slot=%d of %d, scan %d < %d%s"),WayPointList.size(),multicalc_slot, numslots,scanstart,scanend,NEWLINE);
	#endif
  }

  int overtarg=GetOvertargetIndex();
  if (overtarg<0) overtarg=999999;

  for(i=scanstart;i<scanend;i++) {
    // signed Overtgarget -1 becomes a very high number, casted unsigned
    if ( ( ((WayPointCalc[i].AltArriv[AltArrivMode] >=0)||(WayPointList[i].Visible)) && (WayPointCalc[i].IsLandable || (WayPointList[i].Style==STYLE_THERMAL))) 
	|| WaypointInTask(i) || (i==(unsigned int)overtarg) ) {

	DistanceBearing(DrawInfo.Latitude, DrawInfo.Longitude, WayPointList[i].Latitude, WayPointList[i].Longitude, 
		&waypointDistance, &waypointBearing);

	WayPointCalc[i].Distance=waypointDistance; 
	WayPointCalc[i].Bearing=waypointBearing;

	WayPointCalc[i].GR = CalculateGlideRatio(waypointDistance,
		 DerivedDrawInfo.NavAltitude - WayPointList[i].Altitude - GetSafetyAltitude(i));


	altitudeRequired = GlidePolar::MacCreadyAltitude (GetMacCready(i,0), waypointDistance, waypointBearing, 
						DerivedDrawInfo.WindSpeed, DerivedDrawInfo.WindBearing, 0,0,true,0) 
			+ WayPointList[i].Altitude + GetSafetyAltitude(i) - DerivedDrawInfo.EnergyHeight;


	WayPointCalc[i].AltReqd[AltArrivMode] = altitudeRequired;
	WayPointList[i].AltArivalAGL = DerivedDrawInfo.NavAltitude - altitudeRequired; 
      
	if(WayPointList[i].AltArivalAGL >=0){

		WayPointList[i].Reachable = TRUE;

		if (CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo, waypointDistance, waypointBearing)) {
			if(ValidTaskPoint(ActiveTaskPoint) && (i != (unsigned)TASKINDEX)) {
				LandableReachable = true;
			}
		} else {
			WayPointList[i].Reachable = FALSE;
		}
	} else {
		WayPointList[i].Reachable = FALSE;
	}
	#if DEBUGCW
	numwpscanned++;
	#endif

    } // if landable or in task
  } // for all waypoints

  // This is wrong, because multicalc will not necessarily find the LandableReachable at each pass
  // As of nov 2011 it is better not to change it, and let further investigation after 3.0
  if (!LandableReachable) // indentation wrong here

  for(i=scanstart;i<scanend;i++) {
    if(!WayPointList[i].Visible && WayPointList[i].FarVisible)  {
	// visible but only at a distance (limit this to 100km radius)

	if(  WayPointCalc[i].IsLandable ) {
		#if DEBUGCW
		numwpscanned++;
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
					DerivedDrawInfo.WindSpeed, DerivedDrawInfo.WindBearing, 0,0,true,0)
					+ WayPointList[i].Altitude + GetSafetyAltitude(i);
                  
               		altitudeDifference = DerivedDrawInfo.NavAltitude + DerivedDrawInfo.EnergyHeight - altitudeRequired;                                      
                	WayPointList[i].AltArivalAGL = altitudeDifference;

			WayPointCalc[i].AltReqd[AltArrivMode] = altitudeRequired;

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
  #if DEBUGCW
  StartupStore(_T("...... processed wps: %d\n"),numwpscanned);
  #endif
}


