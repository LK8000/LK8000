/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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
BEXTMODE LKBitmap hTurnPoint;
BEXTMODE LKBitmap hInvTurnPoint;
BEXTMODE LKBitmap hSmall;
BEXTMODE LKBitmap hInvSmall;
BEXTMODE LKBitmap hTerrainWarning;
BEXTMODE LKBitmap hAirspaceWarning;
BEXTMODE LKBitmap hLogger;
BEXTMODE LKBitmap hLoggerOff;
BEXTMODE LKBitmap hLoggerDisabled;
BEXTMODE LKBitmap hFLARMTraffic;
BEXTMODE LKBitmap hBatteryFull;
BEXTMODE LKBitmap hBatteryFullC;
BEXTMODE LKBitmap hBattery96;
BEXTMODE LKBitmap hBattery84;
BEXTMODE LKBitmap hBattery72;
BEXTMODE LKBitmap hBattery60;
BEXTMODE LKBitmap hBattery48;
BEXTMODE LKBitmap hBattery36;
BEXTMODE LKBitmap hBattery24;
BEXTMODE LKBitmap hBattery12;

BEXTMODE LKBitmap hNoTrace;
BEXTMODE LKBitmap hFullTrace;
BEXTMODE LKBitmap hClimbTrace;
BEXTMODE LKBitmap hHeadUp;
BEXTMODE LKBitmap hHeadRight;
BEXTMODE LKBitmap hNorthUp;


BEXTMODE LKBitmap hMM0;
BEXTMODE LKBitmap hMM1;
BEXTMODE LKBitmap hMM2;
BEXTMODE LKBitmap hMM3;
BEXTMODE LKBitmap hMM4;
BEXTMODE LKBitmap hMM5;
BEXTMODE LKBitmap hMM6;
BEXTMODE LKBitmap hMM7;
BEXTMODE LKBitmap hMM8;

BEXTMODE LKBitmap hBmpThermalSource;
BEXTMODE LKBitmap hBmpTarget;
BEXTMODE LKBitmap hBmpMarker;
BEXTMODE LKBitmap hBmpTeammatePosition;
BEXTMODE LKBitmap hAboveTerrainBitmap;

#ifdef HAVE_HATCHED_BRUSH
BEXTMODE LKBitmap hAirspaceBitmap[NUMAIRSPACEBRUSHES];
#endif

BEXTMODE LKBitmap hBmpLeft32;
BEXTMODE LKBitmap hBmpRight32;
BEXTMODE LKBitmap hScrollBarBitmapTop;
BEXTMODE LKBitmap hScrollBarBitmapMid;
BEXTMODE LKBitmap hScrollBarBitmapBot;

// Map icons 
BEXTMODE LKBitmap hMountop;
BEXTMODE LKBitmap hMountpass;
BEXTMODE LKBitmap hBridge;
BEXTMODE LKBitmap hIntersect;
BEXTMODE LKBitmap hDam;
BEXTMODE LKBitmap hSender;
BEXTMODE LKBitmap hNdb;
BEXTMODE LKBitmap hVor;
BEXTMODE LKBitmap hCoolTower;
BEXTMODE LKBitmap hTunnel;
BEXTMODE LKBitmap hCastle;
BEXTMODE LKBitmap hPowerPlant;
BEXTMODE LKBitmap hLKThermal;
BEXTMODE LKBitmap hLKThermalRed;
BEXTMODE LKBitmap hLKPictori;

//
// PROFILE DEPENDENT BITMAPS
//
BEXTMODE LKBitmap hBmpAirportReachable;
BEXTMODE LKBitmap hBmpAirportUnReachable;
BEXTMODE LKBitmap hBmpFieldReachable;
BEXTMODE LKBitmap hBmpFieldUnReachable;
BEXTMODE LKBitmap hCruise;
BEXTMODE LKBitmap hClimb;
BEXTMODE LKBitmap hFinalGlide;

#endif
