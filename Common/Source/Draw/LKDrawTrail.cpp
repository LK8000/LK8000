/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */


#include "externs.h"
#include "ScreenProjection.h"

void MapWindow::LKDrawTrail(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
    static RasterPoint snail_polyline[array_size(SnailTrail)];
    
    if (!TrailActive) return;

    const double display_time = DrawInfo.Time;
    const bool use_colors = (MapWindow::zoom.RealScale() < 2); // 1.5 is also quite good;

    //  Trail size
    int num_trail_max = TRAILSIZE;
    if (TrailActive == 2) {
        // scan only recently for lift magnitude (10 min)
        num_trail_max /= TRAILSHRINK;
    }
    if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
        num_trail_max /= TRAILSHRINK; // ( 2 min for short track ? )
    }    
    
    const SNAIL_POINT* end_iterator = std::next(SnailTrail,iSnailNext);
    const SNAIL_POINT* cur_iterator = end_iterator;
    if(cur_iterator != std::begin(SnailTrail)) {
        cur_iterator = std::prev(cur_iterator);
    } else {
        cur_iterator = std::prev(std::end(SnailTrail));
    }
        
    RasterPoint* polyline_iterator = &snail_polyline[0];

    unsigned short prev_color = 2;
    if(use_colors) {
        prev_color = cur_iterator->Colour;
    }
    Surface.SelectObject(hSnailPens[prev_color]);
    
    const bool trail_is_drifted = (EnableTrailDrift && MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && DerivedDrawInfo.WindSpeed >= 1);
    
    double traildrift_lat = 0;
    double traildrift_lon = 0;
    if(trail_is_drifted) {
        double tlat1, tlon1;
        FindLatitudeLongitude(DrawInfo.Latitude, DrawInfo.Longitude,
                DerivedDrawInfo.WindBearing, DerivedDrawInfo.WindSpeed,
                &tlat1, &tlon1);

        traildrift_lat = (DrawInfo.Latitude - tlat1);
        traildrift_lon = (DrawInfo.Longitude - tlon1);
    }
   
    while( (num_trail_max--) > 0 &&  cur_iterator->Time && cur_iterator != end_iterator) {
        
        double this_lat = cur_iterator->Latitude;
        double this_lon = cur_iterator->Longitude;
        if (trail_is_drifted) {
            double dt = std::max(0.0, (display_time - cur_iterator->Time) * cur_iterator->DriftFactor);
            this_lat += traildrift_lat * dt;
            this_lon += traildrift_lon * dt;
        }
        
        (*polyline_iterator) = _Proj.ToRasterPoint(this_lon, this_lat);

        if(use_colors && prev_color != cur_iterator->Colour) {
            // draw polyline before change color.
            Surface.Polyline(snail_polyline, std::distance(snail_polyline, polyline_iterator)+1 , rc);
            // reset polyline
            snail_polyline[0] = *polyline_iterator;
            polyline_iterator = &snail_polyline[1];
            
            // select new Color
            prev_color = cur_iterator->Colour;
            Surface.SelectObject(hSnailPens[prev_color]);
        } else {
            polyline_iterator = std::next(polyline_iterator);
        } 
        
        if(cur_iterator == std::begin(SnailTrail)) {
            cur_iterator = std::end(SnailTrail);
        } 
        cur_iterator = std::prev(cur_iterator);
    }
    Surface.Polyline(snail_polyline, std::distance(snail_polyline, polyline_iterator) , rc);
}

#include "LKDrawLongTrail.cpp"
