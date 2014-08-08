/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include "externs.h"
#include "resource.h"
#include "Message.h"
#include "LKObjects.h"
#include "RGB.h"

//
// The SPLASH screen for startup, shutdown and intermediate reloads
//

static HWND	hStartupWindow = NULL;
static HDC	hStartupDC = NULL;
static bool	doinitprogress=true;

// New LK8000 Startup splash 
#define LKSTARTBOTTOMFONT MapWindowBoldFont


//
// We dont send messages to Progress in LK
//
#if 0 
LRESULT CALLBACK Progress(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;            // structure for paint info
  HDC hDC;
  RECT rc;
  (void)lParam;
  switch (message) {
	case WM_INITDIALOG:
		#if (WINDOWSPC>0)
		GetClientRect(hDlg, &rc);
		MoveWindow(hDlg, 0, 0, rc.right-rc.left, rc.bottom-rc.top, TRUE);
		#endif
		return TRUE;
	case WM_ERASEBKGND:
		hDC = BeginPaint(hDlg, &ps);
		SelectObject(hDC, GetStockObject(WHITE_PEN));
		SelectObject(hDC, GetStockObject(BLACK_BRUSH));
		GetClientRect(hDlg, &rc);
		Rectangle(hDC, rc.left,rc.top,rc.right,rc.bottom);
		DeleteDC(hDC);
		EndPaint(hDlg, &ps);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
  }
  return FALSE;
}
#endif


void CloseProgressDialog() {
   ReleaseDC(hWndMainWindow,hStartupDC); 
   DestroyWindow(hStartupWindow);
   hStartupWindow=NULL;
   doinitprogress=true;
}



