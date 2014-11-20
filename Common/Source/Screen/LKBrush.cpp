/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBrush.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 23 octobre 2014, 20:28
 */
#ifdef WIN32
#include <windows.h>
#endif

#include "LKColor.h"
#include "LKBrush.h"
#include "LKBitmap.h"

#include <utility>

#ifndef WIN32
#warning "TODO: need to implement"
#endif

#ifdef WIN32

const LKBrush LK_WHITE_BRUSH = LKBrush::MakeStock(WHITE_BRUSH);
const LKBrush  LK_BLACK_BRUSH = LKBrush::MakeStock(BLACK_BRUSH);
const LKBrush  LK_HOLLOW_BRUSH = LKBrush::MakeStock(HOLLOW_BRUSH);

LKBrush::LKBrush(LKBrush&& Brush) : _Brush(Brush._Brush) {
    Brush._Brush = nullptr;
}

LKBrush::LKBrush(const LKColor& Color) : _Brush() {
    Create(Color);
}
#endif

LKBrush::~LKBrush() {
#ifdef WIN32    
    if(_Brush) {
        ::DeleteObject(_Brush);
    }
    _Brush = NULL;
#endif    
}

LKBrush& LKBrush::operator= (LKBrush&& Brush) {
#ifdef WIN32
    std::swap(_Brush, Brush._Brush);
#endif    
    return *this;
}

void LKBrush::Create(const LKColor& Color) {
    Release();

#ifdef WIN32
    _Brush = CreateSolidBrush(Color);
#endif
}

void LKBrush::Create(const LKBitmap& Bitmap) {
    Release();

#ifdef WIN32
    _Brush = CreatePatternBrush(Bitmap);
#endif
}

void LKBrush::Release() {
#ifdef WIN32
    if(_Brush) {
        ::DeleteObject(_Brush);
    }
    _Brush = NULL;
#endif
}
