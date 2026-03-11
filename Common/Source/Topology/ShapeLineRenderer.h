#ifndef _SHAPELINERENDERER_H_
#define _SHAPELINERENDERER_H_

#include "Topology/XShape.h"
#include "Screen/Point.hpp"
#include <vector>

#ifdef HAVE_GLES
using ScreenPoint = FloatPoint;
#else
using ScreenPoint = RasterPoint;
#endif

class ShapeLineRenderer : public ShapeRenderer {
 public:
  ShapeLineRenderer(const shapeObj& shape, const LKPen& pen);

  void Draw(LKSurface& Surface, const ScreenProjection& _Proj,
            const PixelRect& ClipRect, callback_ref_t callback) override;

 private:
  const shapeObj& _shape;
  const LKPen& _pen;

  std::vector<ScreenPoint> _points;
};

#endif  // _SHAPELINERENDERER_H_
