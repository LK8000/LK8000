/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"


// this is called only from Task LoadTaskWaypoints
int FindOrAddWaypoint(WAYPOINT *read_waypoint) {
  // this is an invalid pointer!
  read_waypoint->Details = 0;
  read_waypoint->Comment = 0;
  read_waypoint->Name[NAME_SIZE-1] = 0; // prevent overrun if data is bogus
 
  int waypoint_index = FindMatchingWaypoint(read_waypoint);
  if (waypoint_index == -1) {
	// waypoint not found, so add it!
    
	// TODO bug: Set WAYPOINTFILECHANGED so waypoints get saved?
	// NO, we dont save task waypoints inside WP files! 
	WAYPOINT* new_waypoint = GrowWaypointList();
	if (!new_waypoint) {
		// error, can't allocate!
		return false;
	}
	memcpy(new_waypoint, read_waypoint, sizeof(WAYPOINT));
	// 100229 set no-save flag on
	new_waypoint->FileNum=-1;
	waypoint_index = NumberOfWayPoints-1;
  }
  return waypoint_index;
}


bool LoadTaskWaypoints(HANDLE hFile) {
  WAYPOINT read_waypoint;
  DWORD dwBytesRead;

  int i;
  for(i=0;i<MAXTASKPOINTS;i++) {
    if(!ReadFile(hFile,&read_waypoint,sizeof(read_waypoint),&dwBytesRead, (OVERLAPPED *)NULL)
       || (dwBytesRead<sizeof(read_waypoint))) {
      return false;
    }
    if (Task[i].Index != -1) { //  091213 CHECK do not load reserved WP
      Task[i].Index = FindOrAddWaypoint(&read_waypoint);
    }
  }
  for(i=0;i<MAXSTARTPOINTS;i++) {
    if(!ReadFile(hFile,&read_waypoint,sizeof(read_waypoint),&dwBytesRead, (OVERLAPPED *)NULL)
       || (dwBytesRead<sizeof(read_waypoint))) {
      return false;
    }
    if (StartPoints[i].Index != -1) {
      StartPoints[i].Index = FindOrAddWaypoint(&read_waypoint);
    }
  }
  // managed to load everything
  return true;
}

