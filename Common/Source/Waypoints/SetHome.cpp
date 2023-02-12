/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "RasterTerrain.h"

namespace {
  AGeoPoint home_position = {};
  tstring home_name;

  AGeoPoint GetWayPointPosition(size_t idx) {
    return { GetWayPointPosition(WayPointList[idx]), WayPointList[idx].Altitude };
  }

}

bool SetNewHome(size_t idx) {
  WithLock(CritSec_TaskData, [&]() {
    if (ValidWayPointFast(idx)) {
      HomeWaypoint = idx;
      home_position = GetWayPointPosition(idx);
      home_name = WayPointList[HomeWaypoint].Name;
    }
    else {
      // not a valid waypoint, reset home data
      HomeWaypoint = -1;
      home_position = {};
      home_name = {};
    }

    TestLog(_T("... Home set to wp. %d"),HomeWaypoint);
  });

  RefreshTask();

  if (home_position != AGeoPoint({0, 0}, 0)) {
    // SIM mode or no valid fix and before takeoff
    //   -> set aircraft position to new home position
    ScopeLock lock(CritSec_FlightData);
    if (SIMMODE || (GPS_INFO.NAVWarning && !CALCULATED_INFO.Flying)) {
      GPS_INFO.Latitude = home_position.latitude;
      GPS_INFO.Longitude = home_position.longitude;
      GPS_INFO.Altitude = home_position.altitude;
    }

    return true; // Valid HomeWaypoint
  }

  return false; // Invalid HomeWaypoint
}


void SetHome(bool reset) {
  TestLog(TEXT(".... SetHome (current=%d), reset=%d"), HomeWaypoint, reset);

  if (reset || !ValidWayPoint(NUMRESWP) ||
      !ValidNotResWayPoint(HomeWaypoint)) {  // BUGFIX 100213 see if really we have wps!
    TestLog(TEXT(".... Home Reset"));
    HomeWaypoint = -1;
  }

  // If one of the alternates is no longer valid, we reset both of them
  if (Alternate1 != -1) {
    if (!ValidNotResWayPoint(Alternate1)) {
      Alternate1 = -1;
    }
  }
  if (Alternate2 != -1) {
    if (!ValidNotResWayPoint(Alternate2)) {
      Alternate2 = -1;
    }
  }

  // check invalid task ref waypoint or forced reset due to file change
  if (!ValidNotResWayPoint(TeamCodeRefWaypoint)) {
    TeamCodeRefWaypoint = -1;
  }

  if (ValidNotResWayPoint(AirfieldsHomeWaypoint)) {
    HomeWaypoint = AirfieldsHomeWaypoint;
    TestLog(_T(".... Using AirfieldHomeWaypoint home=%d <%s>"), HomeWaypoint, WayPointList[HomeWaypoint].Name);
  }

  if (!ValidNotResWayPoint(HomeWaypoint)) {
    // search for home in waypoint list, if we don't have a home
    HomeWaypoint = -1;
    for (size_t i = NUMRESWP; i < WayPointList.size(); ++i) {
      if ((WayPointList[i].Flags & HOME) == HOME) {
        if (HomeWaypoint < 0) {
          HomeWaypoint = i;
          TestLog(TEXT(".... Using waypoint file found home=%d <%s>"), HomeWaypoint, WayPointList[HomeWaypoint].Name);
        }
      }
    }
  }

  // set team code reference waypoint if we don't have one
  if (TeamCodeRefWaypoint == -1) {
    TeamCodeRefWaypoint = HomeWaypoint;
  }

  // if we still don't have a valid home , search for match against memory home
  // This will fix a problem reloading waypoints after editing, or changing files with similars
  if ((!ValidNotResWayPoint(HomeWaypoint)) && home_position != AGeoPoint({0, 0}, 0)) {
    for (size_t i = NUMRESWP; i < WayPointList.size(); i++) {
      if (GetWayPointPosition(WayPointList[i]) != home_position) {
        continue;
      }

      if( home_name != WayPointList[i].Name) {
        continue;
      }

      HomeWaypoint = i;
      TestLog(_T(".... Using matched home lat/lon in waypoints, home=%d <%s>"), HomeWaypoint, WayPointList[HomeWaypoint].Name);
      break;
    }
  }

  // set team code reference waypoint if we don't have one or set it -1
  if (TeamCodeRefWaypoint == -1) {
    TeamCodeRefWaypoint = HomeWaypoint;
  }

  if (!SetNewHome(HomeWaypoint)) {
    // no home at all, so set it from center of terrain if available
    double lon, lat;
    if (RasterTerrain::GetTerrainCenter(&lat, &lon)) {
      ScopeLock lock(CritSec_FlightData);
      GPS_INFO.Latitude = lat;
      GPS_INFO.Longitude = lon;
      GPS_INFO.Altitude = 0;
      CALCULATED_INFO.TerrainValid = true;
      StartupStore(_T("...... No HomeWaypoint, default position set to terrain center"));
    } else {
      StartupStore(_T("...... HomeWaypoint NOT SET"));
    }
  }
}
