/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "resource.h"
#include "Waypointparser.h"
#include "InfoBoxLayout.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "Message.h"
#include "Geoid.h"
#include "InputEvents.h"
#include "RGB.h"
#include "LKProfiles.h"
#include "Logger.h"
#include "LiveTracker.h"
#include "Dialogs.h"

HBRUSH hBrushSelected;
HBRUSH hBrushUnselected;
HBRUSH hBrushButton;
#ifdef LXMINIMAP
HBRUSH hBrushButtonHasFocus;
#endif
COLORREF ColorSelected = RGB(0xC0,0xC0,0xC0);
COLORREF ColorUnselected = RGB_WHITE;
COLORREF ColorWarning = RGB_RED;
COLORREF ColorOK = RGB_BLUE;
COLORREF ColorButton = RGB_BUTTONS;  
#ifdef LXMINIMAP
COLORREF ColorButtonHasFocus=RGB_DARKYELLOW2;
#endif

static int iTimerID= 0;

#if (((UNDER_CE >= 300)||(_WIN32_WCE >= 0x0300)) && (WINDOWSPC<1))
#define HAVE_ACTIVATE_INFO
#include <aygshell.h>
extern SHACTIVATEINFO s_sai;
extern bool api_has_SHHandleWMActivate;
extern bool api_has_SHHandleWMSettingChange;
#endif

LRESULT	MainMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

extern void AfterStartup();
extern void Shutdown();
extern void StartupLogFreeRamAndStorage();
extern void SIMProcessTimer (void);
extern void ProcessTimer    (void);

