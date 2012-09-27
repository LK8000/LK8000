/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "LKInterface.h"
#include "resource.h"
#include "Waypointparser.h"
#include "InfoBoxLayout.h"
#include "LKProfiles.h"

#include <commctrl.h>
#include <aygshell.h>
#if (WINDOWSPC<1)
#include <sipapi.h>
#endif

#include "InputEvents.h"
#include "Message.h"

#include "Bitmaps.h"
#include "LKObjects.h"
#include "DoInits.h"


extern HBRUSH hBrushSelected;
extern HBRUSH hBrushUnselected;
extern HBRUSH hBrushButton;
#ifdef LXMINIMAP
extern HBRUSH hBrushButtonHasFocus;
#endif
extern COLORREF ColorSelected;
extern COLORREF ColorUnselected;
extern COLORREF ColorWarning;
extern COLORREF ColorOK;
extern COLORREF ColorButton;
#ifdef LXMINIMAP
extern COLORREF ColorButtonHasFocus;
#endif



ATOM	MyRegisterClass (HINSTANCE, LPTSTR);
BOOL	InitInstance    (HINSTANCE, int);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);


extern void FillDataOptions(void);
extern void StartupLogFreeRamAndStorage();
extern void CreateCalculationThread();


void PreloadInitialisation(bool ask) {
  LKLanguageReady=false;
  LKReadLanguageFile();
  FillDataOptions(); // Load infobox list

  if (ask) {
    // Load default profile and status file: we are at an early stage
    LKProfileResetDefault();
    LKProfileLoad(startAircraftFile);
    LKProfileLoad(startPilotFile);
    // if DEFAULT PROFILE does not exist, initialize ResetDefaults!
    // This is because LKProfileLoad will do this at its end, normally.
    // Notice: aircraft and pilot files will not be overridden by defaults
    if (!LKProfileLoad(startProfileFile)) {
	LKProfileInitRuntime();
    }

    StatusFileInit();
  } else {
    // We are in the dialog startup phase
    FullScreen();
    short retstartup;
    do {
	retstartup=dlgStartupShowModal();
    } while (retstartup>0);

    if (retstartup<0) return;

    if (_tcscmp(startProfileFile,_T("PROFILE_RESET"))==0) {
	StartupStore(_T(". USER ASKED FOR PROFILE FULL RESET!%s"),NEWLINE);
	DoStatusMessage(gettext(_T("_@M1757_"))); // LK8000 PROFILES RESET
	LKProfileResetDefault();
	LKProfileInitRuntime();
	// Notice: this is also resetting the default Aircraft and Pilot profiles to demo settings
    } else  {
	if (!LKProfileLoad(startPilotFile)) {
		#if TESTBENCH
		StartupStore(_T(". PilotFile RESET to defaults%s"),NEWLINE);
		#endif
	}
	if (!LKProfileLoad(startAircraftFile)) {
		#if TESTBENCH
		StartupStore(_T(". AircraftFile RESET to defaults%s"),NEWLINE);
		#endif
	}
	LKProfileLoad(startProfileFile); // this is calling adjust and InitRuntime itself
    }

    // LKTOKEN _@M1206_ "Initialising..."
	CreateProgressDialog(gettext(TEXT("_@M1206_"))); 
  }

  // Interface (before interface)
  if (!ask) {
    LKReadLanguageFile();
    InputEvents::readFile();
  }

}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{

  WNDCLASS wc;
  WNDCLASS dc;

  GetClassInfo(hInstance,TEXT("DIALOG"),&dc);

   wc.style                      = CS_HREDRAW | CS_VREDRAW;
//  wc.style                      = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // VENTA3 NO USE
  wc.lpfnWndProc                = (WNDPROC) WndProc;
  wc.cbClsExtra                 = 0;
#if (WINDOWSPC>0)
  wc.cbWndExtra = 0;
#else
  wc.cbWndExtra                 = dc.cbWndExtra ;
#endif
  wc.hInstance                  = hInstance;
  wc.hIcon                      = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XCSOARSWIFT));
  wc.hCursor                    = 0;
  wc.hbrBackground              = (HBRUSH) GetStockObject(BLACK_BRUSH); 
  wc.lpszMenuName               = 0;
  wc.lpszClassName              = szWindowClass;

  if (!RegisterClass (&wc))
    return FALSE;

  // disabling DBLCLK here will make it not working in map window and maplocking failure
  wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)MapWindow::MapWndProc;
  wc.cbClsExtra = 0;

#if (WINDOWSPC>0)
  wc.cbWndExtra = 0 ;
#else
  wc.cbWndExtra = dc.cbWndExtra ;
