/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "MapWindow.h"
#include "Terrain.h"

//
// Draw bearing line to target
//
void MapWindow::DrawGreatCircle(HDC hdc, double startLon, double startLat, double targetLon, double targetLat,
				const RECT rc) {

  HPEN hpOld = (HPEN)SelectObject(hdc, hpBearing);
  POINT pt[2];

  LatLon2Screen(startLon, startLat, pt[0]);
  LatLon2Screen(targetLon, targetLat, pt[1]);

  ClipPolygon(hdc, pt, 2, rc, false);

    SelectObject(hdc, GetStockObject(BLACK_PEN));
  ClipPolygon(hdc, pt, 2, rc, false);

  SelectObject(hdc, hpOld);
}


