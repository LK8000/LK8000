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

#ifndef SCREEN_OPENGL_POLYGONDRAWCALLBACK_H
#define SCREEN_OPENGL_POLYGONDRAWCALLBACK_H

#include "Screen/OpenGL/VertexPointer.hpp"
#include "vector"

class LKSurface;

struct PolygonDrawCallback {
    PolygonDrawCallback() = default;

    explicit PolygonDrawCallback(LKSurface&) {
       
    }

    void operator()(GLenum type, const std::vector<FloatPoint>& vertex) {
        ScopeVertexPointer vp(vertex.data());
        glDrawArrays(type, 0, vertex.size());
    }
};

#endif // SCREEN_OPENGL_POLYGONDRAWCALLBACK_H
