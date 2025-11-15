#include "Compiler.h"
#include "options.h"
#include "AirspaceRenderer.h"
#include "Screen/LKSurface.h"
#include "Screen/PolygonRenderer.h"
#include "Draw/ScreenProjection.h"
#include "MathFunctions.h"
#include "utils/2dpclip.h"
#ifdef USE_GLSL
#include "Screen/OpenGL/Scope.hpp"
#include <glm/gtc/type_ptr.hpp>
#endif

template <typename ScreenPointList>
static void CalculateScreenPolygon(const ScreenProjection& _Proj, const CPoint2DArray& geopoints,
                                   ScreenPointList& screenpoints) {
  using ScreenPoint = typename ScreenPointList::value_type;

  const GeoToScreen<ScreenPoint> ToScreen(_Proj);

  screenpoints.reserve(geopoints.size());
  std::transform(std::begin(geopoints), std::end(geopoints), std::back_inserter(screenpoints), std::ref(ToScreen));

  // close polygon if needed
  if (screenpoints.front() != screenpoints.back()) {
    screenpoints.push_back(screenpoints.front());
  }
}

void AirspaceRenderer::Update(CPoint2DArray& geopoints, bool need_clipping, const RECT& rcDraw,
                              const ScreenProjection& _Proj) {
  // we need to calculate new screen position
  _screenpoints.clear();
  _screenpoints_clipped.clear();

  if (!need_clipping) {
    // clipping is not needed : calculate screen position directly into _screenpoints_clipped
    CalculateScreenPolygon(_Proj, geopoints, _screenpoints_clipped);
  }
  else {
    // clipping is needed : calculate screen position into temp array
    CalculateScreenPolygon(_Proj, geopoints, _screenpoints);

    PixelRect MaxRect(rcDraw);
    MaxRect.Grow(300);  // add space for inner airspace border, avoid artefact on screen border.

    _screenpoints_clipped.reserve(_screenpoints.size());

    LKGeom::ClipPolygon(MaxRect, _screenpoints, _screenpoints_clipped);
  }

  if (_tess_polygon.empty()) {
#ifdef USE_GLSL
    TessPolygonT<FloatPoint> tess_polygon;
#else
    auto& tess_polygon = _tess_polygon;
#endif
    PolygonSaveCallback callback(tess_polygon);
    PolygonRenderer renderer(callback);
    renderer.BeginPolygon();
    renderer.BeginContour();
    for (const auto& point : geopoints) {
      renderer.AddVertex(point.Longitude(), point.Latitude());
    }
    renderer.EndContour();
    renderer.EndPolygon();

#ifdef USE_GLSL
    for (const auto& item : tess_polygon) {
      unsigned type = item.type;
      auto& vertex = item.vertex;
      auto vbo = std::make_unique<GLArrayBuffer>();
      vbo->Load(static_cast<GLsizei>(vertex.size() * sizeof(FloatPoint)), vertex.data());
      _tess_polygon.push_back({type, vertex.size(), std::move(vbo)});
    }
#endif
  }

#ifdef USE_GLSL
  _proj_mat = _Proj.ToGLM();
#else
  _tess_polygon_screen.clear();

  for(const auto& item : _tess_polygon) {

    std::vector<FloatPoint> screenpoints;
    screenpoints.reserve(item.vertex.size());
    std::transform(std::begin(item.vertex), std::end(item.vertex),
                   std::back_inserter(screenpoints),
                   GeoToScreen<FloatPoint>(_Proj));

    TessPolygon<FloatPoint> screenpolygon = {
        item.type,
        std::move(screenpoints),
    };
    _tess_polygon_screen.push_back(std::move(screenpolygon));
  }
#endif
}

void AirspaceRenderer::DrawOutline(LKSurface& Surface, PenReference pen) const {
  auto old = Surface.SelectObject(pen);
  Surface.Polyline(_screenpoints_clipped.data(), _screenpoints_clipped.size());
  Surface.SelectObject(old);
}

void AirspaceRenderer::FillPolygon(LKSurface& Surface, const LKBrush& brush) const {
#ifndef USE_GDI
  if (brush.IsHollow()) {
    return;
  }
#endif

  auto old = Surface.SelectObject(brush);

#ifdef USE_GLSL
  OpenGL::solid_shader->Use();
  glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(_proj_mat));

  brush.Bind();

  std::unique_ptr<const ScopeAlphaBlend> blend;
  if (!brush.IsOpaque()) {
    blend = std::make_unique<const ScopeAlphaBlend>();
  }

  glEnableVertexAttribArray(OpenGL::Attribute::POSITION);
  for (const auto& item : _tess_polygon) {
    item.vbo->Bind();
    glVertexAttribPointer(OpenGL::Attribute::POSITION, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(item.type, 0, item.vertex_count);
    item.vbo->Unbind();
  }
  glDisableVertexAttribArray(OpenGL::Attribute::POSITION);

  glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

#else

  PolygonDrawCallback renderer(Surface);
  std::for_each(_tess_polygon_screen.begin(), _tess_polygon_screen.end(), [&](const auto& item) {
    renderer(item.type, item.vertex);
  });

#endif

  Surface.SelectObject(old);
}
