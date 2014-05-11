/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"


//
// The heading track line, like on Garmin units
//
void MapWindow::DrawHeading(HDC hdc, POINT Orig, RECT rc ) {
    if(DrawInfo.NAVWarning) return; // 100214
    if(mode.Is(MapWindow::Mode::MODE_CIRCLING)) return;

    POINT p2;
    double tmp = 200000*zoom.ResScaleOverDistanceModify();
    if (!(DisplayOrientation == TRACKUP || DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TRACKCIRCLE)) {
        p2.y= Orig.y - (int)(tmp*fastcosine(DrawInfo.TrackBearing));
        p2.x= Orig.x + (int)(tmp*fastsine(DrawInfo.TrackBearing));
    } else {
        p2.x=Orig.x;
        p2.y=Orig.y - (int)tmp;
    }

    // Reduce the rectangle for a better effect
    rc.top+=NIBLSCALE(5);
    rc.right-=NIBLSCALE(5);
    rc.bottom-=NIBLSCALE(5);
    rc.left+=NIBLSCALE(5);

    ForcedClipping=true;
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), Orig, p2, BlackScreen ? RGB_INVDRAW : RGB_BLACK, rc); // 091109
    ForcedClipping=false;
}
