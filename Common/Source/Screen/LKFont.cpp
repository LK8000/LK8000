/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKFont.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 23 octobre 2014, 20:28
 */

#ifdef WIN32
#include <windows.h>
#else
#include "linuxcompat/types.h"
#endif
#include "LKFont.h"

#include <utility>

#ifndef WIN32
#warning "TODO: need to implement"
#endif

#ifdef WIN32
LKFont::LKFont() : _Font(), _Destroy() {
    
}
#endif

LKFont::~LKFont() {
    Release();
}

void LKFont::Release() {
#ifdef WIN32
    if(_Destroy && _Font) {
        ::DeleteObject(_Font);
    }
    _Font = NULL;
#endif
}

#ifdef WIN32    
void LKFont::Create(LOGFONT* pLogFont) {
    Release();
    
    _Font = ::CreateFontIndirect(pLogFont);
    _Destroy = true;
}
#endif
