#include "tchar.h"
#include <vector>
#include <assert.h>

// TODO : remove this soon as possible
//        exist because we need to include everything to use right implementation...
void StartupStore(const TCHAR*, ...) { }
void OutOfMemory(char const*, int) { }
char const* LKGetText(char const*) { return nullptr; }

struct WAYPOINT {};
void WaypointAltitudeFromTerrain(struct WAYPOINT*) { assert(false); }
double __cdecl AngleLimit360(double) { assert(false);  return 0.; }
int globalFileNum;
std::vector<WAYPOINT> WayPointList;
