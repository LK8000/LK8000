/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "StdAfx.h"
#include "wcecompat/ts_string.h"
#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "compatibility.h"
#include "lk8000.h"
#include "Logger.h"

#include "Message.h"
#include "InputEvents.h"

using std::min;
using std::max;

#ifdef DEBUG_TRANSLATIONS
#include <map>
static std::map<TCHAR*, TCHAR*> unusedTranslations;
#endif

#include "utils/heapcheck.h"

#include "winbase.h"


//
// This is common to both real and SIM modes, and thus it is running at 2Hz
//
void CommonProcessTimer()
{
  static unsigned short cp_twohzcounter = 0; // good up to 256 on all platforms
  cp_twohzcounter++;

  // Service the GCE and NMEA queue
  if (ProgramStarted==psNormalOp) {
	InputEvents::DoQueuedEvents();
  }

  // Automatically exit menu buttons mode
  // Note that MenuTimeoutMax is necessarily 2x the users choice, because we are at 2hz here
  if(MenuTimeOut==MenuTimeoutMax) {
	if (!MapWindow::mode.AnyPan()) {
		InputEvents::setMode(TEXT("default"));
		ShowAirspaceWarningsToUser(); // only shows the dialog if needed. OK at 2Hz.
	}
  }
  // setMode in InputEvents is checking that current mode is different from wanted mode.
  // So when we reach timeoutmax, we do call setMode, really, but we exit since we normally
  // are already in default mode. We can live with this solution.
  MenuTimeOut++;

  if (ProgramStarted==psNormalOp) {
	// 1 Hz routines
	if (cp_twohzcounter %2 == 0) {
		UpdateBatteryInfos();
	}
  }

  Message::Render();

  // Compact heap every minute.
  // We then reset the counter for everybody
  if (cp_twohzcounter == 120) {
	MyCompactHeaps();
	cp_twohzcounter = 0;
  }
}


// this part should be rewritten
int ConnectionProcessTimer(int itimeout) {
  LockComm();
  NMEAParser::UpdateMonitor();
  UnlockComm();
  
  static BOOL LastGPSCONNECT = FALSE;
  static BOOL CONNECTWAIT = FALSE;
  static BOOL LOCKWAIT = FALSE;
  
  //
  // replace bool with BOOL to correct warnings and match variable
  // declarations RB
  //
  BOOL gpsconnect = GPSCONNECT;
  
  if (GPSCONNECT) {
    extGPSCONNECT = TRUE;
  } 

  if (!extGPSCONNECT) {
    // if gps is not connected, set navwarning to true so
    // calculations flight timers don't get updated
    LockFlightData();
    GPS_INFO.NAVWarning = true;
    UnlockFlightData();
  }

  GPSCONNECT = FALSE;
  BOOL navwarning = (BOOL)(GPS_INFO.NAVWarning);

  if((gpsconnect == FALSE) && (LastGPSCONNECT == FALSE)) {
	// re-draw screen every five seconds even if no GPS
	TriggerGPSUpdate();
      
	devLinkTimeout(devAll());

	if(LOCKWAIT == TRUE) {
		// gps was waiting for fix, now waiting for connection
		LOCKWAIT = FALSE;
	}
	if(!CONNECTWAIT) {
		// gps is waiting for connection first time
		extGPSCONNECT = FALSE;
  
		CONNECTWAIT = TRUE;
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(TEXT("LK_GREEN.WAV"));
		#endif
		FullScreen();
	} else {
		// restart comm ports on timeouts, but not during managed special communications with devices
		// that will not provide NMEA stream, for example during a binary conversation for task declaration
		// or during a restart. Very careful, it shall be set to zero by the same function who
		// set it to true.
		if ((itimeout % 60 == 0) && !LKDoNotResetComms ) { 
			// no activity for 60/2 seconds (running at 2Hz), then reset.
			// This is needed only for virtual com ports..
			extGPSCONNECT = FALSE;
			if (!(devIsDisabled(0) && devIsDisabled(1))) {
			  InputEvents::processGlideComputer(GCE_COMMPORT_RESTART);
			  RestartCommPorts();
			}
	  
			itimeout = 0;
		}
	}
  }

  // Force RESET of comm ports on demand
  if (LKForceComPortReset) {
	StartupStore(_T(". ComPort RESET ordered%s"),NEWLINE);
	LKForceComPortReset=false;
	LKDoNotResetComms=false;
	if (MapSpaceMode != MSM_WELCOME)
		InputEvents::processGlideComputer(GCE_COMMPORT_RESTART);

	RestartCommPorts();
  }
  
  if((gpsconnect == TRUE) && (LastGPSCONNECT == FALSE)) {
	itimeout = 0; // reset timeout
      
	if(CONNECTWAIT) {
		TriggerGPSUpdate();
		CONNECTWAIT = FALSE;
	}
  }
  
  if((gpsconnect == TRUE) && (LastGPSCONNECT == TRUE)) {
	if((navwarning == TRUE) && (LOCKWAIT == FALSE)) {
		TriggerGPSUpdate();
	  
		LOCKWAIT = TRUE;
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(TEXT("LK_GREEN.WAV")); // 100404
		#endif
		FullScreen();
	} else {
		if((navwarning == FALSE) && (LOCKWAIT == TRUE)) {
			TriggerGPSUpdate();
			LOCKWAIT = FALSE;
		}
	}
  }
  
  LastGPSCONNECT = gpsconnect;
  return itimeout;
}

// Running at 2Hz, set and called by WndProc
void ProcessTimer(void)
{
  static int itimeout = -1;
  itimeout++;

  if (!GPSCONNECT) {
    if (itimeout % 2 == 0) TriggerGPSUpdate();  // Update screen when no GPS every second
  }

  CommonProcessTimer();

  // now check GPS status

  
  // also service replay logger
  ReplayLogger::Update();
  if (ReplayLogger::IsEnabled()) {
    static double timeLast = 0;
	if (GPS_INFO.Time-timeLast>=1.0) {
	TriggerGPSUpdate();
    }
    timeLast = GPS_INFO.Time;
    GPSCONNECT = TRUE;
    extGPSCONNECT = TRUE;
    GPS_INFO.NAVWarning = FALSE;
    GPS_INFO.SatellitesUsed = 6;
    return;
  }
  
  if (itimeout % 10 == 0) {
    // check connection status every 5 seconds
    itimeout = ConnectionProcessTimer(itimeout);
  }
}

void SIMProcessTimer(void)
{

  CommonProcessTimer();

  GPSCONNECT = TRUE;
  extGPSCONNECT = TRUE;
  static int i=0;
  i++;

  if (!ReplayLogger::Update()) {

    // Process timer is run at 2hz, so this is bringing it back to 1hz
    if (i%2==0) return;

    extern void LKSimulator(void);
    LKSimulator();
  }

  if (i%2==0) return;

#ifdef DEBUG
  // use this to test FLARM parsing/display
  NMEAParser::TestRoutine(&GPS_INFO);
#endif

  TriggerGPSUpdate();

}






