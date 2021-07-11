/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   GLShapeRenderer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on August 26, 2015, 7:37 PM
 */

#ifndef GLSHAPERENDERER_H
#define	GLSHAPERENDERER_H

#include "Screen/OpenGL/PolygonRenderer.h"
#include "../ShapeSpecialRenderer.h"

class Brush;
class XShape;
class ScreenProjection;

class GLShapeRenderer final : protected PolygonRenderer {
public:
    GLShapeRenderer() = default;

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
    FloatPoint curr_LabelPos;
};

#endif	/* GLSHAPERENDERER_H */

