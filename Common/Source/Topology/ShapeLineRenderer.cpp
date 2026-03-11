#include "ShapeLineRenderer.h"
#include "Screen/LKSurface.h"
#include "Draw/ScreenProjection.h"
#include "utils/array_adaptor.h"

namespace {

ScreenPoint shape2Screen(const lineObj& line,
                                const ScreenProjection& _Proj,
                                std::vector<ScreenPoint>& points) {
  using scalar_type = ScreenPoint::scalar_type;

  if (line.numpoints < 1) {
    return {std::numeric_limits<scalar_type>::max(),
            std::numeric_limits<scalar_type>::max()};
  }

  const int last = line.numpoints - 1;
  points.clear();
  points.reserve(line.numpoints + 1);

  const GeoToScreen<ScreenPoint> ToScreen(_Proj);

  // Process the first point outside the loop
  ScreenPoint leftPoint = ToScreen(line.point[0]);
  points.push_back(leftPoint);

  for (int i = 1; i <= last; ++i) {
    ScreenPoint pt = ToScreen(line.point[i]);
    if (pt.x <= leftPoint.x) {
      leftPoint = pt;
    }
    const ScreenPoint& prev_pt = points.back();  // Use reference to avoid copying
    if (lround(std::abs(prev_pt.x - pt.x) + std::abs(prev_pt.y - pt.y)) > 2) {
      points.push_back(std::move(pt));
    }
  }

  return leftPoint;
}

}  // namespace

ShapeLineRenderer::ShapeLineRenderer(const shapeObj& shape, const LKPen& pen)
    : _shape(shape), _pen(pen) {}

void ShapeLineRenderer::Draw(LKSurface& Surface, const ScreenProjection& _Proj,
                             const PixelRect& ClipRect,
                             callback_ref_t callback) {
  if (!_pen) {
    return;
  }

  auto old = Surface.SelectObject(_pen);

  for (const lineObj& line : make_array(_shape.line, _shape.numlines)) {
    const ScreenPoint pt = shape2Screen(line, _Proj, _points);
    Surface.Polyline(_points.data(), _points.size(), ClipRect);
    callback({pt.x, pt.y});
  }

  Surface.SelectObject(old);
}
