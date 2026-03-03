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

void PolygonDrawCallback::operator()(GLenum type, const std::vector<FloatPoint>& vertex) {
  switch(type) {
    case GL_TRIANGLES:
      for (size_t i = 0; i + 2 < vertex.size(); i += 3) {
        Surface.FillTriangle(vertex[i], vertex[i + 1], vertex[i + 2]);
      }
      break;
    case GL_TRIANGLE_STRIP:
      if (vertex.size() >= 3) {
        for (size_t i = 0; i < vertex.size() - 2; ++i) {
          Surface.FillTriangle(vertex[i], vertex[i + 1], vertex[i + 2]);
        }
      }
      break;
    case GL_TRIANGLE_FAN:
      if (vertex.size() >= 3) {
        for (size_t i = 1; i < vertex.size() - 1; ++i) {
          Surface.FillTriangle(vertex[0], vertex[i], vertex[i + 1]);
        }
      }
      break;
    default:
      break;
  } 
}
