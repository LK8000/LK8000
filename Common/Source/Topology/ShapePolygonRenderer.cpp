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
#include "Screen/PolygonRenderer.h"
#include "Screen/LKSurface.h"
#include <span>
#include "Draw/ScreenProjection.h"
#include "PolyLabel.h"
#include <optional>

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Scope.hpp"
#ifdef USE_GLSL
#include "Screen/OpenGL/Program.hpp"
#include <glm/gtc/type_ptr.hpp>
#endif
#endif

/*
 OpenGL cannot draw complex polygons so we need to use a Tessallator to draw the
 polygon using a GL_TRIANGLE_FAN
 */

ShapePolygonRenderer::ShapePolygonRenderer(const shapeObj& shape,
                                           const LKBrush& brush)
    : _brush(brush) {
  _label_position = PolyLabel(shape);

  PolygonSaveCallback callback(_tess_polygon);
  PolygonRenderer renderer(callback);

  renderer.BeginPolygon();
  for (const lineObj& line : std::span(shape.line, shape.numlines)) {
    renderer.BeginContour();
    for (const pointObj& point : std::span(line.point, line.numpoints)) {
      renderer.AddVertex(point.x, point.y);
    }
    renderer.EndContour();
  }
  renderer.EndPolygon();
}

void ShapePolygonRenderer::Draw(LKSurface& Surface,
                                const ScreenProjection& _Proj,
                                [[maybe_unused]] const PixelRect& ClipRect,
                                callback_ref_t callback) {
  if (!_brush) {
    return;
  }

#ifndef USE_GDI
  if (_brush.IsHollow()) {
    return;
  }
#endif

  auto oldPen = Surface.SelectObject(LK_NULL_PEN);
  auto oldBrush = Surface.SelectObject(_brush);

#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
  const auto& _tess_polygon_screen = _tess_polygon;
  glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(_Proj.ToGLM()));

  _brush.Bind();

  std::optional<ScopeAlphaBlend> blend;
  if (!_brush.IsOpaque()) {
    blend.emplace();
  }
#else
  TessPolygonsT<FloatPoint> _tess_polygon_screen;
  TransformPolygon(_tess_polygon_screen, _tess_polygon, GeoToScreen<FloatPoint>(_Proj));
#endif

  std::for_each(_tess_polygon_screen.begin(), _tess_polygon_screen.end(),
                PolygonDrawCallback(Surface));

#ifdef USE_GLSL
  glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
#endif

  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldBrush);

  callback(GeoToScreen<RasterPoint>{_Proj}(_label_position));
}
