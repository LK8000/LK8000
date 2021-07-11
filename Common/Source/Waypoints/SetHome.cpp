/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "RasterTerrain.h"


void SetHome(bool reset)
{

  #if TESTBENCH
  StartupStore(TEXT(".... SetHome (current=%d), reset=%d%s"),HomeWaypoint,reset,NEWLINE);
  #endif

  unsigned int i;
  bool resetalternates=false;

  if (reset || !ValidWayPoint(NUMRESWP) || !ValidNotResWayPoint(HomeWaypoint) ) { // BUGFIX 100213 see if really we have wps!
	#if TESTBENCH
	StartupStore(TEXT(".... Home Reset%s"),NEWLINE);
	#endif
	HomeWaypoint = -1;
  }

  // If one of the alternates is no longer valid, we reset both of them
  if (Alternate1 !=-1 ) {
	 if (!ValidNotResWayPoint(Alternate1) ) {
	     resetalternates=true;
	 }
  }
  if (Alternate2 !=-1 ) {
	 if (!ValidNotResWayPoint(Alternate2) ) {
	     resetalternates=true;
	 }
  }
  if (reset || resetalternates) {
      Alternate1= -1; Alternate2= -1;
  }


  // check invalid task ref waypoint or forced reset due to file change
  if (reset || !ValidNotResWayPoint(TeamCodeRefWaypoint)) {
    TeamCodeRefWaypoint = -1;
  }

  if ( ValidNotResWayPoint(AirfieldsHomeWaypoint) ) {
	HomeWaypoint = AirfieldsHomeWaypoint;
	#if TESTBENCH
	StartupStore(TEXT(".... Using AirfieldHomeWaypoint home=%d <%s>%s"),HomeWaypoint,WayPointList[HomeWaypoint].Name,NEWLINE);
	#endif
  }
  if (!ValidNotResWayPoint(HomeWaypoint)) {
    // search for home in waypoint list, if we don't have a home
    HomeWaypoint = -1;
    for(i=NUMRESWP;i<WayPointList.size();++i) {
	if( (WayPointList[i].Flags & HOME) == HOME) {
		if (HomeWaypoint < 0) {
			HomeWaypoint = i;
			#if TESTBENCH
			StartupStore(TEXT(".... Using waypoint file found home=%d <%s>%s"),
				HomeWaypoint,WayPointList[HomeWaypoint].Name,NEWLINE);
			#endif
		}
	}
    }
  }

  // set team code reference waypoint if we don't have one
  if (TeamCodeRefWaypoint== -1) {
    TeamCodeRefWaypoint = HomeWaypoint;
  }

  // if we still don't have a valid home , search for match against memory home
  // This will fix a problem reloading waypoints after editing, or changing files with similars
  if ( (!ValidNotResWayPoint(HomeWaypoint)) && (WpHome_Lat!=0)) {
	for(i=NUMRESWP;i<WayPointList.size();i++) {
		if( WayPointList[i].Latitude  ==  WpHome_Lat)
			if( WayPointList[i].Longitude  == WpHome_Lon)
				if ( _tcscmp(WayPointList[i].Name,WpHome_Name) == 0 ) {
					HomeWaypoint=i;
					#if TESTBENCH
					StartupStore(TEXT(".... Using matched home lat/lon in waypoints, home=%d <%s>%s"),
						HomeWaypoint,WayPointList[HomeWaypoint].Name,NEWLINE);
					#endif
					break;
				}
	}
  }

  // set team code reference waypoint if we don't have one or set it -1
  if (TeamCodeRefWaypoint== -1) {
    TeamCodeRefWaypoint = HomeWaypoint;
  }

  if (ValidNotResWayPoint(HomeWaypoint)) { // 091213
	StartupStore(_T(". HomeWaypoint set to <%s> wpnum=%d%s"),WayPointList[HomeWaypoint].Name,HomeWaypoint,NEWLINE);
	GPS_INFO.Latitude = WayPointList[HomeWaypoint].Latitude;
	GPS_INFO.Longitude = WayPointList[HomeWaypoint].Longitude;
	GPS_INFO.Altitude = WayPointList[HomeWaypoint].Altitude;

	RasterTerrain::Lock();
    RasterTerrain::SetTerrainRounding(0,0);
    const short Alt = RasterTerrain::GetTerrainHeight(GPS_INFO.Latitude, GPS_INFO.Longitude);
	RasterTerrain::Unlock();

    if(Alt!=TERRAIN_INVALID) { // terrain invalid is now positive  ex. 32767
      CALCULATED_INFO.TerrainValid = true;
    }
	// Update memory HomeWaypoint
	WpHome_Lat=WayPointList[HomeWaypoint].Latitude; // 100213
	WpHome_Lon=WayPointList[HomeWaypoint].Longitude;
	_tcscpy(WpHome_Name,WayPointList[HomeWaypoint].Name);

  } else {

    // no home at all, so set it from center of terrain if available
    double lon, lat;
    if (RasterTerrain::GetTerrainCenter(&lat, &lon)) {
      GPS_INFO.Latitude = lat;
	  GPS_INFO.Longitude = lon;
	  GPS_INFO.Altitude = 0;
      CALCULATED_INFO.TerrainValid = true;
	  StartupStore(_T("...... No HomeWaypoint, default position set to terrain center%s"),NEWLINE);
    } else
	  StartupStore(_T("...... HomeWaypoint NOT SET%s"),NEWLINE);
  }
}
