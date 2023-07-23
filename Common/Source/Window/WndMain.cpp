/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndMain.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 9 novembre 2014, 16:51
 */

#include "externs.h"
#include "WndMain.h"
#include "LKLanguage.h"
#include "MapWindow.h"
#include "Dialogs.h"
#include "InputEvents.h"
#include "resource.h"
#include "Waypointparser.h"
#include "InfoBoxLayout.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "Message.h"
#include "Geoid.h"
#include "RGB.h"
#include "LKProfiles.h"
#include "Logger.h"
#include "FlightDataRec.h"

#include "Event/Event.h"
#include "Sound/Sound.h"
#include "Kobo/System.hpp"
#include "Dialogs/dlgProgress.h"
#include "Draw/ScreenProjection.h"

#include "Airspace/Sonar.h"
#include "OS/RotateScreen.h"
#include "ChangeScreen.h"
#include "IO/Async/GlobalIOThread.hpp"
#include "Tracking/Tracking.h"
#include "OS/Sleep.h"

WndMain::WndMain() : WndMainBase(), _MouseButtonDown(), _isRunning() {
}

WndMain::~WndMain() {
}

extern void WaitThreadCalculation();

void BeforeShutdown(void) {

  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(MsgToken(1219));

  LKSound(_T("LK_DISCONNECT.WAV"));
  Sleep(500); // real WAV length is 410+ms
  if (!GlobalRunning) { // shutdown on startup (before sim/fly or clicking on the window X)
	StartupStore(_T(". Quick shutdown requested before terminating startup%s"),NEWLINE);
	// force exit mode for the case of being in welcome screen: OnTimerNotify will catch it
	RUN_MODE=RUN_SHUTDOWN;
	CloseCalculations();
	CloseGeoid();
	DeInitCustomHardware();
	LKRunStartEnd(false);
    CloseProgressDialog();
	return;
  }

  StartupStore(_T(". Entering shutdown %s%s"), WhatTimeIsIt(),NEWLINE);
  MapWindow::Event_Pan(0);  // return from PAN restores the Task in case of Turnpoint moving
  #if TESTBENCH
  StartupLogFreeRamAndStorage();
  #endif

	DeinitAirspaceSonar();
	
  // turn off all displays
  GlobalRunning = false;

  // LKTOKEN _@M1220_ "Shutdown, saving logs..."
  CreateProgressDialog(MsgToken(1220));

  // In case we quit while are still flying
  UpdateLogBook(false); // false=only log if still flying
  // stop logger
  guiStopLogger(true);

  // LKTOKEN _@M1221_ "Shutdown, saving profile..."
  CreateProgressDialog(MsgToken(1221));
  LKPilotSave(defaultPilotFile);
  LKAircraftSave(defaultAircraftFile);
  LKProfileSave(defaultProfileFile);
  LKDeviceSave(defaultDeviceFile);

  SaveRecentList();
  // Stop sound

  // Stop drawing
  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(MsgToken(1219));

  // Stop COM devices first to avoid mutex race condition...
  StartupStore(TEXT(". Stop COM devices%s"),NEWLINE);
  devCloseAll();

  // 100526 this is creating problem in SIM mode when quit is called from X button, and we are in waypoint details
  // or probably in other menu related screens. However it cannot happen from real PNA or PDA because we don't have
  // that X button.
  MapWindow::CloseDrawingThread();

  // Stop calculating too (wake up)
  dataTriggerEvent.set();

  // Clear data
  // LKTOKEN _@M1222_ "Shutdown, saving task..."
  CreateProgressDialog(MsgToken(1222));

  #if TESTBENCH
  StartupStore(TEXT(".... Save default task%s"),NEWLINE);
  #endif

  SaveDefaultTask();

  #if TESTBENCH
  StartupStore(TEXT(".... Clear task data%s"),NEWLINE);
  #endif

  LockTaskData();
  CloseWayPoints();
  UnlockTaskData();

  // LKTOKEN _@M1219_ "Shutdown, please wait..."
  CreateProgressDialog(MsgToken(1219));
  #if TESTBENCH
  StartupStore(TEXT(".... CloseTerrainTopology%s"),NEWLINE);
  #endif

  LockTerrainDataGraphics();
  CloseTopology();
  CloseTerrainRenderer();
  RasterTerrain::CloseTerrain();
  UnlockTerrainDataGraphics();

  tracking::DeInitialize();
  DeinitialiseIOThread();

  CloseFlightDataRecorder();

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

   CAirspaceManager::Instance().CloseAirspaces();
  #if TESTBENCH
  StartupStore(TEXT(".... Delete Critical Sections%s"),NEWLINE);
  #endif

  // Wait end of Calculation thread before deinit critical section.
  WaitThreadCalculation();

  #if TESTBENCH
  StartupStore(TEXT(".... Close Calculations%s"),NEWLINE);
  #endif
  CloseCalculations();

  CloseGeoid();
  DeInitCustomHardware();

  #if TESTBENCH
  StartupStore(TEXT(".... Close Windows%s"),NEWLINE);
  #endif

  if (RUN_MODE == RUN_FLY) {
    for (unsigned i = 0; i < NUMDEV; i++) {
      const DeviceDescriptor_t& dev = DeviceList[i];
      if (dev.Status != CPS_UNUSED) {
        StartupStore(_T(". ComPort %d: status=%d Rx=%u Tx=%u ErrRx=%u ErrTx=%u"), i,
                                dev.Status, dev.Rx, dev.Tx, dev.ErrRx, dev.ErrTx);
      }
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

  main_window->PostQuit();
#if TESTBENCH
  StartupLogFreeRamAndStorage();
  #endif

}


void WndMain::OnCreate() {
    MapWindow::_OnCreate(*this, GetWidth(), GetHeight());
    WndMainBase::OnCreate();
    StartTimer(500);
    _isRunning = true;
}

bool WndMain::OnClose() {
    ScopeLockScreen LockSreen;
    if(!_isRunning) {
        return WndMainBase::OnClose();
    }
    if (MessageBoxX(MsgToken(198), // LKTOKEN  _@M198_ = "Confirm Exit?"
                    TEXT("LK8000"),
                    mbYesNo) == IdYes) {

        BeforeShutdown();
        return WndMainBase::OnClose();
    }
    return true; 
}

void WndMain::OnDestroy() {
    StartupStore(_T(". WndMain::OnDestroy" NEWLINE));
    _isRunning = false;
    StopTimer();
    MapWindow::_OnDestroy();
    return WndMainBase::OnDestroy();
}

bool WndMain::OnSize(int cx, int cy) {
    MapWindow::_OnSize(cx, cy);
	if (ScreenHasChanged()) {
        ReinitScreen();
    } 
    return true;
}

extern StartupState_t ProgramStarted;
bool WndMain::OnPaint(LKSurface& Surface, const RECT& Rect) {
#ifdef ENABLE_OPENGL
    if (ProgramStarted==psInitDone) {
        ProgramStarted = psFirstDrawDone;
    } else {
        if (children.IsCoveredByChild(*this)) {
            return true;
        }
    }
    if (MapDirty) {
        BackBufferSurface.Begin(Surface);
        MapWindow::Render(BackBufferSurface, Rect);
        BackBufferSurface.Commit(Surface);
    } else {
        BackBufferSurface.CopyTo(Surface);
    }
#else
    if(ProgramStarted >= psFirstDrawDone) {
        ScopeLock Lock(BackBuffer_Mutex);
        BackBufferSurface.CopyTo(Surface);
    } else {

	}

#endif
    return true;
}

void WndMain::OnKillFocus() {
    _MouseButtonDown = false;
    return  WndMainBase::OnKillFocus();
}


bool WndMain::OnMouseMove(const POINT& Pos) {
    if(_MouseButtonDown) {
        SetCapture();
        MapWindow::_OnDragMove(Pos);
    }
    return true;
}

bool WndMain::OnLButtonDown(const POINT& Pos) {
    _MouseButtonDown = true;
    MapWindow::_OnLButtonDown(Pos);
    return true;
}

bool WndMain::OnLButtonUp(const POINT& Pos) {
    ReleaseCapture();
    _MouseButtonDown = false;
    MapWindow::_OnLButtonUp(Pos);
    return true;
}

bool WndMain::OnLButtonDblClick(const POINT& Pos) {
    MapWindow::_OnLButtonDblClick(Pos);
    return true;
}

bool WndMain::OnKeyDown(unsigned KeyCode) {
    MapWindow::_OnKeyDown(KeyCode);
    return true;
}

void AfterStartup() {

  #if TESTBENCH
  StartupStore(TEXT(". CloseProgressDialog%s"),NEWLINE);
  #endif
  CloseProgressDialog();

  if (SIMMODE) {
	StartupStore(TEXT(". GCE_STARTUP_SIMULATOR%s"),NEWLINE);
	InputEvents::processGlideComputer(GCE_STARTUP_SIMULATOR);
  } else {
	StartupStore(TEXT(". GCE_STARTUP_REAL%s"),NEWLINE);
	InputEvents::processGlideComputer(GCE_STARTUP_REAL);
  }

  // Trigger first redraw
  MapWindow::zoom.Reset();
  FullScreen();
  MapWindow::RefreshMap();
}


extern void SIMProcessTimer(void);
extern void ProcessTimer(void);

void WndMain::OnTimer() {
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
}
