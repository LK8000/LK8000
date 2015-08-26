/* 
 * File:   GLShapeRenderer.cpp
 * Author: bruno
 * 
 * Created on August 26, 2015, 7:37 PM
 */

#include "GLShapeRenderer.h"

#include <stdlib.h>  
#include <memory>
#include "utils/make_unique.h"

#include "Screen/OpenGL/System.hpp"
#include "Screen/OpenGL/Canvas.hpp"
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/VertexPointer.hpp"

#include "Screen/Point.hpp"
#include "externs.h"
#include "MapWindow.h"
#include "Topology.h"


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
  const GLubyte *estring;
  estring = gluErrorString(errorCode);
  fprintf(stderr, "Tessellation Error: %d %s\n", errorCode, estring);
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

    glPopMatrix();
    gluDeleteTess(tess);
}

void GLShapeRenderer::renderPolygon(LKSurface& Surface, const XShape& shape, Brush& brush) {
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
  
  curr_LabelPos = clipRect.GetBottomRight();
  
  const shapeObj& shp = shape.shape;

  static RasterPoint pt;
  
  gluTessBeginPolygon(tess, this );
  for (int j = 0; j < shp.numlines; j++) {
    gluTessBeginContour(tess);
    for (int i = 0; i < shp.line[j].numpoints; i++) {
      MapWindow::LatLon2Screen(shp.line[j].point[i].x, shp.line[j].point[i].y, pt);
      if (!noLabel &&  (pt.x<=curr_LabelPos.x)) {
        curr_LabelPos = pt;
      }  
      vertex_t& vertex = *(pointers.insert(pointers.end(), vertex_t({(GLdouble)pt.x, (GLdouble)pt.y, 0.0})));
      gluTessVertex(tess, vertex.data(), vertex.data());
    }
    gluTessEndContour(tess);
  }
  gluTessEndPolygon(tess);
  
  if(shape.HasLabel() && clipRect.IsInside(curr_LabelPos)) {
    shape.renderSpecial(Surface, curr_LabelPos.x, curr_LabelPos.y, clipRect);
  }
  
  pointers.clear();
}

void GLShapeRenderer::polygonBegin(GLenum type) {
  curr_type = type;
  curr_polygon.clear();  
}
  
void GLShapeRenderer::polygonVertex(GLdouble *vertex) {
  curr_polygon.insert(curr_polygon.end(), RasterPoint(vertex[0], vertex[1]));
}

void GLShapeRenderer::polygonCombine(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], void **outData) {  
  auto It = pointers.insert(pointers.end(), vertex_t({coords[0], coords[1], coords[2]}));
  *outData = It->data();
}

void GLShapeRenderer::polygonEnd() {
  ScopeVertexPointer vp(curr_polygon.data());
  glDrawArrays(curr_type, 0, curr_polygon.size());
}