HWND CreateProgressDialog(const TCHAR* text) {

  static int yFontSize, xFontSize;
  HDC	hTempDC = NULL;


  if (doinitprogress) {

	doinitprogress=false;

	DWORD Style=0;
	Style = WS_CHILD | ES_MULTILINE | ES_CENTER | ES_READONLY | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	hStartupWindow=CreateWindow(TEXT("STATIC"), TEXT("\0"), Style, 0, 0, ScreenSizeX, ScreenSizeY, hWndMainWindow, NULL, hInst, NULL);
	if (hStartupWindow==NULL) {
		StartupStore(_T("***** CRITIC, no startup window!%s"),NEWLINE);
		return NULL;
	}
	if (!(hStartupDC = GetDC(hStartupWindow))) {
		StartupStore(_T("------ Cannot state startup window%s"),NEWLINE);
		return(NULL);
	}
/*
	SHFullScreen(hProgress, SHFS_HIDETASKBAR |SHFS_HIDESIPBUTTON |SHFS_HIDESTARTICON);
	SetWindowPos(hProgress,HWND_TOP,0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
*/
	//RECT rt;
	//GetClientRect(hStartupWindow,&rt);
	//FillRect(hStartupDC,&rt,(HBRUSH)GetStockObject(BLACK_BRUSH));
	//SetWindowPos(hStartupWindow,HWND_TOP,0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
	//SHFullScreen(hStartupWindow, SHFS_HIDETASKBAR |SHFS_HIDESIPBUTTON |SHFS_HIDESTARTICON); 
	//SetForegroundWindow(hStartupWindow);
	//UpdateWindow(hStartupWindow);

	ShowWindow(hStartupWindow,SW_SHOWNORMAL);
	BringWindowToTop(hStartupWindow);

	// Load welcome screen bitmap
	HBITMAP hWelcomeBitmap=NULL;
	TCHAR sDir[MAX_PATH];
        TCHAR srcfile[MAX_PATH];
        LocalPath(sDir,TEXT(LKD_BITMAPS));

	// first look for lkstart_480x272.bmp for example
	_stprintf(srcfile,_T("%s\\LKSTART_%s.BMP"),sDir, GetSizeSuffix() );

        if (  GetFileAttributes(srcfile) == 0xffffffff ) {
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
	#if (WINDOWSPC>0)
	hWelcomeBitmap=(HBITMAP)LoadImage(GetModuleHandle(NULL),srcfile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	#else
	hWelcomeBitmap=(HBITMAP)SHLoadDIBitmap(srcfile);
	#endif
	// still nothing? use internal (poor man) resource
	if (hWelcomeBitmap==NULL)
		hWelcomeBitmap=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SWIFT));

	hTempDC = CreateCompatibleDC(hStartupDC);

	// AA
	HBITMAP oldBitmap = (HBITMAP)SelectObject(hTempDC, hWelcomeBitmap);
    HFONT   oldFont = (HFONT) SelectObject(hTempDC, LKSTARTBOTTOMFONT);
	SIZE TextSize;
        GetTextExtentPoint(hTempDC, _T("X"),1, &TextSize);
        yFontSize = TextSize.cy;
        xFontSize = TextSize.cx;

	BITMAP bm;
	GetObject(hWelcomeBitmap,sizeof(bm), &bm);

	StretchBlt(hStartupDC,0,0, 
		ScreenSizeX,ScreenSizeY-1, 
		hTempDC, 0, 0, 
		2,2,
		BLACKNESS);

	if ( (bm.bmWidth >ScreenSizeX)||(bm.bmHeight>ScreenSizeY)) {
		StretchBlt(hStartupDC,0,0, 
			ScreenSizeX,ScreenSizeY-NIBLSCALE(2)-(yFontSize*2)-1, 
			hTempDC, 0, 0, 
			bm.bmWidth,bm.bmHeight,
			SRCCOPY);
	} else {
		BitBlt(hStartupDC,(ScreenSizeX-bm.bmWidth)/2,0,bm.bmWidth,bm.bmHeight,hTempDC, 0, 0, SRCCOPY);
	}



	// AA


	SelectObject(hTempDC, oldBitmap);
	SelectObject(hTempDC, oldFont);
	DeleteObject(hWelcomeBitmap);
	if (DeleteDC(hTempDC)==0) StartupStore(_T("**** Cannot delete hTempDC\n"));
  }

  BringWindowToTop(hStartupWindow); // we shall return here also on shutdown and file reloads

  // RECT is left, top, right, bottom
  RECT PrintAreaR; 
  PrintAreaR.left   = NIBLSCALE(2);
  PrintAreaR.bottom = ScreenSizeY-NIBLSCALE(2);
  PrintAreaR.top    = PrintAreaR.bottom - (yFontSize*2);
  PrintAreaR.right  = ScreenSizeX - NIBLSCALE(2);

  HFONT oldFont=(HFONT)SelectObject(hStartupDC,LKSTARTBOTTOMFONT);

  HBRUSH hB=LKBrush_Petrol;
  FillRect(hStartupDC,&PrintAreaR, hB);

  // Create text area

  // we cannot use LKPen here because they are not still initialised for startup menu. no problem
  HPEN hP=(HPEN)  CreatePen(PS_SOLID,NIBLSCALE(1),RGB_GREEN);
  HPEN ohP = (HPEN)SelectObject(hStartupDC,hP);
  HBRUSH ohB = (HBRUSH)SelectObject(hStartupDC,hB);
  Rectangle(hStartupDC, PrintAreaR.left,PrintAreaR.top,PrintAreaR.right,PrintAreaR.bottom);
  SelectObject(hStartupDC,ohP);
  DeleteObject(hP);

  hP=(HPEN)  CreatePen(PS_SOLID,NIBLSCALE(1),RGB_BLACK);
  ohP = (HPEN)SelectObject(hStartupDC,hP);
  Rectangle(hStartupDC, PrintAreaR.left+NIBLSCALE(2),PrintAreaR.top+NIBLSCALE(2),PrintAreaR.right-NIBLSCALE(2),PrintAreaR.bottom-NIBLSCALE(2));

  SetTextColor(hStartupDC,RGB_WHITE);
  SetBkMode(hStartupDC,TRANSPARENT);

  unsigned int maxchars= (ScreenSizeX/xFontSize)-1;
  if (_tcslen(text) <maxchars) {
	maxchars=_tcslen(text);
  }
  ExtTextOut(hStartupDC,PrintAreaR.left+(xFontSize/2),PrintAreaR.top + ((PrintAreaR.bottom - PrintAreaR.top)/2)-(yFontSize/2),
	ETO_OPAQUE,NULL,text,maxchars,NULL);

  SelectObject(hStartupDC,ohB);
  SelectObject(hStartupDC,ohP);
  SelectObject(hStartupDC,oldFont);
  // Sleep(300); // Slow down display of data? No because in case of important things, Sleep is set by calling part

  DeleteObject(hP);


  return hStartupWindow;
}




