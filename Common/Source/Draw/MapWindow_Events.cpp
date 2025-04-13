/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "InputEvents.h"
#include "Dialogs.h"
#include "Dialogs/dlgMultiSelectList.h"
#include "Sound/Sound.h"
#include "ScreenProjection.h"
#include "NavFunctions.h"


bool MapWindow::Event_NearestWaypointDetails(double lon, double lat) {

    double Dist;
    const double range = zoom.RealScale()*500;
    double dyn_range = std::max(5000.0, range*1.0);

    //StartupStore(_T("RANGE=%f\n"),dyn_range);
    //LKSound(TEXT("LK_BELL.WAV"));

start_search:

  LockFlightData();
  DistanceBearing(lat, lon, GPS_INFO.Latitude, GPS_INFO.Longitude, &Dist, NULL);
  UnlockFlightData();

  if (Dist < dyn_range) {
#ifdef OWN_POS_MS
    DlgMultiSelect::AddItem(im_own_pos(), Dist);
#endif
#ifdef ORACLE_MS
    LockFlightData();
    int nearest_waypoint = FindNearestWayPoint(GPS_INFO.Longitude, GPS_INFO.Latitude,
                                               100000.0);  // big range limit
    UnlockFlightData();
    DlgMultiSelect::AddItem(im_oracle{nearest_waypoint}, Dist);
#endif
#ifdef TEAM_CODE_MS
    if (ValidWayPoint(TeamCodeRefWaypoint))
    DlgMultiSelect::AddItem(im_team(), Dist);
#endif
  }

  for (size_t i = NUMRESWP; i < WayPointList.size(); ++i) {  // Consider only valid markers
    if ((WayPointCalc[i].WpType == WPT_AIRPORT) || (WayPointCalc[i].WpType == WPT_OUTLANDING)) {
      DistanceBearing(lat, lon, WayPointList[i].Latitude, WayPointList[i].Longitude, &Dist, NULL);
      if (Dist < dyn_range) {
        DlgMultiSelect::AddItem(im_waypoint{i}, Dist);
      }
    }
  }

#ifdef FLARM_MS
  LastDoTraffic = 0;
  DoTraffic(&DrawInfo, &DerivedDrawInfo);
  for (int i = 0; i < FLARM_MAX_TRAFFIC; ++i) {
    if (LKTraffic[i].Status != LKT_EMPTY) {
      LockFlightData();
      DistanceBearing(lat, lon, LKTraffic[i].Latitude, LKTraffic[i].Longitude, &Dist, NULL);
      UnlockFlightData();
      StartupStore(_T("%s Dist=%6.0f\n"), LKTraffic[i].Name, Dist);
      if (Dist < dyn_range /* 5*range*/) {
        StartupStore(_T("Inside=%6.0f\n"), dyn_range);
        DlgMultiSelect::AddItem(im_flarm{i}, Dist);
      }
    }
  }
#endif

#ifdef WEATHERST_MS
  LockFlightData();
  for (size_t i = 0;i < MAXFANETWEATHER;i++){
    if (GPS_INFO.FANET_Weather[i].Time_Fix != 0){
      DistanceBearing(lat,lon, GPS_INFO.FANET_Weather[i].Latitude, GPS_INFO.FANET_Weather[i].Longitude, &Dist, NULL);
      //StartupStore(_T("%s Dist=%6.0f\n"), GPS_INFO.FANET_Weather[i].Name   ,Dist);
      if(Dist < dyn_range/* 5*range*/) {
        StartupStore(_T("Inside=%6.0f\n"),dyn_range);
        DlgMultiSelect::AddItem(im_weatherst{i}, Dist);
      }
    }
  }  
  UnlockFlightData();
#endif

    int  HorDist=0, Bearing=0, VertDist=0;
    CAirspaceList reslist = CAirspaceManager::Instance().GetNearAirspacesAtPoint(lon, lat, dyn_range / 2);

    for (auto& pAsp : reslist) {
      pAsp->CalculateDistance(&HorDist, &Bearing, &VertDist,lon, lat);
      DlgMultiSelect::AddItem(im_airspace{pAsp}, HorDist);
    }

    for (size_t i = 1; i < WayPointList.size(); i++) {  // Consider only valid markers
      if ((WayPointCalc[i].WpType != WPT_AIRPORT) || (WayPointCalc[i].WpType != WPT_OUTLANDING)) {
        DistanceBearing(lat, lon, WayPointList[i].Latitude, WayPointList[i].Longitude, &Dist, NULL);
        if (Dist < (dyn_range)) {
          DlgMultiSelect::AddItem(im_waypoint{i}, Dist);
        }
      }
    }

    // TASK MULTISELECT
    double Bear=0;
    bool Angleinside = false;

    LockTaskData();
    for(int i=0; ValidTaskPoint(i); i++) {
        LKASSERT(Task[i].Index <=(int)WayPointList.size());
        DistanceBearing(lat,lon,
            WayPointList[Task[i].Index].Latitude,
            WayPointList[Task[i].Index].Longitude, &Dist, &Bear);

        sector_param param = GetTaskSectorParameter(i);

        Angleinside = true;
        if (param.radius < dyn_range) {
          param.radius = dyn_range;
        }

        if ((Dist < param.radius) && Angleinside) {
          DlgMultiSelect::AddItem(im_task_pt{i}, Dist);
        }
    }
    UnlockTaskData();

    if(DlgMultiSelect::GetItemCount() ==0) {
        if(dyn_range < 120000) {
            dyn_range *=2;
            goto start_search;
        } else {
            DoStatusMessage(MsgToken<2248>());  // _@M2248_  "No Near Object found!"
        }
    } else {
        LKSound(TEXT("LK_TOCK.WAV"));
        DlgMultiSelect::ShowModal();
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


/**
 * @vswitch
 *   -1 : toggle
 *   -2 : supertoggle
 *    1 : on
 *    0 : off
 */
void MapWindow::Event_Pan(int vswitch) {
  bool oldPan = mode.AnyPan();

  if (vswitch != 1) {
    if(ValidTaskPoint(PanTaskEdit)) {
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
    mode.Special(Mode::MODE_SPECIAL_PAN, !oldPan); // toggle
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
