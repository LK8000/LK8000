/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "MapWindow.h"

#define DONTDRAWTHEMAP !mode.AnyPan()&&MapSpaceMode!=MSM_MAP


DWORD MapWindow::timestamp_newdata=0;
bool userasked = false;



void MapWindow::UpdateTimeStats(bool start) {
  if (start) {
    timestamp_newdata = ::GetTickCount();
  }
}


//
// CALLED BY THE DRAW_THREAD MAIN LOOP
//
void MapWindow::RenderMapWindow(RECT rc)
{
  HFONT hfOld;
  DWORD fpsTime = ::GetTickCount();

  // only redraw map part every 800 s unless triggered
  if (((fpsTime-fpsTime0)>800)||(fpsTime0== 0)||(userasked)) {
    fpsTime0 = fpsTime;
    userasked = false;
  }
  MapWindow::UpdateTimeStats(true);

  if (LockModeStatus) LockMode(9); // check if unlock is now possible 
  
  POINT Orig, Orig_Aircraft;

  SetAutoOrientation(false); // false for no reset Old values
  CalculateOrigin(rc, &Orig);

  // this is calculating waypoint visible, and must be executed before rendermapwindowbg which calls   
  // CalculateWayPointReachable new, setting values for visible wps!
  // This is also calculating CalculateScreenBounds 0.0  and placing it inside MapWindow::screenbounds_latlon
  CalculateScreenPositions(Orig, rc, &Orig_Aircraft);

  LKUpdateOlc();

  RenderMapWindowBg(hdcDrawWindow, rc, Orig, Orig_Aircraft);

  if (DONTDRAWTHEMAP) {
  	DrawFlightMode(hdcDrawWindow, rc);
  	DrawGPSStatus(hdcDrawWindow, rc);
	DrawLKAlarms(hdcDrawWindow, rc);
	#if (WINDOWSPC<1)
	LKBatteryManager();
	#endif

	return;
  }
  // overlays

  hfOld = (HFONT)SelectObject(hdcDrawWindow, MapWindowFont);
  
  DrawMapScale(hdcDrawWindow,rc, zoom.BigZoom());

  DrawCompass(hdcDrawWindow, rc);
  
  DrawFlightMode(hdcDrawWindow, rc);
  
  if (!mode.AnyPan()) {
    // REMINDER TODO let it be configurable for not circling also, as before
    if ((mode.Is(Mode::MODE_CIRCLING)) )
      if (ThermalBar) DrawThermalBand(hdcDrawWindow, rc); // 091122
    
    DrawFinalGlide(hdcDrawWindow,rc);
  }
  
  // DrawSpeedToFly(hdcDrawWindow, rc);  // Usable

  DrawGPSStatus(hdcDrawWindow, rc);

  #if (WINDOWSPC<1)
  LKBatteryManager();
  #endif

  DrawLKAlarms(hdcDrawWindow, rc);

  SelectObject(hdcDrawWindow, hfOld);

}


