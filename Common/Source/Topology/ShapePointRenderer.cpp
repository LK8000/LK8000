#include "ShapePointRenderer.h"
#include "utils/array_adaptor.h"
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

  for (const lineObj& line : make_array(_shape.line, _shape.numlines)) {
    for (const pointObj& point : make_array(line.point, line.numpoints)) {
      const RasterPoint sc = _Proj.ToRasterPoint(point.y, point.x);
      if (callback({sc.x, sc.y})) {
        MapWindow::DrawBitmapIn(Surface, sc, _bitmap);
      }
    }
  }
}
