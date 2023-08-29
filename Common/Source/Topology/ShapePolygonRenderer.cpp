/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ShapePolygonRenderer.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on August 26, 2015, 7:37 PM
 */

#include "ShapePolygonRenderer.h"

#include <memory>
#include "utils/array_adaptor.h"

#include "externs.h"
#include "Topology.h"
#include "../Draw/ScreenProjection.h"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"

#ifdef USE_GLSL
#include "Screen/OpenGL/Program.hpp"
#endif

#endif

void ShapePolygonRenderer::renderPolygon(ShapeSpecialRenderer& renderer, LKSurface& Surface, const XShape& shape, const Brush& brush, const ScreenProjection& _Proj) {
  /*
   OpenGL cannot draw complex polygons so we need to use a Tessallator to draw the polygon using a GL_TRIANGLE_FAN
   */
#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
#endif

#ifdef ENABLE_OPENGL
  brush.Bind();

  std::unique_ptr<const ScopeAlphaBlend> blend;
  if(!brush.IsOpaque()) {
    blend = std::make_unique<const ScopeAlphaBlend>();
  }
#endif

#ifdef HAVE_GLES  
  using ScreenPoint = FloatPoint;
#else
  using ScreenPoint = RasterPoint;
#endif

  curr_LabelPos =  { clipRect.right, clipRect.bottom };

  const shapeObj& shp = shape.shape;

  const GeoToScreen<ScreenPoint> ToScreen(_Proj);

  BeginPolygon();
  for( const lineObj& line : make_array(shp.line , shp.numlines)) {
    BeginContour();
    for( const pointObj &point : make_array(line.point, line.numpoints)) {
      const auto pt = ToScreen(point);
      if (!noLabel &&  (pt.x<=curr_LabelPos.x)) {
        curr_LabelPos = { pt.x, pt.y };
      }
      AddVertex((GLdouble) pt.x, (GLdouble) pt.y);
    }
    EndContour();
  }
  EndPolygon();

  if(shape.HasLabel() && clipRect.IsInside(curr_LabelPos)) {
    shape.renderSpecial(renderer, Surface, curr_LabelPos.x, curr_LabelPos.y, clipRect);
  }
}