HWND hWndWithFocus=NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  long wdata;

  switch (message)
    {

    case WM_ERASEBKGND:
      return TRUE; // JMW trying to reduce screen flicker
      break;
    case WM_COMMAND:
      return MainMenu(hWnd, message, wParam, lParam);
      break;
    case WM_CTLCOLORSTATIC:
      wdata = GetWindowLong((HWND)lParam, GWL_USERDATA);
      switch(wdata) {
      case 0:
        SetBkColor((HDC)wParam, ColorUnselected);
        SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
        return (LRESULT)hBrushUnselected;
      case 1:
        SetBkColor((HDC)wParam, ColorSelected);
        SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
        return (LRESULT)hBrushSelected;
      case 2:
	SetBkColor((HDC)wParam, ColorUnselected);
        SetTextColor((HDC)wParam, ColorWarning);
	return (LRESULT)hBrushUnselected;
      case 3:
	SetBkColor((HDC)wParam, ColorUnselected);
        SetTextColor((HDC)wParam, ColorOK);
	return (LRESULT)hBrushUnselected;
      case 4:
	// black on light green
        SetTextColor((HDC)wParam, RGB_BLACK); 
	SetBkColor((HDC)wParam, ColorButton);
	return (LRESULT)hBrushButton;
      case 5:
	// grey on light green
	SetBkColor((HDC)wParam, ColorButton);
        SetTextColor((HDC)wParam, RGB(0x80,0x80,0x80));
	return (LRESULT)hBrushButton;
#ifdef LXMINIMAP
      case 6:
        // black on dark yellow
        SetTextColor((HDC)wParam, RGB_BLACK);
        SetBkColor((HDC)wParam, ColorButtonHasFocus);
        return (LRESULT)hBrushButtonHasFocus;
      case 7:
        // grey on dark yellow
        SetTextColor((HDC)wParam, RGB(0x80,0x80,0x80));
        SetBkColor((HDC)wParam, ColorButtonHasFocus);
        return (LRESULT)hBrushButtonHasFocus;
#endif

      }
      break;
    case WM_CREATE:
#ifdef HAVE_ACTIVATE_INFO
      memset (&s_sai, 0, sizeof (s_sai));
      s_sai.cbSize = sizeof (s_sai);
#endif
      if (iTimerID == 0) {
        iTimerID = SetTimer(hWnd,1000,500,NULL); // 500ms  2 times per second
      }

      break;

    case WM_ACTIVATE:

      if(LOWORD(wParam) != WA_INACTIVE)
        {
          SetWindowPos(hWndMainWindow,HWND_TOP,
                 0, 0, 0, 0,
                 SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);

#ifdef HAVE_ACTIVATE_INFO
         SHFullScreen(hWndMainWindow,SHFS_HIDETASKBAR|SHFS_HIDESIPBUTTON|SHFS_HIDESTARTICON);
#endif

        }
#ifdef HAVE_ACTIVATE_INFO
      if (api_has_SHHandleWMActivate) {
        SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
      } else {
        #ifdef TESTBENCH
        StartupStore(TEXT("... SHHandleWMActivate not available%s"),NEWLINE);
        #endif
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
#endif
      break;

    case WM_SETTINGCHANGE:
#ifdef HAVE_ACTIVATE_INFO
      if (api_has_SHHandleWMSettingChange) {
        SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
      } else {
        #ifdef TESTBENCH
        StartupStore(TEXT("... SHHandleWMSettingChange not available%s"),NEWLINE);
        #endif
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
#endif
      break;

	#if DEBUG_FOCUS
    case WM_KILLFOCUS:
	// This is happening when focus is given to another window, either internally inside LK
	// or externally, for example to explorer..
	// SO: if we select MapWindow, we get here a KILLFOCUS from it.
	// When we select another process/program, or click on the desktop, the old window having focus is
	// receiving KILLFOCUS. So in case MapWindow was working, the signal will be sent over there, not here.
	// 
	StartupStore(_T("............ WNDPROC LOST FOCUS (KILLFOCUS)\n"));
	break;
	#endif

    case WM_SETFOCUS:
	// When explorer/desktop is giving focus to LK, this is where we get the signal.
	// But we must return focus to previous windows otherwise keys will not be working.
	// Mouse is another story, because mouse click is pertinent to a screen area which is mapped.
	// A mouse click will be sent to the window in the background, whose handler will receive the event.
	//
	// Each event handler receiving focus has to save it in hWndWithFocus, in LK.
	// Each event handler must thus handle SETFOCUS!
	//
	#if DEBUG_FOCUS
	StartupStore(_T("............ WNDPROC HAS FOCUS  (SETFOCUS)\n"));
	if (hWndWithFocus==NULL)
		StartupStore(_T(".....(no Wnd to give focus to)\n"));
	else
		StartupStore(_T(".....(passing focus to other window)\n"));
	#endif
	if (hWndWithFocus!=NULL) SetFocus(hWndWithFocus);
      break;

    case WM_KEYUP:
      break;

    case WM_TIMER:
	// WM_TIMER is run at about 2hz.
	LKHearthBeats++; // 100213
      //      ASSERT(hWnd==hWndMainWindow);
      if (ProgramStarted > psInitInProgress) {
	if (SIMMODE)
		SIMProcessTimer();
	else
		ProcessTimer();
	if (ProgramStarted==psFirstDrawDone) {
	  AfterStartup();
	  ProgramStarted = psNormalOp;
          StartupStore(_T(". ProgramStarted=NormalOp %s%s"), WhatTimeIsIt(),NEWLINE);
          StartupLogFreeRamAndStorage();

	}
      }
      break;

    case WM_INITMENUPOPUP:
      if (ProgramStarted > psInitInProgress) {
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOCK,MF_CHECKED|MF_BYCOMMAND);
	
	if(LoggerActive)
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOGGER,MF_CHECKED|MF_BYCOMMAND);
	else
	  CheckMenuItem((HMENU) wParam,IDM_FILE_LOGGER,MF_UNCHECKED|MF_BYCOMMAND);
      }
      break;

    case WM_CLOSE:

      LKASSERT(hWnd==hWndMainWindow);
      if((hWnd==hWndMainWindow) && 
         (MessageBoxX(hWndMainWindow,
		// LKTOKEN  _@M198_ = "Confirm Exit?"
               	gettext(TEXT("_@M198_")),
                      TEXT("LK8000"),
                      MB_YESNO|MB_ICONQUESTION) == IDYES)) 
        {
          if(iTimerID) {
            KillTimer(hWnd,iTimerID);
            iTimerID = 0;
          }

          Shutdown();
        }
      break;

    case WM_DESTROY:
      if (hWnd==hWndMainWindow) {
        PostQuitMessage(0);
      }
      break;

#ifdef PNA
#if TESTBENCH
    case WM_DEVICECHANGE:
	 TCHAR serr[50];
	 static WPARAM oldwparam=0;
	 StartupStore(_T("DEVICE CHANGE DETECTED, CODE=0x%x%s"),wParam,NEWLINE);

	 if (wParam!=oldwparam) {
		 oldwparam=wParam;
	 	wsprintf(serr,_T("DEVICE CHANGE DETECTED\nCODE=0x%x"),wParam);
        	 MessageBoxX(hWndMainWindow, serr, TEXT("LK8000"), MB_OK|MB_ICONQUESTION, true);
		 oldwparam=0;
	 }
	 return TRUE; // acknowledge
	 break;
#endif
#endif

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  return 0;
}



