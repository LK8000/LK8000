/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PolygonDrawCallback.h
 * Author: Bruno de Lacheisserie
 *
 * Created on August 28, 2023, 10:37 PM
 */

#ifndef SCREEN_DEFAULT_POLYGONDRAWCALLBACK_H
#define SCREEN_DEFAULT_POLYGONDRAWCALLBACK_H

#include "Library/glutess/glutess.h"
#include "Math/Point2D.hpp"
#include <vector>

class LKSurface;

class PolygonDrawCallback {
   
    PolygonDrawCallback() = delete;

public:
    explicit PolygonDrawCallback(LKSurface& s) : Surface(s) {

    }

    void operator()(GLenum type, const std::vector<FloatPoint>& vertex);

private:
    LKSurface& Surface;
};

#endif // SCREEN_DEFAULT_POLYGONDRAWCALLBACK_H
