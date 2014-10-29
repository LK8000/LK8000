/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "resource.h"
#include "LKMapWindow.h"
#include "utils/stl_utils.h"
#include <functional>
#define STATIC_BITMAPS
#include "Bitmaps.h"

using std::placeholders::_1;

unsigned short Bitmaps_Errors = 0;
static std::set<std::tstring> setMissingBitmap;

LKBitmap LKLoadBitmap(const TCHAR *srcfile) {
    LKBitmap hBmp;
    if (!hBmp.LoadFromFile(srcfile)) {
        auto ib = setMissingBitmap.insert(srcfile);
        if(ib.second) {
            StartupStore(_T(".... Failed to load file : <%s>%s"), srcfile, NEWLINE);
        }
        hBmp.LoadFromResource(MAKEINTRESOURCE(IDB_EMPTY));
        ++Bitmaps_Errors;
    }
    return hBmp;
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
}


//
// Unloading bitmaps can be done only after removing brushes!
// No need to set them NULL, because they are removed on exit.
//

void LKUnloadFixedBitmaps(void) {

#if TESTBENCH
    StartupStore(_T("... Unload Fixed Bitmaps\n"));
#endif

    hTurnPoint.Release();
    hSmall.Release();
    hInvTurnPoint.Release();
    hInvSmall.Release();
    hFLARMTraffic.Release();
    hTerrainWarning.Release();
    hAirspaceWarning.Release();
    hLogger.Release();
    hLoggerOff.Release();

    hBatteryFull.Release();
    hBatteryFull.Release();
    hBattery96.Release();
    hBattery84.Release();
    hBattery72.Release();
    hBattery60.Release();
    hBattery48.Release();
    hBattery36.Release();
    hBattery24.Release();
    hBattery12.Release();


    hNoTrace.Release();
    hFullTrace.Release();
    hClimbTrace.Release();
    hHeadRight.Release();
    hNorthUp.Release();
    hHeadUp.Release();

    hMM0.Release();
    hMM1.Release();
    hMM2.Release();
    hMM3.Release();
    hMM4.Release();
    hMM5.Release();
    hMM6.Release();
    hMM7.Release();
    hMM8.Release();

    hBmpThermalSource.Release();
    hBmpTarget.Release();
    hBmpTeammatePosition.Release();
    hBmpMarker.Release();

    std::for_each(begin(hAirspaceBitmap), end(hAirspaceBitmap), std::bind(&LKBitmap::Release, _1));

    hAboveTerrainBitmap.Release();

    hBmpLeft32.Release();
    hBmpRight32.Release();

    hScrollBarBitmapTop.Release();
    hScrollBarBitmapMid.Release();
    hScrollBarBitmapBot.Release();

    hMountop.Release();
    hMountpass.Release();
    hBridge.Release();
    hIntersect.Release();
    hDam.Release();
    hSender.Release();
    hNdb.Release();
    hVor.Release();
    hCoolTower.Release();
    hTunnel.Release();
    hPowerPlant.Release();
    hCastle.Release();
    hLKThermal.Release();
    hLKThermalRed.Release();
    hLKPictori.Release();

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
}

void LKUnloadProfileBitmaps(void) {

#if TESTBENCH
    StartupStore(_T("... Unload Profile Bitmaps\n"));
#endif

    hBmpAirportReachable.Release();
    hBmpAirportUnReachable.Release();
    hBmpFieldReachable.Release();
    hBmpFieldUnReachable.Release();

    hCruise.Release();
    hClimb.Release();
    hFinalGlide.Release();
}
