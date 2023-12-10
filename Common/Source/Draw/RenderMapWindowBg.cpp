/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Message.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "Multimap.h"
#include "Sound/Sound.h"
#include "ScreenProjection.h"

extern bool FastZoom;
extern bool TargetDialogOpen;
extern unsigned int DrawTerrainTimer;


void MapWindow::RenderOverlayGauges(LKSurface& Surface, const RECT& rc) {
    DrawThermalBand(Surface, rc);
    if(LKVarioBar > vBarDisabled) {
        LKDrawVario(Surface, rc);
    }
    DrawFinalGlide(Surface, rc);
}

void MapWindow::RenderMapWindowBg(LKSurface& Surface, const RECT& rc) {

    if ( (LKSurface::AlphaBlendSupported() && BarOpacity < 100) || mode.AnyPan() ) {
        RECT newRect = {0, 0, ScreenSizeX, ScreenSizeY};
        MapWindow::ChangeDrawRect(newRect);
    } else {
        RECT newRect = {0, 0, ScreenSizeX, ScreenSizeY - BottomSize - (ScreenSizeY-MapRect.bottom)-1};
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

    if (PGZoomTrigger) {
        if (!mode.Is(Mode::MODE_PANORAMA)) {
            mode.Special(Mode::MODE_SPECIAL_PANORAMA, true);
            LastZoomTrigger = DrawInfo.Time;

            Message::AddMessage(1000, 3, MsgToken<872>()); // LANDSCAPE ZOOM FOR 20s
            LKSound(TEXT("LK_TONEUP.WAV"));
        } else {
            // previously called, see if time has passed
            if (DrawInfo.Time > (LastZoomTrigger + 20.0)) {
                // time has passed, lets go back
                LastZoomTrigger = 0; // just for safety
                mode.Special(Mode::MODE_SPECIAL_PANORAMA, false);
                PGZoomTrigger = false;
                Message::AddMessage(1500, 3, MsgToken<873>()); // BACK TO NORMAL ZOOM
                LKSound(TEXT("LK_TONEDOWN.WAV"));
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
    if (DONTDRAWTHEMAP) {
        const bool isMultimap = IsMultiMapShared(); // DrawMapSpace can change "MapSpaceMode", get this before.
        DrawMapSpace(Surface, rc);
        // Is this a "shared map" environment?
        if (isMultimap) {
            // Shared map, of course not MSN_MAP, since dontdrawthemap was checked
            //
            if (IsMultimapOverlaysGauges()) {
                RenderOverlayGauges(Surface, rc);
            }
            if (IsMultimapOverlaysText()) {
                DrawLook8000(Surface, rc);
            }

        } else {
            // Not in map painting environment
            // ex. nearest pages, but also MAPRADAR..
        }

        //
        DrawBottomBar(Surface, rc);
        // no need to do SelectObject as at the bottom of function
        return;
    }

    POINT Orig, Orig_Aircraft;
    CalculateOrigin(rc, &Orig);
    const ScreenProjection _Proj = CalculateScreenPositions(Orig, rc, &Orig_Aircraft);

    // When no terrain is painted, set a background0
    // Remember that in this case we have plenty of cpu time to spend for best result
    if (!IsMultimapTerrain() || !DerivedDrawInfo.TerrainValid || !RasterTerrain::isTerrainLoaded()) {
        // We force LK painting black values on screen depending on the background color in use
        // TODO make it an array once settled
        // blackscreen would force everything to be painted white, instead
        LKTextBlack = BgMapColorTextBlack[BgMapColor];
        if (BgMapColor > 6) BlackScreen = true;
        else BlackScreen = false;
    } else {
        LKTextBlack = false;
        BlackScreen = false;
    }

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

    bool terrainpainted = false;

    if ((IsMultimapTerrain() && (DerivedDrawInfo.TerrainValid) && RasterTerrain::isTerrainLoaded())) {
        // sunelevation is never used, it is still a todo in Terrain
        double sunelevation = 40.0;
        double sunazimuth = GetAzimuth(DrawInfo, DerivedDrawInfo);

        LockTerrainDataGraphics();
        if (DONTDRAWTHEMAP) { // 100318
            UnlockTerrainDataGraphics();
            goto QuickRedraw;
        }

        if(DrawTerrain(Surface, DrawRect, _Proj, sunazimuth, sunelevation)) {
            terrainpainted = true;
        }

        if (DONTDRAWTHEMAP) {
            UnlockTerrainDataGraphics();
            goto QuickRedraw;
        }
        if (!QUICKDRAW) {
            // SHADED terrain unreachable, aka glide amoeba. This is not the outlined perimeter!
            if (((FinalGlideTerrain == 2) || (FinalGlideTerrain == 4)) && DerivedDrawInfo.TerrainValid) {
                DrawTerrainAbove(Surface, DrawRect);
            }
        }
        UnlockTerrainDataGraphics();
    }

    //
    // REMINDER: WE ARE IN MAIN MAP HERE: MSM_MAP ONLY, OR PANNING MODE!
    // MAPSPACEMODE CAN STILL CHANGE, DUE TO USER INPUT. BUT WE GOT HERE IN
    // EITHER PAN OR MSM_MAP.
    //

    if (DONTDRAWTHEMAP) {
        goto QuickRedraw;
    }

    if(!terrainpainted) {
        // fill background..
        Surface.FillRect(&rc, hInvBackgroundBrush[BgMapColor]);
    }


    if (IsMultimapTopology()) {
        DrawTopology(Surface, DrawRect, _Proj);
    } else {
        // If no topology wanted, but terrain painted, we paint only water stuff
        if (terrainpainted) {
            DrawTopology(Surface, DrawRect, _Proj, true);
        }
    }

    // Topology labels are printed first, using OLD wps positions from previous run!
    // Reset for topology labels decluttering engine occurs also in another place here!
    ResetLabelDeclutter();

    if ((Flags_DrawTask || TargetDialogOpen) && ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1)) {
        DrawTaskAAT(Surface, DrawRect);
    }


    if (DONTDRAWTHEMAP) {
        goto QuickRedraw;
    }

    if (IsMultimapAirspace()) {
        DrawAirSpace(Surface, rc, _Proj);
    }

    if (DONTDRAWTHEMAP) {
        goto QuickRedraw;
    }

    // In QUICKDRAW dont draw trail, thermals, glide terrain
    if (QUICKDRAW) {
        goto _skip_stuff;
    }

    DrawThermalEstimate(Surface, DrawRect, _Proj);
    if (OvertargetMode == OVT_THER) DrawThermalEstimateMultitarget(Surface, DrawRect, _Proj);

    // draw red cross on glide through terrain marker
    if (FinalGlideTerrain && DerivedDrawInfo.TerrainValid) {
        DrawGlideThroughTerrain(Surface, DrawRect, _Proj);
    }

    if (DONTDRAWTHEMAP) {
        goto QuickRedraw;
    }

_skip_stuff:

    if (IsMultimapAirspace() && AirspaceWarningMapLabels) {
        DrawAirspaceLabels(Surface, DrawRect, _Proj, Orig_Aircraft);
        if (DONTDRAWTHEMAP) { // 100319
            goto QuickRedraw;
        }
    }

    if (IsMultimapWaypoints()) {
        DrawWaypointsNew(Surface, DrawRect, _Proj);
    }
    if (TrailActive) {
        LKDrawLongTrail(Surface, DrawRect, _Proj);
        LKDrawTrail(Surface, DrawRect, _Proj);
    }
    if (DONTDRAWTHEMAP) {
        goto QuickRedraw;
    }

    if ((Flags_DrawTask || TargetDialogOpen) && ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1)) {
        DrawTask(Surface, DrawRect, _Proj, Orig_Aircraft);

    }
    if (Flags_DrawFAI) {
        if (MapWindow::DerivedDrawInfo.Flying) { // FAI optimizer does not depend on tasks, being based on trace
            DrawFAIOptimizer(Surface, DrawRect, _Proj, Orig_Aircraft);
        } else { // not flying => show FAI sectors for the task
            if (ValidTaskPoint(ActiveTaskPoint) && ValidTaskPoint(1)) {
                DrawTaskSectors(Surface, DrawRect, _Proj);
            }
        }
    }

    if (AdditionalContestRule != CContestMgr::ContestRule::NONE &&
        ( OvertargetMode ==  OVT_XC || Flags_DrawXC ) ) {   // if we target the XC closing point we also draw the current best XC triangle if available
      DrawXC(Surface, DrawRect, _Proj, Orig_Aircraft);
    }

    // In QUICKDRAW do not paint other useless stuff
    if (QUICKDRAW) {
        if (extGPSCONNECT) DrawBearing(Surface, DrawRect, _Proj);
        goto _skip_2;
    }

    // ---------------------------------------------------

    DrawTeammate(Surface, rc, _Proj);

    if (extGPSCONNECT) {
        DrawBestCruiseTrack(Surface, Orig_Aircraft);
        DrawBearing(Surface, DrawRect, _Proj);
    }

    // draw wind vector at aircraft
    if (NOTANYPAN) {
        DrawWindAtAircraft2(Surface, Orig_Aircraft, DrawRect);
    } else if (mode.Is(Mode::MODE_TARGET_PAN)) {
        DrawWindAtAircraft2(Surface, Orig, rc);
    }

    if (DONTDRAWTHEMAP) {
        goto QuickRedraw;
    }

    // Draw traffic and other specifix LK gauges
    LKDrawFLARMTraffic(Surface, DrawRect, _Proj, Orig_Aircraft);

    // Draw FANET-Data on Map
    LKDrawFanetData(Surface, DrawRect, _Proj, Orig_Aircraft);

    // ---------------------------------------------------
_skip_2:

    if (NOTANYPAN) {

        if (IsMultimapOverlaysGauges()) {
            RenderOverlayGauges(Surface, rc);
        }

        if (TrackBar) {
            DrawHeading(Surface, Orig, DrawRect);
            if (ISGAAIRCRAFT) {
                DrawFuturePos(Surface, Orig, DrawRect);
            }
        }

        if (ISGAAIRCRAFT) {
            DrawGAscreen(Surface, Orig_Aircraft, DrawRect);
        }

        if (IsMultimapOverlaysText()) {
            DrawLook8000(Surface, rc);
        }
        DrawBottomBar(Surface, rc);
    }

    if (DONTDRAWTHEMAP) {
        goto QuickRedraw;
    }

    // Draw glider or paraglider
    if (extGPSCONNECT) {
        DrawAircraft(Surface, Orig_Aircraft);
    }

    if (!INPAN) {
        DrawMapScale(Surface, rc, _Proj); // unused BigZoom
        DrawCompass(Surface, rc, DisplayAngle);
    }
}
