/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRendererDae.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 3:02 AM
 */
#include "externs.h"
#include "TaskRendererDae.h"

TaskRendererDae::TaskRendererDae(const GeoPoint& center, double start, double end) {

    const int start_bearing = std::ceil(start);
    const int end_bearing = std::floor((end > start) ? end : end + 360);
    LKASSERT(end_bearing > start_bearing);

    _GeoPoints.reserve(360 + 4);

    // order of point is very important, need to be odered to allow
    // drawing polygon using OpenGL triangle fan

    _GeoPoints.push_back(center.Direct(start, 500));
    _GeoPoints.push_back(center.Direct(start, 10e3));

    for (int i = start_bearing; i <= end_bearing; ++i) {
        _GeoPoints.push_back(center.Direct(static_cast<double> (i), 10e3));
    }

    _GeoPoints.push_back(center.Direct(end, 10e3));
    _GeoPoints.push_back(center.Direct(end, 500));

    for (int i = end_bearing + 1; i < start_bearing + 360; ++i) {
        _GeoPoints.push_back(center.Direct(static_cast<double> (i), 500));
    }

    _bounds.minx = _bounds.maxx = center.longitude;
    _bounds.miny = _bounds.maxy = center.latitude;

    for (GeoPoint& Pt : _GeoPoints) {
        _bounds.minx = std::min(Pt.longitude, _bounds.minx);
        _bounds.maxx = std::max(Pt.longitude, _bounds.maxx);
        _bounds.miny = std::min(Pt.latitude, _bounds.miny);
        _bounds.maxy = std::max(Pt.latitude, _bounds.maxy);
    }
}
