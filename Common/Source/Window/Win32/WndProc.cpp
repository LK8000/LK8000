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

extern void AfterStartup();
extern void StartupLogFreeRamAndStorage();
extern void SIMProcessTimer (void);
extern void ProcessTimer    (void);


LRESULT CALLBACK WndMainBase::WinMsgHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

        case WM_ERASEBKGND:
            return 1; // JMW trying to reduce screen flicker
            break;
        case WM_CREATE:
#ifdef HAVE_ACTIVATE_INFO
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
#endif
            if (iTimerID == 0) {
                iTimerID = SetTimer(hWnd, 1000, 500, NULL); // 500ms  2 times per second
            }

            break;

        case WM_ACTIVATE:
            if(LOWORD(wParam) == WA_INACTIVE) {
                HWND hWndFocus = ::GetFocus();
                if(hWndFocus && ::IsChild(_hWnd, hWndFocus)) {
                   //  Save Focus if Main Window Loses activation. 
                    _hWndFocus = hWndFocus;
                }
            } else {
                SetWindowPos(_hWnd, HWND_TOP,
                        0, 0, 0, 0,
                        SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);

#ifdef HAVE_ACTIVATE_INFO
                SHFullScreen(_hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
#endif

            }
#ifdef HAVE_ACTIVATE_INFO
            if (api_has_SHHandleWMActivate) {
                SHHandleWMActivate(_hWnd, wParam, lParam, &s_sai, FALSE);
            } else {
#ifdef TESTBENCH
                StartupStore(TEXT("... SHHandleWMActivate not available%s"), NEWLINE);
#endif
                break;
            }
#endif
            return 0;

        case WM_SETTINGCHANGE:
#ifdef HAVE_ACTIVATE_INFO
            if (api_has_SHHandleWMSettingChange) {
                SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            } else {
#ifdef TESTBENCH
                StartupStore(TEXT("... SHHandleWMSettingChange not available%s"), NEWLINE);
#endif
                break;
            }
#endif
            return 0;

        case WM_SETFOCUS:
        	if(::IsWindow(_hWndFocus) && IsChild(_hWnd, _hWndFocus)) {
        		// Set the Focus to the last know focus window
        		::SetFocus(_hWndFocus);
        		return 0; 
        	}
        	_hWndFocus = NULL;
        	break; // process default.
        case WM_KEYUP:
            return 0; // we have processes this message;

        case WM_TIMER:
            // WM_TIMER is run at about 2hz.
            LKHearthBeats++; // 100213
            if (ProgramStarted > psInitInProgress) {
                if (SIMMODE) {
                    SIMProcessTimer();
                } else {
                    ProcessTimer();
                }
                if (ProgramStarted == psFirstDrawDone) {
                    AfterStartup();
                    ProgramStarted = psNormalOp;
                    StartupStore(_T(". ProgramStarted=NormalOp %s%s"), WhatTimeIsIt(), NEWLINE);
                    StartupLogFreeRamAndStorage();

                }
            }
            return 0; // we have processes this message;

        case WM_DESTROY:
            if (iTimerID) {
                KillTimer(hWnd, iTimerID);
                iTimerID = 0;
            }
            PostQuitMessage(0);
            return 0; // we have processes this message;

        default:
            break;
    }
    return WndPaint::WinMsgHandler(hWnd, uMsg, wParam, lParam);
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
  drawTriggerEvent.set();
}


extern void WaitThreadCalculation();

void Shutdown(void) {
  int i;

  LKSound(_T("LK_DISCONNECT.WAV")); Poco::Thread::sleep(500); // real WAV length is 410+ms
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
  dataTriggerEvent.set();
  drawTriggerEvent.set();

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

  #ifdef LXMINIMAP
  hBrushButtonHasFocus.Release();
  #endif

  extern void DeInitialiseFonts(void);
  DeInitialiseFonts();  
  CAirspaceManager::Instance().CloseAirspaces();
  #if TESTBENCH
  StartupStore(TEXT(".... Delete Critical Sections%s"),NEWLINE);
  #endif

  // Wait end of Calculation thread before deinit critical section.
  WaitThreadCalculation();

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

  MainWindow.Destroy();

  #if TESTBENCH
  StartupLogFreeRamAndStorage();
  #endif
  for (i=0;i<NUMDEV;i++) {
	if (ComPortStatus[i]!=0) {
		StartupStore(_T(". ComPort %d: status=%d Rx=%ld Tx=%ld ErrRx=%ld + ErrTx=%ld (==%ld)%s"), i,
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

