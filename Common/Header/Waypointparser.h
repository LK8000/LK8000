#if !defined(AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>


#define wpTerrainBoundsYes    100
#define wpTerrainBoundsYesAll 101
#define wpTerrainBoundsNo     102
#define wpTerrainBoundsNoAll  103

// for extended formats, returns the type of file
///int ReadWayPointFile(HANDLE hFile);
void ReadWayPoints(void);
void SetHome(bool reset);
int FindNearestWayPoint(double X, double Y, double MaxRange, bool exhaustive=false);
int FindNearestFarVisibleWayPoint(double X, double Y, double MaxRange, short wpType);
void CloseWayPoints(void);
int dlgWaypointOutOfTerrain(TCHAR *Message);
void WaypointWriteFiles(void);
void WaypointAltitudeFromTerrain(WAYPOINT* wpt);
WAYPOINT* GrowWaypointList();
int FindMatchingWaypoint(WAYPOINT *waypoint);
void InitWayPointCalc(void); 
void AddReservedWaypoints();
void InitVirtualWaypoints();
bool AllocateWaypointList(void);
int FindOrAddWaypoint(WAYPOINT *read_waypoint);

void WaypointFlagsToString(int FlagsNum, TCHAR *Flags);
void WaypointLongitudeToString(double Longitude, TCHAR *Buffer);
void WaypointLatitudeToString(double Latitude, TCHAR *Buffer);
void LongitudeToCUPString(double Longitude, TCHAR *Buffer);
void LatitudeToCUPString(double Latitude, TCHAR *Buffer);
int dlgWaypointOutOfTerrain(TCHAR *Message);
double ReadAltitude(TCHAR *temp);
double ReadLength(TCHAR *temp);
double CUPToLat(TCHAR *temp);
double CUPToLon(TCHAR *temp);
int ReadWayPointFile(ZZIP_FILE *fp, TCHAR *CurrentWpFileName);
int ParseDAT(TCHAR *String,WAYPOINT *Temp);
void CleanCupCode(TCHAR* TpCode);
bool ParseCUPWayPointString(TCHAR *String,WAYPOINT *Temp);
bool ParseOZIWayPointString(TCHAR *mTempString,WAYPOINT *Temp);
bool ParseCOMPEWayPointString(TCHAR *mTempString,WAYPOINT *Temp);
bool WaypointInTerrainRange(WAYPOINT *List);





#endif

