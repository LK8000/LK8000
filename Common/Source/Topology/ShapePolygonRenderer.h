/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ShapePolygonRenderer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on August 26, 2015, 7:37 PM
 */

#ifndef SHAPEPOLYGONRENDERER_H
#define	SHAPEPOLYGONRENDERER_H

#include "Topology/XShape.h"
#include "Screen/PolygonSaveCallback.h"

// if it's a water area (nolabels), print shape up to defaultShape, but print
// labels only up to custom label levels

class ShapePolygonRenderer final : public ShapeRenderer {
 public:
  ShapePolygonRenderer(const shapeObj& shape, const LKBrush& brush);

  void Draw(LKSurface& Surface, const ScreenProjection& _Proj,
            const PixelRect& ClipRect, callback_ref_t callback) override;

 private:
  // triangulated polygon in geographic coordinates
  TessPolygonsT<FloatPoint> _tess_polygon;
  const LKBrush& _brush;
  pointObj _label_position = {};
};

#endif /* SHAPEPOLYGONRENDERER_H */
