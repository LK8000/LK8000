/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKIcon.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on June 19, 2015, 11:07 PM
 */
#include "LKIcon.h"
#include "LKSurface.h"
#include "resource_data.h"

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Scope.hpp"

#ifdef USE_GLSL
#include "Screen/OpenGL/Shaders.hpp"
#include "Screen/OpenGL/Program.hpp"
#else
#include "Screen/OpenGL/Compatibility.hpp"
#endif
#endif

LKIcon& LKIcon::operator=(LKBitmap&& orig) { 
    _bitmap = std::forward<LKBitmap>(orig);
    _size = _bitmap.GetSize();
#ifndef ENABLE_OPENGL
    _size.cx /= 2;
#endif
    return (*this); 
}

bool LKIcon::LoadFromResource(const TCHAR* ResourceName) { 
    if(_bitmap.LoadFromResource(ResourceName)) {
        _size = _bitmap.GetSize();
#ifndef ENABLE_OPENGL
        _size.cx /= 2;
#endif       
    }
    return _bitmap.IsDefined();
}
    
void LKIcon::Draw(LKSurface& Surface, const int x, const int y, const int cx, const int cy) const  {
#ifdef USE_GDI
    HGDIOBJ old = ::SelectObject(Surface.GetTempDC(), (HBITMAP) _bitmap);

    if (_size.cx != cx || _size.cy != cy) {
        ::StretchBlt(Surface, x, y, cx, cy, Surface.GetTempDC(), 0, 0, _size.cx, _size.cy, SRCPAINT);
        ::StretchBlt(Surface, x, y, cx, cy, Surface.GetTempDC(), _size.cx, 0, _size.cx, _size.cy, SRCAND);
    } else {
        ::BitBlt(Surface, x, y, cx, cy, Surface.GetTempDC(), 0, 0, SRCPAINT);
        ::BitBlt(Surface, x, y, cx, cy, Surface.GetTempDC(), _size.cx, 0, SRCAND);
    }

    ::SelectObject(Surface.GetTempDC(), old);
#elif defined(ENABLE_OPENGL)
#ifdef USE_GLSL
  OpenGL::texture_shader->Use();
#else
  const GLEnable<GL_TEXTURE_2D> scope;
  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#endif
  const ScopeAlphaBlend blend;

  GLTexture &texture = *_bitmap.GetNative();
  texture.Bind();
  texture.Draw(x, y, cx, cy,  0, 0, _size.cx, _size.cy);

#else
    Canvas& canvas = Surface;
    if(canvas.IsDefined() && _bitmap.IsDefined()) {
        if (_size.cx != cx || _size.cy != cy) {
            canvas.StretchOr(x, y, cx, cy, _bitmap, 0, 0, _size.cx, _size.cy);
            canvas.StretchAnd(x, y, cx, cy, _bitmap, _size.cx, 0, _size.cx, _size.cy);
        } else {
            canvas.CopyOr(x, y, cx, cy, _bitmap, 0, 0);
            canvas.CopyAnd(x, y, cx, cy, _bitmap, _size.cx, 0);
        }
    }
#endif
}


