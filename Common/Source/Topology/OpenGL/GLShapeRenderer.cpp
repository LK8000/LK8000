/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GLShapeRenderer.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on August 26, 2015, 7:37 PM
 */

#include "GLShapeRenderer.h"

#include <memory>
#include "utils/make_unique.h"

#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/VertexPointer.hpp"

#include "externs.h"
#include "Topology.h"
#include "../Draw/ScreenProjection.h"


#ifdef USE_GLSL
#include "Screen/OpenGL/Shaders.hpp"
#include "Screen/OpenGL/Program.hpp"
#endif

GLvoid GLAPIENTRY beginCallback(GLenum type, void* polygon_data) {
  static_cast<GLShapeRenderer*>(polygon_data)->polygonBegin(type);
}

GLvoid GLAPIENTRY endCallback(void* polygon_data) {
  static_cast<GLShapeRenderer*>(polygon_data)->polygonEnd();
}

GLvoid GLAPIENTRY combineDataCallback(GLdouble coords[3], GLdouble* vertex_data[4],
                                      GLfloat weight[4], void** dataOut, void* polygon_data) {
  static_cast<GLShapeRenderer*>(polygon_data)->polygonCombine(coords, vertex_data, weight, dataOut);
}

GLvoid GLAPIENTRY vertexCallback(GLdouble *vertex, void* polygon_data) {
  static_cast<GLShapeRenderer*>(polygon_data)->polygonVertex(vertex);
}

GLvoid GLAPIENTRY errorCallback(GLenum errorCode) {
  fprintf(stderr, "Tessellation Error: %d\n", errorCode);
  assert(false);
}

GLShapeRenderer::GLShapeRenderer() {

  tess = gluNewTess();
          
  gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (_GLUfuncptr) beginCallback);
  gluTessCallback(tess, GLU_TESS_ERROR, (_GLUfuncptr) errorCallback);
  gluTessCallback(tess, GLU_TESS_END_DATA, (_GLUfuncptr) endCallback);
  gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) combineDataCallback);
  gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (_GLUfuncptr) vertexCallback);
}

GLShapeRenderer::~GLShapeRenderer() {
    gluDeleteTess(tess);
}

void GLShapeRenderer::renderPolygon(ShapeSpecialRenderer& renderer, LKSurface& Surface, const XShape& shape, Brush& brush, const ScreenProjection& _Proj) {
  /*
   OpenGL cannot draw complex polygons so we need to use a Tessallator to draw the polygon using a GL_TRIANGLE_FAN
   */  
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif
  
  brush.Bind();
    
  std::unique_ptr<const GLBlend> blend; 
  if(!brush.IsOpaque()) {
    blend = std::make_unique<const GLBlend>(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  curr_LabelPos.x = clipRect.right;
  curr_LabelPos.y = clipRect.bottom;
  
  const shapeObj& shp = shape.shape;

  FloatPoint prev_pt = {
          std::numeric_limits<FloatPoint::scalar_type>::max(),
          std::numeric_limits<FloatPoint::scalar_type>::max()
  };

  const GeoToScreen<FloatPoint> ToScreen(_Proj);

  gluTessBeginPolygon(tess, this );
  for (int j = 0; j < shp.numlines; j++) {
    gluTessBeginContour(tess);
    const lineObj &line = shp.line[j];
    for (int i =0; i < line.numpoints; i++) {
      const pointObj &point = line.point[i];
      const FloatPoint pt = ToScreen(point);
      if (!noLabel &&  (pt.x<=curr_LabelPos.x)) {
        curr_LabelPos = pt;
      }
      if(ManhattanDistance(prev_pt, pt) >= 1) {
        vertex_t &vertex = *(pointers.insert(pointers.end(),
                                             vertex_t({{(GLdouble) pt.x, (GLdouble) pt.y, 0.}})));
        gluTessVertex(tess, vertex.data(), vertex.data());
        prev_pt = pt;
      }
    }
    gluTessEndContour(tess);
  }
  gluTessEndPolygon(tess);
  
  if(shape.HasLabel() && clipRect.IsInside(curr_LabelPos)) {
    shape.renderSpecial(renderer, Surface, curr_LabelPos.x, curr_LabelPos.y, clipRect);
  }
  
  pointers.clear();
}

void GLShapeRenderer::polygonBegin(GLenum type) {
  curr_type = type;
  curr_polygon.clear();  
}
  
void GLShapeRenderer::polygonVertex(GLdouble *vertex) {
  curr_polygon.insert(curr_polygon.end(), FloatPoint(vertex[0], vertex[1]));
}

void GLShapeRenderer::polygonCombine(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], void **outData) {  
  auto It = pointers.insert(pointers.end(), vertex_t({{coords[0], coords[1], coords[2]}}));
  *outData = It->data();
}

void GLShapeRenderer::polygonEnd() {
  ScopeVertexPointer vp(curr_polygon.data());
  glDrawArrays(curr_type, 0, curr_polygon.size());
}
