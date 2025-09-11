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

void TriggerRedraws() {

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

class CalculationThread final : public Thread {
public:
    CalculationThread() : Thread("Calculation") { }

    void Run() override {
        // wait for proper startup signal
        while (!MapWindow::IsDisplayRunning()) {
            Sleep(100);
        }

        Sleep(1000); // 091213  BUGFIX need to synchronize !!! TOFIX02 TODO

        while (Wait()) {

#ifdef HAVE_CPU_FREQUENCY
            const ScopeLockCPU cpu;
#endif
            // make local copy before editing...
            UpdateLocalFlightData();

            bool needcalculationsslow = false;

            if (DoCalculations(&Local_NMEA, &Local_DERIVED)) {
                if (!INPAN) {
                    MapWindow::MapDirty = true;
                }
                needcalculationsslow = true;

                if (Local_DERIVED.Circling) {
                    MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CIRCLING);
                }
                else if (Local_DERIVED.FinalGlide) {
                    MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_FINAL_GLIDE);
                }
                else {
                    MapWindow::mode.Fly(MapWindow::Mode::MODE_FLY_CRUISE);
                }
            }

            // values changed, so copy them back now: ONLY CALCULATED INFO
            // should be changed in DoCalculations, so we only need to write
            // that one back (otherwise we may write over new data)
            // Need to be Done before TriggerRedraw
            UpdateCalculatedFlightData();

            // This is activating another run for Thread Draw
            TriggerRedraws();

            if (needcalculationsslow || (SIMMODE && ReplayLogger::IsEnabled())) {
                DoCalculationsSlow(&Local_NMEA, &Local_DERIVED);
                needcalculationsslow = false;

                // CALCULATED_INFO need to copy back second time if data are updated by DoCalculationsSlow;
                UpdateCalculatedFlightData();
            }

            // update live tracker with new values
            // this is a nonblocking call, live tracker runs on different thread
            tracking::Update(Local_NMEA, Local_DERIVED);

            UpdateFlightDataRecorder(Local_NMEA, Local_DERIVED);
            CheckAltitudeAlarms(Local_NMEA, Local_DERIVED);

            ExternalDeviceSendTarget();
            SendDataToExternalDevice(Local_NMEA, Local_DERIVED);
        }
    }

    void SignalNewData() {
        ScopeLock lock(mtx);
        new_data = true;
        cond.Signal();
    }

    void RequestStop() {
        ScopeLock lock(mtx);
        run = false;
        cond.Signal();
    }

private:

    void UpdateLocalFlightData() {
        ScopeLock lock(CritSec_FlightData);
        FLARM_RefreshSlots(&GPS_INFO);
        Fanet_RefreshSlots(&GPS_INFO); //refresh slots of FANET
        Local_NMEA = GPS_INFO;
        Local_DERIVED = CALCULATED_INFO;
    }

    void UpdateCalculatedFlightData() {
        ScopeLock lock(CritSec_FlightData);
        CALCULATED_INFO = Local_DERIVED;
    }

    bool Wait() {
        ScopeLock lock(mtx);
        while (run && !new_data) {
            cond.Wait(mtx);
        }
        new_data = false;
        return run;
    }

    NMEA_INFO Local_NMEA;
    DERIVED_INFO Local_DERIVED;

    Mutex mtx;
    Cond cond;
    bool run = true;
    bool new_data = false;
};

CalculationThread _CalculationThread;

void CreateCalculationThread() {
    _CalculationThread.Start();
}

void StopThreadCalculation() {
    // Stop and Wait end of thread.
    _CalculationThread.RequestStop();
    _CalculationThread.Join();
}

void TriggerGPSUpdate() {
    _CalculationThread.SignalNewData();
}
