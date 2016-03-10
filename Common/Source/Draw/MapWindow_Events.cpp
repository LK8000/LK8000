/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "Sound/Sound.h"
#include "ScreenProjection.h"

bool MapWindow::Event_NearestWaypointDetails(double lon, double lat) {

    double Dist;
    unsigned int i;
    const double range = zoom.RealScale()*500;  
    double dyn_range = std::max(5000.0, range*3.5);

    //StartupStore(_T("RANGE=%f\n"),dyn_range);
    LKSound(TEXT("LK_BELL.WAV"));

start_search:

#ifdef BUTTONS_MS
    LockFlightData();
    DistanceBearing(lat,lon, GPS_INFO.Latitude,GPS_INFO.Longitude, &Dist, NULL);
    UnlockFlightData();

    if(Dist < dyn_range) {
	#ifdef OWN_POS_MS
  	dlgAddMultiSelectListItem(NULL,0, IM_OWN_POS, Dist);
	#endif
	#ifdef ORACLE_MS
  	dlgAddMultiSelectListItem(NULL,0, IM_ORACLE, Dist);
	#endif
	#ifdef TEAM_CODE_MS
  	dlgAddMultiSelectListItem(NULL,0, IM_TEAM, Dist);
	#endif
    }
#endif	// BUTTONS_MS


    for(size_t i=NUMRESWP;i<WayPointList.size();++i) {    // Consider only valid markers
        if ((WayPointCalc[i].WpType==WPT_AIRPORT)|| (WayPointCalc[i].WpType==WPT_OUTLANDING)) {
            DistanceBearing(lat,lon, WayPointList[i].Latitude, WayPointList[i].Longitude, &Dist, NULL);
            if(Dist < dyn_range) {
    	        dlgAddMultiSelectListItem(NULL,i, IM_WAYPOINT, Dist);
            }
        }
    }

#ifdef FLARM_MS
    if((MapWindow::mode.Is(MapWindow::Mode::MODE_PAN) || MapWindow::mode.Is(MapWindow::Mode::MODE_TARGET_PAN))) {
        LastDoTraffic=0;
        DoTraffic(&DrawInfo,&DerivedDrawInfo);
        for (unsigned i=0; i<FLARM_MAX_TRAFFIC; ++i) {
	    if (LKTraffic[i].Status != LKT_EMPTY) {
                DistanceBearing(lat,lon, LKTraffic[i].Latitude, LKTraffic[i].Longitude, &Dist, NULL);
                if(Dist < range) {
    	            dlgAddMultiSelectListItem((long*)&LKTraffic[i],i, IM_FLARM, Dist);
                }
            }
        }
    }
#endif

    int  HorDist=0, Bearing=0, VertDist=0; 
    CAirspaceList reslist = CAirspaceManager::Instance().GetNearAirspacesAtPoint(lon, lat, (int)(dyn_range/2));

    for (CAirspaceList::const_iterator it = reslist.begin(); it != reslist.end(); ++it) {
        LKASSERT((*it));
        (*it)->CalculateDistance(&HorDist, &Bearing, &VertDist,lon, lat);
        dlgAddMultiSelectListItem((long*) (*it),0, IM_AIRSPACE, HorDist);
    }


    for(i=1;i<WayPointList.size();i++) {    // Consider only valid markers
        if ((WayPointCalc[i].WpType != WPT_AIRPORT)|| (WayPointCalc[i].WpType != WPT_OUTLANDING)) {
            DistanceBearing(lat,lon, WayPointList[i].Latitude, WayPointList[i].Longitude, &Dist, NULL);
            if(Dist < (dyn_range)) {
    	        dlgAddMultiSelectListItem(NULL,i, IM_WAYPOINT, Dist);
            }
        }
    }


    // TASK MULTISELECT
    int SecType= DAe;
    double SecRadius =0;
    double Bear=0;
    bool Angleinside = false;

    LockTaskData();
    for(i=0; ValidTaskPoint(i); i++) {
        LKASSERT(Task[i].Index <=(int)WayPointList.size());
        DistanceBearing(lat,lon,
            WayPointList[Task[i].Index].Latitude,
            WayPointList[Task[i].Index].Longitude, &Dist, &Bear);

        GetTaskSectorParameter(i, &SecType, &SecRadius);

        Angleinside = true;
        if( SecRadius < (dyn_range)) SecRadius =dyn_range;

        if((Dist < SecRadius) && Angleinside) {
  	    dlgAddMultiSelectListItem(NULL,i, IM_TASK_PT, Dist);
        }
    }
    UnlockTaskData();

    #if (WINDOWSPC>0)
  //  if (EnableSoundModes) Poco::Thread::sleep(1000); // let the sound be heard in sequence
    #endif
    if(dlgGetNoElements() ==0) { 
        if(dyn_range < 120000) {
            dyn_range *=2;
            goto start_search;
        } else {
            DoStatusMessage(gettext(TEXT("_@M2248_")));  // _@M2248_  "No Near Object found!"
        }
    } else {
        LKSound(TEXT("LK_GREEN.WAV"));
        dlgMultiSelectListShowModal();
        return true;
    }

    return false;
}



bool MapWindow::Event_InteriorAirspaceDetails(double lon, double lat) {
    if(mode.Is(Mode::MODE_PAN))
        return false;
    return Event_NearestWaypointDetails( lon,  lat);
}



void MapWindow::Event_PanCursor(int dx, int dy) {
  const ScreenProjection _Proj;
  RasterPoint pt = {
    (MapRect.right+MapRect.left)/2,
    (MapRect.bottom+MapRect.top)/2
  };
  double Xstart, Ystart, Xnew, Ynew;

  _Proj.Screen2LonLat(pt, Xstart, Ystart);

  pt.x += (MapRect.right-MapRect.left)*dx/4;
  pt.y += (MapRect.bottom-MapRect.top)*dy/4;
  _Proj.Screen2LonLat(pt, Xnew, Ynew);

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
	  Task[PanTaskEdit].Index = RealActiveWaypoint;
	  RefreshTask();
	  PanTaskEdit = -1;
	  RealActiveWaypoint = -1;
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



