/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Bitmaps.h"

void MapWindow::DrawTeammate(LKSurface& Surface, const RECT& rc) {
    POINT point;

    if (TeammateCodeValid) {
        if (PointVisible(TeammateLongitude, TeammateLatitude)) {
            LatLon2Screen(TeammateLongitude, TeammateLatitude, point);
            Surface.DrawMaskedBitmap(point.x - NIBLSCALE(10), point.y - NIBLSCALE(10), IBLSCALE(20), IBLSCALE(20), hBmpTeammatePosition, 20, 20);
        }
    }
}

