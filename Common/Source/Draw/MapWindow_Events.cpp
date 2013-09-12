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
extern int GetTaskSectorParameter(int TskIdx, int *SecType, double *SecRadius);

bool MapWindow::Event_NearestWaypointDetails(double lon, double lat, 
                                             double range,
                                             bool pan) {

#ifndef	MULTISELECT

#else
double Dist;
unsigned int i;
double dyn_range = range*2.5;
bool  landablefound = false;


  if(pan && (mode.Is(Mode::MODE_PAN) || mode.Is(Mode::MODE_TARGET_PAN)))
  {
	lon = PanLongitude;
	lat = PanLatitude;
  }

if(dyn_range < 5000)
	dyn_range = 5000;

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
    	  landablefound = true;
        }
      }
    }
  }

  int  HorDist=99999, Bearing, VertDist;
  {
	CAirspaceList reslist = CAirspaceManager::Instance().GetNearAirspacesAtPoint(lon, lat, (int)(dyn_range/2));


	CAirspaceList::iterator it;
	for (it = reslist.begin(); it != reslist.end(); ++it)
	{

	  LKASSERT((*it));
	  (*it)->CalculateDistance(&HorDist, &Bearing, &VertDist,lon, lat);
	  HorDist = 99999;
	  dlgAddMultiSelectListItem((long*) (*it),0, IM_AIRSPACE, HorDist);
	}
  }


    for(i=1/* RESWP_FIRST_MARKER*/ ;i<NumberOfWayPoints;i++)
    {    // Consider only valid markers
      if (   (WayPointCalc[i].WpType != WPT_AIRPORT)||
    		 (WayPointCalc[i].WpType != WPT_OUTLANDING)
    	  )
      {
        DistanceBearing(lat,lon,
                      WayPointList[i].Latitude,
                      WayPointList[i].Longitude, &Dist, NULL);
        if(Dist < (dyn_range/2))
        {
    	  dlgAddMultiSelectListItem(NULL,i, IM_WAYPOINT, Dist);
        }
      }
    }


#ifdef TASK_MULTISELECT
int SecType= DAe;
double SecRadius =0;
double Bear=0;
bool Angleinside = false;
LockTaskData();
  for(i=0; ValidTaskPoint(i); i++)
  {
	LKASSERT(Task[i].Index <=(int)NumberOfWayPoints);
    DistanceBearing(lat,lon,
                    WayPointList[Task[i].Index].Latitude,
                    WayPointList[Task[i].Index].Longitude, &Dist, &Bear);

    GetTaskSectorParameter(i, &SecType, &SecRadius);

    Angleinside = true;
    if( SecRadius < (dyn_range))
      SecRadius =dyn_range;

   if((Dist < SecRadius) && Angleinside)
    {
  	  dlgAddMultiSelectListItem(NULL,i, IM_TASK_PT, Dist);
    }
  }
UnlockTaskData();

#endif
  if((dlgGetNoElements() ==0) && pan )
  {
    DoStatusMessage(gettext(TEXT("_@M2248_")));  // _@M2248_  "No Near Object found!"
  }
  else
  {
	dlgMultiSelectListShowModal();

    return true; // nothing found..
  }
#endif
  return false;
}



bool MapWindow::Event_InteriorAirspaceDetails(double lon, double lat) {


#if TESTBENCH
//StartupStore(_T("... Airspace Map Select!\n"));
#endif
if(mode.Is(Mode::MODE_PAN))
	return false;
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
  if(vswitch !=1)
  {
	if(ValidTaskPoint(PanTaskEdit))
	{
	  Task[PanTaskEdit].Index = RealActiceWaypoint;
	  RefreshTask();
	  PanTaskEdit = -1;
	  RealActiceWaypoint = -1;
	  WayPointList[RESWP_PANPOS].Longitude = RESWP_INVALIDNUMBER;
	  WayPointList[RESWP_PANPOS].Latitude  = RESWP_INVALIDNUMBER;
	}
  }



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



