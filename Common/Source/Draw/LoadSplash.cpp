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

void LoadSplash(LKSurface& Surface, const TCHAR *splashfile){

 LKBitmap hWelcomeBitmap;
 TCHAR sDir[MAX_PATH];
 TCHAR srcfile[MAX_PATH];
 TCHAR fprefix[20];

 _tcscpy(fprefix,splashfile);

 SystemPath(sDir,TEXT(LKD_BITMAPS));

    // first look for lkstart_480x272.bmp for example
    _stprintf(srcfile,_T("%s" DIRSEP "%s_%s." IMG_EXT),sDir, fprefix,GetSizeSuffix() );
    if (!lk::filesystem::exist(srcfile)) {
        switch(ScreenGeometry) {
            case SCREEN_GEOMETRY_43:
            if (ScreenLandscape)
                _stprintf(srcfile,_T("%s" DIRSEP "%s_640x480." IMG_EXT),sDir, fprefix );
            else
                _stprintf(srcfile,_T("%s" DIRSEP "%s_480x640." IMG_EXT),sDir, fprefix );
	        break;
	    case SCREEN_GEOMETRY_53:
            if (ScreenLandscape)
                _stprintf(srcfile,_T("%s" DIRSEP "%s_800x480." IMG_EXT),sDir, fprefix );
            else
                _stprintf(srcfile,_T("%s" DIRSEP "%s_480x800." IMG_EXT),sDir, fprefix );
                break;
	    case SCREEN_GEOMETRY_169:
            if (ScreenLandscape)
                _stprintf(srcfile,_T("%s" DIRSEP "%s_480x272." IMG_EXT),sDir, fprefix );
            else
                _stprintf(srcfile,_T("%s" DIRSEP "%s_272x480." IMG_EXT),sDir, fprefix );
                break;
	    default:
	        break;
    	}
    }

    if(hWelcomeBitmap.LoadFromFile(srcfile) && hWelcomeBitmap) {
        const PixelSize bmSize = hWelcomeBitmap.GetSize();

        Surface.Blackness(0,0,ScreenSizeX,ScreenSizeY);
        Surface.DrawBitmap(0,0,ScreenSizeX,MulDiv(bmSize.cy,ScreenSizeX,bmSize.cx),hWelcomeBitmap,bmSize.cx,bmSize.cy);
    }
}
