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


#define NULLBMP	LoadBitmap(hInst,MAKEINTRESOURCE(IDB_EMPTY))

HBITMAP LKLoadBitmap(const TCHAR *srcfile) {
 #if (WINDOWSPC>0)
 return (HBITMAP)LoadImage(GetModuleHandle(NULL),srcfile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
 #else
 return (HBITMAP)SHLoadDIBitmap(srcfile);
 #endif
}



//
// Load bitmaps that have no profile dependencies
//
// This is called once per session, at startup by the CREATE draw thread
// We are still missing TOPOLOGY shape icons to be added here in an array
// Kalman or Richard? I think it will speed up map drawing a bit.
//
void LKLoadFixedBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Loading Fixed Bitmaps\n"));
  #endif

  TCHAR srcfile[MAX_PATH];
  TCHAR sDir[MAX_PATH];
  TCHAR hires_suffix[4];

  LocalPath(sDir,TEXT(LKD_BITMAPS));

  if (UseHiresBitmap)
	_tcscpy(hires_suffix,_T("_H"));
  else
	_tcscpy(hires_suffix,_T(""));

  _stprintf(srcfile,_T("%s\\TPOINT_BIG%s.BMP"),sDir,hires_suffix);
  hTurnPoint=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\TPOINT_BIG_INV%s.BMP"),sDir,hires_suffix);
  hInvTurnPoint=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\TPOINT_SML%s.BMP"),sDir,hires_suffix);
  hSmall=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\TPOINT_SML_INV%s.BMP"),sDir,hires_suffix);
  hInvSmall=LKLoadBitmap(srcfile);


  _stprintf(srcfile,_T("%s\\BATTERY_FULL.BMP"),sDir);
  hBatteryFull=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_70.BMP"),sDir);
  hBattery70=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_50.BMP"),sDir);
  hBattery50=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_25.BMP"),sDir);
  hBattery25=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_15.BMP"),sDir);
  hBattery15=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\BUTTONLEFT32.BMP"),sDir);
  hBmpLeft32=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BUTTONRIGHT32.BMP"),sDir);
  hBmpRight32=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\THERMALSOURCE.BMP"),sDir);
  hBmpThermalSource=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\AAT_TARGET.BMP"),sDir);
  hBmpTarget=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\SCROLLBARTOP.BMP"),sDir);
  hScrollBarBitmapTop=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\SCROLLBARMID.BMP"),sDir);
  hScrollBarBitmapMid=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\SCROLLBARBOT.BMP"),sDir);
  hScrollBarBitmapBot=LKLoadBitmap(srcfile);


  _stprintf(srcfile,_T("%s\\MARKER%s.BMP"),sDir,hires_suffix);
  hBmpMarker=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\FLARMTRAFFIC.BMP"),sDir);
  hFLARMTraffic=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\LOGGER1.BMP"),sDir);
  hLogger=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\LOGGER0.BMP"),sDir);
  hLoggerOff=LKLoadBitmap(srcfile);

  // For low zooms, we use Small icon (a dot in fact)
  _stprintf(srcfile,_T("%s\\MOUNTOP%s.BMP"),sDir,hires_suffix);
  hMountop=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MOUNTPASS%s.BMP"),sDir,hires_suffix);
  hMountpass=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRIDGE%s.BMP"),sDir,hires_suffix);
  hBridge=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\INTERSECT%s.BMP"),sDir,hires_suffix);
  hIntersect=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\TERRWARNING%s.BMP"),sDir,hires_suffix);
  hTerrainWarning=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\ASPWARNING%s.BMP"),sDir,hires_suffix);
  hAirspaceWarning=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\TEAMMATEPOS%s.BMP"),sDir,hires_suffix);
  hBmpTeammatePosition=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE0.BMP"),sDir);
  hAirspaceBitmap[0]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE1.BMP"),sDir);
  hAirspaceBitmap[1]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE2.BMP"),sDir);
  hAirspaceBitmap[2]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE3.BMP"),sDir);
  hAirspaceBitmap[3]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE4.BMP"),sDir);
  hAirspaceBitmap[4]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE5.BMP"),sDir);
  hAirspaceBitmap[5]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE6.BMP"),sDir);
  hAirspaceBitmap[6]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BRUSH_AIRSPACE7.BMP"),sDir);
  hAirspaceBitmap[7]=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\BRUSH_ABOVETERR.BMP"),sDir);
  hAboveTerrainBitmap=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\DAM%s.BMP"),sDir,hires_suffix);
  hDam=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\SENDER%s.BMP"),sDir,hires_suffix);
  hSender=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\NDB%s.BMP"),sDir,hires_suffix);
  hNdb=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\LKTHERMAL%s.BMP"),sDir,hires_suffix);
  hLKThermal=LKLoadBitmap(srcfile);

  if (hTurnPoint==NULL)
	StartupStore(_T("... ERROR! MISSING BITMAPS!! CHECK SYSTEM BITMAPS!%s"),NEWLINE);

  //
  // Careful: the LK code using bitmaps will not check if they exist. If a bitmap
  // is not loaded, the Select will silently fail and the old bitmap in use will
  // be kept in use, resulting in confusing painting. So we load a null bitmap here.
  //

  if (hTurnPoint==NULL) hTurnPoint=NULLBMP;
  if (hInvTurnPoint==NULL) hInvTurnPoint=NULLBMP;
  if (hSmall==NULL) hSmall=NULLBMP;
  if (hInvSmall==NULL) hInvSmall=NULLBMP;

  if (hBatteryFull==NULL) hBatteryFull=NULLBMP;
  if (hBattery70==NULL) hBattery70=NULLBMP;
  if (hBattery50==NULL) hBattery50=NULLBMP;
  if (hBattery25==NULL) hBattery25=NULLBMP;
  if (hBattery15==NULL) hBattery15=NULLBMP;

  if (hBmpLeft32==NULL) hBmpLeft32=NULLBMP;
  if (hBmpRight32==NULL) hBmpRight32=NULLBMP;

  if (hBmpThermalSource==NULL) hBmpThermalSource=NULLBMP;
  if (hBmpTarget==NULL) hBmpTarget=NULLBMP;

  if (hScrollBarBitmapTop==NULL) hScrollBarBitmapTop=NULLBMP;
  if (hScrollBarBitmapMid==NULL) hScrollBarBitmapMid=NULLBMP;
  if (hScrollBarBitmapBot==NULL) hScrollBarBitmapBot=NULLBMP;

  if (hFLARMTraffic==NULL) hFLARMTraffic=NULLBMP;
  if (hLogger==NULL) hLogger=NULLBMP;
  if (hLoggerOff==NULL) hLoggerOff=NULLBMP;
  if (hBmpMarker==NULL) hBmpMarker=NULLBMP;

  if (hMountop==NULL) hMountop=NULLBMP;
  if (hMountpass==NULL) hMountpass=NULLBMP;
  if (hBridge==NULL) hBridge=NULLBMP;
  if (hIntersect==NULL) hIntersect=NULLBMP;

  if (hTerrainWarning==NULL) hTerrainWarning=NULLBMP;
  if (hAirspaceWarning==NULL) hAirspaceWarning=NULLBMP;
  if (hBmpTeammatePosition==NULL) hBmpTeammatePosition=NULLBMP;

  if (hAirspaceBitmap[0]==NULL) hAirspaceBitmap[0]=NULLBMP;
  if (hAirspaceBitmap[1]==NULL) hAirspaceBitmap[1]=NULLBMP;
  if (hAirspaceBitmap[2]==NULL) hAirspaceBitmap[2]=NULLBMP;
  if (hAirspaceBitmap[3]==NULL) hAirspaceBitmap[3]=NULLBMP;
  if (hAirspaceBitmap[4]==NULL) hAirspaceBitmap[4]=NULLBMP;
  if (hAirspaceBitmap[5]==NULL) hAirspaceBitmap[5]=NULLBMP;
  if (hAirspaceBitmap[6]==NULL) hAirspaceBitmap[6]=NULLBMP;
  if (hAirspaceBitmap[7]==NULL) hAirspaceBitmap[7]=NULLBMP;

  if (hAboveTerrainBitmap==NULL) hAboveTerrainBitmap=NULLBMP;

  if (hDam==NULL) hDam=NULLBMP;
  if (hSender==NULL) hSender=NULLBMP;
  if (hNdb==NULL) hNdb=NULLBMP;
  if (hLKThermal==NULL) hLKThermal=NULLBMP;
}


