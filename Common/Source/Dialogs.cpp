/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Dialogs.cpp,v 8.3 2010/12/10 23:57:13 root Exp root $

*/

#include "StdAfx.h"

#include <commdlg.h>
#include <commctrl.h>
#include "aygshell.h"

#include "compatibility.h"

#include "Dialogs.h"
#include "Logger.h"
#include "resource.h"
#include "Utils.h"
#include "Utils2.h"
#include "externs.h"
#include "Port.h"
#include "AirfieldDetails.h"
#include "device.h"
#include "Units.h"
#include "InputEvents.h"
#include "Message.h"
#include "LKObjects.h"

#ifdef DEBUG_TRANSLATIONS
#include <map>

static GetTextSTRUCT GetTextData[MAXSTATUSMESSAGECACHE];
static int GetTextData_Size = 0;
#endif

#include "utils/heapcheck.h"

void ReadWayPoints(void);
void ReadAirspace(void);
int FindIndex(HWND hWnd);
void ReadNewTask(HWND hDlg);


LRESULT CALLBACK Progress(HWND hDlg, UINT message, 
                          WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;            // structure for paint info
  HDC hDC;
  RECT rc;
  (void)lParam;
  switch (message)
    {
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
      if (LOWORD(wParam) == IDOK)
        {
          EndDialog(hDlg, LOWORD(wParam));
          return TRUE;
        }
      break;
    }
  return FALSE;
}


// ARH: Status Message functions
// Used to show a brief status message to the user
// Could be used to display debug messages
// or info messages like "Map panning OFF"

// Each instance of the StatusMessage window has some
// unique data associated with it, rather than using
// global variables.  This allows multiple instances
// in a thread-safe manner.
class CStatMsgUserData {
public:
  HFONT   hFont;
  WNDPROC fnOldWndProc;
  BOOL    bCapturedMouse;
  DWORD   texpiry; //
  
  // Initialize to sensible values
  CStatMsgUserData()
    : hFont(NULL), fnOldWndProc(NULL), bCapturedMouse(FALSE), texpiry(0) {};
  
  // Clean up mess
  ~CStatMsgUserData() {
    if (hFont) {
      DeleteObject(hFont);
      hFont = NULL;
    }
    fnOldWndProc = NULL;
  }
};


bool forceDestroyStatusMessage = false;

void ClearStatusMessages(void) {
  forceDestroyStatusMessage = true;
}


// Intercept messages destined for the Status Message window
LRESULT CALLBACK StatusMsgWndTimerProc(HWND hwnd, UINT message, 
				       WPARAM wParam, LPARAM lParam)
{

  CStatMsgUserData *data;
  POINT pt;
  RECT  rc;

  // Grab hold of window specific data
  data = (CStatMsgUserData*) GetWindowLong(hwnd, GWL_USERDATA);

/*
  if (data==NULL) {
    // Something wrong here!
    DestroyWindow(hwnd);  // ups
    return 1;
  }
*/

  switch (message) {
  case WM_LBUTTONDOWN:

    if (data==NULL) {
      // Something wrong here!
      DestroyWindow(hwnd);  // ups
      return 1;
    }

    // Intercept mouse messages while stylus is being dragged
    // This is necessary to simulate a WM_LBUTTONCLK event
    SetCapture(hwnd);
    data->bCapturedMouse = TRUE;
    return 0;
  case WM_LBUTTONUP :

    if (data==NULL) {
      // Something wrong here!
      DestroyWindow(hwnd);  // ups
      return 1;
    }

    //if (data->bCapturedMouse) ReleaseCapture();
    ReleaseCapture();

    if (!data->bCapturedMouse) return 0;

    data->bCapturedMouse = FALSE;

    // Is stylus still within this window?
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    GetClientRect(hwnd, &rc);

    if (!PtInRect(&rc, pt)) return 0;

    DestroyWindow(hwnd);
    return 0;

  case WM_TIMER :

    // force destruction of window...
    if (forceDestroyStatusMessage) {
      DestroyWindow(hwnd);
      return 0;
    }

    if ((data->texpiry>0) && (::GetTickCount()>data->texpiry)) {
      DestroyWindow(hwnd);
    }
    return 0;
  case WM_DESTROY :

    // Clean up after ourselves
    if (data != NULL){
      delete data;
      // hack ... try to find execption point
      data = NULL;
      // Attach window specific data to the window
      SetWindowLong(hwnd, GWL_USERDATA, (LONG) data);
    }
    MapWindow::RequestFastRefresh();
    // JMW do this so airspace warning gets refreshed

    return 0;
  }

  // Pass message on to original window proc
  if (data != NULL)
    return CallWindowProc(data->fnOldWndProc, hwnd, message, wParam, lParam);
  else
    return(0);

}


