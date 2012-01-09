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


void MapWindow::UpdateTimeStats(bool start) {
  if (start) {
    timestamp_newdata = ::GetTickCount();
  }
}

#define QUICKDRAW (FastZoom || zoom.BigZoom())
bool FastZoom;

//
// CALLED BY THE DRAW_THREAD MAIN LOOP
//
void MapWindow::RenderMapWindow(RECT rc)
{
  HFONT hfOld;
  static DWORD fastzoomStart=0;
  // static short ZoomDelayTimes=0; // alternate rebouncing, with the lowest possible interval: 1 loop

  // We do this in order to wait for a late zoom request after another.
  // Did we get a BigZoom request?
  FastZoom=zoom.BigZoom();
  if (FastZoom) {
	zoom.BigZoom(false);
	// How many times we shall loop waiting for a next bigzoom to come
	// ZoomDelayTimes=1; shortest possible
	fastzoomStart=GetTickCount(); // time granted delay
  } else {
	if (fastzoomStart) {
		// no bigzoom, but we wait a bit to detect another click for zoom
		// and avoid to redraw entirely in the meantime. We shall fall down here
		// because we have forced a map redraw after the first zoom, even with not
		// a click pressed.
		//if (ZoomDelayTimes >0) {
		//	ZoomDelayTimes--;
		if ( (GetTickCount()-fastzoomStart) <350 ) {
			#if (WINDOWSPC>0)	
			  #if TESTBENCH
			  FastZoom=true;
			  #endif
			#else
			  FastZoom=true;
			#endif
		} else {
			fastzoomStart=0;
		}
	} 
  }

  MapWindow::UpdateTimeStats(true);

  if (LockModeStatus) LockMode(9); // check if unlock is now possible 
  
  POINT Orig, Orig_Aircraft;

  SetAutoOrientation(false); // false for no reset Old values
  CalculateOrigin(rc, &Orig);

  //
  // When BigZoom trigger, we shall not calculate waypoints and olc.
  //
  if (!QUICKDRAW) {
    //
    // this is calculating waypoint visible, and must be executed before rendermapwindowbg which calls   
    // CalculateWayPointReachable new, setting values for visible wps!
    // This is also calculating CalculateScreenBounds 0.0  and placing it inside MapWindow::screenbounds_latlon
    //
    CalculateScreenPositions(Orig, rc, &Orig_Aircraft);
    LKUpdateOlc();
  } else {
	FastZoom=true;
  }

  RenderMapWindowBg(hdcDrawWindow, rc, Orig, Orig_Aircraft);

  // No reason to check for bigzoom here, because we are not drawing the map
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
  
  DrawMapScale(hdcDrawWindow,rc, zoom.BigZoom()); // unused BigZoom 

  //
  // When fast zoom requested, do not loose time with frills
  //
  if (QUICKDRAW) {
	DrawFlightMode(hdcDrawWindow, rc);
	SelectObject(hdcDrawWindow, hfOld);
	// do a mapdirty and rerun the loop
	MapWindow::RefreshMap();
	return;
  }

  DrawCompass(hdcDrawWindow, rc);
  
  DrawFlightMode(hdcDrawWindow, rc);
  
  if (!mode.AnyPan()) {
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


