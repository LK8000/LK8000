/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererCircle.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 12:44 AM
 */
#include "externs.h"
#include "TaskRendererCircle.h"

TaskRendererCircle::TaskRendererCircle(const GeoPoint& center, double radius) {

    _bounds.minx = _bounds.maxx = center.longitude;
    _bounds.miny = _bounds.maxy = center.latitude;

    _GeoPoints.reserve(360);

    for (unsigned i = 0; i < 360; ++i) {
        _GeoPoints.push_back(center.Direct(static_cast<double> (i), radius));

        _bounds.minx = std::min(_GeoPoints.back().longitude, _bounds.minx);
        _bounds.maxx = std::max(_GeoPoints.back().longitude, _bounds.maxx);
        _bounds.miny = std::min(_GeoPoints.back().latitude, _bounds.miny);
        _bounds.maxy = std::max(_GeoPoints.back().latitude, _bounds.maxy);
    }
}
