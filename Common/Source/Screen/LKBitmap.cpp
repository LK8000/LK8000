/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include <limits>
#include <stdio.h>
#include "MessageLog.h"

#ifdef _WIN32
#include "Screen/GDI/WICImageLoad.h"
#include "Screen/GDI/WICContext.h"
#else
#include "resource_data.h"
#endif

#ifdef ANDROID
#include "ResourceId.hpp"
#endif

LKBitmap::LKBitmap() {

}

LKBitmap::LKBitmap(const void* data, size_t size) {
    LoadFromMemory(data, size);
}

LKBitmap::~LKBitmap() {
    Release();
}

bool LKBitmap::LoadFromFile(const TCHAR* FilePath) {
    Reset();
#ifdef _WIN32
    bitmap = (HBITMAP) LoadImage(GetModuleHandle(NULL), FilePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    return IsDefined();
#elif defined(ANDROID)
    return LoadFile(FilePath);
#else
    return LoadPNGFile(FilePath);
#endif
    return false;
}

#ifdef WIN32
    extern HINSTANCE _hInstance;
#endif

bool LKBitmap::LoadFromResource(const TCHAR* ResourceName) {
    Reset();
#ifdef _WIN32
    bitmap = LoadBitmap(_hInstance, ResourceName);
    if (bitmap) {
        return true;
    }
#elif defined(ANDROID)

    if((ptrdiff_t)ResourceName < (ptrdiff_t)std::numeric_limits<unsigned short>::max()) {
        // we have resource ID
        return Load(ResourceId((unsigned short)(ptrdiff_t)ResourceName));
    }
#else
    if(ResourceName) {
        return Load(GetNamedResource(ResourceName), Type::STANDARD);
    }
#endif
    return false;
}

bool LKBitmap::LoadFromMemory(const void* data, size_t size) {
  Reset();
  if (!data || size == 0) {
    return false;
  }
#ifdef _WIN32
  try {
    WICContext ctx;
    (*this) = ::LoadImageFromMemoryWIC(ctx, data, size);
    return IsDefined();
  }
  catch (std::exception& e) {
    StartupStore(_T("LoadFromMemory failed : %s"), to_tstring(e.what()).c_str());
    return false;
  }
#else
  Bitmap::Load(ConstBuffer<void>(data, size));
  return IsDefined();
#endif
}
