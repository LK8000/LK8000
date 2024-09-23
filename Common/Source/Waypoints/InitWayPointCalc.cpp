/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"




// LK8000: important!  ALL values in WPCALC must be initialized here!
// There is no other init of this structure elsewhere!
void InitWayPointCalc() {

  DisableBestAlternate = true;
  WayPointCalc.resize(WayPointList.size());

  for (unsigned int i=0; i< WayPointList.size(); i++) {

	WayPointCalc[i].Preferred = false;
	WayPointCalc[i].Distance=-1;
	WayPointCalc[i].Bearing=-1;
	WayPointCalc[i].GR=-1;
	WayPointCalc[i].VGR=-1;
	WayPointCalc[i].NextETE=0;
	WayPointCalc[i].NextAvrETE=0;

	if ( (WayPointList[i].Flags & AIRPORT) == AIRPORT) {
		WayPointCalc[i].IsAirport=true;
		WayPointCalc[i].IsLandable=true;
		WayPointCalc[i].IsOutlanding=false;
		WayPointCalc[i].WpType=WPT_AIRPORT;

        DisableBestAlternate = false;
	} else {
		if ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT) {
			WayPointCalc[i].IsAirport=false;
			WayPointCalc[i].IsLandable=true;
			WayPointCalc[i].IsOutlanding=true;
			WayPointCalc[i].WpType=WPT_OUTLANDING;

            DisableBestAlternate = false;
		} else {
			WayPointCalc[i].IsAirport=false;
			WayPointCalc[i].IsLandable=false;
			WayPointCalc[i].IsOutlanding=false;
			WayPointCalc[i].WpType=WPT_TURNPOINT;
		}
	}
	for (short j=0; j<ALTA_SIZE; j++) {
		WayPointCalc[i].AltArriv[j]=-1;
		WayPointCalc[i].AltReqd[j]=-1;
	}

  }
}
