/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   TaskRenderer.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on October 21, 2016, 12:41 AM
 */

#include "externs.h"
#include "TaskRenderer.h"
#include "Draw/ScreenProjection.h"
#include "Math/Point2D.hpp"
#include "Topology/shapelib/mapserver.h"

void TaskRenderer::Draw(LKSurface& Surface, const RECT &rc, bool bFill) const {
    if (!_Visible || _ScreenPoints.empty()) {
        return;
    }

    if (bFill) {
        if(IsPolygon()) {
#ifdef ENABLE_OPENGL
            Canvas& canvas = Surface;
            canvas.DrawTriangleFan(_ScreenPoints.data(), _ScreenPoints.size());
#else
            Surface.Polygon(_ScreenPoints.data(), _ScreenPoints.size(), rc);
#endif
        }
    } else {
        Surface.Polyline(_ScreenPoints.data(), _ScreenPoints.size(), rc);
    }
}

void TaskRenderer::CalculateScreenPosition(const rectObj &screenbounds, const ScreenProjection& _Proj) {

    GeoToScreen<ScreenPoint> ToScreen(_Proj);

    _ScreenPoints.clear();
    _Visible = msRectOverlap(&_bounds, &screenbounds);
    if (_Visible) {
        _ScreenPoints.reserve(_GeoPoints.size() + 1); // +1 to close polygon

        const GeoPoint& first = _GeoPoints.front();
        _ScreenPoints.push_back(ToScreen(first));

        for (GeoPoints_t::const_iterator It = std::next(_GeoPoints.begin()); It != _GeoPoints.end(); ++It) {
            const ScreenPoint pt = ToScreen(*It);
            if (ManhattanDistance(pt, _ScreenPoints.back()) > 2) {
                _ScreenPoints.push_back(pt);
            }
        }
        _ScreenPoints.push_back(_ScreenPoints.front());
    }
}
#ifdef USE_GDI
PixelRect TaskRenderer::GetScreenBounds() const {
    PixelRect bounds(0, 0, 0, 0);
    if (_Visible) {

        ScreenPoints_t::const_iterator It = _ScreenPoints.begin();
        bounds.left = bounds.right = It->x;
        bounds.top = bounds.bottom = It->y;

        while (++It != _ScreenPoints.end()) {
            bounds.left = std::min(It->x, bounds.left);
            bounds.right = std::max(It->x, bounds.right);
            bounds.bottom = std::max(It->y, bounds.bottom);
            bounds.top = std::min(It->y, bounds.top);
        }
    }
    return bounds;
}
#endif
