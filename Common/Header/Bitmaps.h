/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Bitmaps.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#if !defined(BITMAPS_H)
#define BITMAPS_H

#include "Screen/LKBitmap.h"

#if defined(STATIC_BITMAPS)
#define BEXTMODE
#undef  STATIC_BITMAPS

#else
#undef  BEXTMODE
#define BEXTMODE extern

extern void LKLoadFixedBitmaps(void);
extern void LKLoadProfileBitmaps(void);
extern void LKUnloadFixedBitmaps(void);
extern void LKUnloadProfileBitmaps(void);
#endif

//
// FIXED BITMAPS
//
BEXTMODE LKIcon hTurnPoint;
BEXTMODE LKIcon hInvTurnPoint;
BEXTMODE LKIcon hSmall;
BEXTMODE LKIcon hInvSmall;
BEXTMODE LKIcon hTerrainWarning;
BEXTMODE LKIcon hAirspaceWarning;
BEXTMODE LKIcon hLogger;
BEXTMODE LKIcon hLoggerOff;
BEXTMODE LKIcon hLoggerDisabled;
BEXTMODE LKIcon hFLARMTraffic;

BEXTMODE LKIcon hBatteryFull;
BEXTMODE LKIcon hBatteryFullC;
BEXTMODE LKIcon hBattery96;
BEXTMODE LKIcon hBattery84;
BEXTMODE LKIcon hBattery72;
BEXTMODE LKIcon hBattery60;
BEXTMODE LKIcon hBattery48;
BEXTMODE LKIcon hBattery36;
BEXTMODE LKIcon hBattery24;
BEXTMODE LKIcon hBattery12;

#if defined(DITHER) || (defined(ANDROID) && defined(__arm__))
  BEXTMODE LKIcon hKB_BatteryFull;
  BEXTMODE LKIcon hKB_BatteryFullC;
  BEXTMODE LKIcon hKB_Battery96;
  BEXTMODE LKIcon hKB_Battery84;
  BEXTMODE LKIcon hKB_Battery72;
  BEXTMODE LKIcon hKB_Battery60;
  BEXTMODE LKIcon hKB_Battery48;
  BEXTMODE LKIcon hKB_Battery36;
  BEXTMODE LKIcon hKB_Battery24;
  BEXTMODE LKIcon hKB_Battery12;
#else
  #define hKB_BatteryFull   hBatteryFull
  #define hKB_BatteryFullC  hBatteryFullC
  #define hKB_Battery96     hBattery96
  #define hKB_Battery84     hBattery84
  #define hKB_Battery72     hBattery72
  #define hKB_Battery60     hBattery60
  #define hKB_Battery48     hBattery48
  #define hKB_Battery36     hBattery36
  #define hKB_Battery24     hBattery24
  #define hKB_Battery12     hBattery12
#endif

BEXTMODE LKIcon hNoTrace;
BEXTMODE LKIcon hFullTrace;
BEXTMODE LKIcon hClimbTrace;
BEXTMODE LKIcon hHeadUp;
BEXTMODE LKIcon hHeadRight;
BEXTMODE LKIcon hNorthUp;


BEXTMODE LKIcon hMM0;
BEXTMODE LKIcon hMM1;
BEXTMODE LKIcon hMM2;
BEXTMODE LKIcon hMM3;
BEXTMODE LKIcon hMM4;
BEXTMODE LKIcon hMM5;
BEXTMODE LKIcon hMM6;
BEXTMODE LKIcon hMM7;
BEXTMODE LKIcon hMM8;
BEXTMODE LKIcon hIMM0;
BEXTMODE LKIcon hIMM1;
BEXTMODE LKIcon hIMM2;
BEXTMODE LKIcon hIMM3;
BEXTMODE LKIcon hIMM4;
BEXTMODE LKIcon hIMM5;
BEXTMODE LKIcon hIMM6;
BEXTMODE LKIcon hIMM7;
BEXTMODE LKIcon hIMM8;

BEXTMODE LKIcon hBmpThermalSource;
BEXTMODE LKIcon hBmpTarget;
BEXTMODE LKIcon hBmpMarker;
BEXTMODE LKIcon hBmpTeammatePosition;


#ifdef HAVE_HATCHED_BRUSH
BEXTMODE LKBitmap hAboveTerrainBitmap;
BEXTMODE LKBitmap hAirspaceBitmap[NUMAIRSPACEBRUSHES];
#endif

BEXTMODE LKIcon hBmpLeft32;
BEXTMODE LKIcon hBmpRight32;
BEXTMODE LKIcon hScrollBarBitmapTop;
BEXTMODE LKIcon hScrollBarBitmapMid;
BEXTMODE LKIcon hScrollBarBitmapBot;

// Map icons
BEXTMODE LKIcon hMountop;
BEXTMODE LKIcon hMountpass;
BEXTMODE LKIcon hBridge;
BEXTMODE LKIcon hIntersect;
BEXTMODE LKIcon hDam;
BEXTMODE LKIcon hSender;
BEXTMODE LKIcon hNdb;
BEXTMODE LKIcon hVor;
BEXTMODE LKIcon hCoolTower;
BEXTMODE LKIcon hTunnel;
BEXTMODE LKIcon hCastle;
BEXTMODE LKIcon hPowerPlant;
BEXTMODE LKIcon hLKThermal;
BEXTMODE LKIcon hLKThermalRed;
BEXTMODE LKBitmap hLKPictori;

//
// PROFILE DEPENDENT BITMAPS
//
BEXTMODE LKIcon hBmpAirportReachable;
BEXTMODE LKIcon hBmpAirportUnReachable;
BEXTMODE LKIcon hBmpFieldReachable;
BEXTMODE LKIcon hBmpFieldUnReachable;

BEXTMODE LKBitmap hCruise;
BEXTMODE LKIcon hClimb;
BEXTMODE LKBitmap hFinalGlide;

BEXTMODE LKIcon hMcVario;

BEXTMODE LKIcon hXCFF;
BEXTMODE LKIcon hXCFT;
BEXTMODE LKIcon hXCFAI;
#endif
