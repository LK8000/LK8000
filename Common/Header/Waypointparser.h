#if !defined(AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_)
#define AFX_WAYPOINTPARSER_H__695AAC30_F401_4CFF_9BD9_FE62A2A2D0D2__INCLUDED_

#include <vector>
#include <unordered_map>
#include "tchar.h"
#include "Util/tstring.hpp"

struct WAYPOINT;
struct TASK_POINT;


#define wpTerrainBoundsYes    100
#define wpTerrainBoundsYesAll 101
#define wpTerrainBoundsNo     102
#define wpTerrainBoundsNoAll  103

// for extended formats, returns the type of file
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

void SetWaypointComment(WAYPOINT& waypoint, const TCHAR* string);
void SetWaypointDetails(WAYPOINT& waypoint, const TCHAR* string);

int FindMatchingWaypoint(WAYPOINT *waypoint);
int FindMatchingAirfield(WAYPOINT *waypoint);
int FindOrAddWaypoint(WAYPOINT *read_waypoint, bool look_for_airfield);

void WaypointFlagsToString(int FlagsNum, TCHAR *Flags);

void WaypointLongitudeToString(double Longitude, TCHAR *Buffer, size_t size);

template<size_t size>
void WaypointLongitudeToString(double Longitude, TCHAR (&Buffer)[size]) {
    WaypointLongitudeToString(Longitude, Buffer, size);
}

void WaypointLatitudeToString(double Latitude, TCHAR *Buffer, size_t size);

template<size_t size>
void WaypointLatitudeToString(double Latitude, TCHAR (&Buffer)[size]) {
    WaypointLatitudeToString(Latitude, Buffer, size);
}

void LongitudeToCUPString(double Longitude, TCHAR *Buffer, size_t size);

template<size_t size>
void LongitudeToCUPString(double Longitude, TCHAR (&Buffer)[size]) {
    LongitudeToCUPString(Longitude, Buffer, size);
}

void LatitudeToCUPString(double Latitude, TCHAR *Buffer, size_t size);

template<size_t size>
void LatitudeToCUPString(double Latitude, TCHAR (&Buffer)[size]) {
    LatitudeToCUPString(Latitude, Buffer, size);
}

double ReadAltitude(const char* str);
double ReadAltitude(const wchar_t* str);

double ReadLength(const char *str);
double CUPToLat(const char *str);
double CUPToLon(const char *str);
int ReadWayPointFile(std::istream& stream, int fileformat);
int ParseDAT(const TCHAR *String, WAYPOINT *Temp);

std::vector<std::string> CupStringToFieldArray(std::string_view row);

using cup_header_t = std::unordered_map<tstring, size_t>;
cup_header_t CupStringToHeader(std::string_view row);

bool ParseCUPWayPointString(const cup_header_t& cup_header, std::string_view row, WAYPOINT *Temp);

bool ParseOZIWayPointString(const TCHAR *mTempString, WAYPOINT *Temp);
bool ParseCOMPEWayPointString(const TCHAR *mTempString,WAYPOINT *Temp);
bool WaypointInTerrainRange(WAYPOINT *List);
bool ParseOpenAIP(std::streambuf& stream);




#endif
