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
#include "ScreenProjection.h"
//
// Draw bearing line to target
//
void MapWindow::DrawGreatCircle(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, 
        double startLon, double startLat, double targetLon, double targetLat) {

    POINT pt[2] = {
        _Proj.ToRasterPoint(startLon, startLat),
        _Proj.ToRasterPoint(targetLon, targetLat)
    };


    if(LKGeom::ClipLine(rc, pt[0], pt[1])) {
        const auto hpOld = Surface.SelectObject(LKPen_GABRG);
        Surface.Polyline(pt, 2);
        Surface.SelectObject(LK_BLACK_PEN);
        Surface.Polyline(pt, 2);
        Surface.SelectObject(hpOld);
    }
}