// DoMessage is designed to delegate what to do for a message
// The "what to do" can be defined in a configuration file
// Defaults for each message include:
//	- Text to display (including multiple languages)
//	- Text to display extra - NOT multiple language
//		(eg: If Airspace Warning - what details - airfield name is in data file, already 
//		covers multiple languages).
//	- ShowStatusMessage - including font size and delay
//	- Sound to play - What sound to play
//	- Log - Keep the message on the log/history window (goes to log file and history)
//
// TODO code: (need to discuss) Consider moving almost all this functionality into AddMessage ?

void DoStatusMessage(const TCHAR* text, const TCHAR *data) {
  Message::Lock();

  StatusMessageSTRUCT LocalMessage;
  LocalMessage = StatusMessageData[0];

  int i;
  // Search from end of list (allow overwrites by user)
  for (i=StatusMessageData_Size - 1; i>0; i--) {
    if (wcscmp(text, StatusMessageData[i].key) == 0) {
      LocalMessage = StatusMessageData[i];
      break;
    }
  }
  
  if (EnableSoundModes && LocalMessage.doSound)
    PlayResource(LocalMessage.sound);
  
  // TODO code: consider what is a sensible size?
  TCHAR msgcache[1024];
  if (LocalMessage.doStatus) {
    
    wcscpy(msgcache, gettext(text));
    if (data != NULL) {
      wcscat(msgcache, TEXT(" "));
      wcscat(msgcache, data);
    }
    
    Message::AddMessage(LocalMessage.delay_ms, 1, msgcache);
  }

  Message::Unlock();
}


#ifdef DEBUG_TRANSLATIONS
/*

  WriteMissingTranslations - write all missing translations found
  during runtime to a lanuage file in data dir

*/
template<class _Ty> 
struct lessTCHAR: public std::binary_function<_Ty, _Ty, bool>
{	// functor for operator<
  bool operator()(const _Ty& _Left, const _Ty& _Right) const
  {	// apply operator< to operands
    return (_tcscmp(_Left, _Right) < 0);
  }
};

std::map<TCHAR*, TCHAR*, lessTCHAR<TCHAR*> > unusedTranslations;

void WriteMissingTranslations() {
  std::map<TCHAR*, TCHAR*, lessTCHAR<TCHAR*> >::iterator 
    s=unusedTranslations.begin(),e=unusedTranslations.end();

  TCHAR szFile1[MAX_PATH] = TEXT("%LOCAL_PATH%\\\\localization_todo.xcl\0");
  FILE *fp=NULL;

  ExpandLocalPath(szFile1);
  fp  = _tfopen(szFile1, TEXT("w+"));
  
  if (fp != NULL) {
    while (s != e) {
      TCHAR* p = (s->second);
      if (p) {
        while (*p) {
          if (*p != _T('\n')) {
            fwprintf(fp, TEXT("%c"), *p);
          } else {
            fwprintf(fp, TEXT("\\n"));
          }
          p++;
        }
        fwprintf(fp, TEXT("=\n"));
      }
      s++;
    }
    fclose(fp);
  }
}

#endif



