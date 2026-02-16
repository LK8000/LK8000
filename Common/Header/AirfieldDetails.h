#ifndef AIRFIELDDETAILS_H
#define AIRFIELDDETAILS_H

#include <vector>
#include "Util/tstring.hpp"

void ReadAirfieldFile();

const std::vector<tstring>* GetWaypointImages(unsigned waypoint_index);
const std::vector<tstring>* GetWaypointFiles(unsigned waypoint_index);
const TCHAR* GetWaypointReportingPoint(unsigned waypoint_index);

#endif
