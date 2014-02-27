/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"
#include "LKMapWindow.h"
#include <shellapi.h>

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
  _stprintf(srcfile,_T("%s\\BATTERY_FULLC.BMP"),sDir);
  hBatteryFullC=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\BATTERY_96.BMP"),sDir);
  hBattery96=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_84.BMP"),sDir);
  hBattery84=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_72.BMP"),sDir);
  hBattery72=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_60.BMP"),sDir);
  hBattery60=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_48.BMP"),sDir);
  hBattery48=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_36.BMP"),sDir);
  hBattery36=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_24.BMP"),sDir);
  hBattery24=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\BATTERY_12.BMP"),sDir);
  hBattery12=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\TRACE_NO.BMP"),sDir);
  hNoTrace=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\TRACE_FULL.BMP"),sDir);
  hFullTrace=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\TRACE_CLIMB.BMP"),sDir);
  hClimbTrace=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\HEAD_UP.BMP"),sDir);
  hHeadUp=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\NORTH_UP.BMP"),sDir);
  hNorthUp=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\HEAD_RIGHT.BMP"),sDir);
  hHeadRight=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\MM0.BMP"),sDir);
  hMM0=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM1.BMP"),sDir);
  hMM1=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM2.BMP"),sDir);
  hMM2=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM3.BMP"),sDir);
  hMM3=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM4.BMP"),sDir);
  hMM4=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM5.BMP"),sDir);
  hMM5=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM6.BMP"),sDir);
  hMM6=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM7.BMP"),sDir);
  hMM7=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\MM8.BMP"),sDir);
  hMM8=LKLoadBitmap(srcfile);

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
  _stprintf(srcfile,_T("%s\\VOR%s.BMP"),sDir,hires_suffix);
  hVor=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\COOLTOWER%s.BMP"),sDir,hires_suffix);
  hCoolTower=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\TUNNEL%s.BMP"),sDir,hires_suffix);
  hTunnel=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\POWERPLANT%s.BMP"),sDir,hires_suffix);
  hPowerPlant=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\CASTLE%s.BMP"),sDir,hires_suffix);
  hCastle=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\LKTHERMAL%s.BMP"),sDir,hires_suffix);
  hLKThermal=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%s\\LKTHERMAL_RED%s.BMP"),sDir,hires_suffix);
  hLKThermalRed=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%s\\PICTORI%s.BMP"),sDir,hires_suffix);
  hLKPictori=LKLoadBitmap(srcfile);

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
  if (hBatteryFullC==NULL) hBatteryFullC=NULLBMP;
  if (hBattery96==NULL) hBattery96=NULLBMP;
  if (hBattery84==NULL) hBattery84=NULLBMP;
  if (hBattery72==NULL) hBattery72=NULLBMP;
  if (hBattery60==NULL) hBattery60=NULLBMP;
  if (hBattery48==NULL) hBattery48=NULLBMP;
  if (hBattery36==NULL) hBattery36=NULLBMP;
  if (hBattery24==NULL) hBattery24=NULLBMP;
  if (hBattery12==NULL) hBattery12=NULLBMP;

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
  if (hVor==NULL) hVor=NULLBMP;
  if (hCoolTower==NULL) hCoolTower=NULLBMP;
  if (hTunnel==NULL) hTunnel=NULLBMP;
  if (hPowerPlant==NULL) hPowerPlant=NULLBMP;
  if (hCastle==NULL) hCastle=NULLBMP;
  if (hLKThermal==NULL) hLKThermal=NULLBMP;
  if (hLKThermalRed==NULL) hLKThermalRed=NULLBMP;
  
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
  if (hBatteryFullC!=NULL) DeleteObject(hBatteryFull);
  if (hBattery96!=NULL) DeleteObject(hBattery96);
  if (hBattery84!=NULL) DeleteObject(hBattery84);
  if (hBattery72!=NULL) DeleteObject(hBattery72);
  if (hBattery60!=NULL) DeleteObject(hBattery60);
  if (hBattery48!=NULL) DeleteObject(hBattery48);
  if (hBattery36!=NULL) DeleteObject(hBattery36);
  if (hBattery24!=NULL) DeleteObject(hBattery24);
  if (hBattery12!=NULL) DeleteObject(hBattery12);


  if (hNoTrace   !=NULL) DeleteObject(hNoTrace);
  if (hFullTrace !=NULL) DeleteObject(hFullTrace);
  if (hClimbTrace!=NULL) DeleteObject(hClimbTrace);
  if (hHeadRight !=NULL) DeleteObject(hHeadRight);
  if (hNorthUp   !=NULL) DeleteObject(hNorthUp);
  if (hHeadUp    !=NULL) DeleteObject(hHeadUp);

  if (hMM0	 !=NULL) DeleteObject(hMM0);
  if (hMM1	 !=NULL) DeleteObject(hMM1);
  if (hMM2	 !=NULL) DeleteObject(hMM2);
  if (hMM3	 !=NULL) DeleteObject(hMM3);
  if (hMM4	 !=NULL) DeleteObject(hMM4);
  if (hMM5	 !=NULL) DeleteObject(hMM5);
  if (hMM6	 !=NULL) DeleteObject(hMM6);
  if (hMM7	 !=NULL) DeleteObject(hMM7);
  if (hMM8	 !=NULL) DeleteObject(hMM8);

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
  if (hVor!=NULL) DeleteObject(hVor);
  if (hCoolTower!=NULL) DeleteObject(hCoolTower);
  if (hTunnel!=NULL) DeleteObject(hTunnel);
  if (hPowerPlant!=NULL) DeleteObject(hPowerPlant);
  if (hCastle!=NULL) DeleteObject(hCastle);
  if (hLKThermal!=NULL) DeleteObject(hLKThermal);
  if (hLKThermalRed!=NULL) DeleteObject(hLKThermalRed);
  if (hLKPictori!=NULL) DeleteObject(hLKPictori);

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
#ifdef OLD_WINPILOT_BITMAPS
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
#endif
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

