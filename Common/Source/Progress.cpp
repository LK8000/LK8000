/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "resource.h"
#include "LKObjects.h"
#include "RGB.h"

//
// The SPLASH screen for startup, shutdown and intermediate reloads
//

extern HINSTANCE _hInstance;      // The current instance


static HWND	hStartupWindow = NULL;
static bool	doinitprogress=true;

// New LK8000 Startup splash 
#define LKSTARTBOTTOMFONT MapWindowBoldFont



void CloseProgressDialog() {
   DestroyWindow(hStartupWindow);
   hStartupWindow=NULL;
   doinitprogress=true;
}



void CreateProgressDialog(const TCHAR* text) {

  static int yFontSize, xFontSize;

  if (doinitprogress) {

	doinitprogress=false;

	DWORD Style=0;
	Style = WS_CHILD | ES_MULTILINE | ES_CENTER | ES_READONLY | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    
	hStartupWindow=CreateWindow(TEXT("STATIC"), TEXT("\0"), Style, 0, 0, ScreenSizeX, ScreenSizeY, MainWindow.Handle(), NULL, _hInstance, NULL);
	if (hStartupWindow==NULL) {
		StartupStore(_T("***** CRITIC, no startup window!%s"),NEWLINE);
		return;
	}

	ShowWindow(hStartupWindow,SW_SHOWNORMAL);
	BringWindowToTop(hStartupWindow);

	// Load welcome screen bitmap
	LKBitmap hWelcomeBitmap;
	TCHAR sDir[MAX_PATH];
        TCHAR srcfile[MAX_PATH];
        LocalPath(sDir,TEXT(LKD_BITMAPS));

	// first look for lkstart_480x272.bmp for example
	_stprintf(srcfile,_T("%s\\LKSTART_%s.BMP"),sDir, GetSizeSuffix() );

     if ( !lk::filesystem::exist(srcfile) ) {
		// no custom file, get a generic one
		switch(ScreenSize) {
			case ss800x480:
			case ss640x480:
			case ss720x408:
			case ss896x672:
				_stprintf(srcfile,_T("%s\\LKSTART_LB.BMP"),sDir);
				break;

			case ss480x272:
			case ss480x234:
			case ss400x240:
			case ss320x240:
				_stprintf(srcfile,_T("%s\\LKSTART_LS.BMP"),sDir);
				break;

			case ss480x640:
			case ss480x800:
				_stprintf(srcfile,_T("%s\\LKSTART_PB.BMP"),sDir);
				break;

			case ss240x320:
			case ss272x480:
				_stprintf(srcfile,_T("%s\\LKSTART_PS.BMP"),sDir);
				break;

			default:
				_stprintf(srcfile,_T("%s\\LKSTART_DEFAULT.BMP"),sDir);
				break;
		}
	}
    if(!hWelcomeBitmap.LoadFromFile(srcfile)) {
        // still nothing? use internal (poor man) resource
        hWelcomeBitmap.LoadFromResource(MAKEINTRESOURCE(IDB_SWIFT));
    }

    LKWindowSurface hStartupDC(hStartupWindow);

    const auto oldFont = hStartupDC.SelectObject(LKSTARTBOTTOMFONT);
	SIZE TextSize;
    hStartupDC.GetTextSize(_T("X"),1, &TextSize);
    yFontSize = TextSize.cy;
    xFontSize = TextSize.cx;

    hStartupDC.Blackness(0,0,ScreenSizeX,ScreenSizeY);
    
    const SIZE bmpSize = hWelcomeBitmap.GetSize();
    
	if ( (bmpSize.cx >ScreenSizeX)||(bmpSize.cy>ScreenSizeY)) {
        hStartupDC.DrawBitmap(0,0, ScreenSizeX,ScreenSizeY-NIBLSCALE(2)-(yFontSize*2)-1, hWelcomeBitmap, bmpSize.cx, bmpSize.cy);
	} else {
		hStartupDC.DrawBitmap((ScreenSizeX-bmpSize.cx)/2,0,bmpSize.cx, bmpSize.cy, hWelcomeBitmap, bmpSize.cx, bmpSize.cy);
	}
	hStartupDC.SelectObject(oldFont);
  }

  BringWindowToTop(hStartupWindow); // we shall return here also on shutdown and file reloads

  // RECT is left, top, right, bottom
  RECT PrintAreaR; 
  PrintAreaR.left   = NIBLSCALE(2);
  PrintAreaR.bottom = ScreenSizeY-NIBLSCALE(2);
  PrintAreaR.top    = PrintAreaR.bottom - (yFontSize*2);
  PrintAreaR.right  = ScreenSizeX - NIBLSCALE(2);

  LKWindowSurface hStartupDC(hStartupWindow);
 
  const auto oldFont = hStartupDC.SelectObject(LKSTARTBOTTOMFONT);

  const LKBrush &hB = LKBrush_Petrol;
  hStartupDC.FillRect(&PrintAreaR, hB);

  // Create text area

  // we cannot use LKPen here because they are not still initialised for startup menu. no problem
  LKPen hP(PEN_SOLID,NIBLSCALE(1),RGB_GREEN);
  auto ohP = hStartupDC.SelectObject(hP);
  const auto ohB = hStartupDC.SelectObject(hB);
  hStartupDC.Rectangle(PrintAreaR.left,PrintAreaR.top,PrintAreaR.right,PrintAreaR.bottom);
  hStartupDC.SelectObject(ohP);
  hP.Release();

  hP.Create(PEN_SOLID,NIBLSCALE(1),RGB_BLACK);
  ohP = hStartupDC.SelectObject(hP);
  hStartupDC.Rectangle(PrintAreaR.left+NIBLSCALE(2),PrintAreaR.top+NIBLSCALE(2),PrintAreaR.right-NIBLSCALE(2),PrintAreaR.bottom-NIBLSCALE(2));

  hStartupDC.SetTextColor(RGB_WHITE);
  hStartupDC.SetBkMode(TRANSPARENT);

  unsigned int maxchars= (ScreenSizeX/xFontSize)-1;
  if (_tcslen(text) <maxchars) {
	maxchars=_tcslen(text);
  }
  hStartupDC.DrawText(PrintAreaR.left+(xFontSize/2),PrintAreaR.top + ((PrintAreaR.bottom - PrintAreaR.top)/2)-(yFontSize/2),text,maxchars);

  hStartupDC.SelectObject(ohB);
  hStartupDC.SelectObject(ohP);
  hStartupDC.SelectObject(oldFont);
}
