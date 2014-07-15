/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"
#include <shellapi.h>

void LoadSplash(HDC hDC, const TCHAR *splashfile){

 HBITMAP hWelcomeBitmap=NULL;
 TCHAR sDir[MAX_PATH];
 TCHAR srcfile[MAX_PATH];
 bool fullsize=true;
 TCHAR fprefix[20];

 _tcscpy(fprefix,splashfile);

 LocalPath(sDir,TEXT(LKD_BITMAPS));

    // first look for lkstart_480x272.bmp for example
    _stprintf(srcfile,_T("%s\\%s_%s.BMP"),sDir, fprefix,GetSizeSuffix() );
    if (!lk::filesystem::exist(srcfile)) {
        fullsize = false;
        switch (ScreenSize) {
            case ss800x480:
            case ss640x480:
            case ss720x408:
            case ss896x672:
                _stprintf(srcfile, _T("%s\\%s_LB.BMP"), sDir, fprefix);
                break;

            case ss480x272:
            case ss480x234:
            case ss400x240:
            case ss320x240:
                _stprintf(srcfile, _T("%s\\%s_LS.BMP"), sDir, fprefix);
                break;

            case ss480x640:
            case ss480x800:
                _stprintf(srcfile, _T("%s\\%s_PB.BMP"), sDir, fprefix);
                break;

            case ss240x320:
            case ss272x480:
                _stprintf(srcfile, _T("%s\\%s_PS.BMP"), sDir, fprefix);
                break;

            default:
                _stprintf(srcfile, _T("%s\\%s_LS.BMP"), sDir, fprefix);
                break;
        }
    }

 #if (WINDOWSPC>0)
 hWelcomeBitmap=(HBITMAP)LoadImage(GetModuleHandle(NULL),srcfile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
 #else
 hWelcomeBitmap=(HBITMAP)SHLoadDIBitmap(srcfile);
 #endif
 if (hWelcomeBitmap==NULL) hWelcomeBitmap=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SWIFT));

 HDC hTempDC = CreateCompatibleDC(hDC);
 HBITMAP oldBitmap=(HBITMAP)SelectObject(hTempDC, hWelcomeBitmap);

 BITMAP bm;
 GetObject(hWelcomeBitmap,sizeof(bm), &bm);

 StretchBlt(hDC,0,0,
	ScreenSizeX,ScreenSizeY,
	hTempDC, 0, 0,
	2,2,
	BLACKNESS);

  if (fullsize) {
	BitBlt(hDC,0,0,bm.bmWidth,bm.bmHeight,hTempDC, 0, 0, SRCCOPY);
  } else {
  	if ( (bm.bmWidth >ScreenSizeX)||(bm.bmHeight>ScreenSizeY)) {
		StretchBlt(hDC,0,0,
			ScreenSizeX,ScreenSizeY-NIBLSCALE(35),
			hTempDC, 0, 0,
			bm.bmWidth,bm.bmHeight,
			SRCCOPY);
	} else {
  		if ( (bm.bmWidth <ScreenSizeX)||(bm.bmHeight<ScreenSizeY)) {
			StretchBlt(hDC,NIBLSCALE(20),0,
				ScreenSizeX-NIBLSCALE(40), ScreenSizeY-BottomSize-NIBLSCALE(20),
				hTempDC, 0, 0,
				bm.bmWidth,bm.bmHeight,
				SRCCOPY);
		} else {
			BitBlt(hDC,(ScreenSizeX-bm.bmWidth)/2,0,bm.bmWidth,IBLSCALE(260),hTempDC, 0, 0, SRCCOPY);
		}
	}
  }

  SelectObject(hTempDC, oldBitmap);
  DeleteDC(hTempDC);
  DeleteObject(hWelcomeBitmap);

}


