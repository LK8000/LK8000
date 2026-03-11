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

#include "Screen/PolygonSaveCallback.h"
#include <vector>

class LKSurface;

class PolygonDrawCallback {
   
    PolygonDrawCallback() = delete;

public:
    explicit PolygonDrawCallback(LKSurface& s) : Surface(s) {

    }

    void operator()(const TessPolygonT<FloatPoint>& polygon) {
      (*this)(polygon.type, polygon.vertex);
    }

    void operator()(unsigned type, const std::vector<FloatPoint>& vertex);

private:
    LKSurface& Surface;
};

#endif // SCREEN_DEFAULT_POLYGONDRAWCALLBACK_H
