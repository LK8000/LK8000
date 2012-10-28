/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Logger.h"
#include "LiveTracker.h"

// PulseEvent is unreliable. But it does not matter anymore, since we should
// change approach for compatibility with unix.
void TriggerRedraws(NMEA_INFO *nmea_info, DERIVED_INFO *derived_info) {
        (void)nmea_info;
        (void)derived_info;

  if (MapWindow::IsDisplayRunning()) {
	MapWindow::MapDirty = true;
	PulseEvent(drawTriggerEvent);
  }
}


extern void Cpustats(int *acc, FILETIME *a, FILETIME *b, FILETIME *c, FILETIME *d);
  extern bool FlightDataRecorderActive;
  extern void UpdateFlightDataRecorder(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

DWORD CalculationThread (LPVOID lpvoid) {
	(void)lpvoid;
  bool needcalculationsslow;

  NMEA_INFO     tmp_GPS_INFO;
  DERIVED_INFO  tmp_CALCULATED_INFO;
  #ifdef CPUSTATS
  FILETIME CreationTime, ExitTime, StartKernelTime, EndKernelTime, StartUserTime, EndUserTime ;
  #endif
  needcalculationsslow = false;

  // let's not create a deadlock here, setting the go after another race condition
  goCalculationThread=true; // 091119 CHECK
  // wait for proper startup signal
  while (!MapWindow::IsDisplayRunning()) {
    Sleep(100);
  }

  // while (!goCalculating) Sleep(100);
  Sleep(1000); // 091213  BUGFIX need to syncronize !!! TOFIX02 TODO

  while (!MapWindow::CLOSETHREAD) {

    WaitForSingleObject(dataTriggerEvent, 5000);
    ResetEvent(dataTriggerEvent);
    if (MapWindow::CLOSETHREAD) break; // drop out on exit

    #ifdef CPUSTATS
    GetThreadTimes( hCalculationThread, &CreationTime, &ExitTime,&StartKernelTime,&StartUserTime);
    #endif

    // make local copy before editing...
    LockFlightData();
      FLARM_RefreshSlots(&GPS_INFO);
      memcpy(&tmp_GPS_INFO,&GPS_INFO,sizeof(NMEA_INFO));
      memcpy(&tmp_CALCULATED_INFO,&CALCULATED_INFO,sizeof(DERIVED_INFO));
    UnlockFlightData();

    DoCalculationsVario(&tmp_GPS_INFO,&tmp_CALCULATED_INFO);
    if (!tmp_GPS_INFO.VarioAvailable) {
    	TriggerVarioUpdate(); // emulate vario update
    } 
    
    if(DoCalculations(&tmp_GPS_INFO,&tmp_CALCULATED_INFO)){
        MapWindow::MapDirty = true;
        needcalculationsslow = true;

        if (tmp_CALCULATED_INFO.Circling)
          MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CIRCLING);
        else if (tmp_CALCULATED_INFO.FinalGlide)
          MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
        else
          MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CRUISE);
    }
        
    if (MapWindow::CLOSETHREAD) break; // drop out on exit

    // 121028 Do not set MapDirty when we are fast panning, otherwise we shall overpass the
    // timeout (700ms) there, resulting in messy refreshes.
    if (!MapWindow::mode.Is(MapWindow::Mode::MODE_PAN))
       TriggerRedraws(&tmp_GPS_INFO, &tmp_CALCULATED_INFO);

    if (MapWindow::CLOSETHREAD) break; // drop out on exit

    if (SIMMODE) {
	if (needcalculationsslow || ( ReplayLogger::IsEnabled() ) ) { 
		DoCalculationsSlow(&tmp_GPS_INFO,&tmp_CALCULATED_INFO);
		needcalculationsslow = false;
	}
    } else {
	if (needcalculationsslow) {
		DoCalculationsSlow(&tmp_GPS_INFO,&tmp_CALCULATED_INFO);
		needcalculationsslow = false;
	}
    }

    if (MapWindow::CLOSETHREAD) break; // drop out on exit

    // values changed, so copy them back now: ONLY CALCULATED INFO
    // should be changed in DoCalculations, so we only need to write
    // that one back (otherwise we may write over new data)
    LockFlightData();
    memcpy(&CALCULATED_INFO,&tmp_CALCULATED_INFO,sizeof(DERIVED_INFO));
    UnlockFlightData();

    // update live tracker with new values
    // this is a nonblocking call, live tracker runs on different thread
     LiveTrackerUpdate(&tmp_GPS_INFO, &tmp_CALCULATED_INFO);

    if (FlightDataRecorderActive) UpdateFlightDataRecorder(&tmp_GPS_INFO,&tmp_CALCULATED_INFO);
   
    
    #ifdef CPUSTATS
    if ( (GetThreadTimes( hCalculationThread, &CreationTime, &ExitTime,&EndKernelTime,&EndUserTime)) == 0) {
               Cpu_Calc=9999;
    } else {
               Cpustats(&Cpu_Calc,&StartKernelTime, &EndKernelTime, &StartUserTime, &EndUserTime);
    }
    #endif
  }
  return 0;
}





// Since the calling function want to be sure that threads are created, they now flag a go status
// and we save 500ms at startup. 
// At the end of thread creation, we expect goCalc and goInst flags are true
void CreateCalculationThread() {
  // Create a read thread for performing calculations
  if ((hCalculationThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE )CalculationThread, 0, 0, &dwCalcThreadID)) != NULL)
  {
	SetThreadPriority(hCalculationThread, THREAD_PRIORITY_NORMAL); 
  } else {
	LKASSERT(1);
  }

}


