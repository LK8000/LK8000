/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PolygonRenderer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on November 15, 2017, 8:42 PM
 */

#ifndef POLYGONRENDERER_H
#define POLYGONRENDERER_H

#include "Library/glutess/glutess.h"
#include <vector>
#include <array>
#include <list>
#include <functional>
#include "Screen/Point.hpp"
#include "Math/Point2D.hpp"
#include "Screen/PolygonDrawCallback.h"


class PolygonRenderer {

  PolygonRenderer() = delete;
  PolygonRenderer(const PolygonRenderer& orig) = delete;
  PolygonRenderer(PolygonRenderer&& orig) = delete;

public:

  using draw_callback_t = std::function<void(GLenum, std::vector<FloatPoint>)>;

  explicit PolygonRenderer(draw_callback_t&& callback);

  virtual ~PolygonRenderer();

  void BeginPolygon() {
    gluTessBeginPolygon(tess, this);
  }

  void BeginContour() {
    gluTessBeginContour(tess);
  }

  void AddVertex(GLdouble x, GLdouble y) { 
    vertex_t &vertex = *(pointers.emplace(pointers.end(),vertex_t({{x, y, 0.}})));
    gluTessVertex(tess, vertex.data(), vertex.data());
  }

  void EndContour() {
    gluTessEndContour(tess);
  }
  void EndPolygon() {
    gluTessEndPolygon(tess);
    pointers.clear();
  }

private:
  draw_callback_t draw_callback;

  GLUtesselator* tess;
  GLenum curr_type;
  std::vector<FloatPoint> curr_polygon;

  typedef std::array<GLdouble,3> vertex_t;
  std::list<vertex_t> pointers;

  
  friend GLvoid GLAPIENTRY beginCallback(GLenum type, void* polygon_data);
  friend GLvoid GLAPIENTRY endCallback(void* polygon_data);
  friend GLvoid GLAPIENTRY combineDataCallback(GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4], void** dataOut, void* polygon_data);
  friend GLvoid GLAPIENTRY vertexCallback(GLdouble *vertex, void* polygon_data);

  void polygonBegin(GLenum type);
  void polygonVertex(GLdouble *vertex);
  void polygonCombine(GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4], void **dataOut);
  void polygonEnd();      
};

#endif /* POLYGONRENDERER_H */
