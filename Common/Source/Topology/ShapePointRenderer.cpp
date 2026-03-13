#include "ShapePointRenderer.h"
#include <span>
#include "Draw/ScreenProjection.h"
#include "MapWindow.h"

ShapePointRenderer::ShapePointRenderer(const shapeObj& shape,
                                       const LKIcon& bitmap)
    : _shape(shape), _bitmap(bitmap) {}

void ShapePointRenderer::Draw(LKSurface& Surface, const ScreenProjection& _Proj,
                              const PixelRect& ClipRect,
                              callback_ref_t callback) {
  if (!_bitmap) {
    return;
  }

  for (const lineObj& line : std::span(_shape.line, _shape.numlines)) {
    for (const pointObj& point : std::span(line.point, line.numpoints)) {
      const RasterPoint sc = _Proj.ToRasterPoint(point.y, point.x);
      if (callback({sc.x, sc.y})) {
        MapWindow::DrawBitmapIn(Surface, sc, _bitmap);
      }
    }
  }
}
