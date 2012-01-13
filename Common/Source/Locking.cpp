/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"


CRITICAL_SECTION  CritSec_FlightData;
bool csFlightDataInitialized = false;
CRITICAL_SECTION  CritSec_EventQueue;
bool csEventQueueInitialized = false;
CRITICAL_SECTION  CritSec_TerrainDataGraphics;
bool csTerrainDataGraphicsInitialized = false;
CRITICAL_SECTION  CritSec_TerrainDataCalculations;
bool csTerrainDataCalculationsInitialized = false;
CRITICAL_SECTION  CritSec_Comm;
bool csCommInitialized = false;
CRITICAL_SECTION  CritSec_TaskData;
bool csTaskDataInitialized = false;


CRITICAL_SECTION  CritSec_StartupStore;
bool csStartupStoreInitialized = false;


static int csCount_TaskData = 0;
static int csCount_FlightData = 0;
static int csCount_EventQueue = 0;


void InitCriticalSections() {

  InitializeCriticalSection(&CritSec_EventQueue);
  csEventQueueInitialized = true;
  InitializeCriticalSection(&CritSec_TaskData);
  csTaskDataInitialized = true;
  InitializeCriticalSection(&CritSec_FlightData);
  csFlightDataInitialized = true;
  InitializeCriticalSection(&CritSec_Comm);
  csCommInitialized = true;
  InitializeCriticalSection(&CritSec_TerrainDataGraphics);
  csTerrainDataGraphicsInitialized = true;
  InitializeCriticalSection(&CritSec_TerrainDataCalculations);
  csTerrainDataCalculationsInitialized = true;

  InitializeCriticalSection(&CritSec_StartupStore);
  csStartupStoreInitialized = true;
}

void DeInitCriticalSections() {

  DeleteCriticalSection(&CritSec_EventQueue);
  csEventQueueInitialized = false;
  DeleteCriticalSection(&CritSec_TaskData);
  csTaskDataInitialized = false;
  DeleteCriticalSection(&CritSec_FlightData);
  csFlightDataInitialized = false;
  DeleteCriticalSection(&CritSec_Comm);
  csCommInitialized = false;
  DeleteCriticalSection(&CritSec_TerrainDataCalculations);
  csTerrainDataGraphicsInitialized = false;
  DeleteCriticalSection(&CritSec_TerrainDataGraphics);
  csTerrainDataCalculationsInitialized = false;

  DeleteCriticalSection(&CritSec_StartupStore);
  csStartupStoreInitialized = false;
}



void LockComm() {
#ifdef HAVEEXCEPTIONS
  if (!csCommInitialized) throw TEXT("LockComm Error");
#endif
  EnterCriticalSection(&CritSec_Comm);
}

void UnlockComm() {
#ifdef HAVEEXCEPTIONS
  if (!csCommInitialized) throw TEXT("LockComm Error");
#endif
  LeaveCriticalSection(&CritSec_Comm);
}


void LockTaskData() {
#ifdef HAVEEXCEPTIONS
  if (!csTaskDataInitialized) throw TEXT("LockTaskData Error");
#endif
  EnterCriticalSection(&CritSec_TaskData);
  csCount_TaskData++;
}

void UnlockTaskData() {
#ifdef HAVEEXCEPTIONS
  if (!csTaskDataInitialized) throw TEXT("LockTaskData Error");
#endif
  if (csCount_TaskData) 
    csCount_TaskData--;
  LeaveCriticalSection(&CritSec_TaskData);
}


void LockFlightData() {
#ifdef HAVEEXCEPTIONS
  if (!csFlightDataInitialized) throw TEXT("LockFlightData Error");
#endif
  EnterCriticalSection(&CritSec_FlightData);
  csCount_FlightData++;
}

void UnlockFlightData() {
#ifdef HAVEEXCEPTIONS
  if (!csFlightDataInitialized) throw TEXT("LockFlightData Error");
#endif
  if (csCount_FlightData)
    csCount_FlightData--;
  LeaveCriticalSection(&CritSec_FlightData);
}

void CheckAndLockFlightData() {
  if (csFlightDataInitialized) {
    LockFlightData();
  }
}
void CheckAndUnlockFlightData() {
  if (csFlightDataInitialized) {
    UnlockFlightData();
  }
}


void LockTerrainDataCalculations() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataCalculationsInitialized) throw TEXT("LockTerrainDataCalculations Error");
#endif
  EnterCriticalSection(&CritSec_TerrainDataCalculations);
}

void UnlockTerrainDataCalculations() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataCalculationsInitialized) throw TEXT("LockTerrainDataCalculations Error");
#endif
  LeaveCriticalSection(&CritSec_TerrainDataCalculations);
}

void LockTerrainDataGraphics() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataGraphicsInitialized) throw TEXT("LockTerrainDataGraphics Error");
#endif
  EnterCriticalSection(&CritSec_TerrainDataGraphics);
}

void UnlockTerrainDataGraphics() {
#ifdef HAVEEXCEPTIONS
  if (!csTerrainDataGraphicsInitialized) throw TEXT("LockTerrainDataGraphics Error");
#endif
  LeaveCriticalSection(&CritSec_TerrainDataGraphics);
}


void LockEventQueue() {
#ifdef HAVEEXCEPTIONS
  if (!csEventQueueInitialized) throw TEXT("LockEventQueue Error");
#endif
  EnterCriticalSection(&CritSec_EventQueue);
  csCount_EventQueue++;
}

void UnlockEventQueue() {
#ifdef HAVEEXCEPTIONS
  if (!csEventQueueInitialized) throw TEXT("LockEventQueue Error");
#endif
  if (csCount_EventQueue) 
    csCount_EventQueue--;
  LeaveCriticalSection(&CritSec_EventQueue);
}

void LockStartupStore() {
	if (csStartupStoreInitialized)	{
		EnterCriticalSection(&CritSec_StartupStore);
	}
}

void UnlockStartupStore() {
	if (csStartupStoreInitialized)	{
		LeaveCriticalSection(&CritSec_StartupStore);
	}
}

