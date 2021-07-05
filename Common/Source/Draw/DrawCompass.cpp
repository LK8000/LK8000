/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"
#include "ScreenGeometry.h"

void MapWindow::DrawCompass(LKSurface& Surface, const RECT& rc, const double angle) {

    POINT ArrowL[] = { {0,-11}, {-5,9}, {0,3}, {0,-11} };
    POINT ArrowR[] = { {0,-11}, { 5,9}, {0,3}, {0,-11} };

    const POINT Start = {
        rc.right - NIBLSCALE(11),
        rc.top + NIBLSCALE(11)
    };

    // North arrow
    PolygonRotateShift(ArrowL, std::size(ArrowL), Start.x, Start.y, -angle);
    PolygonRotateShift(ArrowR, std::size(ArrowR), Start.x, Start.y, -angle);


    LKBrush CompassBrush;
    CompassBrush.Create(OverColorRef);

    const auto hpOld = Surface.SelectObject(LKPen_Black_N0);
    const auto hbOld = Surface.SelectObject(CompassBrush);

    Surface.Polygon(ArrowL,std::size(ArrowL));

    LKBrush CompassShaddowBrush;
    CompassShaddowBrush.Create(RGB_GREY.MixColors(OverColorRef, 0.6));

    Surface.SelectObject(CompassShaddowBrush);

    Surface.Polygon(ArrowR,std::size(ArrowL));

    Surface.SelectObject(hbOld);
    Surface.SelectObject(hpOld);
}
