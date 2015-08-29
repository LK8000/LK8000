/* 
 * File:   GLShapeRenderer.h
 * Author: bruno
 *
 * Created on August 26, 2015, 7:37 PM
 */

#ifndef GLSHAPERENDERER_H
#define	GLSHAPERENDERER_H

#include "Library/glutess/glutess.h"
#include <list>
#include <array>
#include <vector>
#include "mapshape.h"
#include "Screen/Point.hpp"
#include "Screen/LKSurface.h"

class Brush;
class XShape;

class GLShapeRenderer final {
public:
    GLShapeRenderer();
    ~GLShapeRenderer();
    
    void setClipRect(const PixelRect& rect) {
        clipRect = rect;
    }
    
    void setNoLabel(bool b) {
        noLabel = b;
    }
    
    void renderPolygon(LKSurface& Surface, const XShape& shape, Brush& brush);
    
private:
    GLUtesselator* tess;
    GLenum curr_type;
    std::vector<RasterPoint> curr_polygon;
    
    typedef std::array<GLdouble,3> vertex_t;
    std::list<vertex_t> pointers;
    
    bool noLabel;
    PixelRect clipRect;
    RasterPoint curr_LabelPos;
    
protected:    
  friend GLvoid GLAPIENTRY beginCallback(GLenum type, void* polygon_data);
  friend GLvoid GLAPIENTRY endCallback(void* polygon_data);
  friend GLvoid GLAPIENTRY combineDataCallback(GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4], void** dataOut, void* polygon_data);
  friend GLvoid GLAPIENTRY vertexCallback(GLdouble *vertex, void* polygon_data);

  void polygonBegin(GLenum type);
  void polygonVertex(GLdouble *vertex);
  void polygonCombine(GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4], void **dataOut);
  void polygonEnd();    
};

#endif	/* GLSHAPERENDERER_H */

