/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "LKObjects.h"
#include "utils/2dpclip.h"

//
// Draw bearing line to target
//
void MapWindow::DrawGreatCircle(HDC hdc, double startLon, double startLat, double targetLon, double targetLat,
				const RECT rc) {

  POINT pt[2];

  LatLon2Screen(startLon, startLat, pt[0]);
  LatLon2Screen(targetLon, targetLat, pt[1]);

    if(LKGeom::ClipLine((POINT) {rc.left, rc.top}, (POINT) {rc.right, rc.bottom}, pt[0], pt[1])) {
        HPEN hpOld = (HPEN)SelectObject(hdc, LKPen_GABRG);
        Polyline(hdc, pt, 2);
        SelectObject(hdc, GetStockObject(BLACK_PEN));
        Polyline(hdc, pt, 2);
        SelectObject(hdc, hpOld);
    }
}