void AfterStartup() {

  #if TESTBENCH
  StartupStore(TEXT(". CloseProgressDialog%s"),NEWLINE);
  #endif
  CloseProgressDialog();

  // NOTE: Must show errors AFTER all windows ready
  int olddelay = StatusMessageData[0].delay_ms;
  StatusMessageData[0].delay_ms = 20000; // 20 seconds

  if (SIMMODE) {
	StartupStore(TEXT(". GCE_STARTUP_SIMULATOR%s"),NEWLINE);
	InputEvents::processGlideComputer(GCE_STARTUP_SIMULATOR);
  } else {
	StartupStore(TEXT(". GCE_STARTUP_REAL%s"),NEWLINE);
	InputEvents::processGlideComputer(GCE_STARTUP_REAL);
  }
  StatusMessageData[0].delay_ms = olddelay; 

  // Create default task if none exists
  #if TESTBENCH
  StartupStore(TEXT(". Create default task%s"),NEWLINE);
  #endif
  DefaultTask();

  // Trigger first redraw
  MapWindow::MapDirty = true;
  MapWindow::zoom.Reset(); 
  FullScreen();
  SetEvent(drawTriggerEvent);
}



void Shutdown(void) {
  int i;

  LKSound(_T("LK_DISCONNECT.WAV")); Sleep(500); // real WAV length is 410+ms
  if (!GlobalRunning) { // shutdown on startup (before sim/fly or clicking on the window X)
	StartupStore(_T(". Quick shutdown requested before terminating startup%s"),NEWLINE);
	// force exit mode for the case of being in welcome screen: OnTimerNotify will catch it
	RUN_MODE=RUN_SHUTDOWN;
	CloseCalculations();
	CloseGeoid();
	DeInitCustomHardware();
	LKRunStartEnd(false);
	return;
  }
  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(gettext(TEXT("_@M1219_")));

  StartupStore(_T(". Entering shutdown %s%s"), WhatTimeIsIt(),NEWLINE);
  MapWindow::Event_Pan(0);  // return from PAN restores the Task in case of Turnpoint moving
  #if TESTBENCH
  StartupLogFreeRamAndStorage();
  #endif

  // turn off all displays
  GlobalRunning = false;

  // LKTOKEN _@M1220_ "Shutdown, saving logs..."
  CreateProgressDialog(gettext(TEXT("_@M1220_")));

  // In case we quit while are still flying
  UpdateLogBook(false); // false=only log if still flying
  // stop logger
  guiStopLogger(true);

  // LKTOKEN _@M1221_ "Shutdown, saving profile..."
  CreateProgressDialog(gettext(TEXT("_@M1221_")));
  extern void LKAircraftSave(const TCHAR *szFile);
  extern void LKPilotSave(const TCHAR *szFile);
  extern void LKDeviceSave(const TCHAR *szFile);
  LKPilotSave(defaultPilotFile);
  LKAircraftSave(defaultAircraftFile);
  LKProfileSave(defaultProfileFile);
  LKDeviceSave(defaultDeviceFile);

  #if TESTBENCH
  StartupStore(TEXT(". Save_Recent_WP_history%s"),NEWLINE);
  #endif
  SaveRecentList();
  // Stop sound

  // Stop drawing
  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(gettext(TEXT("_@M1219_")));
 
  StartupStore(TEXT(". CloseDrawingThread%s"),NEWLINE);
  // 100526 this is creating problem in SIM mode when quit is called from X button, and we are in waypoint details
  // or probably in other menu related screens. However it cannot happen from real PNA or PDA because we don't have
  // that X button.
  MapWindow::CloseDrawingThread();

  // Stop calculating too (wake up)
  SetEvent(dataTriggerEvent);
  SetEvent(drawTriggerEvent);

  // Clear data
  // LKTOKEN _@M1222_ "Shutdown, saving task..."
  CreateProgressDialog(gettext(TEXT("_@M1222_")));

  #if TESTBENCH
  StartupStore(TEXT(".... Save default task%s"),NEWLINE);
  #endif

  SaveDefaultTask();

  #if TESTBENCH
  StartupStore(TEXT(".... Clear task data%s"),NEWLINE);
  #endif

  LockTaskData();
  Task[0].Index = -1;  ActiveWayPoint = -1; 
  AATEnabled = FALSE;
  CloseWayPoints();
  UnlockTaskData();

  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(gettext(TEXT("_@M1219_")));
  #if TESTBENCH
  StartupStore(TEXT(".... CloseTerrainTopology%s"),NEWLINE);
  #endif

  RasterTerrain::CloseTerrain();

  CloseTopology();
  #if USETOPOMARKS
  TopologyCloseMarks();
  #endif
  CloseTerrainRenderer();

  LiveTrackerShutdown();

  extern void CloseFlightDataRecorder(void);
  CloseFlightDataRecorder();
  
  // Stop COM devices
  StartupStore(TEXT(". Stop COM devices%s"),NEWLINE);
  devCloseAll();

  CloseFLARMDetails();

  ProgramStarted = psInitInProgress;

  // Kill windows
  #if TESTBENCH
  StartupStore(TEXT(".... Close Messages%s"),NEWLINE);
  #endif
  Message::Destroy();
  #if TESTBENCH 
  StartupStore(TEXT(".... Destroy Button Labels%s"),NEWLINE);
  #endif
  ButtonLabel::Destroy();

  #if TESTBENCH
  StartupStore(TEXT(".... Delete Objects%s"),NEWLINE);
  #endif
  
  // Kill graphics objects

  DeleteObject(hBrushSelected);
  DeleteObject(hBrushUnselected);
  DeleteObject(hBrushButton);
  #ifdef LXMINIMAP
  DeleteObject(hBrushButtonHasFocus);
  #endif

  extern void DeInitialiseFonts(void);
  DeInitialiseFonts();  
  CAirspaceManager::Instance().CloseAirspaces();
  #if TESTBENCH
  StartupStore(TEXT(".... Delete Critical Sections%s"),NEWLINE);
  #endif

  // Wait end of Calculation thread before deinit critical section.
  WaitForSingleObject(hCalculationThread, INFINITE);
  CloseHandle(hCalculationThread);

  #if TESTBENCH
  StartupStore(TEXT(".... Close Progress Dialog%s"),NEWLINE);
  #endif
  CloseProgressDialog();
  #if TESTBENCH
  StartupStore(TEXT(".... Close Calculations%s"),NEWLINE);
  #endif
  CloseCalculations();

  CloseGeoid();
  DeInitCustomHardware();

  #if TESTBENCH
  StartupStore(TEXT(".... Close Windows%s"),NEWLINE);
  #endif
  DestroyWindow(hWndMapWindow);
  DestroyWindow(hWndMainWindow);
  #if TESTBENCH
  StartupStore(TEXT(".... Close Event Handles%s"),NEWLINE);
  #endif
  CloseHandle(drawTriggerEvent);
  CloseHandle(dataTriggerEvent);

  #if TESTBENCH
  StartupLogFreeRamAndStorage();
  #endif
  for (i=0;i<NUMDEV;i++) {
	if (ComPortStatus[i]!=0) {
		StartupStore(_T(". ComPort %d: status=%d Rx=%d Tx=%d ErrRx=%d + ErrTx=%d (==%d)%s"), i,
		ComPortStatus[i], ComPortRx[i],ComPortTx[i], ComPortErrRx[i],ComPortErrTx[i],ComPortErrors[i],NEWLINE);
	}
  }
  StartupStore(_T(". Finished shutdown %s%s"), WhatTimeIsIt(),NEWLINE);
  LKRunStartEnd(false);

#ifdef DEBUG
  TCHAR foop[80];
  TASK_POINT wp;
  TASK_POINT *wpr = &wp;
  _stprintf(foop,TEXT(". Sizes %d %d %d%s"),
	    sizeof(TASK_POINT), 
	    ((long)&wpr->AATTargetLocked)-((long)wpr),
	    ((long)&wpr->Target)-((long)wpr), NEWLINE
	    );
  StartupStore(foop);
#endif
}


LRESULT MainMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // int wmId = LOWORD(wParam);
  // int wmEvent = HIWORD(wParam);
  HWND wmControl = (HWND)lParam;

  if(wmControl != NULL) {
    if (ProgramStarted==psNormalOp) {

      FullScreen();

      Message::CheckTouch(wmControl);

      if (ButtonLabel::CheckButtonPress(wmControl)) {
        return TRUE; // don't continue processing..
      }

    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}


