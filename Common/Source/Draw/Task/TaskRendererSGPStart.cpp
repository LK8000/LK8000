/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererSGPStart.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on May 14, 2024, 00:19 AM
 */
#include "externs.h"
#include "TaskRendererSGPStart.h"

TaskRendererSGPStart::TaskRendererSGPStart(const GeoPoint& center, double radius, double radial) {
    _GeoPoints.reserve(3);

    const double start = radial - 90;
    const double end = radial + 90;

    const int start_bearing = std::ceil(start);
    const int end_bearing = std::floor((end > start) ? end : end + 360);

    _GeoPoints.reserve(6 + (end_bearing - start_bearing));

    _GeoPoints.push_back(center.Direct(start, radius));
    _GeoPoints.push_back(center);
    _GeoPoints.push_back(center.Direct(start, 2500));

    for (int i = start_bearing; i <= end_bearing; ++i) {
        _GeoPoints.push_back(center.Direct(static_cast<double> (i), 2500));
    }

    _GeoPoints.push_back(center.Direct(end, 2500));
    _GeoPoints.push_back(center);
    _GeoPoints.push_back(center.Direct(end, radius));

    _bounds.minx = _bounds.maxx = center.longitude;
    _bounds.miny = _bounds.maxy = center.latitude;

    for (GeoPoint& Pt : _GeoPoints) {
        _bounds.minx = std::min(Pt.longitude, _bounds.minx);
        _bounds.maxx = std::max(Pt.longitude, _bounds.maxx);
        _bounds.miny = std::min(Pt.latitude, _bounds.miny);
        _bounds.maxy = std::max(Pt.latitude, _bounds.maxy);
    }
}