/*  -------------------------------- start removable part
   LK8000 1.24 IS NOW USING TOKENIZED LANGUAGE SUPPORT SO WE CAN TRASH THE OLD APPROACH
   Last time I did tokenized stuff, it was year 1990 for french Minitel!! 
   Using AT&T 3B-1000 Unix 3.2 !!
   This comment will self destruct after 1/1/2011 Paolo

TCHAR* gettext(const TCHAR* text) {
  int i;
  if (wcscmp(text, L"") == 0) return (TCHAR*)text;

  //find a translation
  for (i=0; i<GetTextData_Size; i++) {
	if (!text || !GetTextData[i].key) continue;
	if (wcscmp(text, GetTextData[i].key) == 0)
	return GetTextData[i].text;
  }
  return (TCHAR*)text;
}

void SetWindowText_gettext(HWND hDlg, int entry) {
  TCHAR strTemp[1024];
  GetWindowText(GetDlgItem(hDlg,entry),strTemp, 1023);
  SetWindowText(GetDlgItem(hDlg,entry),gettext(strTemp));
#endif
}
-----------------------   end removable part */

static HWND	hStartupWindow = NULL;
static HDC	hStartupDC = NULL;

static HCURSOR oldCursor = NULL;
static bool doinitprogress=true;


void StartHourglassCursor(void) {
  HCURSOR newc = LoadCursor(NULL, IDC_WAIT);
  oldCursor = (HCURSOR)SetCursor(newc);
#if 0
  SetCursorPos(160,120);
#endif
}

void StopHourglassCursor(void) {
  SetCursor(oldCursor);
#if 0
  SetCursorPos(640,480);
#endif
  oldCursor = NULL;
}

void CloseProgressDialog() {
   ReleaseDC(hWndMainWindow,hStartupDC); 
   DestroyWindow(hStartupWindow);
   hStartupWindow=NULL;
   doinitprogress=true;
}

void StepProgressDialog(void) {
}

BOOL SetProgressStepSize(int nSize) {
  return(TRUE);
}

// New LK8000 Startup splash 
#define LKSTARTBOTTOMFONT MapWindowBoldFont

HWND CreateProgressDialog(TCHAR* text) {

  static int yFontSize, xFontSize;
  HDC	hTempDC = NULL;


  if (doinitprogress) {

	doinitprogress=false;

	DWORD Style=0;
	Style = WS_CHILD | ES_MULTILINE | ES_CENTER | ES_READONLY | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	hStartupWindow=CreateWindow(TEXT("STATIC"), TEXT("\0"), Style, 0, 0, ScreenSizeX, ScreenSizeY, hWndMainWindow, NULL, hInst, NULL);
	if (hStartupWindow==NULL) {
		StartupStore(_T("***** CRITIC, no startup window!%s"),NEWLINE);
		FailStore(_T("CRITIC, no startup window!"));
		return NULL;
	}
	if (!(hStartupDC = GetDC(hStartupWindow))) {
		StartupStore(_T("------ Cannot state startup window%s"),NEWLINE);
		FailStore(_T("Cannot state startup window"));
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
        SelectObject(hTempDC, LKSTARTBOTTOMFONT);
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


	DeleteObject(hWelcomeBitmap);

	// AA


	SelectObject(hTempDC, oldBitmap);
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
  SelectObject(hStartupDC,hP);
  SelectObject(hStartupDC,hB);
  Rectangle(hStartupDC, PrintAreaR.left,PrintAreaR.top,PrintAreaR.right,PrintAreaR.bottom);
  DeleteObject(hP);

  hP=(HPEN)  CreatePen(PS_SOLID,NIBLSCALE(1),RGB_BLACK);
  SelectObject(hStartupDC,hP);
  Rectangle(hStartupDC, PrintAreaR.left+NIBLSCALE(2),PrintAreaR.top+NIBLSCALE(2),PrintAreaR.right-NIBLSCALE(2),PrintAreaR.bottom-NIBLSCALE(2));

  SetTextColor(hStartupDC,RGB_WHITE);
  SetBkMode(hStartupDC,TRANSPARENT);

  unsigned int maxchars= (ScreenSizeX/xFontSize)-1;
  if (_tcslen(text) <maxchars) {
	maxchars=_tcslen(text);
  }
  ExtTextOut(hStartupDC,PrintAreaR.left+(xFontSize/2),PrintAreaR.top + ((PrintAreaR.bottom - PrintAreaR.top)/2)-(yFontSize/2),
	ETO_OPAQUE,NULL,text,maxchars,NULL);

  SelectObject(hStartupDC,oldFont);
  // Sleep(300); // Slow down display of data? No because in case of important things, Sleep is set by calling part

  DeleteObject(hP);


  return hStartupWindow;
}