//
// Unloading bitmaps can be done only after removing brushes!
// No need to set them NULL, because they are removed on exit.
//
void LKUnloadFixedBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Unload Fixed Bitmaps\n"));
  #endif

  if (hTurnPoint!=NULL)	DeleteObject(hTurnPoint);
  if (hSmall!=NULL) DeleteObject(hSmall);
  if (hInvTurnPoint!=NULL) DeleteObject(hInvTurnPoint);
  if (hInvSmall!=NULL) DeleteObject(hInvSmall);
  if (hFLARMTraffic!=NULL) DeleteObject(hFLARMTraffic);
  if (hTerrainWarning!=NULL) DeleteObject(hTerrainWarning);
  if (hAirspaceWarning!=NULL) DeleteObject(hAirspaceWarning);
  if (hLogger!=NULL) DeleteObject(hLogger);
  if (hLoggerOff!=NULL) DeleteObject(hLoggerOff);
  if (hBatteryFull!=NULL) DeleteObject(hBatteryFull);
  if (hBattery70!=NULL) DeleteObject(hBattery70);
  if (hBattery50!=NULL) DeleteObject(hBattery50);
  if (hBattery25!=NULL) DeleteObject(hBattery25);
  if (hBattery15!=NULL) DeleteObject(hBattery15);
    
  if (hBmpThermalSource!=NULL) DeleteObject(hBmpThermalSource);
  if (hBmpTarget!=NULL) DeleteObject(hBmpTarget);
  if (hBmpTeammatePosition!=NULL) DeleteObject(hBmpTeammatePosition);
  if (hBmpMarker!=NULL) DeleteObject(hBmpMarker);

  for(short i=0;i<NUMAIRSPACEBRUSHES;i++) {
	if (hAirspaceBitmap[i]!=NULL) DeleteObject(hAirspaceBitmap[i]);
  }

  if (hAboveTerrainBitmap!=NULL) DeleteObject(hAboveTerrainBitmap);

  if (hBmpLeft32!=NULL) DeleteObject(hBmpLeft32);
  if (hBmpRight32!=NULL) DeleteObject(hBmpRight32);

  if (hScrollBarBitmapTop!=NULL) DeleteObject(hScrollBarBitmapTop);
  if (hScrollBarBitmapMid!=NULL) DeleteObject(hScrollBarBitmapMid);
  if (hScrollBarBitmapBot!=NULL) DeleteObject(hScrollBarBitmapBot);

  if (hMountop!=NULL) DeleteObject(hMountop);
  if (hMountpass!=NULL) DeleteObject(hMountpass);
  if (hBridge!=NULL) DeleteObject(hBridge);
  if (hIntersect!=NULL) DeleteObject(hIntersect);
  if (hDam!=NULL) DeleteObject(hDam);
  if (hSender!=NULL) DeleteObject(hSender);
  if (hNdb!=NULL) DeleteObject(hNdb);
  if (hLKThermal!=NULL) DeleteObject(hLKThermal);
}



