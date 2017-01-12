/*
 * THIS CODE IS INCLUDED IN LKDRAWTRAIL if LONGTRAIL option is selected
 * This is temporary, until the option is permanent and the file is included
 * in the Makefile. 
 */


void MapWindow::LKDrawLongTrail( LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
    static RasterPoint snail_polyline[array_size(LongSnailTrail)+1]; // +1 for last point of "normal" snail trail

    if (TrailActive != 3) return; // only when full trail is selected
    if (iLongSnailNext < 2) return; // no reason to draw a single point

    if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
        return;
    }    

    // pixel manhattan distance
    // It is the sum of x and y differences between previous and next point on screen, in pixels.
    // below this distance, no painting
    const unsigned nearby=10;

    const LONG_SNAIL_POINT* end_iterator = std::begin(LongSnailTrail);
    const LONG_SNAIL_POINT* cur_iterator = std::prev(std::next(LongSnailTrail,iLongSnailNext));
    RasterPoint* polyline_iterator = std::begin(snail_polyline);

    const SNAIL_POINT* last_point = std::next(std::next(SnailTrail, iSnailNext));
    if(last_point == std::end(SnailTrail)) {
        last_point = std::begin(SnailTrail);
    }
    (*polyline_iterator) = _Proj.ToRasterPoint(last_point->Longitude, last_point->Latitude);

    polyline_iterator = std::next(polyline_iterator);

    const auto oldPen = Surface.SelectObject(hSnailPens[3]); // blue color

    while(cur_iterator != end_iterator) {

        (*polyline_iterator) = _Proj.ToRasterPoint(cur_iterator->Longitude, cur_iterator->Latitude);

        if(manhattan_distance(*std::prev(polyline_iterator),(*polyline_iterator)) > nearby) {
            polyline_iterator = std::next(polyline_iterator);
        }
        cur_iterator = std::prev(cur_iterator);
    }
    Surface.Polyline(snail_polyline, std::distance(snail_polyline, polyline_iterator) , rc);

    Surface.SelectObject(oldPen);
}
