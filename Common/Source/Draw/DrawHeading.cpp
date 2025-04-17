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

    using PointScalarT = decltype(Orig.x);
    const auto margin = NIBLSCALE<PointScalarT>(5);

    // Reduce the rectangle for a better effect
    const PixelRect ClipRect = {
        rc.left + margin,
        rc.top + margin,
        rc.right - margin,
        rc.bottom - margin
    };

    // max length of the line (diagonal of the Screen minus the borders)
    PointScalarT tmp = std::sqrt((ClipRect.GetSize().cx * ClipRect.GetSize().cx) + (ClipRect.GetSize().cy * ClipRect.GetSize().cy));
    POINT p2;
    if (  MapWindow::mode.autoNorthUP() ||   !( DisplayOrientation == TRACKUP || DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TARGETCIRCLE || DisplayOrientation == TARGETUP)) {   // NorthUP
      p2.y = Orig.y - tmp * fastcosine(DrawInfo.TrackBearing);
      p2.x = Orig.x + tmp * fastsine(DrawInfo.TrackBearing);
    } else {  // TrackUP
      p2.x = Orig.x;
      p2.y = Orig.y - tmp;
    }

    Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), Orig, p2, BlackScreen ? RGB_INVDRAW : RGB_BLACK, ClipRect); // 091109
}
