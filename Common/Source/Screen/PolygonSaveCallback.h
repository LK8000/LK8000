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
#include <algorithm>
#include "Math/Point2D.hpp"

class LKSurface;

template<typename PointT>
struct TessPolygonT {
    unsigned type;
    std::vector<PointT> vertex;
};

template<typename PointT>
using TessPolygonsT = std::vector<TessPolygonT<PointT>>;

template <typename OutPointT, typename InPointT, typename UnaryOperation>
void TransformPolygon(TessPolygonsT<OutPointT>& output,
               const TessPolygonsT<InPointT>& polygon,
               UnaryOperation unary_op) {

  output.clear();
  output.reserve(polygon.size());
  for (const auto& item : polygon) {
    std::vector<OutPointT> screenpoints;
    screenpoints.reserve(item.vertex.size());
    std::transform(std::begin(item.vertex), std::end(item.vertex),
                   std::back_inserter(screenpoints), std::ref(unary_op));
    output.push_back({item.type, std::move(screenpoints)});
  }
}

template<typename PointT>
class PolygonSaveCallback {
public:
    PolygonSaveCallback() = delete;

    explicit PolygonSaveCallback(TessPolygonsT<PointT> &Polygons) : TessPolygons(Polygons) { }

    void operator()(unsigned type, const std::vector<PointT>& vertex) {
        TessPolygons.push_back({type, vertex});
    }
private:
    TessPolygonsT<PointT>& TessPolygons;
};

#endif // SCREEN_POLYGONSAVECALLBACK_H
