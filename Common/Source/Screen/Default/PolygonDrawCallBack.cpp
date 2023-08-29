/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PolygonDrawCallback.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on August 28, 2023, 10:37 PM
 */

#include "PolygonDrawCallback.h"
#include "Screen/Point.hpp"
#include "Screen/LKSurface.h"
#include <array>

namespace {

inline
RasterPoint ToRasterPoint(FloatPoint p) {
  return {
    static_cast<RasterPoint::scalar_type>(p.x), 
    static_cast<RasterPoint::scalar_type>(p.y)
  };
}

inline
std::array<RasterPoint, 4> ToTriangle(const FloatPoint& p1, const FloatPoint& p2, const FloatPoint& p3) {
  return {
    ToRasterPoint(p1),
    ToRasterPoint(p2),
    ToRasterPoint(p3),
    ToRasterPoint(p1)
  };
}

} // namespace


void PolygonDrawCallback::operator()(GLenum type, const std::vector<FloatPoint>& vertex) {
  switch(type) {
    case GL_TRIANGLES:
      for (size_t i = 0; i < vertex.size(); i += 3) {
        Surface.Polygon(ToTriangle(vertex[i], vertex[i + 1], vertex[i + 2]));
      }
      break;
    case GL_TRIANGLE_STRIP:
      for (size_t i = 0; i < vertex.size() - 2; ++i) {
        Surface.Polygon(ToTriangle(vertex[i], vertex[i + 1], vertex[i + 2]));
      }
      break;
    case GL_TRIANGLE_FAN:
      for (size_t i = 1; i < vertex.size() - 1; ++i) {
        Surface.Polygon(ToTriangle(vertex[0], vertex[i], vertex[i + 1]));
      }
      break;
    default:
      break;
  } 
}
