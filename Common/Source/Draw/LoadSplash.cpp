/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"

#ifdef USE_GDI
#define IMG_EXT "BMP"
#else
#define IMG_EXT "PNG"
#endif

void LoadSplash(LKSurface& Surface, const TCHAR *splashfile){

 LKBitmap hWelcomeBitmap;
 TCHAR sDir[MAX_PATH];
 TCHAR srcfile[MAX_PATH];
 bool fullsize=true;
 TCHAR fprefix[20];

 _tcscpy(fprefix,splashfile);

 LocalPath(sDir,TEXT(LKD_BITMAPS));

    // first look for lkstart_480x272.bmp for example
    _stprintf(srcfile,_T("%s" DIRSEP "%s_%s." IMG_EXT),sDir, fprefix,GetSizeSuffix() );
    if (!lk::filesystem::exist(srcfile)) {
        fullsize = false;
        
        // Get correct splash bitmap size and orientation
        switch (ScreenSize) {
            case ss800x480:
            case ss640x480:
            case ss720x408:
            case ss896x672:
                _stprintf(srcfile, _T("%s" DIRSEP "%s_LB." IMG_EXT), sDir, fprefix);
                break;

            case ss480x272:
            case ss480x234:
            case ss400x240:
            case ss320x240:
                _stprintf(srcfile, _T("%s" DIRSEP "%s_LS." IMG_EXT), sDir, fprefix);
                break;

            case ss480x640:
            case ss480x800:
                _stprintf(srcfile, _T("%s" DIRSEP "%s_PB." IMG_EXT), sDir, fprefix);
                break;

            case ss240x320:
            case ss272x480:
                _stprintf(srcfile, _T("%s" DIRSEP "%s_PS." IMG_EXT), sDir, fprefix);
                break;

            default:
                // Screen undefined
                
                if (ScreenSizeX>ScreenSizeY) {
                    // Landscape
                    if (ScreenSizeY<400)
                        _stprintf(srcfile, _T("%s" DIRSEP "%s_LS." IMG_EXT), sDir, fprefix);
                    else
                        _stprintf(srcfile, _T("%s" DIRSEP "%s_LB." IMG_EXT), sDir, fprefix);
                } else {
                    // Portrait
                   if (ScreenSizeX<400)
                        _stprintf(srcfile, _T("%s" DIRSEP "%s_PS." IMG_EXT), sDir, fprefix);
                    else
                        _stprintf(srcfile, _T("%s" DIRSEP "%s_PB." IMG_EXT), sDir, fprefix);
                }
                break;
        }
    }

    if(hWelcomeBitmap.LoadFromFile(srcfile) && hWelcomeBitmap) {
        const PixelSize bmSize = hWelcomeBitmap.GetSize();

        Surface.Blackness(0,0,ScreenSizeX,ScreenSizeY);

        if (fullsize) {
            Surface.DrawBitmap(0,0,bmSize.cx,bmSize.cy,hWelcomeBitmap,bmSize.cx,bmSize.cy);
        } else if ( (bmSize.cx > ScreenSizeX)||(bmSize.cy > ScreenSizeY)) {
            Surface.DrawBitmap(0,0,ScreenSizeX,ScreenSizeY-NIBLSCALE(35),hWelcomeBitmap,bmSize.cx,bmSize.cy);
        } else if ( (bmSize.cx < ScreenSizeX)||(bmSize.cy < ScreenSizeY)) {
            Surface.DrawBitmap(NIBLSCALE(20),0,ScreenSizeX-NIBLSCALE(40), ScreenSizeY-BottomSize-NIBLSCALE(20),hWelcomeBitmap,bmSize.cx,bmSize.cy);
         } else {
            Surface.DrawBitmap((ScreenSizeX-bmSize.cx)/2,0,bmSize.cx,IBLSCALE(260),hWelcomeBitmap,bmSize.cx,bmSize.cy);
        }
    }
}
