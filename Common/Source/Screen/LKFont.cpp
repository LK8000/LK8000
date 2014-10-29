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
#warning "TODO: need to implement"
#endif
#include "LKFont.h"

#ifndef WIN32
#warning "TODO: need to implement"
#endif

#ifdef WIN32
LKFont::LKFont() : _Font(), _Destroy() {
    
}
LKFont::LKFont(LKFont&& Font) : _Font(), _Destroy() {
    *this = Font;     
}

LKFont::LKFont(const LKFont& Font) : _Font(), _Destroy() {
    *this = Font;
}
#endif

LKFont::~LKFont() {
    Release();
}

LKFont& LKFont::operator=(LKFont&& Font) {
    Release();
#ifdef WIN32
    _Font = Font._Font;
    _Destroy = Font._Destroy;
    
    Font._Font = NULL;
    Font._Destroy = false;
#endif
    return *this;
}

LKFont& LKFont::operator=(const LKFont& Font) {
    Release();
#ifdef WIN32
    _Font = Font._Font;
    _Destroy = false;
#endif
    return *this;
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
