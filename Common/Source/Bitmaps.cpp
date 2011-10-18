/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"
#include "MapWindow.h"
#include "LKMapWindow.h"
#include "Utils.h"

#define STATIC_BITMAPS
#include "Bitmaps.h"

#if (WINDOWSPC>0)
#include <wingdi.h>
#endif


//
// Load bitmaps that have no profile dependencies
//
// This is called once per session, at startup by the CREATE draw thread
// We are still missing TOPOLOGY shape icons to be added here in an array
// Kalman or Richard? I think it will speed up map drawing a bit.
//
// Soon we shall load all bitmaps from the filesystem, and drop internal resources.
//
void LKLoadFixedBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Load Fixed Bitmaps\n"));
  #endif

  hFLARMTraffic=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FLARMTRAFFIC));
  hTerrainWarning=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TERRAINWARNING));
  hAirspaceWarning=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACEWARNING));
  hTurnPoint=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TURNPOINT));
  hInvTurnPoint=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_INVTURNPOINT));
  hSmall=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SMALL));
  hInvSmall=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_INVSMALL));
  hLogger=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LOGGER));
  hLoggerOff=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LOGGEROFF));
  hBmpTeammatePosition = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TEAMMATE_POS));
  hBmpMarker = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MARK));
  hBatteryFull=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BATTERY_FULL_SMALL));
  hBattery70=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BATTERY_70_SMALL));
  hBattery50=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BATTERY_50_SMALL));
  hBattery25=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BATTERY_25_SMALL));
  hBattery15=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BATTERY_15_SMALL));

  // airspace brushes and colours

  hAirspaceBitmap[0]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE0));
  hAirspaceBitmap[1]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE1));
  hAirspaceBitmap[2]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE2));
  hAirspaceBitmap[3]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE3));
  hAirspaceBitmap[4]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE4));
  hAirspaceBitmap[5]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE5));
  hAirspaceBitmap[6]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE6));
  hAirspaceBitmap[7]=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRSPACE7));

  hAboveTerrainBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ABOVETERRAIN));

  hBmpThermalSource = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_THERMALSOURCE));
  hBmpTarget = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TARGET));

  hBmpLeft32 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DLGBUTTONLEFT32));
  hBmpRight32 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DLGBUTTONRIGHT32));

  hScrollBarBitmapTop=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARTOP));
  hScrollBarBitmapMid=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARMID));
  hScrollBarBitmapBot=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCROLLBARBOT));


}


//
// Unloading bitmaps can be done only after removing brushes!
// No need to set them NULL, because they are removed on exit.
//
void LKUnloadFixedBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Unload Fixed Bitmaps\n"));
  #endif

  DeleteObject(hTurnPoint);
  DeleteObject(hSmall);
  DeleteObject(hInvTurnPoint);
  DeleteObject(hInvSmall);
  DeleteObject(hFLARMTraffic);
  DeleteObject(hTerrainWarning);
  DeleteObject(hAirspaceWarning);
  DeleteObject(hLogger);
  DeleteObject(hLoggerOff);
  DeleteObject(hBatteryFull);
  DeleteObject(hBattery70);
  DeleteObject(hBattery50);
  DeleteObject(hBattery25);
  DeleteObject(hBattery15);
    
  DeleteObject(hBmpThermalSource);
  DeleteObject(hBmpTarget);
  DeleteObject(hBmpTeammatePosition);
  DeleteObject(hBmpMarker);

  for(short i=0;i<NUMAIRSPACEBRUSHES;i++)
	DeleteObject(hAirspaceBitmap[i]);

  DeleteObject(hAboveTerrainBitmap);

  DeleteObject(hBmpLeft32);
  DeleteObject(hBmpRight32);

  DeleteObject(hScrollBarBitmapTop);
  DeleteObject(hScrollBarBitmapMid);
  DeleteObject(hScrollBarBitmapBot);
}

//
// Load bitmaps that are affected by a profile 
//
void LKLoadProfileBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Load Profile Bitmaps\n"));
  #endif

  if ( ISPARAGLIDER ) {
	hCruise=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CRUISEPARA));
	hClimb=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CLIMBPARA));
	hFinalGlide=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FINALGLIDEPARA));
  } else {
	hCruise=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CRUISE));
	hClimb=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CLIMB));
	hFinalGlide=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FINALGLIDE));
  }

  //
  // Landables icons
  //
  switch (Appearance.IndLandable) {
	// WinPilot style
	case wpLandableDefault:
		hBmpAirportReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REACHABLE));
		hBmpAirportUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LANDABLE));
		hBmpFieldReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REACHABLE));
		hBmpFieldUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LANDABLE));
		break;
 
	// LK style 
	case wpLandableAltA:
	default:
		hBmpAirportReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRPORT_REACHABLE));
		hBmpAirportUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_AIRPORT_UNREACHABLE));
		hBmpFieldReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OUTFILED_REACHABLE));
		hBmpFieldUnReachable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OUTFILED_UNREACHABLE));
		break;

  }
}


void LKUnloadProfileBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Unload Profile Bitmaps\n"));
  #endif

  if (hBmpAirportReachable!=NULL)
	DeleteObject(hBmpAirportReachable);
  if (hBmpAirportUnReachable!=NULL)
	DeleteObject(hBmpAirportUnReachable);
  if (hBmpFieldReachable!=NULL)
	DeleteObject(hBmpFieldReachable);
  if (hBmpFieldUnReachable!=NULL)
	DeleteObject(hBmpFieldUnReachable);

  if (hCruise!=NULL)
	DeleteObject(hCruise);
  if (hClimb!= NULL)
	DeleteObject(hClimb);
  if (hFinalGlide!= NULL)
	DeleteObject(hFinalGlide);

  hBmpAirportReachable=NULL;
  hBmpAirportUnReachable=NULL;
  hBmpFieldReachable=NULL;
  hBmpFieldUnReachable=NULL;

  hCruise=NULL;
  hClimb=NULL;
  hFinalGlide=NULL;


}

