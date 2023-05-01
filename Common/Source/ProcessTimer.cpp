/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "Logger.h"
#include "Message.h"
#include "InputEvents.h"
#include "OS/Memory.h"

extern int ConnectionProcessTimer(int itimeout);
extern void CommonProcessTimer(void);
extern void LKSimulator(void);
//
// This is called at 2Hz from WndProc TIMER, which is set to 500ms by WndProc CREATE
// This is only for FLY mode.  THIS IS NOT CALLED FOR SIM MODE, remember.
//
void ProcessTimer(void)
{
  static int itimeout = -1;
  itimeout++;
  static int p_twohzcounter = 0;
  p_twohzcounter++;

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

  extGPSCONNECT = TRUE;

  if (!ReplayLogger::Update()) {
	if (i%2==0) return;
	// Process timer is run at 2hz, so this is bringing it back to 1hz
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

  // 0.5 Hz routines
  if (cp_twohzcounter %4 == 0) {
	UpdateBatteryInfos();
  }

  Message::Render();

  // Compact heap every minute, then reset the counter for everybody
  if (cp_twohzcounter == 120) {
	MyCompactHeaps();
	cp_twohzcounter = 0;
  }
}
