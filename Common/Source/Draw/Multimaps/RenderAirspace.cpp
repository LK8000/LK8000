/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "McReady.h"
#include "Dialogs.h"
#include "Atmosphere.h"
#include "LKInterface.h"
#include "Airspace/LKAirspace.h"
#include "RGB.h"
#include "Sideview.h"
#include "LKObjects.h"
#include "Multimap.h"
#include "Topology.h"
#include "InputEvents.h"
#include "utils/2dpclip.h"
#include "NavFunctions.h"
#include "Asset.hpp"

extern int Sideview_iNoHandeldSpaces;
extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];

double fSplitFact = 0.30;
double fOffset = 0.0;
using std::min;
using std::max;
double fZOOMScale[NUMBER_OF_SHARED_MULTIMAPS] = {1.0, 1.0, 1.0, 1.0};
double fDelta = MIN_OFFSET;
extern POINT startScreen;
extern LKColor Sideview_TextColor;

double PirkerAnalysis(NMEA_INFO *Basic, DERIVED_INFO *Calculated, const double this_bearing, const double GlideSlope);


#define TBSIZE 80
#define ADDITIONAL_INFO_THRESHOLD 0.7

//#define USE_TCOLORS 1
//#define SHOWLD	// optional

void MapWindow::RenderAirspace(LKSurface& Surface, const RECT& rc_input) {

    RECT rci = rc_input;

    if (LKVarioBar > 0 && LKVarioBar <= vBarVarioGR)
        if (IsMultimapOverlaysGauges())
            rci.left += LKVarioSize;
    zoom.SetLimitMapScale(false);
    /****************************************************************/
    if (GetSideviewPage() == IM_NEAR_AS)
        return RenderNearAirspace(Surface, rci);
    /****************************************************************/

    int x = 0, y = 0;
    static bool bHeightScale = false;
    RECT rc = rci; /* rectangle for sideview */
    bool bInvCol = true; //INVERTCOLORS;
    static double fHeigtScaleFact = MIN_OFFSET;
    double fDist = 55.0 * 1000; // kmbottom
    if (ISPARAGLIDER) fDist = 20.0 * 1000.0;
    double aclat, aclon, acb, speed, calc_average30s;
    double GPSbrg = 0;
    double wpt_brg;
    double wpt_dist;
    double wpt_altarriv;
    double wpt_altitude;
    double wpt_altarriv_mc0;
    double calc_terrainalt;
    double calc_altitudeagl;
    double fMC0 = 0.0f;
    int overindex = -1;
    bool show_mc0 = true;
#ifdef SHOWLD
    double fLD;
#endif
    SIZE tsize;
    TCHAR text[TBSIZE + 1];
    TCHAR text2[TBSIZE + 1];
    TCHAR buffer[TBSIZE + 1];
    BOOL bDrawRightSide = false;
    LKColor GREEN_COL = RGB_GREEN;
    //LKColor RED_COL       = RGB_LIGHTORANGE;
    LKColor BLUE_COL = RGB_BLUE;
    LKColor LIGHTBLUE_COL = RGB_LIGHTBLUE;
    LKColor col = RGB_BLACK;
    double zoomfactor = 1;

    int *iSplit = &Multimap_SizeY[Get_Current_Multimap_Type()];
    unsigned short getsideviewpage = GetSideviewPage();
    LKASSERT(getsideviewpage < NO_SIDEVIEW_PAGES);
    static_assert(std::size(fZOOMScale) == NUMBER_OF_SHARED_MULTIMAPS, "wrong array size");

    if (Current_Multimap_SizeY != *iSplit) {
        Current_Multimap_SizeY = *iSplit;
        SetSplitScreenSize(*iSplit);
    }
    
    if (Current_Multimap_SizeY < SIZE4)
        zoomfactor = ZOOMFACTOR;
    else
        zoomfactor = 2.0;

#if 0
    StartupStore(_T("...Type=%d  CURRENT=%d  Multimap_size=%d = isplit=%d\n"),
            Get_Current_Multimap_Type(), Current_Multimap_SizeY, Multimap_SizeY[Get_Current_Multimap_Type()], *iSplit);
#endif

    if (bInvCol)
        col = RGB_WHITE;

    LKPen hpHorizon(PEN_SOLID, IBLSCALE(1), col);
    LKBrush hbHorizon(col);
    const auto OldPen = Surface.SelectObject(hpHorizon);
    const auto OldBrush = Surface.SelectObject(hbHorizon);


    //bool bFound = false;
    Surface.SelectObject(OldPen);
    Surface.SelectObject(OldBrush);

    RECT rct = rc; /* rectangle for topview */
    rc.top = (long) ((double) (rci.bottom - rci.top) * fSplitFact)+rci.top;
    rct.bottom = rc.top;
    // Expose the topview rect size in use..
    Current_Multimap_TopRect = rct;


    if (bInvCol)
        Sideview_TextColor = INV_GROUND_TEXT_COLOUR;
    else
        Sideview_TextColor = RGB_WHITE;

    Surface.SetTextColor(Sideview_TextColor);

    /****************************************************************/
    switch (LKevent) {
        case LKEVENT_NEWRUN:
            // CALLED ON ENTRY: when we select this page coming from another mapspace
            ///fZOOMScale = 1.0;
            bHeightScale = false;
            if (IsMultimapTopology()) ForceVisibilityScan = true;
            //	fHeigtScaleFact = 1000;
            break;
        case LKEVENT_UP:
            // click on upper part of screen, excluding center
            if (bHeightScale)
                fHeigtScaleFact -= fDelta;
            else
                fZOOMScale[getsideviewpage] /= zoomfactor;

            if (IsMultimapTopology()) ForceVisibilityScan = true;
            break;

        case LKEVENT_DOWN:
            // click on lower part of screen,  excluding center
            if (bHeightScale)
                fHeigtScaleFact += fDelta;
            else
                fZOOMScale[getsideviewpage] *= zoomfactor;

            if (IsMultimapTopology()) ForceVisibilityScan = true;
            break;

        case LKEVENT_LONGCLICK:

            if (MapSpaceMode == MSM_VISUALGLIDE) {
                LKevent = LKEVENT_NONE;
                if ((fSplitFact * 100) == SIZE4) { // TopView full screen?
                    break;

                }
                if (Sideview_VGBox_Number == 0) {
                    break;
                }

                for (unsigned short i = 0; i < MAXBSLOT; i++) {
                    if (Sideview_VGBox[i].bottom == 0) continue;
                    if (Sideview_VGBox[i].right == 0) continue;
                    if (PtInRect(&Sideview_VGBox[i], startScreen)) {
                        if (ValidWayPoint(Sideview_VGWpt[i])) {
                            /*
                             * we can't show dialog from Draw thread
                             * instead, new event is queued, dialog will be popup by main thread
                             */
                            InputEvents::processPopupDetails(InputEvents::PopupWaypoint, Sideview_VGWpt[i]);
                        }
                    }
                }
            } else {
                bool bShow = false;
                for (int k = 0; k <= Sideview_iNoHandeldSpaces; k++) {
                    if (Sideview_pHandeled[k].psAS != NULL) {
                        if (PtInRect(&(Sideview_pHandeled[k].rc), startScreen)) {
                            dlgAddMultiSelectListItem((long*) Sideview_pHandeled[k].psAS, 0, IM_AIRSPACE, 0);
                            bShow = true;
                        }
                    }
                }
                if (bShow) {
                    /*
                     * we can't show dialog from Draw thread
                     * instead, new event is queued, dialog will be popup by main thread
                     */
                    InputEvents::processGlideComputer(GCE_POPUP_MULTISELECT);

                    // reset event, otherwise Distance/height zoom mod are also triggered
                    LKevent = LKEVENT_NONE;
                }
            }
            if (LKevent != LKEVENT_NONE) {
                if (PtInRect(&rc, startScreen)) {
                    bHeightScale = !bHeightScale;
                } else if (PtInRect(&rct, startScreen)) {
                    bHeightScale = false;
                }
            }
            break;

        case LKEVENT_PAGEUP:
#ifdef OFFSET_SETP
            if (bHeightScale)
                fOffset -= OFFSET_SETP;
            else
#endif
            {
                if (*iSplit == SIZE1) *iSplit = SIZE0;
                if (*iSplit == SIZE2) *iSplit = SIZE1;
                if (*iSplit == SIZE3) *iSplit = SIZE2;
                if (*iSplit == SIZE4) *iSplit = SIZE3;
            }
            break;
        case LKEVENT_PAGEDOWN:
#ifdef OFFSET_SETP
            if (bHeightScale)
                fOffset += OFFSET_SETP;
            else
#endif
            {
                if (*iSplit == SIZE3) *iSplit = SIZE4;
                if (*iSplit == SIZE2) *iSplit = SIZE3;
                if (*iSplit == SIZE1) *iSplit = SIZE2;
                if (*iSplit == SIZE0) *iSplit = SIZE1;
            }

            break;
        case LKEVENT_SHORTCLICK:
            // normally we are clicking in the center map, so it is not a zoom
            // and we ignore it.
            break;
        default:
#if TESTBENCH
            if (LKevent != 0) StartupStore(_T("... RenderAirspace, UNKNOWN EVENT: %d\n"), LKevent);
#endif
            break;
    }
    LKevent = LKEVENT_NONE;

    // Current_Multimap_SizeY is global, and must be used by all multimaps!
    // It is defined in Multimap.cpp and as an external in Multimap.h
    // It is important that it is updated, because we should resize terrain
    // only if needed! Resizing terrain takes some cpu and some time.
    // So we need to know when this is not necessary, having the same size of previous
    // multimap, if we are switching.
    // The current implementation is terribly wrong by managing resizing of sideview in
    // each multimap: it should be done by a common layer.
    // CAREFUL:
    // If for any reason DrawTerrain() is called after resizing to 0 (full sideview)
    // LK WILL CRASH with no hope to recover.
    if (Current_Multimap_SizeY != *iSplit) {
        Current_Multimap_SizeY = *iSplit;
        SetSplitScreenSize(*iSplit);
        rc.top = (long) ((double) (rci.bottom - rci.top) * fSplitFact)+rci.top;
        rct.bottom = rc.top;
    }

    double hmin = max(0.0, DerivedDrawInfo.NavAltitude - 0);
    double hmax = max(MAXALTTODAY, DerivedDrawInfo.NavAltitude + 0);
#ifdef OFFSET_SETP
    if ((hmax + fOffset) > MAX_ALTITUDE)
        fOffset -= OFFSET_SETP;
    if ((hmin + fOffset) < 0.0)
        fOffset += OFFSET_SETP;

    hmin += fOffset;
    hmax += fOffset;
#endif

    if ((fHeigtScaleFact) < MIN_ALTITUDE)
        fHeigtScaleFact = MIN_ALTITUDE;
    //	if(  (DerivedDrawInfo.NavAltitude +  fHeigtScaleFact) > MAX_ALTITUDE)
    //	  fHeigtScaleFact -= fDelta;

    hmax = DerivedDrawInfo.NavAltitude + fHeigtScaleFact;
    hmin = DerivedDrawInfo.NavAltitude - 2 * fHeigtScaleFact;
    if (hmin < 0)
        hmin = 0;


    if (bInvCol) {
        GREEN_COL = GREEN_COL.ChangeBrightness(0.6);
        //RED_COL       = RGB_RED.ChangeBrightness(0.6);;
        BLUE_COL = BLUE_COL.ChangeBrightness(0.6);
        LIGHTBLUE_COL = LIGHTBLUE_COL.ChangeBrightness(0.4);
    }
    //  LockFlightData();
    {
        fMC0 = GlidePolar::SafetyMacCready;
        aclat = DrawInfo.Latitude;
        aclon = DrawInfo.Longitude;
        acb = DrawInfo.TrackBearing;
        GPSbrg = acb;

        acb = DrawInfo.TrackBearing;
        GPSbrg = DrawInfo.TrackBearing;
        speed = DrawInfo.Speed;

        calc_average30s = DerivedDrawInfo.Average30s;
        calc_terrainalt = DerivedDrawInfo.TerrainAlt;
        calc_altitudeagl = DerivedDrawInfo.AltitudeAGL;
    }
    //  UnlockFlightData();

    auto hfOld = Surface.SelectObject(LK8PanelUnitFont);
    overindex = GetOvertargetIndex();
    wpt_brg = AngleLimit360(acb);
    wpt_dist = 0.0;
    wpt_altarriv = 0.0;
    wpt_altarriv_mc0 = 0.0;
    wpt_altitude = 0.0;
    fMC0 = 0.0;
#ifdef SHOWLD
    fLD = 0.0;
#endif

    double overindex_brg = 0;
    if (getsideviewpage == IM_NEXT_WP) {
        // Show towards target
        if (overindex >= 0) {
            double wptlon = WayPointList[overindex].Longitude;
            double wptlat = WayPointList[overindex].Latitude;
            DistanceBearing(aclat, aclon, wptlat, wptlon, &wpt_dist, &acb);
            overindex_brg = AngleLimit360(acb);

            wpt_brg = AngleLimit360(wpt_brg - acb + 90.0);
            fDist = max(5.0 * 1000.0, wpt_dist * 1.31); // 30% more distance to show, minimum 5km
            wpt_altitude = WayPointList[overindex].Altitude;
            // calculate the MC=0 arrival altitude



            //LockFlightData();
            wpt_altarriv_mc0 = DerivedDrawInfo.NavAltitude -
                    GlidePolar::MacCreadyAltitude(fMC0,
                    wpt_dist,
                    acb,
                    DerivedDrawInfo.WindSpeed,
                    DerivedDrawInfo.WindBearing,
                    0, 0, true,
                    0) - WayPointList[overindex].Altitude;
            if (IsSafetyAltitudeInUse(overindex)) wpt_altarriv_mc0 -= (SAFETYALTITUDEARRIVAL / 10);

            wpt_altarriv = DerivedDrawInfo.NavAltitude -
                    GlidePolar::MacCreadyAltitude(MACCREADY,
                    wpt_dist,
                    acb,
                    DerivedDrawInfo.WindSpeed,
                    DerivedDrawInfo.WindBearing,
                    0, 0, true,
                    0) - WayPointList[overindex].Altitude;

#ifdef SHOWLD
            if ((DerivedDrawInfo.NavAltitude - wpt_altarriv + wpt_altitude) != 0)
                fLD = (int) wpt_dist / (DerivedDrawInfo.NavAltitude - wpt_altarriv + wpt_altitude);
            else
                fLD = 999;
#endif

            if (IsSafetyAltitudeInUse(overindex)) wpt_altarriv -= (SAFETYALTITUDEARRIVAL / 10);


            //UnlockFlightData();

        }// valid overindex
        else {
            // Not valid overindex, we paint like HEADING
            wpt_brg = 90.0;
        }
    } // NEXTWP

    // Else in MAPWPT with no overtarget we paint a track heading

    if (fZOOMScale[getsideviewpage] != 1.0) {
        if ((fDist * fZOOMScale[getsideviewpage]) > 750000)
            fZOOMScale[getsideviewpage] /= zoomfactor;

        if ((fDist * fZOOMScale[getsideviewpage]) < 500)
            fZOOMScale[getsideviewpage] *= zoomfactor;
    }
    fDist *= fZOOMScale[getsideviewpage];

    DiagrammStruct sDia;
    sDia.fXMin = -9500.0f;

#if 0
    if (sDia.fXMin > (-0.1f * fDist))
        sDia.fXMin = -0.1f * fDist;
    if (-sDia.fXMin > (fDist))
        sDia.fXMin = -fDist;
#else
    if (IsMultimapOverlaysText()/*|| IsMultimapOverlaysGauges()*/) {
        sDia.fXMin = -0.31f * fDist;
    } else {
        sDia.fXMin = -0.12f * fDist;
    }
#endif

    sDia.fXMax = fDist;
    sDia.fYMin = hmin;
    sDia.fYMax = hmax;


    // What is this text color used for.. apparently there is no difference to take it off.
    Surface.SetTextColor(GROUND_TEXT_COLOUR);
    if (bInvCol)
        if (sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
            Surface.SetTextColor(INV_GROUND_TEXT_COLOUR);

    // This is tricky and bad, a lot misleading in fact.
    // We manage the long click belonging to the PREVIOUS drawvisualglide.
    // It is reset here before a possible recalculation, otherwise we risk to
    // check for long clicks on no more drawn areas.
    Sideview_VGBox_Number = 0;

    if (getsideviewpage == IM_VISUALGLIDE) {

        // VisualGlide not full screen?
        if (fSplitFact > 0.0) {
            sDia.rc = rct;
            sDia.rc.bottom -= 1;
            MapWindow::SharedTopView(Surface, &sDia, GPSbrg, 90.0);
            sDia.rc = rct;
        }


        if ((fSplitFact * 100) < SIZE4) { // TopView not full screen?
            const RECT& oldRc = sDia.rc;
            sDia.rc = rc;
            DrawVisualGlide(Surface, sDia);
            sDia.rc = oldRc;
            DrawMultimap_SideTopSeparator(Surface, rct);
        }
        zoom.SetLimitMapScale(true);
        return;
    }


    if (fSplitFact > 0.0) {
        sDia.rc = rct;
        sDia.rc.bottom -= 1;
        if (getsideviewpage == IM_HEADING)
            MapWindow::SharedTopView(Surface, &sDia, GPSbrg, 90.0);

        if (getsideviewpage == IM_NEXT_WP)
            MapWindow::SharedTopView(Surface, &sDia, acb, wpt_brg);

        //sDia.rc = rcc;

    }


    RECT rcc = rc;
    if ((Current_Multimap_SizeY < SIZE4) && (sDia.fYMin < GC_SEA_LEVEL_TOLERANCE))
        rcc.bottom -= SV_BORDER_Y; /* scale witout sea  */
    sDia.rc = rcc;


    if ((Current_Multimap_SizeY < SIZE4) || (GetMMNorthUp(getsideviewpage) != NORTHUP))
        RenderAirspaceTerrain(Surface, aclat, aclon, acb, (DiagrammStruct*) & sDia);



    int x0 = CalcDistanceCoordinat(0, &sDia);
    int y0 = CalcHeightCoordinat(0, &sDia);

    double xtick = 0.5;
    double fRange = fabs(sDia.fXMax - sDia.fXMin);
    if (fRange > 2.0 * 1000.0) xtick = 1.0;
    if (fRange > 8.0 * 1000.0) xtick = 2.0;
    if (fRange > 15 * 1000.0) xtick = 5.0;
    if (fRange > 50.0 * 1000.0) xtick = 10.0;
    if (fRange > 100.0 * 1000.0) xtick = 20.0;
    if (fRange > 200.0 * 1000.0) xtick = 25.0;
    if (fRange > 250.0 * 1000.0) xtick = 50.0;
    if (fRange > 500.0 * 1000.0) xtick = 100.0;
    if (fRange > 1000.0 * 1000.0) xtick = 500.0;

    if (bInvCol) {
        Surface.SelectObject(LK_BLACK_PEN);
        Surface.SelectObject(LKBrush_Black);
    } else {
        Surface.SelectObject(LK_WHITE_PEN);
        Surface.SelectObject(LKBrush_White);
    }

    Surface.SelectObject(LK8PanelUnitFont);
    LKColor txtCol = GROUND_TEXT_COLOUR;
    if (bInvCol)
        if (sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
            txtCol = INV_GROUND_TEXT_COLOUR;
    Surface.SetBackgroundTransparent();
    Surface.SetTextColor(txtCol);

    _stprintf(text, TEXT("%s"), Units::GetUnitName(Units::GetUserDistanceUnit()));

    if (Current_Multimap_SizeY < SIZE4)
        switch (GetMMNorthUp(getsideviewpage)) {
            case NORTHUP:
            default:
                DrawXGrid(Surface, rc, xtick / DISTANCEMODIFY, xtick, 0, TEXT_ABOVE_LEFT, Sideview_TextColor, &sDia, text);
                break;

            case TRACKUP:
                DrawXGrid(Surface, rci, xtick / DISTANCEMODIFY, xtick, 0, TEXT_ABOVE_LEFT, Sideview_TextColor, &sDia, text);
                break;
        }
    Surface.SetTextColor(Sideview_TextColor);

    double fHeight = sDia.fYMax - sDia.fYMin;
    double ytick = 100.0;
    if (fHeight > 500.0) ytick = 200.0;
    if (fHeight > 1000.0) ytick = 500.0;
    if (fHeight > 2000.0) ytick = 1000.0;
    if (fHeight > 4000.0) ytick = 2000.0;
    if (fHeight > 8000.0) ytick = 4000.0;

    if (Units::GetUserAltitudeUnit() == unFeet)
        ytick = ytick * FEET_FACTOR;

    _stprintf(text, TEXT("%s"), Units::GetUnitName(Units::GetUserAltitudeUnit()));

    // Draw only if topview is not fullscreen
    if (Current_Multimap_SizeY < SIZE4)
        DrawYGrid(Surface, rcc, ytick / ALTITUDEMODIFY, ytick, 0, TEXT_UNDER_RIGHT, Sideview_TextColor, &sDia, text);

    POINT line[4];

    // draw target symbolic line

    bool bConicalFinal = false;
    double Slope = 0.0, VOpt = 0.0, Sink = 0.0;
    POINT BestReach = {0,0};

    if (overindex >= 0) {
        int iWpPos = CalcDistanceCoordinat(wpt_dist, &sDia);
        if ((getsideviewpage == IM_NEXT_WP) && (Current_Multimap_SizeY < SIZE4)) {

            if( (OvertargetMode == OVT_TASK) && DoOptimizeRoute() ) {
                if ( ValidTaskPoint(ActiveTaskPoint)) {
                    double AltBase = 0.0, Radius = 0.0;
                    LockTaskData();
                    sector_type_t Type = sector_type_t::CIRCLE;
                    GetTaskSectorParameter(ActiveTaskPoint, &Type, nullptr);
                    const double baselon = WayPointList[Task[ActiveTaskPoint].Index].Longitude;
                    const double baselat = WayPointList[Task[ActiveTaskPoint].Index].Latitude;
                    if(Type == sector_type_t::CONE) {
                        AltBase = Task[ActiveTaskPoint].PGConeBase;
                        Radius = Task[ActiveTaskPoint].PGConeBaseRadius;
                        Slope = Task[ActiveTaskPoint].PGConeSlope;
                    }
                    UnlockTaskData();
                    if(Type == sector_type_t::CONE && Slope > 0.) {
                        double basedist = 0.;
                        DistanceBearing(aclat, aclon, baselat, baselon, &basedist, NULL);
                        VOpt = GlidePolar::FindSpeedForSlope(Slope);
                        Sink = GlidePolar::SinkRate(VOpt);

                        double ck = (basedist + Slope * AltBase - Radius) / Slope;
                        double dOpt = (ck - DerivedDrawInfo.NavAltitude) / (VOpt + Sink * Slope) * Slope * VOpt;
                        double AltOpt = DerivedDrawInfo.NavAltitude + dOpt/VOpt*Sink;
                        double ConeBase = (Slope * AltBase - Radius) / Slope;

                        line[1].x = CalcDistanceCoordinat( basedist,  &sDia);
                        line[1].y = CalcHeightCoordinatOutbound(ConeBase, &sDia);
                        line[0].x = CalcDistanceCoordinat( 0,  &sDia);
                        line[0].y = CalcHeightCoordinatOutbound(ck, &sDia);

                        if(AltOpt > ConeBase) {
                            bConicalFinal = true;
                            show_mc0 = false;

                            line[2].x = CalcDistanceCoordinat( 0, &sDia);
                            line[2].y = CalcHeightCoordinat ( DerivedDrawInfo.NavAltitude, &sDia );
                            BestReach.x = CalcDistanceCoordinat( dOpt, &sDia);
                            BestReach.y = CalcHeightCoordinatOutbound( AltOpt,  &sDia );

                            #ifdef NO_DASH_LINES
                            Surface.DrawLine(PEN_SOLID, 2, line[2], BestReach,  RGB_BLUE, rc);
                            #else
                            Surface.DrawDashLine(3, line[2], BestReach,  RGB_BLUE, rc);
                            #endif
                        } else {
                            bConicalFinal = false;
                        }

                        if(LKGeom::ClipLine((RECT) {rc.left, rc.top, rc.right, rc.bottom}, line[0],  line[1])) {
                            #ifdef NO_DASH_LINES
                            Surface.DrawLine(PEN_SOLID, ScreenThinSize, line[0], line[1],  RGB_DARKBLUE, rc);
                            #else
                            Surface.DrawDashLine(NIBLSCALE(1), line[0], line[1],  RGB_DARKBLUE, rc);
                            #endif
                        }


                        line[1].x = CalcDistanceCoordinat( basedist,  &sDia);
                        line[1].y = CalcHeightCoordinatOutbound((Slope * AltBase - Radius) / Slope, &sDia);
                        line[0].x = CalcDistanceCoordinat( basedist*2,  &sDia);
                        line[0].y = CalcHeightCoordinatOutbound((basedist + Slope * AltBase - Radius) / Slope, &sDia);

                        if(LKGeom::ClipLine((RECT) {rc.left, rc.top, rc.right, rc.bottom}, line[0],  line[1])) {
                            #ifdef NO_DASH_LINES
                            Surface.DrawLine(PEN_SOLID, ScreenThinSize, line[0], line[1],  RGB_DARKBLUE, rc);
                            #else
                            Surface.DrawDashLine(NIBLSCALE(1), line[0], line[1],  RGB_DARKBLUE, rc);
                            #endif
                        }
                    }
                }
            }


            if (WayPointCalc[overindex].IsLandable == 0) {
                // Not landable - Mark wpt with a vertical marker line
                line[0].x = CalcDistanceCoordinat(wpt_dist, &sDia);
                line[0].y = y0;
                line[1].x = line[0].x;
                line[1].y = rc.top;
                #ifdef NO_DASH_LINES
                Surface.DrawLine(PEN_SOLID, 2, line[0], line[1],  RGB_DARKGREY, rc);
                #else
                Surface.DrawDashLine(4, line[0], line[1], RGB_DARKGREY, rc);
                #endif
            } else {
                // Landable
                line[0].x = iWpPos;
                line[0].y = CalcHeightCoordinat(wpt_altitude, &sDia);
                line[1].x = line[0].x;
                line[1].y = CalcHeightCoordinat((SAFETYALTITUDEARRIVAL / 10) + wpt_altitude, &sDia);
                Surface.DrawLine(PEN_SOLID, 5, line[0], line[1], RGB_ORANGE, rc);


                float fArrHight = 0.0f;
                if (wpt_altarriv > 0.0f) {
                    fArrHight = wpt_altarriv;
                    line[0].x = iWpPos;
                    line[0].y = CalcHeightCoordinat((SAFETYALTITUDEARRIVAL / 10) + wpt_altitude, &sDia);
                    line[1].x = line[0].x;
                    line[1].y = CalcHeightCoordinat((SAFETYALTITUDEARRIVAL / 10) + wpt_altitude + fArrHight, &sDia);
                    Surface.DrawLine(PEN_SOLID, 4, line[0], line[1], RGB_GREEN, rc);
                }
                // Mark wpt with a vertical marker line
                line[0].x = iWpPos;
                line[0].y = CalcHeightCoordinat((SAFETYALTITUDEARRIVAL / 10) + wpt_altitude + fArrHight, &sDia);
                line[1].x = line[0].x;
                line[1].y = rc.top;
                #ifdef NO_DASH_LINES
                Surface.DrawLine(PEN_SOLID, 2, line[0], line[1],  RGB_DARKGREY, rc);
                #else
                Surface.DrawDashLine(4, line[0], line[1], RGB_DARKGREY, rc);
                #endif
            }
        }
    } // overindex is valid

    // Draw estimated gliding line (blue)
    // if (speed>10.0)
    if (Current_Multimap_SizeY < SIZE4) {

        if ((getsideviewpage == IM_NEXT_WP) && (overindex >= 0)) {
            double altarriv;

            // Draw estimated gliding line MC=0 (green)
            if (!ISCAR && show_mc0) {
                altarriv = wpt_altarriv_mc0 + wpt_altitude;
                if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL / 10);
                line[0].x = CalcDistanceCoordinat(0, &sDia);
                line[0].y = CalcHeightCoordinat(DerivedDrawInfo.NavAltitude, &sDia);
                line[1].x = CalcDistanceCoordinat(wpt_dist, &sDia);
                line[1].y = CalcHeightCoordinatOutbound(altarriv, &sDia);
                #ifdef NO_DASH_LINES
                Surface.DrawLine(PEN_SOLID, 2, line[0], line[1],  RGB_BLUE, rc);
                #else
                Surface.DrawDashLine(3, line[0], line[1], RGB_BLUE, rc);
                #endif
            }
            altarriv = wpt_altarriv + wpt_altitude;
            if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL / 10);
            line[0].x = CalcDistanceCoordinat(0, &sDia);
            line[0].y = CalcHeightCoordinat(DerivedDrawInfo.NavAltitude, &sDia);

            if (ISCAR) {
                line[1].x = CalcDistanceCoordinat(wpt_dist, &sDia);
                line[1].y = CalcHeightCoordinatOutbound(wpt_altitude, &sDia);
            } else {
                line[1].x = CalcDistanceCoordinat(wpt_dist, &sDia);
                line[1].y = CalcHeightCoordinatOutbound(altarriv, &sDia);
            }
            #ifdef NO_DASH_LINES
            Surface.DrawLine(PEN_SOLID, 2, line[0], line[1],  RGB_BLUE, rc);
            #else
            Surface.DrawDashLine(3, line[0], line[1], RGB_BLUE, rc);
            #endif

        } else {
            //  double t = fDist/(speed!=0?speed:1);
            if (SIMMODE && !DerivedDrawInfo.Flying) {
                calc_average30s = -GlidePolar::SinkRateBestLd();
                if (calc_average30s == 0) calc_average30s = 0.01; // should be impossible, but we must check
                double t = fabs(DerivedDrawInfo.NavAltitude / calc_average30s);
                line[0].x = CalcDistanceCoordinat(0, &sDia);
                line[0].y = CalcHeightCoordinat(DerivedDrawInfo.NavAltitude, &sDia);
                line[1].x = CalcDistanceCoordinat(GlidePolar::Vbestld() * t, &sDia);
                line[1].y = CalcHeightCoordinat(0, &sDia);
            } else {
                if (calc_average30s == 0) calc_average30s = 0.01; // limit the horizontal infinite
                double t = fabs(DerivedDrawInfo.NavAltitude / calc_average30s);
                line[0].x = CalcDistanceCoordinat(0, &sDia);
                line[0].y = CalcHeightCoordinat(DerivedDrawInfo.NavAltitude, &sDia);
                line[1].x = CalcDistanceCoordinat(speed * t, &sDia);
                line[1].y = CalcHeightCoordinat(0, &sDia);
            }
            // Limit climb rate to flat, for free flyers
            if (ISGLIDER || ISPARAGLIDER)
                if (line[1].y < line[0].y) line[1].y = line[0].y;

            #ifdef NO_DASH_LINES
            Surface.DrawLine(PEN_SOLID, 2, line[0], line[1],  RGB_BLUE, rc);
            #else
            Surface.DrawDashLine(3, line[0], line[1], RGB_BLUE, rc);
            #endif
        }
    }


    Surface.SetTextColor(Sideview_TextColor);
    Surface.SelectObject(LKBrush_White);


    //
    // M2:  NEXT WAYPOINT PAGE, print textual overlays
    //
    if ((getsideviewpage == IM_NEXT_WP) && (Current_Multimap_SizeY < SIZE4) && (overindex >= 0)) {

        // Print current Elevation
        Surface.SetTextColor(RGB_BLACK);
        if ((calc_terrainalt - hmin) > 0) {
            Units::FormatUserAltitude(calc_terrainalt, buffer, 7);
            LK_tcsncpy(text, MsgToken(1743), TBSIZE - _tcslen(buffer));
            _tcscat(text, buffer);
            Surface.GetTextSize(text, &tsize);
            x = CalcDistanceCoordinat(0, &sDia) - tsize.cx / 2;
            y = CalcHeightCoordinat((calc_terrainalt), &sDia);
            if ((ELV_FACT * tsize.cy) < abs(rc.bottom - y)) {
                Surface.DrawText(x, rc.bottom - (int) (ELV_FACT * tsize.cy) /* rc.top-tsize.cy*/, text);
            }
        }


        // Print arrival Elevation
        Surface.SetTextColor(RGB_BLACK);
        if ((wpt_altitude - hmin) > 0) {
            Units::FormatUserAltitude(wpt_altitude, buffer, 7);
            LK_tcsncpy(text, MsgToken(1743), TBSIZE - _tcslen(buffer));
            _tcscat(text, buffer);
            Surface.GetTextSize(text, &tsize);
            x0 = CalcDistanceCoordinat(wpt_dist, &sDia) - tsize.cx / 2;
            if (abs(x - x0) > tsize.cx) {
                y = CalcHeightCoordinat((wpt_altitude), &sDia);
                if ((ELV_FACT * tsize.cy) < abs(rc.bottom - y)) {
                    Surface.DrawText(x0, rc.bottom - (int) (ELV_FACT * tsize.cy) /* rc.top-tsize.cy*/, text);
                }
            }
        }


        line[0].x = CalcDistanceCoordinat(wpt_dist, &sDia);

        //
        // Overtarget next to marker line, with distance
        //

        GetOvertargetName(text);
        text[10] = '\0'; // truncate to T>xxxxxxxx
        if (text[_tcslen(text) - 1] == ' ')
            _tcscat(text, _T(" "));
        else
            _tcscat(text, _T("  "));
        Units::FormatUserDistance(wpt_dist, text2, 7);
        _tcscat(text, text2);

        Surface.GetTextSize(text, &tsize);
        x = line[0].x - tsize.cx - NIBLSCALE(5);

        if (x < x0) bDrawRightSide = true;
        bDrawRightSide = true;
        if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
        y = rc.top + 2 * tsize.cy;


        Surface.SelectObject(LK_BLACK_PEN);
        if (!IsDithered()) {
            Surface.SelectObject(INVERTCOLORS ? LKBrush_Petrol : LKBrush_LightCyan);
        } else {
            Surface.SelectObject(INVERTCOLORS ? LKBrush_Black : LKBrush_White);
        }

        MapWindow::LKWriteBoxedText(Surface, rc, text, line[0].x, y - 3, WTALIGN_CENTER, RGB_WHITE, RGB_BLACK);

        y = line[0].y - 2 * tsize.cy;

        double altarriv = 0;
        if (fSplitFact >= ADDITIONAL_INFO_THRESHOLD) goto _after_additionals;
        if (wpt_altarriv == wpt_altarriv_mc0) goto _skip_mc0;
        if(bConicalFinal) goto _skip_mc0;

        //
        // ALTITUDE ARRIVAL AT MACCREADY 0
        //

        altarriv = wpt_altarriv_mc0; // + wpt_altitude;

        // size a template
        _stprintf(text, TEXT("Mc 99.9: +12345"));
        Surface.GetTextSize(text, &tsize);

        if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL / 10);

        _stprintf(text, TEXT("Mc %3.1f: "), (LIFTMODIFY * fMC0));
        if (wpt_altarriv_mc0 > ALTDIFFLIMIT) {
            Units::FormatUserArrival(wpt_altarriv_mc0, buffer, 7);
            _tcscat(text, buffer);
        } else {
            _tcscat(text, TEXT("---"));
        }
        x = line[0].x - tsize.cx - NIBLSCALE(5);
        if (bDrawRightSide) x = line[0].x + NIBLSCALE(5); // Show on right side if left not possible
        y = CalcHeightCoordinat(SAFETYALTITUDEARRIVAL / 10 + wpt_altarriv_mc0 + wpt_altitude, &sDia);
        y -= (int) (1.3 * tsize.cy);
        // We don't know if there are obstacles for mc0
        Surface.SelectObject(LK_BLACK_PEN);
        if (!IsDithered()) {
          Surface.SelectObject(LKBrush_Nlight);
        } else {
          Surface.SelectObject(LKBrush_White);
        }

        MapWindow::LKWriteBoxedText(Surface, rc, text, x, y, WTALIGN_LEFT, RGB_BLACK, RGB_BLACK);

_skip_mc0:



        if (SAFETYALTITUDEARRIVAL > 0) {
            // Print arrival AGL
            altarriv = wpt_altarriv;
            if (IsSafetyAltitudeInUse(overindex)) altarriv += (SAFETYALTITUDEARRIVAL / 10);
            if (altarriv > 100) {
                Units::FormatUserAltitude(altarriv, buffer, 7);
                LK_tcsncpy(text, MsgToken(1742), TBSIZE - _tcslen(buffer));
                _tcscat(text, buffer);
                Surface.GetTextSize(text, &tsize);
                x = line[0].x - NIBLSCALE(5);

                y = CalcHeightCoordinat((SAFETYALTITUDEARRIVAL / 10 + wpt_altitude + altarriv)*7 / 10, &sDia);
                Surface.SelectObject(LK_BLACK_PEN);
                Surface.SelectObject(LKBrush_Nlight);

                MapWindow::LKWriteBoxedText(Surface, rc, text, x, y, WTALIGN_RIGHT, RGB_BLACK, RGB_BLACK);
            }
        }


        //
        // ALTITUDE ARRIVAL AT CURRENT MACCREADY
        //

        _stprintf(text, TEXT("Mc %3.1f: "), (LIFTMODIFY * MACCREADY));
        if (wpt_altarriv > ALTDIFFLIMIT) {
            Units::FormatUserArrival(wpt_altarriv, buffer, 7);
            _tcscat(text, buffer);
        } else {
            _tcscat(text, TEXT("---"));
        }
        Surface.GetTextSize(text, &tsize);
        Surface.SelectObject(LK_BLACK_PEN);
        if (ValidWayPoint(overindex) && WayPointList[overindex].Reachable) {
            Surface.SelectObject(LKBrush_LightGreen);
        } else {
            if (!IsDithered()) {
                Surface.SelectObject(LKBrush_Orange);
            } else {
                Surface.SelectObject(LKBrush_White);
            }
        }
        x = line[0].x - tsize.cx - NIBLSCALE(5);
        if (bDrawRightSide) x = line[0].x + NIBLSCALE(5); // Show on right side if left not possible
        //  y += tsize.cy+NIBLSCALE(3);
        y = CalcHeightCoordinat(SAFETYALTITUDEARRIVAL / 10 + wpt_altitude + wpt_altarriv, &sDia);
        if (wpt_altarriv == wpt_altarriv_mc0)
            y -= tsize.cy / 2;

        if (!IsDithered()) {
            MapWindow::LKWriteBoxedText(Surface, rc, text, x, y, WTALIGN_LEFT, RGB_BLACK, RGB_RED);
        } else {
            MapWindow::LKWriteBoxedText(Surface, rc, text, x, y, WTALIGN_LEFT, RGB_BLACK, RGB_RED);
        }

        //
        // FINAL GLIDE MACCREADY
        //
  if(bConicalFinal) {
    double mc = (Sink * Slope + VOpt) / Slope;
    if( fabs(mc-MACCREADY) > 0.05 ) {
      _stprintf(text, TEXT("Mc %3.1f @%.0f%s"),(LIFTMODIFY*mc), SPEEDMODIFY*VOpt, (Units::GetHorizontalSpeedName()));

      x = BestReach.x - tsize.cx - NIBLSCALE(5);
      if (bDrawRightSide) {
          x = BestReach.x + NIBLSCALE(5);
      }
      Surface.GetTextSize(text, &tsize);
      int yn = BestReach.y;
      if(mc > MACCREADY) {
        if(yn > y + tsize.cy) {
          y=yn;
        } else {
          y+= (int)(1.2*tsize.cy);
        }
      } else {
        if(yn-tsize.cy > y ) {
          y=tsize.cy + y;
        } else {
          y-= (int)(1.2*tsize.cy);
        }
      }
      Surface.SelectObject(LKBrush_Nlight);
      MapWindow::LKWriteBoxedText(Surface, rc, text,  x, y, WTALIGN_LEFT, RGB_BLACK, RGB_BLACK);
     }
  } else {

        if (wpt_altarriv_mc0 > 0) {

            LKASSERT((wpt_dist + 1) != 0);

            double slope = 1;
            BUGSTOP_LKASSERT(ValidWayPoint(overindex));
            if (ValidWayPoint(overindex)) {
                slope =
                        (DerivedDrawInfo.NavAltitude + DerivedDrawInfo.EnergyHeight
                        - WayPointList[overindex].Altitude
                        - (IsSafetyAltitudeInUse(overindex) ? (SAFETYALTITUDEARRIVAL / 10) : 0))
                        / (wpt_dist + 1);
            }

            double mc_pirker = PirkerAnalysis(&DrawInfo, &DerivedDrawInfo,
                    overindex_brg,
                    slope);
            mc_pirker = max(0.0, mc_pirker);

            double mcspeed = 0;

            GlidePolar::MacCreadyAltitude(mc_pirker,
                    100.0, // dummy value
                    overindex_brg,
                    DerivedDrawInfo.WindSpeed,
                    DerivedDrawInfo.WindBearing,
                    0,
                    &mcspeed,
                    true,
                    NULL, 1.0e6, 1.0);


            _stprintf(text, TEXT("Mc %3.1f @%.0f%s"), (LIFTMODIFY * mc_pirker), SPEEDMODIFY*mcspeed,
                    (Units::GetHorizontalSpeedName()));

            x = line[0].x - tsize.cx - NIBLSCALE(5);
            if (bDrawRightSide) x = line[0].x + NIBLSCALE(5);
            Surface.GetTextSize(text, &tsize);
            int yn = CalcHeightCoordinat(SAFETYALTITUDEARRIVAL / 10 + wpt_altitude, &sDia); //+0.5*tsize.cy;
            if (yn > y + tsize.cy)
                y = yn;
            else
                y += (int) (1.2 * tsize.cy);

            Surface.SelectObject(LK_BLACK_PEN);
            if (!IsDithered()) {
                Surface.SelectObject(LKBrush_Nlight);
            } else {
                Surface.SelectObject(LKBrush_White);
            }

            MapWindow::LKWriteBoxedText(Surface, rc, text, x, y, WTALIGN_LEFT, RGB_BLACK, RGB_BLACK);

        }
  }

_after_additionals:



#ifdef SHOWLD
        // Working, but useless
        // FIX overindex check if you want to use it!
        if (altarriv > 0) {
            // Print GR
            _stprintf(text, TEXT("1/%i"), (int) fLD);
            Surface.GetTextSize(text, &tsize);
            x = CalcDistanceCoordinat(wpt_dist / 2, &sDia) - tsize.cx / 2;
            y = CalcHeightCoordinat((DerivedDrawInfo.NavAltitude + altarriv) / 2 + wpt_altitude, &sDia) + tsize.cy;
            if (WayPointList[overindex].Reachable) {
                SelectObject(hdc, LKBrush_LightGreen);
            } else {
                SelectObject(hdc, LKBrush_Orange);
            }
            MapWindow::LKWriteBoxedText(hdc, &rc, text, x, y, WTALIGN_CENTER, RGB_BLACK, RGB_BLACK);
        }
#endif

        // Print current AGL
        if (calc_altitudeagl - hmin > 0) {
            if (!IsDithered()) {
                Surface.SetTextColor(LIGHTBLUE_COL);
            } else {
                Surface.SetTextColor(RGB_BLACK);
            }
            Units::FormatUserAltitude(calc_altitudeagl, buffer, 7);
            LK_tcsncpy(text, MsgToken(1742), TBSIZE - _tcslen(buffer));
            _tcscat(text, buffer);
            Surface.GetTextSize(text, &tsize);
            x = CalcDistanceCoordinat(0, &sDia) - tsize.cx / 2;
            y = CalcHeightCoordinat((calc_terrainalt + calc_altitudeagl)*0.8, &sDia);
            if ((tsize.cy) < (CalcHeightCoordinat(calc_terrainalt, &sDia) - y)) {
                Surface.DrawText(x, y, text);
            }
        }
        Surface.SetBackgroundTransparent();
        Surface.SelectObject(hfOld);
    } // IM_NEXT_WP with valid overindex


    if (getsideviewpage == IM_HEADING)
        wpt_brg = 90.0;

    if (Current_Multimap_SizeY < SIZE4)
        RenderPlaneSideview(Surface, 0.0f, DerivedDrawInfo.NavAltitude, wpt_brg, &sDia);

    const auto hfOld2 = Surface.SelectObject(LK8InfoNormalFont);
    Surface.SetTextColor(Sideview_TextColor);

    Surface.SelectObject(hfOld2);
    Surface.SelectObject(hfOld);

    Surface.SetTextColor(GROUND_TEXT_COLOUR);

    if (bInvCol)
        if (sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
            Surface.SetTextColor(INV_GROUND_TEXT_COLOUR);

    Surface.SelectObject(hfOld/* Sender->GetFont()*/);

    if (fSplitFact > 0.0)
        sDia.rc = rct;

    hfOld = Surface.SelectObject(LK8InfoNormalFont/* Sender->GetFont()*/);

    /*
      switch(GetMMNorthUp(getsideviewpage))
      {
             case NORTHUP:
             default:
                     DrawCompass( hdc,  rct, 0);
         break;

             case TRACKUP:
                     DrawCompass( hdc,  rct, acb-90.0);
         break;
      }
     */

    DrawMultimap_SideTopSeparator(Surface, rct);

    /****************************************************************************************************
     * draw selection frame
     ****************************************************************************************************/
    if (bHeightScale)
        DrawSelectionFrame(Surface, rc);
#ifdef TOP_SELECTION_FRAME
    else
        DrawSelectionFrame(hdc, rci);
#endif


    Surface.SelectObject(hfOld/* Sender->GetFont()*/);
    zoom.SetLimitMapScale(true);
}
