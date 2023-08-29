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

#include "Screen/PolygonRenderer.h"
#include "ShapeSpecialRenderer.h"

class Brush;
class XShape;
class ScreenProjection;

class ShapePolygonRenderer final : protected PolygonRenderer {
public:
    using PolygonRenderer::PolygonRenderer;

    void setClipRect(const PixelRect& rect) {
        clipRect = rect;
    }
    
    void setNoLabel(bool b) {
        noLabel = b;
    }
    
    void renderPolygon(ShapeSpecialRenderer& renderer, LKSurface& Surface, const XShape& shape, const Brush& brush, const ScreenProjection& _Proj);
    
private:

    bool noLabel;
    PixelRect clipRect;
    RasterPoint curr_LabelPos;
};

#endif	/* SHAPEPOLYGONRENDERER_H */
