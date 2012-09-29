/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"




bool MapWindow::WaypointInTask(int ind) {
  if (!WayPointList) return false;
  return WayPointList[ind].InTask;
}



double MapWindow::GetApproxScreenRange() {
  return (zoom.Scale() * max(DrawRect.right-DrawRect.left,
                         DrawRect.bottom-DrawRect.top))
    *1000.0/GetMapResolutionFactor();
}


// Used only by Thread_Calculation main loop
bool MapWindow::IsDisplayRunning() {
  return (THREADRUNNING && GlobalRunning && ProgramStarted);
}


bool MapWindow::PointInRect(const double &lon, const double &lat,
                            const rectObj &bounds) {
  if ((lon> bounds.minx) &&
      (lon< bounds.maxx) &&
      (lat> bounds.miny) &&
      (lat< bounds.maxy)) 
    return true;
  else
    return false;
}


bool MapWindow::PointVisible(const double &lon, const double &lat) {
  if ((lon> screenbounds_latlon.minx) &&
      (lon< screenbounds_latlon.maxx) &&
      (lat> screenbounds_latlon.miny) &&
      (lat< screenbounds_latlon.maxy)) 
    return true;
  else
    return false;
}


bool MapWindow::PointVisible(const POINT &P)
{
  if(( P.x >= DrawRect.left ) 
     &&
     ( P.x <= DrawRect.right ) 
     &&
     ( P.y >= DrawRect.top  ) 
     &&
     ( P.y <= DrawRect.bottom  ) 
     )
    return TRUE;
  else
    return FALSE;
}


void MapWindow::ScanVisibility(rectObj *bounds_active) {
  // received when the SetTopoBounds determines the visibility
  // boundary has changed.
  // This happens rarely, so it is good pre-filtering of what is visible.
  // (saves from having to do it every screen redraw)
  const rectObj bounds = *bounds_active;

  // far visibility for snail trail

  SNAIL_POINT *sv= SnailTrail;
  const SNAIL_POINT *se = sv+TRAILSIZE;
  while (sv<se) {
    sv->FarVisible = ((sv->Longitude> bounds.minx) &&
		      (sv->Longitude< bounds.maxx) &&
		      (sv->Latitude> bounds.miny) &&
		      (sv->Latitude< bounds.maxy));
    sv++;
  }

  // far visibility for waypoints

  if (WayPointList) {
    WAYPOINT *wv = WayPointList;
    const WAYPOINT *we = WayPointList+NumberOfWayPoints;
    while (wv<we) {
      // TODO code: optimise waypoint visibility
	// TODO 110203 make it happen in 3 steps, with MULTICALC approach
      wv->FarVisible = ((wv->Longitude> bounds.minx) &&
			(wv->Longitude< bounds.maxx) &&
			(wv->Latitude> bounds.miny) &&
			(wv->Latitude< bounds.maxy));
      wv++;
    }
  }

  // far visibility for airspace
  CAirspaceManager::Instance().SetFarVisible( *bounds_active );

}



// Used by dlgTarget only
void MapWindow::SetTargetPan(bool do_pan, int target_point, DWORD dlgSize /* = 0 */)
{
  static double old_latitude;
  static double old_longitude;

  if(dlgSize)
    targetPanSize = dlgSize;

  if (!mode.Is(Mode::MODE_TARGET_PAN) || (TargetPanIndex != target_point)) {
    targetMoved = false;
  }

  TargetPanIndex = target_point;

  if (do_pan && !mode.Is(Mode::MODE_TARGET_PAN)) {
    old_latitude = PanLatitude;
    old_longitude = PanLongitude;
    mode.Special(do_pan ? Mode::MODE_SPECIAL_TARGET_PAN : Mode::MODE_SPECIAL_PAN, true);
    zoom.SwitchMode();
  }
  if (do_pan) {
    LockTaskData();
    if (ValidTaskPoint(target_point)) {
      PanLongitude = WayPointList[Task[target_point].Index].Longitude;
      PanLatitude = WayPointList[Task[target_point].Index].Latitude;
      if (target_point==0) {
        TargetZoomDistance = max(2e3, (double)StartRadius*2);
      } else if (!ValidTaskPoint(target_point+1)) {
        TargetZoomDistance = max(2e3, (double)FinishRadius*2);
      } else if (AATEnabled) {
        if (Task[target_point].AATType == SECTOR) {
          TargetZoomDistance = max(2e3, Task[target_point].AATSectorRadius*2);
        } else {
          TargetZoomDistance = max(2e3, Task[target_point].AATCircleRadius*2);
        }
      } else {
        TargetZoomDistance = max(2e3, (double)SectorRadius*2);
      }
    }
    UnlockTaskData();
  }
  else if (mode.Is(Mode::MODE_TARGET_PAN)) {
    PanLongitude = old_longitude;
    PanLatitude = old_latitude;
    mode.Special(Mode::MODE_SPECIAL_TARGET_PAN, do_pan);
    zoom.SwitchMode();
    }
  mode.Special(Mode::MODE_SPECIAL_TARGET_PAN, do_pan);
  }





