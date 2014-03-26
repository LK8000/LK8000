/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Logger.h"
#include "LiveTracker.h"
#include "TraceThread.h"

// PulseEvent is unreliable. But it does not matter anymore, since we should
// change approach for compatibility with unix.

void TriggerRedraws(NMEA_INFO *nmea_info, DERIVED_INFO *derived_info) {
    (void) nmea_info;
    (void) derived_info;

    if (MapWindow::IsDisplayRunning()) {
        // 121028 Do not set MapDirty when we are fast panning, otherwise we shall overpass the
        // timeout (700ms) there, resulting in messy refreshes.
#if (WINDOWSPC>0) && !TESTBENCH
#else
        if (!INPAN)
#endif
            MapWindow::MapDirty = true;

        drawTriggerEvent.set();
    }
}

extern bool FlightDataRecorderActive;
extern void UpdateFlightDataRecorder(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

class CalculationThread : public Poco::Runnable {
public:

    virtual void run() {
        bool needcalculationsslow = false;

        // let's not create a deadlock here, setting the go after another race condition
        goCalculationThread = true; // 091119 CHECK
        // wait for proper startup signal
        while (!MapWindow::IsDisplayRunning()) {
            Sleep(100);
        }

#if TRACETHREAD
        _THREADID_CALC = GetCurrentThreadId();
        StartupStore(_T("##############  CALC threadid=%d\n"), GetCurrentThreadId());
#endif

        // while (!goCalculating) Sleep(100);
        Sleep(1000); // 091213  BUGFIX need to syncronize !!! TOFIX02 TODO

        while (!MapWindow::CLOSETHREAD) {

            if (dataTriggerEvent.tryWait(5000)) dataTriggerEvent.reset();
            if (MapWindow::CLOSETHREAD) break; // drop out on exit

            // make local copy before editing...
            LockFlightData();
            FLARM_RefreshSlots(&GPS_INFO);
            memcpy(&tmpGPS, &GPS_INFO, sizeof (NMEA_INFO));
            memcpy(&tmpCALCULATED, &CALCULATED_INFO, sizeof (DERIVED_INFO));
            UnlockFlightData();

            DoCalculationsVario(&tmpGPS, &tmpCALCULATED);
            if (!tmpGPS.VarioAvailable) {
                TriggerVarioUpdate(); // emulate vario update
            }

            if (DoCalculations(&tmpGPS, &tmpCALCULATED)) {
#if (WINDOWSPC>0) && !TESTBENCH
#else
                if (!INPAN)
#endif
                {
                    MapWindow::MapDirty = true;
                }
                needcalculationsslow = true;

                if (tmpCALCULATED.Circling)
                    MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CIRCLING);
                else if (tmpCALCULATED.FinalGlide)
                    MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
                else
                    MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CRUISE);
            }

            if (MapWindow::CLOSETHREAD) break; // drop out on exit

            // This is activating another run for Thread Draw
            TriggerRedraws(&tmpGPS, &tmpCALCULATED);

            if (MapWindow::CLOSETHREAD) break; // drop out on exit

            if (SIMMODE) {
                if (needcalculationsslow || (ReplayLogger::IsEnabled())) {
                    DoCalculationsSlow(&tmpGPS, &tmpCALCULATED);
                    needcalculationsslow = false;
                }
            } else {
                if (needcalculationsslow) {
                    DoCalculationsSlow(&tmpGPS, &tmpCALCULATED);
                    needcalculationsslow = false;
                }
            }

            if (MapWindow::CLOSETHREAD) break; // drop out on exit

            // values changed, so copy them back now: ONLY CALCULATED INFO
            // should be changed in DoCalculations, so we only need to write
            // that one back (otherwise we may write over new data)
            LockFlightData();
            memcpy(&CALCULATED_INFO, &tmpCALCULATED, sizeof (DERIVED_INFO));
            UnlockFlightData();

            // update live tracker with new values
            // this is a nonblocking call, live tracker runs on different thread
            LiveTrackerUpdate(&tmpGPS, &tmpCALCULATED);

            if (FlightDataRecorderActive) UpdateFlightDataRecorder(&tmpGPS, &tmpCALCULATED);
        }
    }
private:
    NMEA_INFO tmpGPS;
    DERIVED_INFO tmpCALCULATED;
};


CalculationThread _CalculationThreadRun;
Poco::Thread _CalculationThread;

// Since the calling function want to be sure that threads are created, they now flag a go status
// and we save 500ms at startup. 
// At the end of thread creation, we expect goCalc and goInst flags are true
void CreateCalculationThread() {
    // Create a read thread for performing calculations
    _CalculationThread.start(_CalculationThreadRun);
    _CalculationThread.setPriority(Poco::Thread::PRIO_NORMAL);
}

void WaitThreadCalculation() {
    _CalculationThread.join();
}

