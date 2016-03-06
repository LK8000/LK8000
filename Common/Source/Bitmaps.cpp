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
#include "resource_data.h"


using std::placeholders::_1;

unsigned short Bitmaps_Errors = 0;
static std::set<tstring> setMissingBitmap;

#ifdef USE_GDI
    #define IMG_EXT "BMP"
#else
    #define IMG_EXT "PNG"
#endif

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

  SystemPath(sDir,TEXT(LKD_BITMAPS));
  _tcscat(sDir, _T(DIRSEP));
  if (UseHiresBitmap)
	_tcscpy(hires_suffix,_T("_H"));
  else
	_tcscpy(hires_suffix,_T(""));

  _stprintf(srcfile,_T("%sTPOINT_BIG%s." IMG_EXT),sDir,hires_suffix);
  hTurnPoint=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sTPOINT_BIG_INV%s." IMG_EXT),sDir,hires_suffix);
  hInvTurnPoint=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sTPOINT_SML%s." IMG_EXT),sDir,hires_suffix);
  hSmall=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sTPOINT_SML_INV%s." IMG_EXT),sDir,hires_suffix);
  hInvSmall=LKLoadBitmap(srcfile);

#ifdef DITHER
  _stprintf(srcfile,_T("%sKB_BATTERY_FULL." IMG_EXT),sDir);
  hKB_BatteryFull=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_FULLC." IMG_EXT),sDir);
  hKB_BatteryFullC=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_96." IMG_EXT),sDir);
  hKB_Battery96=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_84." IMG_EXT),sDir);
  hKB_Battery84=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_72." IMG_EXT),sDir);
  hKB_Battery72=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_60." IMG_EXT),sDir);
  hKB_Battery60=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_48." IMG_EXT),sDir);
  hKB_Battery48=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_36." IMG_EXT),sDir);
  hKB_Battery36=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_24." IMG_EXT),sDir);
  hKB_Battery24=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sKB_BATTERY_12." IMG_EXT),sDir);
  hKB_Battery12=LKLoadBitmap(srcfile);
#endif
  _stprintf(srcfile,_T("%sBATTERY_FULL." IMG_EXT),sDir);
  hBatteryFull=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_FULLC." IMG_EXT),sDir);
  hBatteryFullC=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sBATTERY_96." IMG_EXT),sDir);
  hBattery96=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_84." IMG_EXT),sDir);
  hBattery84=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_72." IMG_EXT),sDir);
  hBattery72=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_60." IMG_EXT),sDir);
  hBattery60=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_48." IMG_EXT),sDir);
  hBattery48=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_36." IMG_EXT),sDir);
  hBattery36=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_24." IMG_EXT),sDir);
  hBattery24=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBATTERY_12." IMG_EXT),sDir);
  hBattery12=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sTRACE_NO." IMG_EXT),sDir);
  hNoTrace=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sTRACE_FULL." IMG_EXT),sDir);
  hFullTrace=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sTRACE_CLIMB." IMG_EXT),sDir);
  hClimbTrace=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sHEAD_UP." IMG_EXT),sDir);
  hHeadUp=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sNORTH_UP." IMG_EXT),sDir);
  hNorthUp=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sHEAD_RIGHT." IMG_EXT),sDir);
  hHeadRight=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sMM0." IMG_EXT),sDir);
  hMM0=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM1." IMG_EXT),sDir);
  hMM1=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM2." IMG_EXT),sDir);
  hMM2=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM3." IMG_EXT),sDir);
  hMM3=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM4." IMG_EXT),sDir);
  hMM4=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM5." IMG_EXT),sDir);
  hMM5=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM6." IMG_EXT),sDir);
  hMM6=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM7." IMG_EXT),sDir);
  hMM7=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMM8." IMG_EXT),sDir);
  hMM8=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sIMM0." IMG_EXT),sDir);
  hIMM0=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM1." IMG_EXT),sDir);
  hIMM1=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM2." IMG_EXT),sDir);
  hIMM2=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM3." IMG_EXT),sDir);
  hIMM3=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM4." IMG_EXT),sDir);
  hIMM4=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM5." IMG_EXT),sDir);
  hIMM5=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM6." IMG_EXT),sDir);
  hIMM6=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM7." IMG_EXT),sDir);
  hIMM7=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sIMM8." IMG_EXT),sDir);
  hIMM8=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sBUTTONLEFT32." IMG_EXT),sDir);
  hBmpLeft32=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBUTTONRIGHT32." IMG_EXT),sDir);
  hBmpRight32=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sTHERMALSOURCE." IMG_EXT),sDir);
  hBmpThermalSource=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sAAT_TARGET." IMG_EXT),sDir);
  hBmpTarget=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sSCROLLBARTOP." IMG_EXT),sDir);
  hScrollBarBitmapTop=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sSCROLLBARMID." IMG_EXT),sDir);
  hScrollBarBitmapMid=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sSCROLLBARBOT." IMG_EXT),sDir);
  hScrollBarBitmapBot=LKLoadBitmap(srcfile);


  _stprintf(srcfile,_T("%sMARKER%s." IMG_EXT),sDir,hires_suffix);
  hBmpMarker=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sFLARMTRAFFIC." IMG_EXT),sDir);
  hFLARMTraffic=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sLOGGER1." IMG_EXT),sDir);
  hLogger=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sLOGGER0." IMG_EXT),sDir);
  hLoggerOff=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sLOGGEROFF." IMG_EXT),sDir);
  hLoggerDisabled=LKLoadBitmap(srcfile);

  // For low zooms, we use Small icon (a dot in fact)
  _stprintf(srcfile,_T("%sMOUNTOP%s." IMG_EXT),sDir,hires_suffix);
  hMountop=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sMOUNTPASS%s." IMG_EXT),sDir,hires_suffix);
  hMountpass=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRIDGE%s." IMG_EXT),sDir,hires_suffix);
  hBridge=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sINTERSECT%s." IMG_EXT),sDir,hires_suffix);
  hIntersect=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sTERRWARNING%s." IMG_EXT),sDir,hires_suffix);
  hTerrainWarning=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sASPWARNING%s." IMG_EXT),sDir,hires_suffix);
  hAirspaceWarning=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sTEAMMATEPOS%s." IMG_EXT),sDir,hires_suffix);
  hBmpTeammatePosition=LKLoadBitmap(srcfile);

