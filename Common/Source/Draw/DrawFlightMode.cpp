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
      Surface.DrawMaskedBitmap( rc.right+IBLSCALE(-8), rc.bottom - BottomSize+NIBLSCALE(4), NIBLSCALE(7),NIBLSCALE(7), LoggerActive?hLogger:hLoggerOff, 7,7);
	}
  }
  

  //
  // Flight mode Icon
  //
  const LKBitmap* pBmpFlightMode = NULL;

  if (IsMultiMapNoMain()) {
	short i=Get_Current_Multimap_Type()-1;
	switch(i) {
		case 1:
			pBmpFlightMode = &hMM1;
			break;
		case 2:
			pBmpFlightMode = &hMM2;
			break;
		case 3:
			pBmpFlightMode = &hMM3;
			break;
		case 4:
			pBmpFlightMode = &hMM4;
			break;
		case 5:
			pBmpFlightMode = &hMM5;
			break;
		case 6:
			pBmpFlightMode = &hMM6;
			break;
		case 7:
			pBmpFlightMode = &hMM7;
			break;
		case 8:
			pBmpFlightMode = &hMM8;
			break;
		default:
			pBmpFlightMode = &hMM0;
			break;
	}
  } else {
    if (mode.Is(Mode::MODE_CIRCLING)) {
      pBmpFlightMode = &hClimb;
    } else {
      if (mode.Is(Mode::MODE_FINAL_GLIDE)) {
          pBmpFlightMode = &hFinalGlide;
      } else {
          pBmpFlightMode = &hCruise;
      }
    }
  }
  if(pBmpFlightMode && (*pBmpFlightMode)) {
    PixelSize IconSize = pBmpFlightMode->GetSize();
    IconSize.cx /= 2;
    offset -= IconSize.cy;

    Surface.DrawMaskedBitmap( rc.right+IBLSCALE(offset-1), rc.bottom+IBLSCALE(-IconSize.cx-1), IBLSCALE(IconSize.cx),IBLSCALE(IconSize.cy),	*pBmpFlightMode, IconSize.cx, IconSize.cy);
  }

  //
  // Battery indicator
  // 

#if TESTBENCH && !defined(KOBO)
  // Battery test in Simmode will be available in testbench mode only
  if (SIMMODE && !(QUICKDRAW)) {; PDABatteryPercent-=1; if (PDABatteryPercent<0) PDABatteryPercent=100; }
  #else
  // If we are not in testbench, no matter simmode is active we shall represent the real battery (as in v5).
  if (!HaveBatteryInfo) return;
  #endif

  const LKBitmap* pBmpBattery = NULL;
  if ((PDABatteryPercent==0 || PDABatteryPercent>100) && PDABatteryStatus==Battery::ONLINE && PDABatteryFlag!=Battery::CHARGING) {
	pBmpBattery = &hBatteryFullC;
	goto _drawbattery;
  }

  if (PDABatteryPercent<=6) {
	if (flip) return;
	pBmpBattery = &hBattery12;
	goto _drawbattery;
  }

  if (PDABatteryPercent<=12) {
	pBmpBattery = &hBattery12;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=24) {
	pBmpBattery = &hBattery24;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=36) {
	pBmpBattery = &hBattery36;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=48) {
	pBmpBattery = &hBattery48;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=60) {
	pBmpBattery = &hBattery60;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=72) {
	pBmpBattery = &hBattery72;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=84) {
	pBmpBattery = &hBattery84;
	goto _drawbattery;
  }
  if (PDABatteryPercent<=96) {
	pBmpBattery = &hBattery96;
	goto _drawbattery;
  }
  if (PDABatteryStatus==Battery::ONLINE)
	pBmpBattery = &hBatteryFullC;
  else
	pBmpBattery = &hBatteryFull;

_drawbattery:
  if (!DisableAutoLogger || LoggerActive) {
      offset-=5;
  }
    if(pBmpBattery) {
        Surface.DrawMaskedBitmap(rc.right+IBLSCALE(offset-1), rc.bottom - BottomSize + NIBLSCALE(2), IBLSCALE(22),IBLSCALE(11),*pBmpBattery, 22, 11);
    }

}
