/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Logger.h"
#include "Tracking/Tracking.h"
#include "FlightDataRec.h"
#include "Hardware/CPU.hpp"
#include "Calc/Vario.h"
#include "LKInterface.h"
#include "OS/Sleep.h"

#ifndef ENABLE_OPENGL
extern bool OnFastPanning;
#endif

// PulseEvent is unreliable. But it does not matter anymore, since we should
// change approach for compatibility with unix.

void TriggerRedraws(NMEA_INFO *nmea_info, DERIVED_INFO *derived_info) {
    (void) nmea_info;
    (void) derived_info;

#ifndef ENABLE_OPENGL
    if (MapWindow::IsDisplayRunning()) {
        // 121028 Do not set MapDirty when we are fast panning, otherwise we shall overpass the
        // timeout (700ms) there, resulting in messy refreshes.

        if (OnFastPanning) {
            MapWindow::RequestFastRefresh();
        } else {
            MapWindow::RefreshMap();
        }
    }
#else
    MapWindow::RefreshMap();
#endif
}

// System boot specific flags
// Give me a go/no-go
static bool goCalculationThread = false;

class CalculationThread : public Thread {
public:
    CalculationThread() : Thread("Calculation") {}

    void Run() override {
        bool needcalculationsslow = false;

        // let's not create a deadlock here, setting the go after another race condition
        goCalculationThread = true; // 091119 CHECK
        // wait for proper startup signal
        while (!MapWindow::IsDisplayRunning()) {
            Sleep(100);
        }

        Sleep(1000); // 091213  BUGFIX need to syncronize !!! TOFIX02 TODO

        while (!MapWindow::CLOSETHREAD) {

            if (dataTriggerEvent.tryWait(5000)) dataTriggerEvent.reset();
            if (MapWindow::CLOSETHREAD) break; // drop out on exit

#ifdef HAVE_CPU_FREQUENCY
            const ScopeLockCPU cpu;
#endif
            // make local copy before editing...
            LockFlightData();
            FLARM_RefreshSlots(&GPS_INFO);
            Fanet_RefreshSlots(&GPS_INFO); //refresh slots of FANET
            memcpy(&tmpGPS, &GPS_INFO, sizeof (NMEA_INFO));
            memcpy(&tmpCALCULATED, &CALCULATED_INFO, sizeof (DERIVED_INFO));
            UnlockFlightData();

            DoCalculationsVario(&tmpGPS, &tmpCALCULATED);
            if (!VarioAvailable(tmpGPS)) {
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

            // values changed, so copy them back now: ONLY CALCULATED INFO
            // should be changed in DoCalculations, so we only need to write
            // that one back (otherwise we may write over new data)
            // Need to be Done before TriggerRedraw            
            LockFlightData();
            memcpy(&CALCULATED_INFO, &tmpCALCULATED, sizeof (DERIVED_INFO));
            UnlockFlightData();            

            // This is activating another run for Thread Draw
            TriggerRedraws(&tmpGPS, &tmpCALCULATED);

            if (MapWindow::CLOSETHREAD) break; // drop out on exit

            bool need_update = false;
            if (SIMMODE) {
                if (needcalculationsslow || (ReplayLogger::IsEnabled())) {
                    DoCalculationsSlow(&tmpGPS, &tmpCALCULATED);
                    needcalculationsslow = false;
                    need_update = true;
                }
            } else {
                if (needcalculationsslow) {
                    DoCalculationsSlow(&tmpGPS, &tmpCALCULATED);
                    needcalculationsslow = false;
                    need_update = true;
                }
            }

            if(need_update) {
                // CALCULATED_INFO need to copy back second time if data are updated by DoCalculationsSlow;
                LockFlightData();
                memcpy(&CALCULATED_INFO, &tmpCALCULATED, sizeof (DERIVED_INFO));
                UnlockFlightData();            
            }            
            
            if (MapWindow::CLOSETHREAD) break; // drop out on exit

            // update live tracker with new values
            // this is a nonblocking call, live tracker runs on different thread
            tracking::Update(tmpGPS, tmpCALCULATED);

            UpdateFlightDataRecorder(tmpGPS, tmpCALCULATED);
            CheckAltitudeAlarms(tmpGPS, tmpCALCULATED);

            ExternalDeviceSendTarget();
            SendDataToExternalDevice(tmpGPS, tmpCALCULATED);
        }
    }

private:
    NMEA_INFO tmpGPS;
    DERIVED_INFO tmpCALCULATED;
};

CalculationThread _CalculationThread;

// Since the calling function want to be sure that threads are created, they now flag a go status
// and we save 500ms at startup.
// At the end of thread creation, we expect goCalc and goInst flags are true
void CreateCalculationThread() {
    // Create a read thread for performing calculations
    _CalculationThread.Start();

    while(!(goCalculationThread)) {
        Sleep(50);
    }
}

void WaitThreadCalculation() {
    _CalculationThread.Join();
}
