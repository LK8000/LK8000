#if !defined(AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include <vector>
#include <unordered_map>
#include "tchar.h"
#include "Util/tstring.hpp"

class zzip_stream;
struct WAYPOINT;
struct TASK_POINT;


#define wpTerrainBoundsYes    100
#define wpTerrainBoundsYesAll 101
#define wpTerrainBoundsNo     102
#define wpTerrainBoundsNoAll  103

// for extended formats, returns the type of file
///int ReadWayPointFile(HANDLE hFile);
void ReadWayPoints(void);
int FindNearestWayPoint(double X, double Y, double MaxRange);
int FindNearestFarVisibleWayPoint(double X, double Y, double MaxRange, short wpType);
void CloseWayPoints(void);
int dlgWaypointOutOfTerrain(TCHAR *Message);
void WaypointWriteFiles(void);
void WaypointAltitudeFromTerrain(WAYPOINT* wpt);
double AltitudeFromTerrain(double Lat, double Lon);
void UpdateTargetAltitude(TASK_POINT& TskPt);

bool AddWaypoint(WAYPOINT& waypoint);
/** For bulk file load only: append to WayPointList; WayPointCalc is filled by InitWayPointCalc(). */
bool AddWaypointBulk(WAYPOINT& waypoint);

void SetWaypointComment(WAYPOINT& waypoint, const TCHAR* string);
void SetWaypointDetails(WAYPOINT& waypoint, const TCHAR* string);

int FindMatchingWaypoint(WAYPOINT *waypoint);
int FindMatchingAirfield(WAYPOINT *waypoint);
int FindOrAddWaypoint(WAYPOINT *read_waypoint, bool look_for_airfield);

void WaypointFlagsToString(int FlagsNum, TCHAR *Flags);
void WaypointLongitudeToString(double Longitude, TCHAR *Buffer);
void WaypointLatitudeToString(double Latitude, TCHAR *Buffer);
void LongitudeToCUPString(double Longitude, TCHAR *Buffer);
void LatitudeToCUPString(double Latitude, TCHAR *Buffer);
double ReadAltitude(const TCHAR *temp);
double ReadLength(const TCHAR *temp);
double CUPToLat(const TCHAR *str);
double CUPToLon(const TCHAR *str);
int ReadWayPointFile(zzip_stream& stream, int fileformat);
int ParseDAT(TCHAR *String,WAYPOINT *Temp);

std::vector<tstring> CupStringToFieldArray(const TCHAR *row);

typedef std::unordered_map<tstring, size_t> cup_header_t;
cup_header_t CupStringToHeader(const TCHAR *row);

bool ParseCUPWayPointString(const cup_header_t& cup_header, const TCHAR *String,WAYPOINT *Temp);
bool ParseOZIWayPointString(TCHAR *mTempString,WAYPOINT *Temp);
bool ParseCOMPEWayPointString(const TCHAR *mTempString,WAYPOINT *Temp);
bool WaypointInTerrainRange(WAYPOINT *List);
bool ParseOpenAIP(zzip_stream& stream);




#endif
