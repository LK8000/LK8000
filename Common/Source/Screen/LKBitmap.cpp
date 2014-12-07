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

LKBitmap::LKBitmap(LKBitmap&& Bitmap) {
#ifdef WIN32    
    bitmap = Bitmap.bitmap;
    Bitmap.bitmap = nullptr;
#else
#warning "TODO: ..."
#endif
}

LKBitmap::LKBitmap() {
    
}

LKBitmap::~LKBitmap() {
    Release();
}

LKBitmap& LKBitmap::operator= (LKBitmap&& Bitmap) {
#ifdef WIN32
    std::swap(bitmap, Bitmap.bitmap);
#else
#warning "TODO: ..."    
#endif
    return * this;
}

bool LKBitmap::LoadFromFile(const TCHAR* FilePath) {
    Reset();
#ifdef WIN32
#ifdef UNDER_CE
    _Bitmap = (HBITMAP) SHLoadDIBitmap(FilePath);
#else
    bitmap = (HBITMAP) LoadImage(GetModuleHandle(NULL), FilePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
#endif
    return IsDefined();
#else 
    return LoadFile(FilePath);
#endif
    return false;
}

#ifdef WIN32
    extern HINSTANCE _hInstance;
#endif
    
bool LKBitmap::LoadFromResource(const TCHAR* ResourceName) {
    Reset();
#ifdef WIN32
    bitmap = LoadBitmap(_hInstance, ResourceName);
    if (bitmap) {
        return true;
    }
#else
#warning "TODO: ..."    
#endif
    return false;
}
