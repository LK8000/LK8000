/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <Message.h>
#include "Terrain.h"
#include "RasterTerrain.h"
#include "LKGeneralAviation.h"
#include "Multimap.h"

#if NEWSMARTZOOM
// We do smart zoom only with real map painted, and in quickdraw mode
// Temporarily, work only in Inversemode to test speed difference on PNAs (Inverse works..inverted!)
#define ONSMARTZOOM	((MAPMODE8000 &&  QUICKDRAW) &&  Appearance.InverseInfoBox )
#define OFFSMARTZOOM	((MAPMODE8000 && !QUICKDRAW) || !Appearance.InverseInfoBox )
#endif

extern bool FastZoom;


void MapWindow::RenderMapWindowBg(HDC hdc, const RECT rc,
				  const POINT &Orig,
				  const POINT &Orig_Aircraft)
{
bool bMainMap = false;
	  switch(MapSpaceMode)
	  {
		case MSM_MAPTRK:
		case MSM_MAPWPT:
		case MSM_MAPASP:
		case MSM_VISUALGLIDE:
		case MSM_MAPRADAR:
			bMainMap = false;
            break;

		default:
			bMainMap = true;
			break;
	  }
	  if((mode.Is(Mode::MODE_PAN) || mode.Is(Mode::MODE_TARGET_PAN)))
			bMainMap = true;
  #if NEWSMARTZOOM
  static double quickdrawscale=0.0;
  static double delta_drawscale=1.0;
  #endif

  #if 0 
  extern void TestChangeRect();
  TestChangeRect();
  #endif

  if ((MapWindow::AlphaBlendSupported() && BarOpacity<100) || mode.AnyPan()) {
	MapWindow::ChangeDrawRect(MapRect);
  } else {
	RECT newRect={0,0,ScreenSizeX,ScreenSizeY-BottomSize};
	MapWindow::ChangeDrawRect(newRect);
  }

  if (QUICKDRAW) {
	goto _skip_calcs;
  }


  // Here we calculate arrival altitude, GD etc for map waypoints. Splitting with multicalc will result in delayed
  // updating of visible landables, for example. The nearest pages do this separately, with their own sorting.
  // Basically we assume -like for nearest- that values will not change that much in the multicalc split time.
  // Target and tasks are recalculated in real time in any case. Nearest too. 
  LKCalculateWaypointReachable(false);

_skip_calcs:
if(bMainMap)
{
// Ulli: removed main map calculations when in other Multimap
//       no need to recalc because we do not draw the Multimap
//       was rising CPU load and interfering the Multimap calculations
//       have not found a drawback so far
  CalculateScreenPositionsAirspace(rc);

  CalculateScreenPositionsThermalSources();

  // Make the glide amoeba out of the latlon points, converting them to screen
  // (This function is updated for supporting multimaps )
  CalculateScreenPositionsGroundline();

  if (PGZoomTrigger) {
    if(!mode.Is(Mode::MODE_PANORAMA)) {
      mode.Special(Mode::MODE_SPECIAL_PANORAMA, true);
		LastZoomTrigger=DrawInfo.Time;
      
		Message::Lock();
	        Message::AddMessage(1000, 3, gettext(TEXT("_@M872_"))); // LANDSCAPE ZOOM FOR 20s
		Message::Unlock();
		#ifndef DISABLEAUDIO
		if (EnableSoundModes) LKSound(TEXT("LK_TONEUP.WAV"));
		#endif
    }
    else {
		// previously called, see if time has passed
		if ( DrawInfo.Time > (LastZoomTrigger + 20.0)) {
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
}
  // 
  // "Checkpoint Charlie"
  // This is were we process stuff for anything else but main map.
  // We let the calculations run also for MapSpace modes.
  // But for multimaps, we can also draw some more stuff..
  // We are also sent back here from next code, when we detect that
  // the MapSpace mode has changed from MAP to something else while we
  // were rendering.
  //
QuickRedraw:
  //
  if (DONTDRAWTHEMAP) 
  {
	DrawMapSpace(hdc, rc);
	// Is this a "shared map" environment? 
	if (IsMultiMapShared()) { 
		// Shared map, of course not MSN_MAP, since dontdrawthemap was checked
		//
		if (IsMultimapOverlaysText()) {
			DrawLook8000(hdc,rc);
		}
		if (IsMultimapOverlaysGauges()) {
			if (LKVarioBar) LKDrawVario(hdc,rc);

            if ( (ThermalBar>=1 && MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) || (ThermalBar==2 && MapWindow::mode.Is(MapWindow::Mode::MODE_CRUISE)) ) {
				DrawThermalBand(hdcDrawWindow, rc);
            }

			DrawFinalGlide(hdcDrawWindow,rc);
		}

	} else {
		// Not in map painting environment 
		// ex. nearest pages, but also MAPRADAR..
	}

	// 
	DrawBottomBar(hdc,rc);
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
  if (!IsMultimapTerrain() || !DerivedDrawInfo.TerrainValid || !RasterTerrain::isTerrainLoaded() ) {

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

  #if NEWSMARTZOOM
  // Copy the old background map with no overlays
  if (ONSMARTZOOM) {

	if (quickdrawscale>0) {
		delta_drawscale=zoom.DrawScale() / quickdrawscale;
	}

// StartupStore(_T("... QuickDrawScale=%.2f new zoom=%.2f  delta=%.2f\n"),quickdrawscale,zoom.DrawScale(),delta_drawscale);

	int dx=MapRect.right-MapRect.left;
	int dy=MapRect.bottom-MapRect.top;

	// notice: zoom in is always ok.. but zoom out starting from high zoom levels will make the picture
	// very small and unusable. We can consider to zoom out in fast zoom normally, in such cases?
	//
	// Notice 2: the delta is not yet working correctly 
	//
	if (delta_drawscale>1.0) {
		// zoom in
		StretchBlt(hdcDrawWindow, 
			0,0,
			dx,dy, 
			hdcQuickDrawWindow,
			(int)((dx/2) - (dx / delta_drawscale)/2),
			(int)((dy/2) - (dy / delta_drawscale)/2),
			(int)(dx / delta_drawscale),
			(int)(dy / delta_drawscale), 
			SRCCOPY);
	} else {
		// zoom out
		StretchBlt(hdcDrawWindow,
			(int)((dx/2) - (dx * delta_drawscale)/2),
			(int)((dy/2) - (dy * delta_drawscale)/2),
			(int)(dx * delta_drawscale),
			(int)(dy * delta_drawscale), 
			hdcQuickDrawWindow,
			0,0,
			dx,dy,  
			SRCCOPY);
	}
  }
  #endif


  // Logic of DONTDRAWTHEMAP is the following:
  // We are rendering the screen page here. If we are here, we passed Checkpoint Charlie.
  // So we were, at charlie, in MSM_MAP: preparing the main map stuff.
  // If we detect that MapSpace has CHANGED while we were doing our job here,
  // it means that the user has clicked meanwhile. He desires another page, so let's
  // reset our intentions and go back to beginning, or nearby..
  // We have a new job to do, for another MapSpace, no more MAP.
  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }

  #if NEWSMARTZOOM
  if ( OFFSMARTZOOM ) {
  #endif

  bool terrainpainted=false;

  if ((IsMultimapTerrain() && (DerivedDrawInfo.TerrainValid) 
       && RasterTerrain::isTerrainLoaded())
	) {
	// sunelevation is never used, it is still a todo in Terrain
	double sunelevation = 40.0;
	double sunazimuth=GetAzimuth();

    LockTerrainDataGraphics();
 	if (DONTDRAWTHEMAP) { // 100318
		UnlockTerrainDataGraphics();
		goto QuickRedraw;
	}
    DrawTerrain(hdc, DrawRect, sunazimuth, sunelevation);
    terrainpainted=true;
 	if (DONTDRAWTHEMAP) {
		UnlockTerrainDataGraphics();
		goto QuickRedraw;
	}
    if (!QUICKDRAW) {
    	// SHADED terrain unreachable, aka glide amoeba. This is not the outlined perimeter!
        #ifdef GTL2
    	if (((FinalGlideTerrain == 2) || (FinalGlideTerrain == 4)) && 
            DerivedDrawInfo.TerrainValid) {
        #else
    	if ((FinalGlideTerrain==2) && DerivedDrawInfo.TerrainValid) {
        #endif
    	  DrawTerrainAbove(hdc, DrawRect);
    	}
    }
    UnlockTerrainDataGraphics();
  }

  #if NEWSMARTZOOM
  }
  #endif

  //
  // REMINDER: WE ARE IN MAIN MAP HERE: MSM_MAP ONLY, OR PANNING MODE!
  // MAPSPACEMODE CAN STILL CHANGE, DUE TO USER INPUT. BUT WE GOT HERE IN
  // EITHER PAN OR MSM_MAP.
  //

  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }

  if (IsMultimapTopology()) {
    DrawTopology(hdc, DrawRect);
  } else {
	// If no topology wanted, but terrain painted, we paint only water stuff
	if (terrainpainted) DrawTopology(hdc, DrawRect,true);
  }
  #if 0
  StartupStore(_T("... Experimental1=%.0f\n"),Experimental1);
  StartupStore(_T("... Experimental2=%.0f\n"),Experimental2);
  Experimental1=0.0;
  Experimental2=0.0;
  #endif

  // Topology labels are printed first, using OLD wps positions from previous run!
  // Reset for topology labels decluttering engine occurs also in another place here!
  ResetLabelDeclutter();
  
  if (Flags_DrawTask && ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) {
	DrawTaskAAT(hdc, DrawRect);
  }

  
  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }

  if (IsMultimapAirspace())
  {
    if ( (GetAirSpaceFillType() == asp_fill_ablend_full) || (GetAirSpaceFillType() == asp_fill_ablend_borders) )
      DrawTptAirSpace(hdc, rc);
    else {
	if ( GetAirSpaceFillType() == asp_fill_border_only)
		DrawAirSpaceBorders(hdc, rc); // full screen, to hide clipping effect on low border
	else
		DrawAirSpace(hdc, rc);	 // full screen, to hide clipping effect on low border
    }
  }

  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }

  // In QUICKDRAW dont draw trail, thermals, glide terrain
  if (QUICKDRAW) goto _skip_stuff;
#define  TRAIL_OVER_AIRFIELD
#ifndef TRAIL_OVER_AIRFIELD
  if(TrailActive) {
	// NEED REWRITING
	LKDrawTrail(hdc, Orig_Aircraft, DrawRect);
  }
#endif
  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }

  DrawThermalEstimate(hdc, DrawRect);
  if (OvertargetMode==OVT_THER) DrawThermalEstimateMultitarget(hdc, DrawRect);
 
  // draw red cross on glide through terrain marker
  if (FinalGlideTerrain && DerivedDrawInfo.TerrainValid) {
    DrawGlideThroughTerrain(hdc, DrawRect);
  }
  
  if (DONTDRAWTHEMAP) { 
	goto QuickRedraw;
  }

_skip_stuff:

  if (IsMultimapAirspace() && AirspaceWarningMapLabels)
  {
	DrawAirspaceLabels(hdc, DrawRect, Orig_Aircraft);
	if (DONTDRAWTHEMAP) { // 100319
		goto QuickRedraw;
	}
  }

  if (IsMultimapWaypoints()) {
	DrawWaypointsNew(hdc,DrawRect);
  }
#ifdef TRAIL_OVER_AIRFIELD
  if(TrailActive) {
	// NEED REWRITING
	LKDrawTrail(hdc, Orig_Aircraft, DrawRect);
  }
#endif
  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }

  if (Flags_DrawTask && ValidTaskPoint(ActiveWayPoint) && ValidTaskPoint(1)) {
	DrawTask(hdc, DrawRect, Orig_Aircraft);
  }

  // FAI optimizer does not depend on tasks, being based on trace
  if (Flags_DrawFAI)  DrawFAIOptimizer(hdc, DrawRect, Orig_Aircraft);

  // In QUICKDRAW do not paint other useless stuff
  if (QUICKDRAW) {
	if (extGPSCONNECT) DrawBearing(hdc, DrawRect);
	goto _skip_2;
  }

  // ---------------------------------------------------

  DrawTeammate(hdc, rc);

  if (extGPSCONNECT) {
    DrawBestCruiseTrack(hdc, Orig_Aircraft);
    DrawBearing(hdc, DrawRect);
  }

  // draw wind vector at aircraft
  if (NOTANYPAN) {
    DrawWindAtAircraft2(hdc, Orig_Aircraft, DrawRect);
  } else if (mode.Is(Mode::MODE_TARGET_PAN)) {
    DrawWindAtAircraft2(hdc, Orig, rc);
  }

  #if NEWSMARTZOOM
  // Save the current rendered map before painting overlays
  if ( OFFSMARTZOOM ) {
	quickdrawscale=zoom.DrawScale();
	BitBlt(hdcQuickDrawWindow, 0, 0, MapRect.right-MapRect.left, MapRect.bottom-MapRect.top,
		hdcDrawWindow, 0, 0, SRCCOPY);
  }
  #endif

  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }

  // Draw traffic and other specifix LK gauges
  LKDrawFLARMTraffic(hdc, DrawRect, Orig_Aircraft);

  // ---------------------------------------------------
