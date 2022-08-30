/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "Asset.hpp"


using std::placeholders::_1;

unsigned short Bitmaps_Errors = 0;
static std::set<tstring> setMissingBitmap;

#ifdef USE_GDI
    #define IMG_EXT "BMP"
#else
    #define IMG_EXT "PNG"
#endif

LKBitmap LKLoadBitmap(const TCHAR *sName, bool Hires) {
    LKBitmap hBmp;

    TCHAR srcfile[MAX_PATH];
#ifdef ANDROID

    _stprintf(srcfile,_T(LKD_BITMAPS "/%s%s." IMG_EXT), sName, Hires?_T("_H"):_T(""));
    bool success = hBmp.LoadAssetsFile(srcfile);
#else
    TCHAR sDir[MAX_PATH];
    SystemPath(sDir,TEXT(LKD_BITMAPS));
    int ret = _sntprintf(srcfile, MAX_PATH, _T("%s" DIRSEP "%s%s." IMG_EXT), sDir, sName, Hires?_T("_H"):_T(""));
    bool success = (ret < (MAX_PATH - 1)); // path too long ? 
    if(success) { 
        success = hBmp.LoadFromFile(srcfile);
    }
#endif
    if (!success) {
        auto ib = setMissingBitmap.insert(srcfile);
        if(ib.second) {
            StartupStore(_T(".... Failed to load file : <%s>"), srcfile);
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

  hTurnPoint = LKLoadBitmap(_T("TPOINT_BIG"), UseHiresBitmap);
  hInvTurnPoint = LKLoadBitmap(_T("TPOINT_BIG_INV"), UseHiresBitmap);
  hSmall = LKLoadBitmap(_T("TPOINT_SML"), UseHiresBitmap);
  hInvSmall = LKLoadBitmap(_T("TPOINT_SML_INV"), UseHiresBitmap);

#if defined(DITHER) || (defined(ANDROID) && defined(__arm__))
  hKB_BatteryFull=LKLoadBitmap(_T("KB_BATTERY_FULL"));
  hKB_BatteryFullC=LKLoadBitmap(_T("KB_BATTERY_FULLC"));
  hKB_Battery96=LKLoadBitmap(_T("KB_BATTERY_96"));
  hKB_Battery84=LKLoadBitmap(_T("KB_BATTERY_84"));
  hKB_Battery72=LKLoadBitmap(_T("KB_BATTERY_72"));
  hKB_Battery60=LKLoadBitmap(_T("KB_BATTERY_60"));
  hKB_Battery48=LKLoadBitmap(_T("KB_BATTERY_48"));
  hKB_Battery36=LKLoadBitmap(_T("KB_BATTERY_36"));
  hKB_Battery24=LKLoadBitmap(_T("KB_BATTERY_24"));
  hKB_Battery12=LKLoadBitmap(_T("KB_BATTERY_12"));
#endif

  hBatteryFull=LKLoadBitmap(_T("BATTERY_FULL"));
  hBatteryFullC=LKLoadBitmap(_T("BATTERY_FULLC"));
  hBattery96=LKLoadBitmap(_T("BATTERY_96"));
  hBattery84=LKLoadBitmap(_T("BATTERY_84"));
  hBattery72=LKLoadBitmap(_T("BATTERY_72"));
  hBattery60=LKLoadBitmap(_T("BATTERY_60"));
  hBattery48=LKLoadBitmap(_T("BATTERY_48"));
  hBattery36=LKLoadBitmap(_T("BATTERY_36"));
  hBattery24=LKLoadBitmap(_T("BATTERY_24"));
  hBattery12=LKLoadBitmap(_T("BATTERY_12"));

  hNoTrace=LKLoadBitmap(_T("TRACE_NO"));
  hFullTrace=LKLoadBitmap(_T("TRACE_FULL"));
  hClimbTrace=LKLoadBitmap(_T("TRACE_CLIMB"));

  hHeadUp=LKLoadBitmap(_T("HEAD_UP"));
  hNorthUp=LKLoadBitmap(_T("NORTH_UP"));
  hHeadRight=LKLoadBitmap(_T("HEAD_RIGHT"));

  hMM0=LKLoadBitmap(_T("MM0"));
  hMM1=LKLoadBitmap(_T("MM1"));
  hMM2=LKLoadBitmap(_T("MM2"));
  hMM3=LKLoadBitmap(_T("MM3"));
  hMM4=LKLoadBitmap(_T("MM4"));
  hMM5=LKLoadBitmap(_T("MM5"));
  hMM6=LKLoadBitmap(_T("MM6"));
  hMM7=LKLoadBitmap(_T("MM7"));
  hMM8=LKLoadBitmap(_T("MM8"));

  hIMM0=LKLoadBitmap(_T("IMM0"));
  hIMM1=LKLoadBitmap(_T("IMM1"));
  hIMM2=LKLoadBitmap(_T("IMM2"));
  hIMM3=LKLoadBitmap(_T("IMM3"));
  hIMM4=LKLoadBitmap(_T("IMM4"));
  hIMM5=LKLoadBitmap(_T("IMM5"));
  hIMM6=LKLoadBitmap(_T("IMM6"));
  hIMM7=LKLoadBitmap(_T("IMM7"));
  hIMM8=LKLoadBitmap(_T("IMM8"));

  hBmpLeft32=LKLoadBitmap(_T("BUTTONLEFT32"));
  hBmpRight32=LKLoadBitmap(_T("BUTTONRIGHT32"));

  hBmpThermalSource=LKLoadBitmap(_T("THERMALSOURCE"));
  hBmpTarget=LKLoadBitmap(_T("AAT_TARGET"));

  hScrollBarBitmapTop=LKLoadBitmap(_T("SCROLLBARTOP"));
  hScrollBarBitmapMid=LKLoadBitmap(_T("SCROLLBARMID"));
  hScrollBarBitmapBot=LKLoadBitmap(_T("SCROLLBARBOT"));

  hBmpMarker=LKLoadBitmap(_T("MARKER"), UseHiresBitmap);

  hFLARMTraffic=LKLoadBitmap(_T("FLARMTRAFFIC"));
  hLogger=LKLoadBitmap(_T("LOGGER1"));
  hLoggerOff=LKLoadBitmap(_T("LOGGER0"));
  hLoggerDisabled=LKLoadBitmap(_T("LOGGEROFF"));

  // For low zooms, we use Small icon (a dot in fact)
  hMountop=LKLoadBitmap(_T("MOUNTOP"), UseHiresBitmap);
  hMountpass=LKLoadBitmap(_T("MOUNTPASS"), UseHiresBitmap);
  hBridge=LKLoadBitmap(_T("BRIDGE"), UseHiresBitmap);
  hIntersect=LKLoadBitmap(_T("INTERSECT"), UseHiresBitmap);

  hTerrainWarning=LKLoadBitmap(_T("TERRWARNING"), UseHiresBitmap);
  hAirspaceWarning=LKLoadBitmap(_T("ASPWARNING"), UseHiresBitmap);
  hBmpTeammatePosition=LKLoadBitmap(_T("TEAMMATEPOS"), UseHiresBitmap);

#ifdef HAVE_HATCHED_BRUSH
  hAirspaceBitmap[0]=LKLoadBitmap(_T("BRUSH_AIRSPACE0"));
  hAirspaceBitmap[1]=LKLoadBitmap(_T("BRUSH_AIRSPACE1"));
  hAirspaceBitmap[2]=LKLoadBitmap(_T("BRUSH_AIRSPACE2"));
  hAirspaceBitmap[3]=LKLoadBitmap(_T("BRUSH_AIRSPACE3"));
  hAirspaceBitmap[4]=LKLoadBitmap(_T("BRUSH_AIRSPACE4"));
  hAirspaceBitmap[5]=LKLoadBitmap(_T("BRUSH_AIRSPACE5"));
  hAirspaceBitmap[6]=LKLoadBitmap(_T("BRUSH_AIRSPACE6"));
  hAirspaceBitmap[7]=LKLoadBitmap(_T("BRUSH_AIRSPACE7"));

  hAboveTerrainBitmap=LKLoadBitmap(_T("BRUSH_ABOVETERR"));
#endif
  
  hDam=LKLoadBitmap(_T("DAM"), UseHiresBitmap);
  hSender=LKLoadBitmap(_T("SENDER"), UseHiresBitmap);
  hNdb=LKLoadBitmap(_T("NDB"), UseHiresBitmap);
  hVor=LKLoadBitmap(_T("VOR"), UseHiresBitmap);
  hCoolTower=LKLoadBitmap(_T("COOLTOWER"), UseHiresBitmap);
  hTunnel=LKLoadBitmap(_T("TUNNEL"), UseHiresBitmap);
  hPowerPlant=LKLoadBitmap(_T("POWERPLANT"), UseHiresBitmap);
  hCastle=LKLoadBitmap(_T("CASTLE"), UseHiresBitmap);
  hLKThermal=LKLoadBitmap(_T("LKTHERMAL"), UseHiresBitmap);
  hLKThermalRed=LKLoadBitmap(_T("LKTHERMAL_RED"), UseHiresBitmap);

  hLKPictori=LKLoadBitmap(_T("PICTORI"), UseHiresBitmap);
  
  hMcVario=LKLoadBitmap(_T("MC_VARIO_TICK"), UseHiresBitmap);

  if (!IsDithered()||IsEinkColored()) {
    hXCFF=LKLoadBitmap(_T("FREE_FLIGHT"), UseHiresBitmap);
    hXCFT=LKLoadBitmap(_T("FLAT_TRIANGLE"), UseHiresBitmap);
    hXCFAI=LKLoadBitmap(_T("FAI_TRIANGLE"), UseHiresBitmap);
  }else{
    hXCFF=LKLoadBitmap(_T("FREE_FLIGHTB"), UseHiresBitmap);
    hXCFT=LKLoadBitmap(_T("FLAT_TRIANGLEB"), UseHiresBitmap);
    hXCFAI=LKLoadBitmap(_T("FAI_TRIANGLEB"), UseHiresBitmap);
  }
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
#if defined(DITHER) || (defined(ANDROID) && defined(__arm__))
    hKB_BatteryFullC.Release();
    hKB_BatteryFull.Release();
    hKB_Battery96.Release();
    hKB_Battery84.Release();
    hKB_Battery72.Release();
    hKB_Battery60.Release();
    hKB_Battery48.Release();
    hKB_Battery36.Release();
    hKB_Battery24.Release();
    hKB_Battery12.Release();
#endif
    hBatteryFullC.Release();
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

    hXCFF.Release();
    hXCFT.Release();
    hXCFAI.Release();

}



//
// Load bitmaps that are affected by a profile 
//
void LKLoadProfileBitmaps(void) {

  #if TESTBENCH
  StartupStore(_T("... Loading Profile Bitmaps\n"));
  #endif

  if ( ISPARAGLIDER ) {
	hCruise=LKLoadBitmap(_T("ICOCRUISE_PG"));
	hClimb=LKLoadBitmap(_T("ICOCLIMB_PG"));
	hFinalGlide=LKLoadBitmap(_T("ICOFINAL_PG"));
  } else {
    hCruise=LKLoadBitmap(_T("ICOCRUISE_AC"));
    hClimb=LKLoadBitmap(_T("ICOCLIMB_AC"));
    hFinalGlide=LKLoadBitmap(_T("ICOFINAL_AC"));
  }

  //
  // Landables icons
  //
  switch (Appearance.IndLandable) {
	// WinPilot style
#ifdef OLD_WINPILOT_BITMAPS
	case wpLandableDefault:
		hBmpAirportReachable=LKLoadBitmap(_T("APT1_REACH"), UseHiresBitmap);
		hBmpAirportUnReachable=LKLoadBitmap(_T("APT1_UNREACH"), UseHiresBitmap);
		hBmpFieldReachable=LKLoadBitmap(_T("FLD1_REACH"), UseHiresBitmap);
		hBmpFieldUnReachable=LKLoadBitmap(_T("FLD1_UNREACH"), UseHiresBitmap);

		break;
#endif
	// LK style 
	case wpLandableAltA:
	default:
      if (!IsDithered()) {             // On KOBO RED is mutch more readeble than green
        hBmpAirportReachable = LKLoadBitmap(_T("APT2_REACH"), UseHiresBitmap);
        hBmpAirportUnReachable = LKLoadBitmap(_T("APT2_UNREACH"), UseHiresBitmap);
        hBmpFieldReachable = LKLoadBitmap(_T("FLD2_REACH"), UseHiresBitmap);
        hBmpFieldUnReachable = LKLoadBitmap(_T("FLD2_UNREACH"), UseHiresBitmap);
      } else {
        hBmpAirportReachable = LKLoadBitmap(_T("APT2_UNREACH"), UseHiresBitmap);
        hBmpAirportUnReachable = LKLoadBitmap(_T("APT2_REACH"), UseHiresBitmap);
        hBmpFieldReachable = LKLoadBitmap(_T("FLD2_UNREACH"), UseHiresBitmap);
        hBmpFieldUnReachable = LKLoadBitmap(_T("FLD2_REACH"), UseHiresBitmap);
      }
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
