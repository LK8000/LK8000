/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#if !defined(BITMAPS_H)
#define BITMAPS_H

#if defined(STATIC_BITMAPS)
  #define EXTMODE 
  #undef  STATIC_BITMAPS
  #define EXTNULL	=NULL

#else
  #undef  EXTMODE
  #define EXTMODE extern
  #define EXTNULL

  extern void LKLoadFixedBitmaps(void);
  extern void LKLoadProfileBitmaps(void);
  extern void LKUnloadFixedBitmaps(void);
  extern void LKUnloadProfileBitmaps(void);
#endif

//
// FIXED BITMAPS
//
EXTMODE  HBITMAP hTurnPoint, hInvTurnPoint;
EXTMODE  HBITMAP hSmall, hInvSmall;
EXTMODE  HBITMAP hTerrainWarning;
EXTMODE  HBITMAP hAirspaceWarning;
EXTMODE  HBITMAP hLogger, hLoggerOff;
EXTMODE  HBITMAP hFLARMTraffic;
EXTMODE  HBITMAP hBatteryFull, hBattery70, hBattery50, hBattery25, hBattery15;

EXTMODE  HBITMAP hBmpThermalSource;
EXTMODE  HBITMAP hBmpTarget;
EXTMODE  HBITMAP hBmpMarker;
EXTMODE  HBITMAP hBmpTeammatePosition;
EXTMODE  HBITMAP hAboveTerrainBitmap;

EXTMODE  HBITMAP hAirspaceBitmap[NUMAIRSPACEBRUSHES];

EXTMODE  HBITMAP hBmpLeft32;
EXTMODE  HBITMAP hBmpRight32;
EXTMODE  HBITMAP hScrollBarBitmapTop;
EXTMODE  HBITMAP hScrollBarBitmapMid;
EXTMODE  HBITMAP hScrollBarBitmapBot;

//
// PROFILE DEPENDENT BITMAPS
//
EXTMODE  HBITMAP hBmpAirportReachable EXTNULL;
EXTMODE  HBITMAP hBmpAirportUnReachable EXTNULL;
EXTMODE  HBITMAP hBmpFieldReachable EXTNULL;
EXTMODE  HBITMAP hBmpFieldUnReachable EXTNULL;
EXTMODE  HBITMAP hCruise EXTNULL;
EXTMODE  HBITMAP hClimb EXTNULL;
EXTMODE  HBITMAP hFinalGlide EXTNULL;

#endif
