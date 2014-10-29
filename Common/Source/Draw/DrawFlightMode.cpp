/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Bitmaps.h"
#include "Multimap.h"

extern bool FastZoom;

//
// Bottom right corner indicators: Flight mode, Battery, Logger
//
void MapWindow::DrawFlightMode(LKSurface& Surface, const RECT& rc)
{
  static bool flip= true; 
  int offset = -3;

  //
  // Logger indicator
  //
  flip = !flip;

  if (!DisableAutoLogger || LoggerActive) {
	if (LoggerActive || flip) {
            const LKBitmap& bmpLogger = LoggerActive?hLogger:hLoggerOff;
            Surface.DrawMaskedBitmap( rc.right+IBLSCALE(-8), rc.bottom - BottomSize+NIBLSCALE(4), NIBLSCALE(7),NIBLSCALE(7), bmpLogger, 7,7);
	}
  }
  

  //
  // Flight mode Icon
  //
  LKBitmap BmpFlightMode;

  if (IsMultiMapNoMain()) {
	short i=Get_Current_Multimap_Type()-1;
	switch(i) {
		case 1:
			BmpFlightMode = hMM1;
			break;
		case 2:
			BmpFlightMode = hMM2;
			break;
		case 3:
			BmpFlightMode = hMM3;
			break;
		case 4:
			BmpFlightMode = hMM4;
			break;
		case 5:
			BmpFlightMode = hMM5;
			break;
		case 6:
			BmpFlightMode = hMM6;
			break;
		case 7:
			BmpFlightMode = hMM7;
			break;
		case 8:
			BmpFlightMode = hMM8;
			break;
		default:
			BmpFlightMode = hMM0;
			break;
	}
  } else {
    if (mode.Is(Mode::MODE_CIRCLING)) {
      BmpFlightMode = hClimb;
    } else {
      if (mode.Is(Mode::MODE_FINAL_GLIDE)) {
          BmpFlightMode = hFinalGlide;
      } else {
          BmpFlightMode = hCruise;
      }
    }
  }

  SIZE IconSize = BmpFlightMode.GetSize();
  IconSize.cx /= 2;

  offset -= IconSize.cy;

  Surface.DrawMaskedBitmap( rc.right+IBLSCALE(offset-1), rc.bottom+IBLSCALE(-IconSize.cx-1), IBLSCALE(IconSize.cx),IBLSCALE(IconSize.cy),	BmpFlightMode, IconSize.cx, IconSize.cy);

  //
  // Battery indicator
  // 

  #if TESTBENCH
  // Battery test in Simmode
  if (SIMMODE && !(QUICKDRAW)) {; PDABatteryPercent-=1; if (PDABatteryPercent<0) PDABatteryPercent=100; }
  #endif

  LKBitmap BmpBattery;
  if ((PDABatteryPercent==0 || PDABatteryPercent>100) && PDABatteryStatus==AC_LINE_ONLINE && PDABatteryFlag!=BATTERY_FLAG_CHARGING) {
	BmpBattery = hBatteryFullC;
	goto _drawbattery;
  }

  if (PDABatteryPercent<=6) {
	if (flip) return;
	BmpBattery = hBattery12;
	goto _drawbattery;
  }

  if (PDABatteryPercent<=12) {
	BmpBattery = hBattery12;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=24) {
	BmpBattery = hBattery24;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=36) {
	BmpBattery = hBattery36;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=48) {
	BmpBattery = hBattery48;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=60) {
	BmpBattery = hBattery60;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=72) {
	BmpBattery = hBattery72;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=84) {
	BmpBattery = hBattery84;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=96) {
	BmpBattery = hBattery96;
	goto _drawbattery;
  }
  if (PDABatteryStatus==AC_LINE_ONLINE)
	BmpBattery = hBatteryFullC;
  else
	BmpBattery = hBatteryFull;

_drawbattery:
  if (!DisableAutoLogger || LoggerActive) {
      offset-=5;
  }
    Surface.DrawMaskedBitmap(rc.right+IBLSCALE(offset-1), rc.bottom - BottomSize + NIBLSCALE(2), IBLSCALE(22),IBLSCALE(11),BmpBattery, 22, 11);
}
