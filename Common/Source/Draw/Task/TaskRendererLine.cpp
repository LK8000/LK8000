/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererLine.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 3:04 AM
 */
#include "externs.h"
#include "TaskRendererLine.h"

TaskRendererLine::TaskRendererLine(const GeoPoint& center, double radius, double radial) {

    _GeoPoints.reserve(3);

    _GeoPoints.push_back(center.Direct(radial - 90, radius));
    _GeoPoints.push_back(center);
    _GeoPoints.push_back(center.Direct(radial + 90, radius));

    _bounds.minx = _bounds.maxx = center.longitude;
    _bounds.miny = _bounds.maxy = center.latitude;

    for (GeoPoint& Pt : _GeoPoints) {
        _bounds.minx = std::min(Pt.longitude, _bounds.minx);
        _bounds.maxx = std::max(Pt.longitude, _bounds.maxx);
        _bounds.miny = std::min(Pt.latitude, _bounds.miny);
        _bounds.maxy = std::max(Pt.latitude, _bounds.maxy);
    }
}
