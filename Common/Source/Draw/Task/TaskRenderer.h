/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRenderer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 12:41 AM
 */

#ifndef TASKRENDERER_H
#define TASKRENDERER_H

#include "Geographic/GeoPoint.h"

class ScreenProjection;
class LKSurface;

class TaskRenderer {
public:

    TaskRenderer() = default;

    virtual ~TaskRenderer() { }

    TaskRenderer(const TaskRenderer&) = delete;
    TaskRenderer& operator=(const TaskRenderer&) = delete;

    void Draw(LKSurface& Surface, const RECT &rc, bool bFill) const;
    void CalculateScreenPosition(const rectObj &screenbounds, const ScreenProjection& _Proj);

    /**
     * return true if task point is polygon, false if shape is polyline.
     */
    virtual bool IsPolygon() const = 0;

#ifdef USE_GDI
    PixelRect GetScreenBounds() const;
#endif

#ifdef HAVE_GLES
    using ScreenPoint = FloatPoint;
#else
    using ScreenPoint = RasterPoint;
#endif

protected:
    using GeoPoints_t = std::vector<GeoPoint>;
    using ScreenPoints_t = std::vector<ScreenPoint>;

    GeoPoints_t _GeoPoints;
    ScreenPoints_t _ScreenPoints;

    bool _Visible = false;
    rectObj _bounds;
};

#endif /* TASKRENDERER_H */
