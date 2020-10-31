/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "ScreenProjection.h"


#ifdef HAVE_GLES
/**
 * quick fix for avoid screen coordinate overflow, if too much cpu overhead, write drawing algoritm for skip point ouside screen and use RasterPoint
 */
typedef FloatPoint ScreenPoint;
#else
typedef RasterPoint ScreenPoint;
#endif

void MapWindow::LKDrawLongTrail( LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
    static ScreenPoint snail_polyline[std::size(LongSnailTrail)+1]; // +1 for last point of "normal" snail trail

    if (TrailActive != 3) return; // only when full trail is selected
    if (iLongSnailNext < 2) return; // no reason to draw a single point

    if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
        return;
    }    

    // pixel manhattan distance
    // It is the sum of x and y differences between previous and next point on screen, in pixels.
    // below this distance, no painting
    const ScreenPoint::scalar_type nearby=10;

    const LONG_SNAIL_POINT* end_iterator = std::begin(LongSnailTrail);
    const LONG_SNAIL_POINT* cur_iterator = std::prev(std::next(LongSnailTrail,iLongSnailNext));
    ScreenPoint* polyline_iterator = std::begin(snail_polyline);

    const SNAIL_POINT* last_point = std::next(std::next(SnailTrail, iSnailNext));
    if(last_point == std::end(SnailTrail)) {
        last_point = std::begin(SnailTrail);
    }

    const GeoToScreen<ScreenPoint> ToScreen(_Proj);

    (*polyline_iterator) = ToScreen(last_point->Latitude, last_point->Longitude);

    polyline_iterator = std::next(polyline_iterator);

    const auto oldPen = Surface.SelectObject(hSnailPens[3]); // blue color

    while(cur_iterator != end_iterator) {

        (*polyline_iterator) = ToScreen(cur_iterator->Latitude, cur_iterator->Longitude);

        if(ManhattanDistance(*std::prev(polyline_iterator),(*polyline_iterator)) > nearby) {
            polyline_iterator = std::next(polyline_iterator);
        }
        cur_iterator = std::prev(cur_iterator);
    }
    Surface.Polyline(snail_polyline, std::distance(snail_polyline, polyline_iterator) , rc);

    Surface.SelectObject(oldPen);
}
