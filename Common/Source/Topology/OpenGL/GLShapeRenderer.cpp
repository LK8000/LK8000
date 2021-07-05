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
#include "utils/array_adaptor.h"

#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/VertexPointer.hpp"

#include "externs.h"
#include "Topology.h"
#include "../Draw/ScreenProjection.h"


#ifdef USE_GLSL
#include "Screen/OpenGL/Shaders.hpp"
#include "Screen/OpenGL/Program.hpp"
#endif

void GLShapeRenderer::renderPolygon(ShapeSpecialRenderer& renderer, LKSurface& Surface, const XShape& shape, const Brush& brush, const ScreenProjection& _Proj) {
  /*
   OpenGL cannot draw complex polygons so we need to use a Tessallator to draw the polygon using a GL_TRIANGLE_FAN
   */
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

  brush.Bind();

  std::unique_ptr<const ScopeAlphaBlend> blend;
  if(!brush.IsOpaque()) {
    blend = std::make_unique<const ScopeAlphaBlend>();
  }

  curr_LabelPos.x = clipRect.right;
  curr_LabelPos.y = clipRect.bottom;

  const shapeObj& shp = shape.shape;

  FloatPoint prev_pt = {
          std::numeric_limits<FloatPoint::scalar_type>::max(),
          std::numeric_limits<FloatPoint::scalar_type>::max()
  };

  const GeoToScreen<FloatPoint> ToScreen(_Proj);

  BeginPolygon();
  for( const lineObj& line : make_array(shp.line , shp.numlines)) {
    BeginContour();
    for( const pointObj &point : make_array(line.point, line.numpoints)) {
      const FloatPoint pt = ToScreen(point);
      if (!noLabel &&  (pt.x<=curr_LabelPos.x)) {
        curr_LabelPos = pt;
      }
      if(ManhattanDistance(prev_pt, pt) > 1) {
        AddVertex((GLdouble) pt.x, (GLdouble) pt.y);
        prev_pt = pt;
      }
    }
    EndContour();
  }
  EndPolygon();

  if(shape.HasLabel() && clipRect.IsInside(curr_LabelPos)) {
    shape.renderSpecial(renderer, Surface, curr_LabelPos.x, curr_LabelPos.y, clipRect);
  }
}