#endif

  wc.hInstance = hInstance;
  wc.hIcon = (HICON)NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH); // refixed 100101
  wc.lpszMenuName = 0;
  wc.lpszClassName = TEXT("MapWindowClass");

  return RegisterClass(&wc);

}


//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  TCHAR szTitle[MAX_LOADSTRING];                        // The title bar text
  TCHAR szWindowClass[MAX_LOADSTRING];                  // The window class name
  RECT rc;

  hInst = hInstance;            // Store instance handle in our global variable


  LoadString(hInstance, IDC_XCSOAR, szWindowClass, MAX_LOADSTRING);
  LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

  // If it is already running, then focus on the window
  // problem was  that if two instances are started within a few seconds, both will survive!
  // We enforceed this with mutex at the beginning of WinMain
  hWndMainWindow = FindWindow(szWindowClass, szTitle);
  if (hWndMainWindow)
    {
      SetForegroundWindow((HWND)((ULONG) hWndMainWindow | 0x00000001));
      return 0;
    }
  InitLKScreen();
  InitLKFonts(); // causing problems with CreateButtonLabels?
  PreloadInitialisation(true);

  MyRegisterClass(hInst, szWindowClass);

  RECT WindowSize;

  WindowSize.left = 0;
  WindowSize.top = 0;
  WindowSize.right = GetSystemMetrics(SM_CXSCREEN);
  WindowSize.bottom = GetSystemMetrics(SM_CYSCREEN);


#if (WINDOWSPC>0)
  WindowSize.right = SCREENWIDTH 
    + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.left = (GetSystemMetrics(SM_CXSCREEN) - WindowSize.right) / 2;
  WindowSize.bottom = SCREENHEIGHT 
    + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  WindowSize.top = (GetSystemMetrics(SM_CYSCREEN) - WindowSize.bottom) / 2;
  /*
  //
  // Custom setup for positioning the window , ready to be used
  //
  WindowSize.top=768;	// top and left corner coords
  WindowSize.left=1024;
  WindowSize.right = SCREENWIDTH + 2*GetSystemMetrics( SM_CXFIXEDFRAME);
  WindowSize.bottom = SCREENHEIGHT + 2*GetSystemMetrics( SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
  */
#endif

  if (!goInstallSystem) Sleep(50); // 091119
  #if TESTBENCH
  StartupStore(TEXT(". Create main window%s"),NEWLINE);
  #endif

  hWndMainWindow = CreateWindow(szWindowClass, szTitle,
                                WS_SYSMENU
                                | WS_CLIPCHILDREN
				| WS_CLIPSIBLINGS,
                                WindowSize.left, WindowSize.top,
				WindowSize.right, WindowSize.bottom,
                                NULL, NULL,
				hInstance, NULL);

  if (!hWndMainWindow)
    {
      return FALSE;
    }


  hBrushSelected = (HBRUSH)CreateSolidBrush(ColorSelected);
  hBrushUnselected = (HBRUSH)CreateSolidBrush(ColorUnselected);
  hBrushButton = (HBRUSH)CreateSolidBrush(ColorButton);
  #ifdef LXMINIMAP
  hBrushButtonHasFocus = (HBRUSH)CreateSolidBrush(ColorButtonHasFocus);
  #endif

  GetClientRect(hWndMainWindow, &rc);

#if (WINDOWSPC>0)
  rc.left = 0;
  rc.right = SCREENWIDTH;
  rc.top = 0;
  rc.bottom = SCREENHEIGHT;
#endif


  LKLoadFixedBitmaps();
  LKLoadProfileBitmaps();
  LKObjects_Create(); 


  ButtonLabel::CreateButtonLabels(rc);
  ButtonLabel::SetLabelText(0,TEXT("MODE"));

  extern void InitialiseFonts(RECT rc);
  InitialiseFonts(rc);
  InitLKFonts();	// reload updating LK fonts after loading profile for fontquality

  ButtonLabel::SetFont(MapWindowBoldFont);

  Message::Initialize(rc); // creates window, sets fonts

  ShowWindow(hWndMainWindow, SW_SHOW);
  #if TESTBENCH
  StartupStore(TEXT(". Create map window%s"),NEWLINE);
  #endif

  hWndMapWindow = CreateWindow(TEXT("MapWindowClass"),NULL,
			       WS_VISIBLE | WS_CHILD
			       | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               0, 0, (rc.right - rc.left),
			       (rc.bottom-rc.top) ,
                               hWndMainWindow, NULL ,hInstance,NULL);

  ShowWindow(hWndMainWindow, nCmdShow);

  UpdateWindow(hWndMainWindow);

  // Since MapWndProc is doing static inits, we want them to be recalculated at the end of 
  // initializations, since some values in use might have been not available yet, for example BottomSize.
  Reset_Single_DoInits(MDI_MAPWNDPROC);
    
  return TRUE;
}



