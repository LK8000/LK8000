/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GLShapeRenderer.h
 * Author: Bruno de Lacheisserie
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
#include "../ShapeSpecialRenderer.h"

class Brush;
class XShape;
class ScreenProjection;

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
    
    void renderPolygon(ShapeSpecialRenderer& renderer, LKSurface& Surface, const XShape& shape, Brush& brush, const ScreenProjection& _Proj);
    
private:
    GLUtesselator* tess;
    GLenum curr_type;
    std::vector<FloatPoint> curr_polygon;
    
    typedef std::array<GLdouble,3> vertex_t;
    std::list<vertex_t> pointers;
    
    bool noLabel;
    PixelRect clipRect;
    FloatPoint curr_LabelPos;
    
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

