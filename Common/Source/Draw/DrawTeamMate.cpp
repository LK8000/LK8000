/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Bitmaps.h"
#include "ScreenProjection.h"

void MapWindow::DrawTeammate(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
    if (TeammateCodeValid) {
        if (PointVisible(TeammateLongitude, TeammateLatitude)) {
            const RasterPoint point = _Proj.ToRasterPoint(TeammateLatitude, TeammateLongitude);
            hBmpTeammatePosition.Draw(Surface, point.x - NIBLSCALE(10), point.y - NIBLSCALE(10), IBLSCALE(20), IBLSCALE(20));
        }
    }
}
