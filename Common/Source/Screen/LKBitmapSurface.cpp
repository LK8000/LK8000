/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBitmapSurface.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 octobre 2014, 21:49
 */

#include "LKBitmapSurface.h"
#include <cassert>

#ifndef USE_GDI
#include "Screen/BufferCanvas.hpp"
#endif

LKBitmapSurface::LKBitmapSurface() : LKSurface()
#ifdef USE_GDI
    , _hBitmap(), _oldBitmap()
#endif
{
}

LKBitmapSurface::~LKBitmapSurface() {
    Release();
}

LKBitmapSurface::LKBitmapSurface(LKSurface& Surface, unsigned width, unsigned height) : LKSurface()
#ifdef USE_GDI
    , _hBitmap(), _oldBitmap()
#endif
{
    Create(Surface, width, height);
}

void LKBitmapSurface::Create(const LKSurface& Surface, unsigned width, unsigned height) {
#ifdef USE_GDI
    Attach(::CreateCompatibleDC(Surface.GetAttribDC()));
    SetAttribDC(Surface.GetAttribDC());

    _Size = { (LONG)width, (LONG)height };
    _hBitmap = LKBitmap (::CreateCompatibleBitmap(GetAttribDC(), width, height));
    _oldBitmap = LKBitmap((HBITMAP)::SelectObject(_OutputDC, _hBitmap));
#else
    _pCanvas = new BufferCanvas(Surface, {width, height});
#endif
}

void LKBitmapSurface::Resize(unsigned width, unsigned height) {
#ifdef USE_GDI
    if (_oldBitmap) {
        ::SelectObject(_OutputDC, _oldBitmap);
        _oldBitmap.Release();
    }
    if (_hBitmap) {
        _hBitmap.Release();
    }

    _Size = { (LONG)width, (LONG)height };
    _hBitmap = LKBitmap (::CreateCompatibleBitmap(GetAttribDC(), width, height));
    _oldBitmap = LKBitmap((HBITMAP)::SelectObject(_OutputDC, _hBitmap));
#else
    if(_pCanvas) {
        static_cast<BufferCanvas*>(_pCanvas)->Resize({width, height});
    }
#endif
}

void LKBitmapSurface::Release() {
#ifdef USE_GDI
    if (_oldBitmap) {
        ::SelectObject(_OutputDC, _oldBitmap);
        _oldBitmap.Release();
    }
    if (_hBitmap) {
        _hBitmap.Release();
    }
#else
    delete _pCanvas;
    _pCanvas = nullptr;
#endif
    LKSurface::Release();
}

void LKBitmapSurface::CopyTo(LKSurface &other) {
    assert(this != &other);
#ifdef USE_GDI
    other.Copy(0,0, _Size.cx, _Size.cy, *this, 0, 0);
#else

    if(IsDefined() && other.IsDefined()) {

#if defined(ENABLE_OPENGL)
        static_cast<BufferCanvas*>(_pCanvas)->CopyTo(other);
#else
        Canvas& dst = other;
        dst.Copy(*_pCanvas);
#endif

    }

#endif
}

#ifdef ENABLE_OPENGL
void LKBitmapSurface::Begin(LKSurface& Surface) {
    static_cast<BufferCanvas*>(_pCanvas)->Begin(Surface);
}

void LKBitmapSurface::Commit(LKSurface& Surface) {
    static_cast<BufferCanvas*>(_pCanvas)->Commit(Surface);
}
#endif

void LKMaskBitmapSurface::Create(const LKSurface& Surface, unsigned width, unsigned height) {
#ifdef USE_GDI
    Attach(::CreateCompatibleDC(Surface.GetAttribDC()));
    SetAttribDC(Surface.GetAttribDC());

    _hBitmap = LKBitmap(::CreateBitmap(width, height, 1, 1, NULL));
    _oldBitmap = LKBitmap((HBITMAP)::SelectObject(_OutputDC, _hBitmap));
#else
    LKBitmapSurface::Create(Surface, width, height);
#endif
}

void LKMaskBitmapSurface::Resize(unsigned width, unsigned height) {
#ifdef USE_GDI
    if (_oldBitmap) {
        ::SelectObject(_OutputDC, _oldBitmap);
        _oldBitmap.Release();
    }
    if (_hBitmap) {
        _hBitmap.Release();
    }

    _hBitmap = LKBitmap(::CreateBitmap(width, height, 1, 1, NULL));
    _oldBitmap = LKBitmap((HBITMAP)::SelectObject(_OutputDC, _hBitmap));
#else
    LKBitmapSurface::Resize(width, height);
#endif
}
