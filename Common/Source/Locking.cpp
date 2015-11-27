/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"


Mutex  CritSec_FlightData;
Mutex  CritSec_TerrainDataGraphics;
Mutex  CritSec_TerrainDataCalculations;
Mutex  CritSec_TaskData;

void LockTaskData() {
  CritSec_TaskData.Lock();
}

void UnlockTaskData() {
  CritSec_TaskData.Unlock();
}

/*
 * LockFlightData is used by Calc thread to protect CALCULATED_INFO and GPS_INFO while
 * and FLARM data inside GPS_INFO of course
 */
void LockFlightData() {
  CritSec_FlightData.Lock();
}

void UnlockFlightData() {
  CritSec_FlightData.Unlock();
}

void CheckAndLockFlightData() {
    LockFlightData();
}
void CheckAndUnlockFlightData() {
    UnlockFlightData();
}


void LockTerrainDataCalculations() {
  CritSec_TerrainDataCalculations.Lock();
}

void UnlockTerrainDataCalculations() {
  CritSec_TerrainDataCalculations.Unlock();
}

void LockTerrainDataGraphics() {
  CritSec_TerrainDataGraphics.Lock();
}

void UnlockTerrainDataGraphics() {
  CritSec_TerrainDataGraphics.Unlock();
}

