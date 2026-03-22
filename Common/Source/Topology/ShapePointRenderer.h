#ifndef _SHAPEPOINTRENDERER_H_
#define _SHAPEPOINTRENDERER_H_

#include "Topology/XShape.h"
#include "Screen/Point.hpp"

class ShapePointRenderer : public ShapeRenderer {
 public:
  ShapePointRenderer(const shapeObj& shape, const LKIcon& bitmap);

  void Draw(LKSurface& Surface, const ScreenProjection& _Proj,
            const PixelRect& ClipRect, callback_ref_t callback) override;

 private:
  const shapeObj& _shape;
  const LKIcon& _bitmap;
};

#endif  // _SHAPEPOINTRENDERER_H_
