/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"
#include "ScreenGeometry.h"
#include "LocalPath.h"

#ifdef USE_GDI
#define IMG_EXT "BMP"
#else
#define IMG_EXT "PNG"
#endif

namespace {

    void setFileSuffix(TCHAR* Out, PixelSize size) {
        if(ScreenLandscape || size.cx < size.cy) {
            _stprintf(Out, _T("_%03dx%03d." IMG_EXT), size.cx, size.cy);
        } else {
            _stprintf(Out, _T("_%03dx%03d." IMG_EXT), size.cy, size.cx);
        }
    }

    LKBitmap LoadSplashBitmap(const TCHAR *path) {
        LKBitmap bitmap;
#ifdef ANDROID
        bitmap.LoadAssetsFile(path);
#else
        bitmap.LoadFromFile(path);
#endif
        return bitmap;
    }
 
} // namespace

LKBitmap LoadSplash(const TCHAR *splashfile) {

    TCHAR srcfile[MAX_PATH];
#ifdef ANDROID
    _tcscpy(srcfile, _T(LKD_BITMAPS DIRSEP) );
#else
    SystemPath(srcfile,_T(LKD_BITMAPS DIRSEP));
#endif
    TCHAR* pSuffixStart = srcfile + _tcslen(srcfile); // end of path
    _tcscpy(pSuffixStart, splashfile); // add filename
    pSuffixStart += _tcslen(pSuffixStart);

    LKBitmap hWelcomeBitmap;


    // first look for lkstart_480x272.bmp for example
    setFileSuffix(pSuffixStart, {ScreenSizeX, ScreenSizeY});
    hWelcomeBitmap = LoadSplashBitmap(srcfile);

    if (!hWelcomeBitmap.IsDefined()) {
        if ((ScreenLandscape && ScreenSizeX > 800) || ScreenSizeX > 480) {
            setFileSuffix(pSuffixStart, {1920, 1080});
            hWelcomeBitmap = LoadSplashBitmap(srcfile);
        }
    }

    if (!hWelcomeBitmap.IsDefined()) {
        switch(ScreenGeometry) {
        case SCREEN_GEOMETRY_43:
            setFileSuffix(pSuffixStart, {640, 480});
	        break;
	    case SCREEN_GEOMETRY_53:
            setFileSuffix(pSuffixStart, {800, 480});
            break;
	    case SCREEN_GEOMETRY_169:
            setFileSuffix(pSuffixStart, {480, 272});
            break;
	    default:
	        break;
	    }
        hWelcomeBitmap = LoadSplashBitmap(srcfile);
    }

    return hWelcomeBitmap;
}


void DrawSplash(LKSurface& Surface, const RECT& rcDraw, const LKBitmap& Bmp) {
    PixelRect rc(rcDraw);
    Surface.Blackness(rc.left,rc.top,rc.GetSize().cx,rc.GetSize().cy);
    if(Bmp) {
        const PixelSize bmSize = Bmp.GetSize();
        const PixelScalar cx = rc.GetSize().cx;
        Surface.DrawBitmap(rc.left,rc.top,cx,_MulDiv<short>(bmSize.cy,cx,bmSize.cx),Bmp,bmSize.cx,bmSize.cy);
    }
}
