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
#include "winbase.h"
#include "TraceThread.h"

extern int ConnectionProcessTimer(int itimeout);
extern bool BOOL2bool(BOOL a);
extern bool ScreenHasChanged(void);
extern void ReinitScreen(void);
extern void CommonProcessTimer(void);

//
// This is called at 2Hz from WndProc TIMER, which is set to 500ms by WndProc CREATE
// This is only for FLY mode. 
//
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

  // The ordinary timed stuff..
  CommonProcessTimer();

  // .. and the communication timed stuff.
  // Check connection status every 5 seconds
  // This is quite delicate because it is running a Lock condition on comm port
  if (itimeout % 10 == 0) {
	itimeout = ConnectionProcessTimer(itimeout);
  }
}

//
// This is the equivalent for SIM mode only.
// Running at 2Hz, set and called by WndProc, as above.
// Note, there is no ConnectionProcessTimer of course.
// Note 2: careful that the 1hz LKSimulator call is only roughly approximated, because it is 
// accumulating a delay due each execution. This is explaining some inaccurate calculations 
// while using SIM mode: the "1 second has passed" is not accurate because it is not based on an
// absolute time. It is in fact based on the time incrementer after each run. Since in the program
// we use a local time, this means that 1 minute passed in real (local) time does not normally
// mean we really had 60 calls of LKSimulator, more likely we had 59, or 58..
//
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



//
// This is common to both real and SIM modes, and thus it is running at 2Hz
// Dialogs will be processed in the background, without halting CPT
//
void CommonProcessTimer()
{
  static unsigned short cp_twohzcounter = 0; // good up to 256 on all platforms
  cp_twohzcounter++;

  // Service the GCE and NMEA queue
  if (ProgramStarted==psNormalOp) {
	InputEvents::DoQueuedEvents();
	// only shows the dialog if needed. Previously up to 2.3s at 2Hz.
	if (cp_twohzcounter %2 == 0) ShowAirspaceWarningsToUser();
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
	if (ScreenHasChanged()) ReinitScreen();
  }

  Message::Render();

  // Compact heap every minute, then reset the counter for everybody
  if (cp_twohzcounter == 120) {
	MyCompactHeaps();
	cp_twohzcounter = 0;
  }
}




// Running at 0.1hz every 10 seconds
// (this part should be rewritten)
int ConnectionProcessTimer(int itimeout) {
  LockComm();
  NMEAParser::UpdateMonitor();
  UnlockComm();

  // dont warn on startup
  static bool s_firstcom=true;

  static bool s_lastGpsConnect = false;
  static bool s_connectWait = false;
  static bool s_lockWait = false;
 
  // save status for this run 
  bool gpsconnect = BOOL2bool(GPSCONNECT);
  
  if (gpsconnect) {
    extGPSCONNECT = TRUE;
  } 

  if ((extGPSCONNECT == FALSE) && (GPS_INFO.NAVWarning!=true)) {
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

		if (EnableSoundModes && !s_firstcom) LKSound(TEXT("LK_GPSNOCOM.WAV"));
		FullScreen();
	} else {
		// restart comm ports on timeouts, but not during managed special communications with devices
		// that will not provide NMEA stream, for example during a binary conversation for task declaration
		// or during a restart. Very careful, it shall be set to zero by the same function who
		// set it to true.
		if ((itimeout % 360 == 0) && !LKDoNotResetComms ) { 
			// no activity for 360/2 seconds (running at 2Hz), then reset.
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
	s_firstcom=false;
      
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
		if (EnableSoundModes) LKSound(TEXT("LK_GPSNOFIX.WAV"));
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

