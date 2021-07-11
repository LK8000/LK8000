/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"


//
// The heading track line, like on Garmin units
//
void MapWindow::DrawHeading(LKSurface& Surface, const POINT& Orig, const RECT& rc ) {
    if(DrawInfo.NAVWarning) return; // 100214
    if(mode.Is(MapWindow::Mode::MODE_CIRCLING)) return;

    // Reduce the rectangle for a better effect
    const PixelRect ClipRect = { rc.left+NIBLSCALE(5), rc.top+NIBLSCALE(5), rc.right-NIBLSCALE(5), rc.bottom-NIBLSCALE(5) };

    int tmp = isqrt4((ClipRect.GetSize().cx*ClipRect.GetSize().cx) + (ClipRect.GetSize().cy*ClipRect.GetSize().cy));
    POINT p2;
    if (  MapWindow::mode.autoNorthUP() ||   !( DisplayOrientation == TRACKUP || DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TARGETCIRCLE || DisplayOrientation == TARGETUP)) {   // NorthUP
        p2.y= Orig.y - (int)(tmp*fastcosine(DrawInfo.TrackBearing));
        p2.x= Orig.x + (int)(tmp*fastsine(DrawInfo.TrackBearing));
    } else {  // TrackUP
        p2.x=Orig.x;
        p2.y=Orig.y - (int)tmp;
    }


    Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), Orig, p2, BlackScreen ? RGB_INVDRAW : RGB_BLACK, ClipRect); // 091109
}
