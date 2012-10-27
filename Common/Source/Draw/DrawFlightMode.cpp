/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Bitmaps.h"

extern bool FastZoom;

//
// Bottom right corner indicators: Flight mode, Battery, Logger
//
void MapWindow::DrawFlightMode(HDC hdc, const RECT rc)
{
  static bool flip= true; 
  int offset = -1;

  //
  // Logger indicator
  //

  if (!DisableAutoLogger || LoggerActive) {
	flip = !flip;
	if (LoggerActive || flip) {
		if (LoggerActive)
			SelectObject(hDCTemp,hLogger);
		else
			SelectObject(hDCTemp,hLoggerOff);

		offset -= 7;

		DrawBitmapX(hdc, rc.right+IBLSCALE(offset),
			rc.bottom - BottomSize+NIBLSCALE(4),
			7,7, hDCTemp, 0,0,SRCPAINT,true);

		DrawBitmapX(hdc, rc.right+IBLSCALE(offset),
			rc.bottom-BottomSize+NIBLSCALE(4),
			7,7, hDCTemp, 7,0,SRCAND,true);

		offset +=7;
	}
  }
  

  //
  // Flight mode Icon
  //

  if (mode.Is(Mode::MODE_CIRCLING)) {
	SelectObject(hDCTemp,hClimb);
  } else
	if (mode.Is(Mode::MODE_FINAL_GLIDE)) {
		SelectObject(hDCTemp,hFinalGlide);
	} else {
		SelectObject(hDCTemp,hCruise);
	}

  offset -= 24;

  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom+IBLSCALE(-20-1),
	24,20,
	hDCTemp,
	0,0,SRCPAINT,true);
    
  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom+IBLSCALE(-20-1),
	24,20,
	hDCTemp,
	24,0,SRCAND,true);


  //
  // Battery indicator
  // 

  #if TESTBENCH
  // Battery test in Simmode
  if (SIMMODE && !(QUICKDRAW)) {; PDABatteryPercent-=5; if (PDABatteryPercent<0) PDABatteryPercent=100; }
  #endif

  if (PDABatteryPercent==0 && PDABatteryStatus==AC_LINE_ONLINE && PDABatteryFlag!=BATTERY_FLAG_CHARGING) {
	SelectObject(hDCTemp,hBatteryFull);
	goto _drawbattery;
  }

  if (PDABatteryPercent<20) {
	SelectObject(hDCTemp,hBattery15);
	goto _drawbattery;
  }
  if (PDABatteryPercent<45) {
	SelectObject(hDCTemp,hBattery25);
	goto _drawbattery;
  }
  if (PDABatteryPercent<65) {
	SelectObject(hDCTemp,hBattery50);
	goto _drawbattery;
  }
  if (PDABatteryPercent<90) {
	SelectObject(hDCTemp,hBattery70);
	goto _drawbattery;
  }
  SelectObject(hDCTemp,hBatteryFull);

_drawbattery:
  if (!DisableAutoLogger || LoggerActive) offset-=5;
  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom - BottomSize + NIBLSCALE(2),
	22,11,
	hDCTemp,
	0,0,SRCPAINT,true);
    
  DrawBitmapX(hdc,
	rc.right+IBLSCALE(offset-1),
	rc.bottom - BottomSize + NIBLSCALE(2),
	22,11,
	hDCTemp,
	22,0,SRCAND,true);

}


