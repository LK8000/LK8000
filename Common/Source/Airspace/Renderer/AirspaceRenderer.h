#ifndef AIRSPACE_RENDERER_AIRSPACERENDERER_H
#define AIRSPACE_RENDERER_AIRSPACERENDERER_H


#include <vector>
#include "Point2D.h"
#include "Screen/Point.hpp"
#include "types.h"
#include "Screen/PolygonSaveCallback.h"
#include "Screen/PenReference.h"
#ifdef USE_GLSL
#include <glm/mat4x4.hpp>
#endif

class LKSurface;
class LKPen;
class LKBrush;
class ScreenProjection;

using CPoint2DArray = std::vector<CPoint2D>;

#ifdef HAVE_GLES
using ScreenPointList = std::vector<FloatPoint>;
#else
using ScreenPointList = std::vector<RasterPoint>;
#endif
using RasterPointList = std::vector<RasterPoint>;

class AirspaceRenderer final {
 public:
  AirspaceRenderer() = default;

  void Update(CPoint2DArray& geopoints, bool need_clipping, const RECT& rcDraw, const ScreenProjection& _Proj);

  void DrawOutline(LKSurface& Surface, PenReference pen) const;
  void FillPolygon(LKSurface& Surface, const LKBrush& brush) const;

 protected:
  // this 2 array are modified by DrawThread, never use it in another thread !!
  ScreenPointList _screenpoints;  // this is member for reduce memory alloc,
                                  // but is used only by CalculateScreenPosition();
  RasterPointList _screenpoints_clipped;

  TessPolygonT<FloatPoint> _tess_polygon;  // triangulated polygon in geographic coordinates
#ifndef HAVE_GLES
  TessPolygonT<FloatPoint> _tess_polygon_screen;
#else
  glm::mat4 _proj_mat;
#endif
};

#endif  // AIRSPACE_RENDERER_AIRSPACERENDERER_H
