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
  int i;
  if(pan && (mode.Is(Mode::MODE_PAN) || mode.Is(Mode::MODE_TARGET_PAN)))
    // nearest to center of screen if in pan mode
    i=FindNearestWayPoint(PanLongitude, PanLatitude, range);
  else
    i=FindNearestWayPoint(lon, lat, range);
  if(i != -1)
    {
      SelectedWaypoint = i;
      PopupWaypointDetails();
      return true;
    }

  return false;
}



bool MapWindow::Event_InteriorAirspaceDetails(double lon, double lat) {
  bool found=false;
  CAirspaceList reslist = CAirspaceManager::Instance().GetVisibleAirspacesAtPoint(lon, lat);
  CAirspaceList::iterator it;
  for (it = reslist.begin(); it != reslist.end(); ++it) {
	dlgAirspaceDetails(*it);
	found = true;
  }
  return found; // nothing found..
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



