/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PolygonRenderer.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on November 15, 2017, 8:42 PM
 */

#include "PolygonRenderer.h"
#include <assert.h>
#include <cstdio>

GLvoid GLAPIENTRY beginCallback(GLenum type, void* polygon_data) {
  static_cast<PolygonRenderer*>(polygon_data)->polygonBegin(type);
}

GLvoid GLAPIENTRY endCallback(void* polygon_data) {
  static_cast<PolygonRenderer*>(polygon_data)->polygonEnd();
}

GLvoid GLAPIENTRY combineDataCallback(GLdouble coords[3], GLdouble* vertex_data[4],
                                      GLfloat weight[4], void** dataOut, void* polygon_data) {
  static_cast<PolygonRenderer*>(polygon_data)->polygonCombine(coords, vertex_data, weight, dataOut);
}

GLvoid GLAPIENTRY vertexCallback(GLdouble *vertex, void* polygon_data) {
  static_cast<PolygonRenderer*>(polygon_data)->polygonVertex(vertex);
}

GLvoid GLAPIENTRY errorCallback(GLenum errorCode) {
  fprintf(stderr, "Tessellation Error: %d\n", errorCode);
  assert(false);
}

PolygonRenderer::PolygonRenderer(draw_callback_t&& callback) : draw_callback(std::forward<draw_callback_t>(callback)) {
  tess = gluNewTess();

  gluTessNormal(tess, 0., 0., 1.); // all polygon are inside x,y plane, we can set normal to (0,0,1) for speedup rendering

  gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (_GLUfuncptr) beginCallback);
  gluTessCallback(tess, GLU_TESS_ERROR, (_GLUfuncptr) errorCallback);
  gluTessCallback(tess, GLU_TESS_END_DATA, (_GLUfuncptr) endCallback);
  gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) combineDataCallback);
  gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (_GLUfuncptr) vertexCallback);
}

PolygonRenderer::~PolygonRenderer() {
    gluDeleteTess(tess);
}

void PolygonRenderer::polygonBegin(GLenum type) {
  curr_type = type;
  curr_polygon.clear();  
}

void PolygonRenderer::polygonVertex(GLdouble *vertex) {
  curr_polygon.emplace_back(vertex[0], vertex[1]);
}

void PolygonRenderer::polygonCombine(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], void **outData) {  
  auto It = pointers.emplace(pointers.end(), vertex_t({{coords[0], coords[1], coords[2]}}));
  *outData = It->data();
}

void PolygonRenderer::polygonEnd() {
  draw_callback(curr_type, curr_polygon);
}
