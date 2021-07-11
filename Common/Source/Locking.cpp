/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"


Mutex  CritSec_FlightData;
Mutex  CritSec_TerrainDataGraphics;
Mutex  CritSec_TerrainDataCalculations;
Mutex  CritSec_TaskData;

void LockTaskData() {
  CritSec_TaskData.lock();
}

void UnlockTaskData() {
  CritSec_TaskData.unlock();
}

/*
 * LockFlightData is used by Calc thread to protect CALCULATED_INFO and GPS_INFO while
 * and FLARM data inside GPS_INFO of course
 */
void LockFlightData() {
  CritSec_FlightData.lock();
}

void UnlockFlightData() {
  CritSec_FlightData.unlock();
}

void CheckAndLockFlightData() {
    LockFlightData();
}
void CheckAndUnlockFlightData() {
    UnlockFlightData();
}



void LockTerrainDataGraphics() {
  CritSec_TerrainDataGraphics.lock();
}

void UnlockTerrainDataGraphics() {
  CritSec_TerrainDataGraphics.unlock();
}
