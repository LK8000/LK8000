/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "Logger.h"

#include "Message.h"
#include "InputEvents.h"

using std::min;
using std::max;


#include "utils/heapcheck.h"
#include "winbase.h"

extern int ConnectionProcessTimer(int itimeout);
extern bool BOOL2bool(BOOL a);

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
	ShowAirspaceWarningsToUser(); // only shows the dialog if needed. OK at 2Hz.
  }

  // Automatically exit menu buttons mode
  // Note that MenuTimeout_Config is necessarily 2x the users choice, because we are at 2hz here
  if(MenuTimeOut==MenuTimeout_Config) {
	if (!MapWindow::mode.AnyPan()) {
		InputEvents::setMode(TEXT("default"));
	}
  }
  // setMode in InputEvents is checking that current mode is different from wanted mode.
  // So when we reach timeoutmax, we do call setMode, really, but we exit since we normally
  // are already in default mode. We can live with this solution.
  MenuTimeOut++;

  // 1 Hz routines
  if (cp_twohzcounter %2 == 0) {
	UpdateBatteryInfos();
  }

  Message::Render();

  // Compact heap every minute, then reset the counter for everybody
  if (cp_twohzcounter == 120) {
	MyCompactHeaps();
	cp_twohzcounter = 0;
  }
}



// Running at 2Hz, set and called by WndProc
void ProcessTimer(void)
{
  static int itimeout = -1;
  itimeout++;
  static int p_twohzcounter = 0;
  p_twohzcounter++;

  if (!GPSCONNECT) {
	// Update screen when no GPS every second
	if (p_twohzcounter % 2 == 0) TriggerGPSUpdate();
  }

  CommonProcessTimer();

  // Check connection status every 5 seconds
  if (itimeout % 10 == 0) {
	itimeout = ConnectionProcessTimer(itimeout);
  }
}



// Running at 2Hz, set and called by WndProc
void SIMProcessTimer(void)
{
  static int i=0;
  i++;

  CommonProcessTimer();

  GPSCONNECT = TRUE;
  extGPSCONNECT = TRUE;

  if (!ReplayLogger::Update()) {
	if (i%2==0) return;
	// Process timer is run at 2hz, so this is bringing it back to 1hz
	extern void LKSimulator(void);
	LKSimulator();
  }

  if (i%2==0) return;

  TriggerGPSUpdate();

}



// Running at 0.1hz every 10 seconds
// (this part should be rewritten)
int ConnectionProcessTimer(int itimeout) {
  LockComm();
  NMEAParser::UpdateMonitor();
  UnlockComm();
  
  static bool s_lastGpsConnect = false;
  static bool s_connectWait = false;
  static bool s_lockWait = false;
  
  bool gpsconnect = BOOL2bool(GPSCONNECT);
  
  if (GPSCONNECT) {
    extGPSCONNECT = TRUE;
  } 

  if (extGPSCONNECT == FALSE) {
	// If gps is not connected, set navwarning to true so
	// calculations flight timers don't get updated
	LockFlightData();
	GPS_INFO.NAVWarning = true;
	UnlockFlightData();
  }

  GPSCONNECT = FALSE;
  bool navwarning = GPS_INFO.NAVWarning;

  if((gpsconnect == false) && (s_lastGpsConnect == false)) {
	// re-draw screen every five seconds even if no GPS
	TriggerGPSUpdate();
      
	devLinkTimeout(devAll());

	if(s_lockWait == true) {
		// gps was waiting for fix, now waiting for connection
		s_lockWait = false;
	}
	if(!s_connectWait) {
		// gps is waiting for connection first time
		extGPSCONNECT = FALSE;
  
		s_connectWait = true;
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
  
  if((gpsconnect == true) && (s_lastGpsConnect == false)) {
	itimeout = 0; // reset timeout
      
	if(s_connectWait) {
		TriggerGPSUpdate();
		s_connectWait = false;
	}
  }
  
  if((gpsconnect == true) && (s_lastGpsConnect == true)) {
	if((navwarning == true) && (s_lockWait == false)) {
		TriggerGPSUpdate();
	  
		s_lockWait = true;
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(TEXT("LK_GREEN.WAV")); // 100404
		#endif
		FullScreen();
	} else {
		if((navwarning == false) && (s_lockWait == true)) {
			TriggerGPSUpdate();
			s_lockWait = false;
		}
	}
  }
  
  s_lastGpsConnect = gpsconnect;
  return itimeout;
}

