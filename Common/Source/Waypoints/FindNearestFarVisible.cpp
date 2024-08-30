/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "NavFunctions.h"




// Returns -1 if no result
int FindNearestFarVisibleWayPoint(double X, double Y, double maxRange, short wpType)
{
  int nearestIndex = -1;
  double nearestDistance, dist;

  int farvisibles=0;

  if(WayPointList.size() <= NUMRESWP ) return -1;
  nearestDistance = maxRange;

  for(unsigned i=NUMRESWP; i<WayPointList.size(); ++i) {

	if (!WayPointList[i].FarVisible) continue;
	if (wpType && (WayPointCalc[i].WpType != wpType)) continue;

	farvisibles++;

	DistanceBearing(Y,X, WayPointList[i].Latitude, WayPointList[i].Longitude, &dist, NULL);

	if(dist < nearestDistance) {
		nearestIndex = i;
		nearestDistance = dist;
	}
  }

  TestLog(_T("...... Checked %d farvisibles waypoints for maxRange=%f, type=%d\n"),farvisibles,maxRange,wpType);

  if(nearestDistance < maxRange) {
	return nearestIndex;
  } else {
	return -1;
  }
}