#ifdef HAVE_HATCHED_BRUSH
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE0." IMG_EXT),sDir);
  hAirspaceBitmap[0]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE1." IMG_EXT),sDir);
  hAirspaceBitmap[1]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE2." IMG_EXT),sDir);
  hAirspaceBitmap[2]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE3." IMG_EXT),sDir);
  hAirspaceBitmap[3]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE4." IMG_EXT),sDir);
  hAirspaceBitmap[4]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE5." IMG_EXT),sDir);
  hAirspaceBitmap[5]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE6." IMG_EXT),sDir);
  hAirspaceBitmap[6]=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sBRUSH_AIRSPACE7." IMG_EXT),sDir);
  hAirspaceBitmap[7]=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sBRUSH_ABOVETERR." IMG_EXT),sDir);
  hAboveTerrainBitmap=LKLoadBitmap(srcfile);
#endif
  
  _stprintf(srcfile,_T("%sDAM%s." IMG_EXT),sDir,hires_suffix);
  hDam=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sSENDER%s." IMG_EXT),sDir,hires_suffix);
  hSender=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sNDB%s." IMG_EXT),sDir,hires_suffix);
  hNdb=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sVOR%s." IMG_EXT),sDir,hires_suffix);
  hVor=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sCOOLTOWER%s." IMG_EXT),sDir,hires_suffix);
  hCoolTower=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sTUNNEL%s." IMG_EXT),sDir,hires_suffix);
  hTunnel=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sPOWERPLANT%s." IMG_EXT),sDir,hires_suffix);
  hPowerPlant=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sCASTLE%s." IMG_EXT),sDir,hires_suffix);
  hCastle=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sLKTHERMAL%s." IMG_EXT),sDir,hires_suffix);
  hLKThermal=LKLoadBitmap(srcfile);
  _stprintf(srcfile,_T("%sLKTHERMAL_RED%s." IMG_EXT),sDir,hires_suffix);
  hLKThermalRed=LKLoadBitmap(srcfile);

  _stprintf(srcfile,_T("%sPICTORI%s." IMG_EXT),sDir,hires_suffix);
  hLKPictori=LKLoadBitmap(srcfile);
  
  _stprintf(srcfile,_T("%sMC_VARIO_TICK%s." IMG_EXT),sDir,hires_suffix);
  hMcVario=LKLoadBitmap(srcfile);
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
    hInvTurnPoint.Release();
    hSmall.Release();
    hInvSmall.Release();
    hTerrainWarning.Release();
    hAirspaceWarning.Release();
    hLogger.Release();
    hLoggerOff.Release();
    hLoggerDisabled.Release();
    hFLARMTraffic.Release();
