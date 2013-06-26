/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <Message.h>
#include "Waypointparser.h"
#include "InputEvents.h"
#include "Dialogs.h"


bool MapWindow::Event_NearestWaypointDetails(double lon, double lat, 
                                             double range,
                                             bool pan) {
double Dist;
unsigned int i;
double dyn_range = range*2;
bool  found = false;
bool  landablefound = false;
//bool airspace_found =false;
if(dyn_range < 1000)
	dyn_range = 1000;
if(dyn_range > 12000)
	dyn_range = 12000;

if (EnableSoundModes)
	PlayResource(TEXT("IDR_WAV_MM0"));

//StartupStore(TEXT("Ulli: Find Objects near lon:%f lat:%f\n"), lon, lat);
  for(i=/*RESWP_FIRST_MARKER*/ NUMRESWP;i<NumberOfWayPoints;i++)
  {    // Consider only valid markers
    if (   (WayPointCalc[i].WpType==WPT_AIRPORT)||
    		 (WayPointCalc[i].WpType==WPT_OUTLANDING)
    	)

    {
      DistanceBearing(lat,lon,
                    WayPointList[i].Latitude,
                    WayPointList[i].Longitude, &Dist, NULL);
      if(Dist < dyn_range)
      {
    	dlgAddMultiSelectListItem(NULL,i, IM_WAYPOINT, Dist);
    	found = true;
    	landablefound = true;
      }
     }
   }

  if(!pan) /* do not look for FLARM objects in PAN mode  */
  {
    LastDoTraffic=0;
    DoTraffic(&DrawInfo,&DerivedDrawInfo);
    for (i=0; i<FLARM_MAX_TRAFFIC; i++)
    {
	  if (LKTraffic[i].Status != LKT_EMPTY)
      {
        DistanceBearing(lat,lon,
    		  LKTraffic[i].Latitude,
    		  LKTraffic[i].Longitude, &Dist, NULL);
        if(Dist < range)
        {
    	  dlgAddMultiSelectListItem((long*)&LKTraffic[i],i, IM_FLARM, Dist);
    	  found = true;
    	  landablefound = true;
        }
      }
    }
  }

  int hDist = 99999;

  {
	CAirspaceList reslist = CAirspaceManager::Instance().GetVisibleAirspacesAtPoint(lon, lat);
	CAirspaceList::iterator it;
	for (it = reslist.begin(); it != reslist.end(); ++it)
	{
	  LKASSERT((*it));
	  dlgAddMultiSelectListItem((long*) (*it),0, IM_AIRSPACE, (double)hDist);
	  found = true;
	}
  }

  if(!landablefound)
  {
    for(i= RESWP_FIRST_MARKER ;i<NumberOfWayPoints;i++)
    {    // Consider only valid markers
      if (   (WayPointCalc[i].WpType != WPT_AIRPORT)||
    		 (WayPointCalc[i].WpType != WPT_OUTLANDING)
    	  )
      {
        DistanceBearing(lat,lon,
                      WayPointList[i].Latitude,
                      WayPointList[i].Longitude, &Dist, NULL);
        if(Dist < dyn_range/2)
        {
    	  dlgAddMultiSelectListItem(NULL,i, IM_WAYPOINT, Dist);
    	  found = true;
        }
      }
    }
  }

  if((!found) && pan )
  {
    DoStatusMessage(_T("No Near Point found!"));
  }

	if(dlgMultiSelectListShowModal() != NULL)
      return true; // nothing found..
	else
	  return false;
}



bool MapWindow::Event_InteriorAirspaceDetails(double lon, double lat) {


#if TESTBENCH
//StartupStore(_T("... Airspace Map Select!\n"));
#endif

return Event_NearestWaypointDetails( lon,  lat,  500*zoom.RealScale(),false);
}



void MapWindow::Event_PanCursor(int dx, int dy) {
  int X= (MapRect.right+MapRect.left)/2;
  int Y= (MapRect.bottom+MapRect.top)/2;
  double Xstart, Ystart, Xnew, Ynew;

  Screen2LatLon(X, Y, Xstart, Ystart);

  X+= (MapRect.right-MapRect.left)*dx/4;
  Y+= (MapRect.bottom-MapRect.top)*dy/4;
  Screen2LatLon(X, Y, Xnew, Ynew);

  if(mode.AnyPan()) {
    PanLongitude += Xstart-Xnew;
    PanLatitude += Ystart-Ynew;
  }
  RefreshMap();
}



void MapWindow::Event_Pan(int vswitch) {
  //  static bool oldfullscreen = 0;  never assigned!
  bool oldPan = mode.AnyPan();
  if (vswitch == -2) { // superpan, toggles fullscreen also
    // new mode
    mode.Special(Mode::MODE_SPECIAL_PAN, !oldPan);
  } else if (vswitch == -1) {
    mode.Special(Mode::MODE_SPECIAL_PAN, !oldPan);
  } else {
    mode.Special(Mode::MODE_SPECIAL_PAN, vswitch != 0); // 0 off, 1 on
  }

  if (mode.AnyPan() != oldPan) {
    if (mode.AnyPan()) {
      PanLongitude = DrawInfo.Longitude;
      PanLatitude = DrawInfo.Latitude;
      InputEvents::setMode(TEXT("pan"));
    } else {
      InputEvents::setMode(TEXT("default"));
      MapWindow::ForceVisibilityScan=true;
    }
	
  }
  RefreshMap();
}



