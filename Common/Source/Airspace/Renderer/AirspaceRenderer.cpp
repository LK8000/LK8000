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
    PolygonSaveCallback callback(_tess_polygon);
    PolygonRenderer renderer(callback);
    renderer.BeginPolygon();
    renderer.BeginContour();
    for (const auto& point : geopoints) {
      renderer.AddVertex(point.Longitude(), point.Latitude());
    }
    renderer.EndContour();
    renderer.EndPolygon();
  }

#ifndef USE_GLSL
  _tess_polygon_screen.clear();

  for(const auto& item : _tess_polygon) {

    std::vector<FloatPoint> screenpoints;
    screenpoints.reserve(geopoints.size());
    std::transform(std::begin(item.vertex), std::end(item.vertex), std::back_inserter(screenpoints),
                   [&](const FloatPoint& p) {
                     return _Proj.ToScreen<FloatPoint>({p.y, p.x});
                   });

    TessPolygon<FloatPoint> screenpolygon = {
        item.type,
        screenpoints,
    };
    _tess_polygon_screen.push_back(std::move(screenpolygon));
  }
#else
  _proj_mat = _Proj.ToGLM();
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
  const auto& _tess_polygon_screen = _tess_polygon;
  glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(_proj_mat));

  brush.Bind();

  std::unique_ptr<const ScopeAlphaBlend> blend;
  if (!brush.IsOpaque()) {
    blend = std::make_unique<const ScopeAlphaBlend>();
  }

#endif

  PolygonDrawCallback renderer(Surface);
  std::for_each(_tess_polygon_screen.begin(), _tess_polygon_screen.end(), [&](const auto& item) {
    renderer(item.type, item.vertex);
  });

#ifdef USE_GLSL
  glUniformMatrix4fv(OpenGL::solid_modelview, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
#endif

  Surface.SelectObject(old);
}
