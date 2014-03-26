/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"


Poco::Mutex  CritSec_FlightData;
Poco::Mutex  CritSec_EventQueue;
Poco::Mutex  CritSec_TerrainDataGraphics;
Poco::Mutex  CritSec_TerrainDataCalculations;
Poco::Mutex  CritSec_Comm;
Poco::Mutex  CritSec_TaskData;
Poco::Mutex  CritSec_StartupStore;


static int csCount_TaskData = 0;
static int csCount_FlightData = 0;
static int csCount_EventQueue = 0;





void LockComm() {
  CritSec_Comm.lock();
}

void UnlockComm() {
  CritSec_Comm.unlock();
}


void LockTaskData() {
  CritSec_TaskData.lock();
  csCount_TaskData++;
}

void UnlockTaskData() {
  if (csCount_TaskData) 
    csCount_TaskData--;
  CritSec_TaskData.unlock();
}


void LockFlightData() {
  CritSec_FlightData.lock();
  csCount_FlightData++;
}

void UnlockFlightData() {
  if (csCount_FlightData)
    csCount_FlightData--;
  CritSec_FlightData.unlock();
}

void CheckAndLockFlightData() {
    LockFlightData();
}
void CheckAndUnlockFlightData() {
    UnlockFlightData();
}


void LockTerrainDataCalculations() {
  CritSec_TerrainDataCalculations.lock();
}

void UnlockTerrainDataCalculations() {
  CritSec_TerrainDataCalculations.unlock();
}

void LockTerrainDataGraphics() {
  CritSec_TerrainDataGraphics.lock();
}

void UnlockTerrainDataGraphics() {
  CritSec_TerrainDataGraphics.unlock();
}


void LockEventQueue() {
  CritSec_EventQueue.lock();
  csCount_EventQueue++;
}

void UnlockEventQueue() {
  if (csCount_EventQueue) 
    csCount_EventQueue--;
  CritSec_EventQueue.unlock();
}

void LockStartupStore() {
	CritSec_StartupStore.lock();
}

void UnlockStartupStore() {
	CritSec_StartupStore.unlock();
}

