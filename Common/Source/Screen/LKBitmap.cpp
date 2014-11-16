/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBitmap.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 16 octobre 2014, 20:07
 */

#include <utility>
#include "tchar.h"
#include "LKBitmap.h"

#ifndef WIN32
#warning "TODO: need to implement"
#endif

#ifdef WIN32

extern HINSTANCE _hInstance;


LKBitmap::LKBitmap() : _Bitmap(), _Destroy() {
}

LKBitmap::LKBitmap(const LKBitmap& Bitmap) : _Bitmap(), _Destroy() {
    *this = Bitmap;
}

LKBitmap::LKBitmap(LKBitmap&& Bitmap) : _Bitmap(), _Destroy() {
    *this = Bitmap;
}
#endif

LKBitmap::~LKBitmap() {
    Release();
}

LKBitmap& LKBitmap::operator=(const LKBitmap& Bitmap) {
    Release();
#ifdef WIN32
    _Bitmap = (HBITMAP) Bitmap;
    _Destroy = false; // don't take ownership
#endif
    return * this;
}

LKBitmap& LKBitmap::operator=(LKBitmap&& Bitmap) {
    Release();
#ifdef WIN32
    _Bitmap = Bitmap._Bitmap;
    Bitmap._Bitmap = NULL;
    _Destroy = Bitmap._Destroy; // tranfert ownership
    Bitmap._Destroy = false;
#endif
    
    return * this;
}

void LKBitmap::Release() {
#ifdef WIN32
    if (_Destroy && _Bitmap) {
        ::DeleteObject(_Bitmap);
    }
    _Bitmap = NULL;
#endif
}

bool LKBitmap::LoadFromFile(const TCHAR* FilePath) {
    Release();
#ifdef WIN32
#ifdef UNDER_CE
    _Bitmap = (HBITMAP) SHLoadDIBitmap(FilePath);
#else
    _Bitmap = (HBITMAP) LoadImage(GetModuleHandle(NULL), FilePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
#endif
    if (_Bitmap) {
        _Destroy = true;
        return true;
    }
#endif
    return false;
}

bool LKBitmap::LoadFromResource(const TCHAR* ResourceName) {
    Release();
#ifdef WIN32
    _Bitmap = LoadBitmap(_hInstance, ResourceName);
    if (_Bitmap) {
        _Destroy = true;
        return true;
    }
#endif
    return false;
}

SIZE LKBitmap::GetSize() const {
#ifdef WIN32
    BITMAP bm;
    GetObject(_Bitmap, sizeof (BITMAP), &bm);

    return (SIZE){bm.bmWidth, bm.bmHeight};
#else
    return (SIZE){0,0};
#endif
}
