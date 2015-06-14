/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

  $Id$
*/


#include "externs.h"
#include "InputEvents.h"
#include "Terrain.h"
#include "Waypointparser.h"
#include "RasterTerrain.h"
#include "McReady.h"
#include "AirfieldDetails.h"
#include "Dialogs.h"

void SettingsEnter() {
  MenuActive = true;

  MapWindow::SuspendDrawingThread();
  // This prevents the map and calculation threads from doing anything
  // with shared data while it is being changed.

  MAPFILECHANGED = FALSE;
  AIRSPACEFILECHANGED = FALSE;
  AIRFIELDFILECHANGED = FALSE;
  WAYPOINTFILECHANGED = FALSE;
  TERRAINFILECHANGED = FALSE;
  TOPOLOGYFILECHANGED = FALSE;
  POLARFILECHANGED = FALSE;
  LANGUAGEFILECHANGED = FALSE;
  INPUTFILECHANGED = FALSE;
  COMPORTCHANGED = FALSE;
  FONTSCHANGED = false;
}


void SettingsLeave() {
  if (!GlobalRunning) return; 

  SwitchToMapWindow();

  // Locking everything here prevents the calculation thread from running,
  // while shared data is potentially reloaded.
 
  LockFlightData();
  LockTaskData();

  MenuActive = false;

  // 101020 LKmaps contain only topology , so no need to force total reload!
  if(MAPFILECHANGED) {
	#if TESTBENCH
	StartupStore(_T(".... MAPFILECHANGED from configuration\n"));
	#endif
	if (LKTopo==0) {
		AIRSPACEFILECHANGED = TRUE;
		AIRFIELDFILECHANGED = TRUE;
		WAYPOINTFILECHANGED = TRUE;
		TERRAINFILECHANGED  = TRUE;
	}
	TOPOLOGYFILECHANGED = TRUE;
  } 

  if (TERRAINFILECHANGED) {
	#if TESTBENCH
	StartupStore(_T(".... TERRAINFILECHANGED from configuration\n"));
	#endif
	RasterTerrain::CloseTerrain();
	RasterTerrain::OpenTerrain();
	// NO! We dont reload waypoints on terrain change.
	// SetHome(WAYPOINTFILECHANGED==TRUE);
	MapWindow::ForceVisibilityScan = true;
  }

  if((WAYPOINTFILECHANGED) || (AIRFIELDFILECHANGED)) {
	#if TESTBENCH
	StartupStore(_T(".... WAYPOINT OR AIRFIELD CHANGED from configuration\n"));
	#endif
	SaveDefaultTask(); //@ 101020 BUGFIX
	ClearTask();
	ReadWayPoints();
	StartupStore(_T(". RELOADED %d WAYPOINTS + %d virtuals%s"),WayPointList.size()-NUMRESWP,NUMRESWP,NEWLINE);
	SetHome(true); // force home reload

	if (WAYPOINTFILECHANGED) {
		#if TESTBENCH
		StartupStore(_T(".... WAYPOINTFILECHANGED from configuration\n"));
		#endif
		SaveRecentList();
		LoadRecentList();
		RangeLandableNumber=0;
		RangeAirportNumber=0;
		RangeTurnpointNumber=0;
		CommonNumber=0;
		SortedNumber=0;
		// SortedTurnpointNumber=0; 101222
		LastDoRangeWaypointListTime=0;
		LKForceDoCommon=true;
		LKForceDoNearest=true;
		LKForceDoRecent=true;
		// LKForceDoNearestTurnpoint=true; 101222
	}
	InputEvents::eventTaskLoad(_T(LKF_DEFAULTASK)); //@ BUGFIX 101020
  } 

  if (TOPOLOGYFILECHANGED) {
	#if TESTBENCH
	StartupStore(_T(".... TOPOLOGYFILECHANGED from configuration\n"));
	#endif
	CloseTopology();
	OpenTopology();
	MapWindow::ForceVisibilityScan = true;
  }
  
  if(AIRSPACEFILECHANGED) {
	#if TESTBENCH
	StartupStore(_T(".... AIRSPACEFILECHANGED from configuration\n"));
	#endif
	CAirspaceManager::Instance().CloseAirspaces();
	CAirspaceManager::Instance().ReadAirspaces();
	CAirspaceManager::Instance().SortAirspaces();
	MapWindow::ForceVisibilityScan = true;
  }  
  
  if (POLARFILECHANGED) {
	#if TESTBENCH
	StartupStore(_T(".... POLARFILECHANGED from configuration\n"));
	#endif
	CalculateNewPolarCoef();
	GlidePolar::SetBallast();
  }
  
  if (AIRFIELDFILECHANGED
      || AIRSPACEFILECHANGED
      || WAYPOINTFILECHANGED
      || TERRAINFILECHANGED
      || TOPOLOGYFILECHANGED
      ) {
	CloseProgressDialog();
	MainWindow.SetFocus();
  }

  extern void ReinitScreen(void);
  if (FONTSCHANGED) {
      ReinitScreen();
  }
  
  UnlockTaskData();
  UnlockFlightData();

  if(!SIMMODE && COMPORTCHANGED) {
      #if TESTBENCH
      StartupStore(_T(".... COMPORTCHANGED from configuration.  ForceComPortReset @%s\n"),WhatTimeIsIt());
      #endif
      LKForceComPortReset=true;
      // RestartCommPorts(); 110605
  }

  MapWindow::ResumeDrawingThread();
  // allow map and calculations threads to continue on their merry way
}


void SystemConfiguration(short mode) {
  if (!SIMMODE) {
	if (LockSettingsInFlight && CALCULATED_INFO.Flying) {
		DoStatusMessage(TEXT("Settings locked in flight"));
		return;
	}
  }

  #if TESTBENCH
  StartupStore(_T("... SETTINGS enter @%s\n"),WhatTimeIsIt());
  #endif
  SettingsEnter();
  dlgConfigurationShowModal(mode);
  #if TESTBENCH
  StartupStore(_T("... SETTINGS leave @%s\n"),WhatTimeIsIt());
  #endif
  SettingsLeave();
}