#ifdef DITHER
    hKB_BatteryFull.Release();
    hKB_BatteryFullC.Release();
    hKB_Battery96.Release();
    hKB_Battery84.Release();
    hKB_Battery72.Release();
    hKB_Battery60.Release();
    hKB_Battery48.Release();
    hKB_Battery36.Release();
    hKB_Battery24.Release();
    hKB_Battery12.Release();
#endif
    hBatteryFull.Release();
    hBatteryFullC.Release();
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
    hNorthUp.Release();
    hHeadRight.Release();
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
    hIMM0.Release();
    hIMM1.Release();
    hIMM2.Release();
    hIMM3.Release();
    hIMM4.Release();
    hIMM5.Release();
    hIMM6.Release();
    hIMM7.Release();
    hIMM8.Release();

    hBmpThermalSource.Release();
    hBmpTarget.Release();
    hBmpMarker.Release();
    hBmpTeammatePosition.Release();

#ifdef HAVE_HATCHED_BRUSH
    hAboveTerrainBitmap.Release();
    std::for_each(std::begin(hAirspaceBitmap), std::end(hAirspaceBitmap), std::bind(&LKBitmap::Release, _1));
#endif

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
    hCastle.Release();
    hPowerPlant.Release();
    hLKThermal.Release();
    hLKThermalRed.Release();
    hLKPictori.Release();

    hMcVario.Release();
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

  SystemPath(sDir,TEXT(LKD_BITMAPS));
  _tcscat(sDir, _T(DIRSEP));
        
  if (UseHiresBitmap)
	_tcscpy(hires_suffix,_T("_H"));
  else
	_tcscpy(hires_suffix,_T(""));


  if ( ISPARAGLIDER ) {
	_stprintf(srcfile,_T("%sICOCRUISE_PG." IMG_EXT),sDir);
	hCruise=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%sICOCLIMB_PG." IMG_EXT),sDir);
	hClimb=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%sICOFINAL_PG." IMG_EXT),sDir);
	hFinalGlide=LKLoadBitmap(srcfile);
  } else {
	_stprintf(srcfile,_T("%sICOCRUISE_AC." IMG_EXT),sDir);
	hCruise=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%sICOCLIMB_AC." IMG_EXT),sDir);
	hClimb=LKLoadBitmap(srcfile);
	_stprintf(srcfile,_T("%sICOFINAL_AC." IMG_EXT),sDir);
	hFinalGlide=LKLoadBitmap(srcfile);
  }

  //
  // Landables icons
  //
  switch (Appearance.IndLandable) {
	// WinPilot style
#ifdef OLD_WINPILOT_BITMAPS
	case wpLandableDefault:
		_stprintf(srcfile,_T("%sAPT1_REACH%s." IMG_EXT),sDir,hires_suffix);
		hBmpAirportReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%sAPT1_UNREACH%s." IMG_EXT),sDir,hires_suffix);
		hBmpAirportUnReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%sFLD1_REACH%s." IMG_EXT),sDir,hires_suffix);
		hBmpFieldReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%sFLD1_UNREACH%s." IMG_EXT),sDir,hires_suffix);
		hBmpFieldUnReachable=LKLoadBitmap(srcfile);

		break;
#endif
	// LK style 
	case wpLandableAltA:
	default:
		_stprintf(srcfile,_T("%sAPT2_REACH%s." IMG_EXT),sDir,hires_suffix);
		hBmpAirportReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%sAPT2_UNREACH%s." IMG_EXT),sDir,hires_suffix);
		hBmpAirportUnReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%sFLD2_REACH%s." IMG_EXT),sDir,hires_suffix);
		hBmpFieldReachable=LKLoadBitmap(srcfile);
		_stprintf(srcfile,_T("%sFLD2_UNREACH%s." IMG_EXT),sDir,hires_suffix);
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