bool MapWindow::TargetMoved(double &longitude, double &latitude) {
  bool retval = false;
  LockTaskData();
  if (targetMoved) {
    longitude = targetMovedLon;
    latitude = targetMovedLat;
    targetMoved = false;
    retval = true;
  }
  UnlockTaskData();
  return retval;
}


// Not used anymore, no mapping, use direct RGB_xxxx color references
// colorcode is taken from a 5 bit AsInt union
// void MapWindow::TextColor(HDC hDC, short colorcode) {
// 
// 	switch (colorcode) {
// 	case TEXTBLACK: 
// 		if (BlackScreen) // 091109
// 		  SetTextColor(hDC,RGB_WHITE);  // black 
// 		else
// 		  SetTextColor(hDC,RGB_BLACK);  // black 
// 	  break;
// 	case TEXTWHITE: 
// 		if (BlackScreen) // 091109
// 		  SetTextColor(hDC,RGB_LIGHTYELLOW);  // white
// 		else
// 		  SetTextColor(hDC,RGB_WHITE);  // white
// 	  break;
// 	case TEXTGREEN: 
// 	  SetTextColor(hDC,RGB_GREEN);  // green
// 	  break;
// 	case TEXTRED:
// 	  SetTextColor(hDC,RGB_RED);  // red
// 	  break;
// 	case TEXTBLUE:
// 	  SetTextColor(hDC,RGB_BLUE);  // blue
// 	  break;
// 	case TEXTYELLOW:
// 	  SetTextColor(hDC,RGB_YELLOW);  // yellow
// 	  break;
// 	case TEXTCYAN:
// 	  SetTextColor(hDC,RGB_CYAN);  // cyan
// 	  break;
// 	case TEXTMAGENTA:
// 	  SetTextColor(hDC,RGB_MAGENTA);  // magenta
// 	  break;
// 	case TEXTLIGHTGREY: 
// 	  SetTextColor(hDC,RGB_LIGHTGREY);  // light grey
// 	  break;
// 	case TEXTGREY: 
// 	  SetTextColor(hDC,RGB_GREY);  // grey
// 	  break;
// 	case TEXTLIGHTGREEN:
// 	  SetTextColor(hDC,RGB_LIGHTGREEN);  //  light green
// 	  break;
// 	case TEXTLIGHTRED:
// 	  SetTextColor(hDC,RGB_LIGHTRED);  // light red
// 	  break;
// 	case TEXTLIGHTYELLOW:
// 	  SetTextColor(hDC,RGB_LIGHTYELLOW);  // light yellow
// 	  break;
// 	case TEXTLIGHTCYAN:
// 	  SetTextColor(hDC,RGB_LIGHTCYAN);  // light cyan
// 	  break;
// 	case TEXTORANGE:
// 	  SetTextColor(hDC,RGB_ORANGE);  // orange
// 	  break;
// 	case TEXTLIGHTORANGE:
// 	  SetTextColor(hDC,RGB_LIGHTORANGE);  // light orange
// 	  break;
// 	case TEXTLIGHTBLUE:
// 	  SetTextColor(hDC,RGB_LIGHTBLUE);  // light blue
// 	  break;
// 	default:
// 	  SetTextColor(hDC,RGB_MAGENTA);  // magenta so we know it's wrong: nobody use magenta..
// 	  break;
// 	}
// 
// }