//
// Load bitmaps that are affected by a profile 
//
void LKLoadProfileBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Loading Profile Bitmaps\n"));
  #endif

  TCHAR srcfile[MAX_PATH];
  TCHAR sDir[MAX_PATH];
  TCHAR hires_suffix[4];

  LocalPath(sDir,TEXT(LKD_BITMAPS));

  if (UseHiresBitmap)
	_tcscpy(hires_suffix,_T("_H"));
  else
	_tcscpy(hires_suffix,_T(""));


  if ( ISPARAGLIDER ) {
	_stprintf(srcfile,_T("%s\\ICOCRUISE_PG.BMP"),sDir);
	hCruise=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%s\\ICOCLIMB_PG.BMP"),sDir);
	hClimb=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%s\\ICOFINAL_PG.BMP"),sDir);
	hFinalGlide=LKLoadBitmap(srcfile);
  } else {
	_stprintf(srcfile,_T("%s\\ICOCRUISE_AC.BMP"),sDir);
	hCruise=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%s\\ICOCLIMB_AC.BMP"),sDir);
	hClimb=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%s\\ICOFINAL_AC.BMP"),sDir);
	hFinalGlide=LKLoadBitmap(srcfile);
  }

  //
  // Landables icons
  //
  switch (Appearance.IndLandable) {
	// WinPilot style
	case wpLandableDefault:
		_stprintf(srcfile,_T("%s\\APT1_REACH%s.BMP"),sDir,hires_suffix);
		hBmpAirportReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%s\\APT1_UNREACH%s.BMP"),sDir,hires_suffix);
		hBmpAirportUnReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%s\\FLD1_REACH%s.BMP"),sDir,hires_suffix);
		hBmpFieldReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%s\\FLD1_UNREACH%s.BMP"),sDir,hires_suffix);
		hBmpFieldUnReachable=LKLoadBitmap(srcfile);

		break;
 
	// LK style 
	case wpLandableAltA:
	default:
		_stprintf(srcfile,_T("%s\\APT2_REACH%s.BMP"),sDir,hires_suffix);
		hBmpAirportReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%s\\APT2_UNREACH%s.BMP"),sDir,hires_suffix);
		hBmpAirportUnReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%s\\FLD2_REACH%s.BMP"),sDir,hires_suffix);
		hBmpFieldReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%s\\FLD2_UNREACH%s.BMP"),sDir,hires_suffix);
		hBmpFieldUnReachable=LKLoadBitmap(srcfile);

		break;

  }

  if (hBmpAirportReachable==NULL) hBmpAirportReachable=NULLBMP;
  if (hBmpAirportUnReachable==NULL) hBmpAirportUnReachable=NULLBMP;
  if (hBmpFieldReachable==NULL) hBmpFieldReachable=NULLBMP;
  if (hBmpFieldUnReachable==NULL) hBmpFieldUnReachable=NULLBMP;

  if (hCruise==NULL) hCruise=NULLBMP;
  if (hClimb==NULL) hClimb=NULLBMP;
  if (hFinalGlide==NULL) hFinalGlide=NULLBMP;

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

