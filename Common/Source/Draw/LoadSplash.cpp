/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"
#include "ScreenGeometry.h"

#ifdef USE_GDI
#define IMG_EXT "BMP"
#else
#define IMG_EXT "PNG"
#endif

LKBitmap LoadSplash(const TCHAR *splashfile) {

#ifdef ANDROID
    const TCHAR* sDir = _T(LKD_BITMAPS);
#else
    TCHAR sDir[MAX_PATH];
    SystemPath(sDir,_T(LKD_BITMAPS));
#endif


    TCHAR srcfile[MAX_PATH];

    LKBitmap hWelcomeBitmap;

    // first look for lkstart_480x272.bmp for example
    _stprintf(srcfile,_T("%s" DIRSEP "%s_%s." IMG_EXT),sDir, splashfile,GetSizeSuffix() );
#ifdef ANDROID
    hWelcomeBitmap.LoadAssetsFile(srcfile);
#else
    hWelcomeBitmap.LoadFromFile(srcfile);
#endif

    if (!hWelcomeBitmap.IsDefined()) {
        if (ScreenLandscape) {
            if (ScreenSizeX > 800) {
                _stprintf(srcfile, _T("%s" DIRSEP "%s_1920x1080." IMG_EXT), sDir, splashfile);
#ifdef ANDROID
                hWelcomeBitmap.LoadAssetsFile(srcfile);
#else
                hWelcomeBitmap.LoadFromFile(srcfile);
#endif
            }
        }else {
            if (ScreenSizeX > 480) {
                _stprintf(srcfile, _T("%s" DIRSEP "%s_1080x1920." IMG_EXT), sDir, splashfile);
#ifdef ANDROID
                hWelcomeBitmap.LoadAssetsFile(srcfile);
#else
                hWelcomeBitmap.LoadFromFile(srcfile);
#endif
            }
        }
    }

    if (!hWelcomeBitmap.IsDefined()) {
        switch(ScreenGeometry) {
            case SCREEN_GEOMETRY_43:
            if (ScreenLandscape)
                _stprintf(srcfile,_T("%s" DIRSEP "%s_640x480." IMG_EXT),sDir, splashfile );
            else
                _stprintf(srcfile,_T("%s" DIRSEP "%s_480x640." IMG_EXT),sDir, splashfile );
	        break;
	    case SCREEN_GEOMETRY_53:
            if (ScreenLandscape)
                _stprintf(srcfile,_T("%s" DIRSEP "%s_800x480." IMG_EXT),sDir, splashfile );
            else
                _stprintf(srcfile,_T("%s" DIRSEP "%s_480x800." IMG_EXT),sDir, splashfile );
                break;
	    case SCREEN_GEOMETRY_169:
            if (ScreenLandscape)
                _stprintf(srcfile,_T("%s" DIRSEP "%s_480x272." IMG_EXT),sDir, splashfile );
            else
                _stprintf(srcfile,_T("%s" DIRSEP "%s_272x480." IMG_EXT),sDir, splashfile );
                break;
	    default:
	        break;
	    }
#ifdef ANDROID
        hWelcomeBitmap.LoadAssetsFile(srcfile);
#else
        hWelcomeBitmap.LoadFromFile(srcfile);
#endif
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
