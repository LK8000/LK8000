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
#else
  #undef  EXTMODE
  #define EXTMODE extern

  extern void LKLoadBitmaps(void);
  extern void LKUnloadBitmaps(void);
#endif

EXTMODE  HBITMAP hTurnPoint, hInvTurnPoint;
EXTMODE  HBITMAP hSmall, hInvSmall;
EXTMODE  HBITMAP hCruise, hClimb, hFinalGlide;
EXTMODE  HBITMAP hTerrainWarning;
EXTMODE  HBITMAP hAirspaceWarning;
EXTMODE  HBITMAP hLogger, hLoggerOff;
EXTMODE  HBITMAP hFLARMTraffic;
EXTMODE  HBITMAP hBatteryFull, hBattery70, hBattery50, hBattery25, hBattery15;

EXTMODE  HBITMAP hBmpAirportReachable;
EXTMODE  HBITMAP hBmpAirportUnReachable;
EXTMODE  HBITMAP hBmpFieldReachable;
EXTMODE  HBITMAP hBmpFieldUnReachable;
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


#endif
