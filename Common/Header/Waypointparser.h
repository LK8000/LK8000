#if !defined(AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

#include "MapWindow.h"

#define wpTerrainBoundsYes    100
#define wpTerrainBoundsYesAll 101
#define wpTerrainBoundsNo     102
#define wpTerrainBoundsNoAll  103

#ifdef CUPSUP
// for extended formats, returns the type of file
int ReadWayPointFile(HANDLE hFile);
#else
void ReadWayPointFile(HANDLE hFile);
#endif
void ReadWayPoints(void);
void SetHome(bool reset);
int FindNearestWayPoint(double X, double Y, double MaxRange, bool exhaustive=false);
void CloseWayPoints(void);
int dlgWaypointOutOfTerrain(TCHAR *Message);
void WaypointWriteFiles(void);
void WaypointAltitudeFromTerrain(WAYPOINT* wpt);
WAYPOINT* GrowWaypointList();
int FindMatchingWaypoint(WAYPOINT *waypoint);
void InitWayPointCalc(void); 
void AddReservedWaypoints();
void InitVirtualWaypoints();

#endif

