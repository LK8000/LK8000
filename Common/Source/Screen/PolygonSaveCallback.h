/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PolygonSaveCallback.h
 * Author: Bruno de Lacheisserie
 *
 * Created on April 13, 2025
 */

#ifndef SCREEN_POLYGONSAVECALLBACK_H
#define SCREEN_POLYGONSAVECALLBACK_H

#include <vector>
#include "Math/Point2D.hpp"

class LKSurface;

template<typename PointT>
struct TessPolygon {
    unsigned type;
    std::vector<PointT> vertex;
};

template<typename PointT>
using TessPolygonT = std::vector<TessPolygon<PointT>>;

template<typename PointT>
class PolygonSaveCallback {
public:
    PolygonSaveCallback() = delete;

    explicit PolygonSaveCallback(TessPolygonT<PointT> &Polygons) : TessPolygons(Polygons) { }

    void operator()(unsigned type, const std::vector<FloatPoint>& vertex) {
        TessPolygons.push_back({type, vertex});
    }
private:
    TessPolygonT<PointT>& TessPolygons;
};

#endif // SCREEN_POLYGONSAVECALLBACK_H