_skip_2:

  if (NOTANYPAN) {
    if ( (ThermalBar>=1 && MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) || (ThermalBar==2 && MapWindow::mode.Is(MapWindow::Mode::MODE_CRUISE)) ) {
        DrawThermalBand(hdcDrawWindow, rc);
    }

    if (IsMultimapOverlaysText()) DrawLook8000(hdc,rc); 
    DrawBottomBar(hdc,rc);
  }

  if (DONTDRAWTHEMAP) {
	goto QuickRedraw;
  }
    
  if (IsMultimapOverlaysGauges() && (LKVarioBar && NOTANYPAN)) 
	LKDrawVario(hdc,rc);
  
  // Draw glider or paraglider
  if (extGPSCONNECT) {
    DrawAircraft(hdc, Orig_Aircraft);
  }

  if (NOTANYPAN) {
	if (TrackBar) DrawHeading(hdc, Orig, DrawRect); 
  }

  #if USETOPOMARKS
  // marks on top...
  DrawMarks(hdc, rc);
  #endif

  if (ISGAAIRCRAFT && IsMultimapOverlaysGauges() && NOTANYPAN) DrawHSI(hdc,Orig,DrawRect); 

  if (!INPAN) {
	DrawMapScale(hdcDrawWindow,rc, zoom.BigZoom()); // unused BigZoom
	DrawCompass(hdcDrawWindow, rc, DisplayAngle);
  }

  if (IsMultimapOverlaysGauges() && NOTANYPAN) DrawFinalGlide(hdcDrawWindow,rc);


#ifdef CPUSTATS
  DrawCpuStats(hdc,rc);
#endif
#ifdef DRAWDEBUG
  DrawDebug(hdc,rc);
#endif

}


