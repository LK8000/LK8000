/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "MapWindow.h"
#include <Message.h>
#include "Terrain.h"
#include "RasterTerrain.h"
#include "LKGeneralAviation.h"


#define DONTDRAWTHEMAP !mode.AnyPan()&&MapSpaceMode!=MSM_MAP
#define MAPMODE8000    !mode.AnyPan()&&MapSpaceMode==MSM_MAP

#define QUICKDRAW (FastZoom || zoom.BigZoom())
extern bool FastZoom;

void MapWindow::RenderMapWindowBg(HDC hdc, const RECT rc,
				  const POINT &Orig,
				  const POINT &Orig_Aircraft)
{
  HFONT hfOld;

  // Calculations are taking time and slow down painting of map, beware
  #define MULTICALC_MINROBIN	5	// minimum split
  #define MULTICALC_MAXROBIN	20	// max split
  static short multicalc_slot=0;// -1 (which becomes immediately 0) will force full loading on startup, but this is not good
				// because currently we are not waiting for ProgramStarted=3
				// and the first scan is made while still initializing other things

  // TODO assign numslots with a function, based also on available CPU time
  short numslots=1;

  // If we have a BigZoom request, we serve it immediately without calculating anything
  // TODO: stretch the old map bitmap to the new zoom, while fastzooming
  if (QUICKDRAW) {
	goto fastzoom;
  }

  if (NumberOfWayPoints>200) {
	numslots=NumberOfWayPoints/400;
	// keep numslots optimal
	if (numslots<MULTICALC_MINROBIN) numslots=MULTICALC_MINROBIN; // seconds for full scan, as this is executed at 1Hz
	if (numslots>MULTICALC_MAXROBIN) numslots=MULTICALC_MAXROBIN;

	// When waypointnumber has changed, we wont be using an exceeded multicalc_slot, which would crash the sw
	// In this case, we shall probably continue for the first round to calculate without going from the beginning
	// but this is not a problem, we are round-robin all the time here.
	if (++multicalc_slot>numslots) multicalc_slot=1;
  } else {
	multicalc_slot=0; // forcing full scan
  }

  // Here we calculate arrival altitude, GD etc for map waypoints. Splitting with multicalc will result in delayed
  // updating of visible landables, for example. The nearest pages do this separately, with their own sorting.
  // Basically we assume -like for nearest- that values will not change that much in the multicalc split time.
  // Target and tasks are recalculated in real time in any case. Nearest too. 
  LKCalculateWaypointReachable(multicalc_slot, numslots);
  CalculateScreenPositionsAirspace();
  CalculateScreenPositionsThermalSources();
  CalculateScreenPositionsGroundline();

  if (PGZoomTrigger) {
    if(!mode.Is(Mode::MODE_PANORAMA)) {
      mode.Special(Mode::MODE_SPECIAL_PANORAMA, true);
		LastZoomTrigger=GPS_INFO.Time;
      
		Message::Lock();
	        Message::AddMessage(1000, 3, gettext(TEXT("_@M872_"))); // LANDSCAPE ZOOM FOR 20s
		Message::Unlock();
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(TEXT("LK_TONEUP.WAV"));
		#endif
    }
    else {
		// previously called, see if time has passed
		if ( GPS_INFO.Time > (LastZoomTrigger + 20.0)) {
			// time has passed, lets go back
			LastZoomTrigger=0; // just for safety
        mode.Special(Mode::MODE_SPECIAL_PANORAMA, false);
			PGZoomTrigger=false;
			Message::Lock(); 
	        	Message::AddMessage(1500, 3, gettext(TEXT("_@M873_"))); // BACK TO NORMAL ZOOM
			Message::Unlock();
			#ifndef DISABLEAUDIO
			if (EnableSoundModes) LKSound(TEXT("LK_TONEDOWN.WAV"));
			#endif
		}
	}
  }
	
  // let the calculations run, but dont draw anything but the look8000 when in MapSpaceMode != MSM_MAP
  if (DONTDRAWTHEMAP) 
  {
QuickRedraw: // 100318 speedup redraw
	DrawLook8000(hdc,rc);
#ifdef CPUSTATS
	DrawCpuStats(hdc,rc);
#endif
#ifdef DRAWDEBUG
	DrawDebug(hdc,rc);
#endif
	// no need to do SelectObject as at the bottom of function
	return;
  }

  // When no terrain is painted, set a background0
  // Remember that in this case we have plenty of cpu time to spend for best result
  if (!EnableTerrain || !DerivedDrawInfo.TerrainValid || !RasterTerrain::isTerrainLoaded() ) {

    // display border and fill background..
	SelectObject(hdc, hInvBackgroundBrush[BgMapColor]);
	SelectObject(hdc, GetStockObject(WHITE_PEN));

	Rectangle(hdc,rc.left,rc.top,rc.right,rc.bottom);
	// We force LK painting black values on screen depending on the background color in use
	// TODO make it an array once settled
	// blackscreen would force everything to be painted white, instead
	LKTextBlack=BgMapColorTextBlack[BgMapColor];
	if (BgMapColor>6 ) BlackScreen=true; else BlackScreen=false; 
  } else {
	LKTextBlack=false;
	BlackScreen=false;
  }

fastzoom:  

  SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  SelectObject(hdc, GetStockObject(BLACK_PEN));
  hfOld = (HFONT)SelectObject(hdc, MapWindowFont);
  
  // ground first...
  
  if (DONTDRAWTHEMAP) { // 100319
	SelectObject(hdcDrawWindow, hfOld);
	goto QuickRedraw;
  }

  if ((EnableTerrain && (DerivedDrawInfo.TerrainValid) 
       && RasterTerrain::isTerrainLoaded())
	) {
	// sunelevation is never used, it is still a todo in Terrain
	double sunelevation = 40.0;
	double sunazimuth=GetAzimuth();

    LockTerrainDataGraphics();
 	if (DONTDRAWTHEMAP) { // 100318
		UnlockTerrainDataGraphics();
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
    DrawTerrain(hdc, rc, sunazimuth, sunelevation); // LOCKED 091105
 	if (DONTDRAWTHEMAP) { // 100318
		UnlockTerrainDataGraphics();
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
    if (!QUICKDRAW) {
    	// shaded terrain unreachable, aka glide amoeba
    	if ((FinalGlideTerrain==2) && DerivedDrawInfo.TerrainValid) {
    	  DrawTerrainAbove(hdc, rc);
    	}
    }
    UnlockTerrainDataGraphics();
  }
 
  if (QUICKDRAW)  {
	if ( !mode.AnyPan()) DrawLook8000(hdc,rc); 
  	SelectObject(hdcDrawWindow, hfOld);
	return;
  }

  if (DONTDRAWTHEMAP) { // 100319
	SelectObject(hdcDrawWindow, hfOld);
	goto QuickRedraw;
  }

  if (EnableTopology) {
    DrawTopology(hdc, rc); // LOCKED 091105
  }
  #if 0
  StartupStore(_T("... Experimental1=%.0f\n"),Experimental1);
  StartupStore(_T("... Experimental2=%.0f\n"),Experimental2);
  Experimental1=0.0;
  Experimental2=0.0;
  #endif

  // Topology labels are printed first, using OLD wps positions from previous run!
  // Reset for topology labels decluttering engine occurs also in another place here!

  nLabelBlocks = 0;
  #if TOPOFASTLABEL
  for (short nvi=0; nvi<SCREENVSLOTS; nvi++) nVLabelBlocks[nvi]=0;
  #endif
  
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) { // 100503
	DrawTaskAAT(hdc, rc);
  }

  
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
 
  if (OnAirSpace > 0)  // Default is true, always true at startup no regsave 
  {
    if ( (GetAirSpaceFillType() == asp_fill_ablend_full) || (GetAirSpaceFillType() == asp_fill_ablend_borders) )
      DrawTptAirSpace(hdc, rc);
    else
      DrawAirSpace(hdc, rc);
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  
  if(TrailActive) {
	// NEED REWRITING
	LKDrawTrail(hdc, Orig_Aircraft, rc);
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  DrawThermalEstimate(hdc, rc);
  if (OvertargetMode==OVT_THER) DrawThermalEstimateMultitarget(hdc, rc);
 
  if (ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) { // 100503
	DrawTask(hdc, rc, Orig_Aircraft);
  }
  
  // draw red cross on glide through terrain marker
  if (FinalGlideTerrain && DerivedDrawInfo.TerrainValid) {
    DrawGlideThroughTerrain(hdc, rc);
  }
  
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  if ((OnAirSpace > 0) && AirspaceWarningMapLabels)
  {
	DrawAirspaceLabels(hdc, rc, Orig_Aircraft);
	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}
  }
  DrawWaypointsNew(hdc,rc);
 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  DrawTeammate(hdc, rc);

  if (extGPSCONNECT) {
    DrawProjectedTrack(hdc, rc, Orig_Aircraft);
    DrawBestCruiseTrack(hdc, Orig_Aircraft);
    DrawBearing(hdc, rc);
  }

  // draw wind vector at aircraft
  if (!mode.AnyPan()) {
    DrawWindAtAircraft2(hdc, Orig_Aircraft, rc);
  } else if (mode.Is(Mode::MODE_TARGET_PAN)) {
    DrawWindAtAircraft2(hdc, Orig, rc);
  }

  // VisualGlide drawn BEFORE lk8000 overlays
  if (!mode.AnyPan() && VisualGlide > 0) {
    DrawGlideCircle(hdc, Orig, rc); 
  }

 	if (DONTDRAWTHEMAP) { // 100319
		SelectObject(hdcDrawWindow, hfOld);
		goto QuickRedraw;
	}

  // Draw traffic and other specifix LK gauges
  	LKDrawFLARMTraffic(hdc, rc, Orig_Aircraft);
	if ( !mode.AnyPan()) DrawLook8000(hdc,rc); 
	if (LKVarioBar && !mode.AnyPan()) // 091214 do not draw Vario when in Pan mode
		LKDrawVario(hdc,rc); // 091111
  
  // finally, draw you!
  // Draw cross air for panmode, instead of aircraft icon
  if (mode.AnyPan() && !mode.Is(Mode::MODE_TARGET_PAN)) {
    DrawCrossHairs(hdc, Orig, rc);
  }

  // Draw glider or paraglider
  if (extGPSCONNECT) {
    DrawAircraft(hdc, Orig_Aircraft);
  }

  if (!mode.AnyPan()) {
	if (TrackBar) DrawHeading(hdc, Orig, rc); 
  }

  #if USETOPOMARKS
  // marks on top...
  DrawMarks(hdc, rc);
  #endif

  if (ISGAAIRCRAFT) DrawHSI(hdc,Orig,rc); 

#ifdef CPUSTATS
  DrawCpuStats(hdc,rc);
#endif
#ifdef DRAWDEBUG
  DrawDebug(hdc,rc);
#endif

  SelectObject(hdcDrawWindow, hfOld);

}


