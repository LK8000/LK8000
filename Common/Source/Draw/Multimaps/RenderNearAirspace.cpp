/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */
#include "externs.h"
#include "Airspace/LKAirspace.h"
#include "RGB.h"
#include "Sideview.h"
#include "Multimap.h"
#include "Dialogs.h"
#include "LKObjects.h"
#include "InputEvents.h"
#include "Dialogs/dlgMultiSelectList.h"


#define THICK_LINE 3 // NOTICE v6.1: we should not use absolute values..TODO

extern double fSplitFact;
extern double fOffset;
extern LKColor Sideview_TextColor;
extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];
extern int Sideview_iNoHandeldSpaces;
extern POINT startScreen;


extern double fZOOMScale[];
#define TBSIZE	80


TCHAR Sideview_szNearAS[TBSIZE + 1];

void MapWindow::RenderNearAirspace(LKSurface& Surface, const RECT& rci) {
    RECT rc = rci; /* rectangle for sideview */
    RECT rct = rc; /* rectangle for topview */

    rc.top = (int) ((double) (rci.bottom - rci.top) * fSplitFact);
    rct.bottom = rc.top;
    // Expose the topview rect size in use..
    Current_Multimap_TopRect = rct;

    auto hfOldFnt = Surface.SelectObject(LK8PanelUnitFont/* Sender->GetFont()*/);

    int *iSplit = &Multimap_SizeY[Get_Current_Multimap_Type()];

    static double fHeigtScaleFact = 1.0;


    double GPSlat, GPSlon, GPSalt, GPSbrg;
    double calc_terrainalt;
    double calc_altitudeagl;

    // double alt;
    TCHAR text[TBSIZE + 1];
    TCHAR buffer[TBSIZE + 1];

    //  bool bFound = false;
    DiagrammStruct sDia;
    bool bAS_Inside = false;
    int iAS_Bearing = 0;
    int iAS_HorDistance = 15000;
    int iABS_AS_HorDistance = 0;
    int iAS_VertDistance = 0;
    bool bValid;
    static bool bHeightScale = false;
    long wpt_brg = 0;
    POINT line[2];
    POINT TxYPt;
    POINT TxXPt;
    SIZE tsize;
    LKColor GREEN_COL = RGB_GREEN;
    LKColor BLUE_COL = RGB_BLUE;
    LKColor LIGHTBLUE_COL = RGB_LIGHTBLUE;
    BOOL bInvCol = true; //INVERTCOLORS

    unsigned short getsideviewpage = GetSideviewPage();
    LKASSERT(getsideviewpage < 3);

    if (bInvCol)
        Sideview_TextColor = INV_GROUND_TEXT_COLOUR;
    else
        Sideview_TextColor = RGB_WHITE;

    switch (LKevent) {
        case LKEVENT_NEWRUN:
            // CALLED ON ENTRY: when we select this page coming from another mapspace
            bHeightScale = false;
            // fZOOMScale[getsideviewpage] = 1.0;
            fHeigtScaleFact = 1.0;
            if (IsMultimapTopology()) ForceVisibilityScan = true;
            break;
        case LKEVENT_UP:
            // click on upper part of screen, excluding center
            if (bHeightScale)
                fHeigtScaleFact /= ZOOMFACTOR;
            else
                fZOOMScale[getsideviewpage] /= ZOOMFACTOR;

            if (IsMultimapTopology()) ForceVisibilityScan = true;
            break;

        case LKEVENT_DOWN:
            // click on lower part of screen,  excluding center
            if (bHeightScale)
                fHeigtScaleFact *= ZOOMFACTOR;
            else
                fZOOMScale[getsideviewpage] *= ZOOMFACTOR;
            if (IsMultimapTopology()) ForceVisibilityScan = true;
            break;

        case LKEVENT_LONGCLICK:
            if (Sideview_iNoHandeldSpaces) {
                bool bShow = false;
                for (int k = 0; k <= Sideview_iNoHandeldSpaces; k++) {
                    if (Sideview_pHandeled[k].psAS) {
                        if (PtInRect(&(Sideview_pHandeled[k].rc), startScreen)) {
                            DlgMultiSelect::AddItem(im_airspace{Sideview_pHandeled[k].psAS}, 0);
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
            }
            break;
        case LKEVENT_PAGEDOWN:
#ifdef OFFSET_SETP
            if (bHeightScale)
                fOffset += OFFSET_SETP;
            else
#endif
            {
                if (*iSplit == SIZE2) *iSplit = SIZE3;
                if (*iSplit == SIZE1) *iSplit = SIZE2;
                if (*iSplit == SIZE0) *iSplit = SIZE1;
            }
            break;

    }

    LKASSERT(((*iSplit == SIZE0) || (*iSplit == SIZE1) || (*iSplit == SIZE2) || (*iSplit == SIZE3) || (*iSplit == SIZE4)));

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
        rc.top = (long) ((double) (rci.bottom - rci.top) * fSplitFact);
        rct.bottom = rc.top;
    }


    if (bInvCol) {
        GREEN_COL = GREEN_COL.ChangeBrightness(0.6);
        BLUE_COL = BLUE_COL.ChangeBrightness(0.6);
        ;
        LIGHTBLUE_COL = LIGHTBLUE_COL.ChangeBrightness(0.4);
        ;
    }

    GPSlat = DrawInfo.Latitude;
    GPSlon = DrawInfo.Longitude;
    GPSalt = DrawInfo.Altitude;
    GPSbrg = DrawInfo.TrackBearing;
    calc_terrainalt = DerivedDrawInfo.TerrainAlt;
    calc_altitudeagl = DerivedDrawInfo.AltitudeAGL;
    GPSalt = DerivedDrawInfo.NavAltitude;

    bValid = false;
    iAS_HorDistance = 5000;
    iAS_Bearing = (int) GPSbrg;
    iAS_VertDistance = 0;

    CAirspaceBase near_airspace;
    WithLock(CAirspaceManager::Instance().MutexRef(), [&](){
        auto found = CAirspaceManager::Instance().GetNearestAirspaceForSideview();
        if (found) {
            near_airspace = CAirspaceManager::Instance().GetAirspaceCopy(found);
            bValid = near_airspace.GetDistanceInfo(bAS_Inside, iAS_HorDistance, iAS_Bearing, iAS_VertDistance);
        }
    });
    iABS_AS_HorDistance = abs(iAS_HorDistance);
    wpt_brg = (long) AngleLimit360(GPSbrg - iAS_Bearing + 90.0);


    // Variables from ASP system here contain the following informations:
    // fAS_HorDistance - always contains horizontal distance from the asp, negative if horizontally inside (This does not mean that we're inside vertically as well!)
    // fAS_Bearing - always contains bearing to the nearest horizontal point
    // bValid - true if bAS_Inside, iAS_HorDistance, iAS_Bearing, iAS_VertDistance contains valid informations
    //          this will be true if the asp border is close enough to be tracked by the warning system
    // bAS_Inside - current position is inside in the asp, calculated by the warning system
    // iAS_HorDistance - horizontal distance to the nearest horizontal border, negative if horizontally inside, calculated by the warning system
    // iAS_Bearing - bearing to the nearest horizontal border, calculated by the warning system
    // iAS_VertDistance - vertical distance to the nearest asp border, negative if the border is above us, positive if the border below us, calculated by the warning system
    // near_airspace.WarningLevel():
    //           awNone - no warning condition exists
    //           awYellow - current position is near to a warning position
    //           awRed - current posisiton is forbidden by asp system, we are in a warning position


    /*********************************************************************
     * calc the horizontal zoom
     *********************************************************************/
    sDia.fXMin = -5000.0;
    sDia.fXMax = 5000.0;
    /* even when invalid the horizontal distance is calculated correctly */


    if (bValid) {
        double fScaleDist = iABS_AS_HorDistance;

        sDia.fXMin = min(-2500.0, fScaleDist * 1.5);
        sDia.fXMax = max(2500.0, fScaleDist * 1.5);

#ifdef NEAR_AS_ZOOM_1000M
        if (((iABS_AS_HorDistance) < 900) && (bValid)) // 1km zoom
        {
            sDia.fXMin = min(-900.0, fScaleDist * 1.5);
            sDia.fXMax = max(900.0, fScaleDist * 1.5);

        }
#endif
#ifdef NEAR_AS_ZOOM_1000FT
        if ((abs(iABS_AS_HorDistance) < 333)) // 1000ft zoom
        {
            sDia.fXMin = min(-333.0, fScaleDist * 1.5);
            sDia.fXMax = max(333.0, fScaleDist * 1.5);
        }
#endif

    }


#define RND_FACT 10.0


    if ((sDia.fXMax * fZOOMScale[getsideviewpage]) > 100000)
        fZOOMScale[getsideviewpage] /= ZOOMFACTOR;

    if ((sDia.fXMax * fZOOMScale[getsideviewpage]) < 500) {
        fZOOMScale[getsideviewpage] *= ZOOMFACTOR;
    }

    double fOldZoomScale = -1;
    if (fZOOMScale[getsideviewpage] != fOldZoomScale) {
        fOldZoomScale = fZOOMScale[getsideviewpage];
        sDia.fXMax = sDia.fXMax * fZOOMScale[getsideviewpage];
        sDia.fXMin = -sDia.fXMax / 5;
    }


    /*********************************************************************
     * calc the vertical zoom
     *********************************************************************/
    sDia.fYMin = max(0.0, GPSalt - 2300);
    sDia.fYMax = max(MAXALTTODAY, GPSalt + 1000);

    if (bValid) {
        double fBottom = near_airspace.Base().Altitude;
        sDia.fYMin = min(fBottom * 0.8, sDia.fYMin);
        sDia.fYMin = max(0.0, sDia.fYMin);
        if (sDia.fYMin < 300) sDia.fYMin = 0;
        sDia.fYMax = max((fBottom * 1.2f), sDia.fYMax);

        if (abs(iAS_VertDistance) < 250) {
            sDia.fYMax = ((int) ((GPSalt + abs(iAS_VertDistance)) / 400) + 2) *400;
            sDia.fYMin = ((int) ((GPSalt - abs(iAS_VertDistance)) / 400) - 1) *400;
            if (sDia.fYMin - MIN_ALTITUDE < 0) sDia.fYMin = 0;
        }

#ifdef VERTICAL_ZOOM_50
        if (abs(iAS_VertDistance) < 50) {
            sDia.fYMax = ((int) ((GPSalt + abs(iAS_VertDistance)) / 100) + 2) *100;
            sDia.fYMin = ((int) ((GPSalt - abs(iAS_VertDistance)) / 100) - 1) *100;
            if (sDia.fYMin - MIN_ALTITUDE < 0) sDia.fYMin = 0;
        }
#endif
        sDia.fYMin = max((double) 0.0f, (double) sDia.fYMin);

#ifdef OFFSET_SETP
        if ((sDia.fYMax + fOffset) > MAX_ALTITUDE)
            fOffset -= OFFSET_SETP;
        if ((sDia.fYMin + fOffset) < 0.0)
            fOffset += OFFSET_SETP;

        sDia.fYMin += fOffset;
        sDia.fYMax += fOffset;
#endif
        //  if(fHeigtScaleFact * sDia.fYMax > MAX_ALTITUDE )
        //    fHeigtScaleFact /=ZOOMFACTOR;

        if (fHeigtScaleFact * sDia.fYMax < MIN_ALTITUDE)
            fHeigtScaleFact *= ZOOMFACTOR;
        sDia.fYMax *= fHeigtScaleFact;
    }


    /****************************************************************************************************
     * draw topview first
     ****************************************************************************************************/

    if (fSplitFact > 0.0) {
        sDia.rc = rct;
        sDia.rc.bottom -= 1;
        SharedTopView(Surface, &sDia, (double) iAS_Bearing, (double) wpt_brg);

    }

    /****************************************************************************************************
     * draw airspace and terrain elements
     ****************************************************************************************************/
    RECT rcc = rc; /* rc corrected      */
    if (sDia.fYMin < GC_SEA_LEVEL_TOLERANCE)
        rcc.bottom -= SV_BORDER_Y; /* scale witout sea  */
    sDia.rc = rcc;

    RenderAirspaceTerrain(Surface, GPSlat, GPSlon, iAS_Bearing, &sDia);

    const auto hfOld = Surface.SelectObject(LK8InfoNormalFont);
    if (bValid) {
        LKASSERT(_tcslen(near_airspace.Name()) < TBSIZE); // Diagnostic only in 3.1j, to be REMOVED
        LK_tcsncpy(Sideview_szNearAS, near_airspace.Name(), TBSIZE);
    } else {
        _stprintf(text, TEXT("%s"), MsgToken<1259>()); // LKTOKEN _@M1259_ "Too far, not calculated"
        Surface.GetTextSize(text, &tsize);
        TxYPt.x = (rc.right - rc.left - tsize.cx) / 2;
        TxYPt.y = (rc.bottom - rc.top) / 2;

        Surface.SetBackgroundTransparent();
        Surface.DrawText(TxYPt.x, TxYPt.y - 20, text);

        _stprintf(Sideview_szNearAS, TEXT("%s"), text);

    }
    Surface.SelectObject(hfOld);
    /****************************************************************************************************
     * draw airspace and terrain elements
     ****************************************************************************************************/

    /****************************************************************************************************
     * draw diagram
     ****************************************************************************************************/
    double xtick = 1.0;
    double fRange = fabs(sDia.fXMax - sDia.fXMin);
    if (fRange > 3.0 * 1000.0) xtick = 2.0;
    if (fRange > 15 * 1000.0) xtick = 5.0;
    if (fRange > 50.0 * 1000.0) xtick = 10.0;
    if (fRange > 100.0 * 1000.0) xtick = 20.0;
    if (fRange > 200.0 * 1000.0) xtick = 25.0;
    if (fRange > 250.0 * 1000.0) xtick = 50.0;
    if (fRange > 500.0 * 1000.0) xtick = 100.0;
    if (fRange > 1000.0 * 1000.0) xtick = 1000.0;

    if (bInvCol) {
        Surface.SelectObject(LK_BLACK_PEN);
        Surface.SelectObject(LKBrush_Black);
    } else {
        Surface.SelectObject(LK_WHITE_PEN);
        Surface.SelectObject(LKBrush_White);
    }

    LKColor txtCol = GROUND_TEXT_COLOUR;
    if (bInvCol)
        if (sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
            txtCol = INV_GROUND_TEXT_COLOUR;
    Surface.SetBackgroundTransparent();
    Surface.SetTextColor(txtCol);
    Surface.SelectObject(LK8PanelUnitFont);
    _stprintf(text, TEXT("%s"), Units::GetName(Units::GetDistanceUnit()));

    switch (GetMMNorthUp(getsideviewpage)) {
        case NORTHUP:
        default:
            DrawXGrid(Surface, rc, Units::FromDistance(xtick), xtick, 0, TEXT_ABOVE_LEFT, RGB_BLACK, &sDia, text);
            break;

        case TRACKUP:
            DrawXGrid(Surface, rci, Units::FromDistance(xtick), xtick, 0, TEXT_ABOVE_LEFT, RGB_BLACK, &sDia, text);
            break;
    }

    Surface.SetTextColor(Sideview_TextColor);

    double fHeight = (sDia.fYMax - sDia.fYMin);
    double ytick = 100.0;
    if (fHeight > 500.0) ytick = 200.0;
    if (fHeight > 1000.0) ytick = 500.0;
    if (fHeight > 2000.0) ytick = 1000.0;
    if (fHeight > 4000.0) ytick = 2000.0;
    if (fHeight > 8000.0) ytick = 4000.0;

    if (Units::GetAltitudeUnit() == unFeet)
        ytick = ytick * FEET_FACTOR;

    _stprintf(text, TEXT("%s"), Units::GetAltitudeName());
    if (sDia.fYMin < GC_SEA_LEVEL_TOLERANCE)
        rc.bottom -= SV_BORDER_Y; /* scale witout sea  */
    DrawYGrid(Surface, rc, Units::FromAltitude(ytick), ytick, 0, TEXT_UNDER_RIGHT, Sideview_TextColor, &sDia, text);


    Surface.SetBkColor(RGB_WHITE);

    if (!bInvCol)
        Surface.SetBackgroundOpaque();
    /****************************************************************************************************
     * draw AGL
     ****************************************************************************************************/
    if (calc_altitudeagl - sDia.fYMin > 500) {
        Surface.SetTextColor(LIGHTBLUE_COL);
        Units::FormatAltitude(calc_altitudeagl, buffer, 7);
        LK_tcsncpy(text, MsgToken<1742>(), TBSIZE - _tcslen(buffer)); // AGL:
        _tcscat(text, buffer);
        Surface.GetTextSize(text, &tsize);
        TxYPt.x = CalcDistanceCoordinat(0, &sDia) - tsize.cx / 2;
        TxYPt.y = CalcHeightCoordinat((calc_terrainalt + calc_altitudeagl)*0.8, &sDia);
        if ((tsize.cy) < (CalcHeightCoordinat(calc_terrainalt, &sDia) - TxYPt.y)) {
            Surface.DrawText(TxYPt.x + IBLSCALE(1), TxYPt.y, text);
        }
    }

    Surface.SetBackgroundTransparent();

    /****************************************************************************************************
     * Print current Elevation
     ****************************************************************************************************/
    Surface.SetTextColor(RGB_BLACK);
    int x, y;
    if ((calc_terrainalt - sDia.fYMin) > 0) {
        Units::FormatAltitude(calc_terrainalt, buffer, 7);
        LK_tcsncpy(text, MsgToken<1743>(), TBSIZE - _tcslen(buffer)); // ELV:
        _tcscat(text, buffer);
        Surface.GetTextSize(text, &tsize);
        x = CalcDistanceCoordinat(0, &sDia) - tsize.cx / 2;
        y = CalcHeightCoordinat(calc_terrainalt, &sDia);
        if ((ELV_FACT * tsize.cy) < abs(rc.bottom - y)) {
            Surface.DrawText(x, rc.bottom - (int) (ELV_FACT * tsize.cy), text);
        }
    }


    /****************************************************************************************************
     * draw side elements
     ****************************************************************************************************/
    Surface.SetTextColor(Sideview_TextColor);
    Surface.SetBackgroundOpaque();
    const auto hfOld2 = Surface.SelectObject(LK8InfoNormalFont);

    //  DrawTelescope      ( hdc, iAS_Bearing-90.0, rc.right  - NIBLSCALE(13),  rc.top   + NIBLSCALE(58));

    Surface.SelectObject(hfOld2);
    Surface.SetBackgroundTransparent();

    Surface.SelectObject(hfOld);
    Surface.SetTextColor(GROUND_TEXT_COLOUR);
    if (bInvCol)
        if (sDia.fYMin > GC_SEA_LEVEL_TOLERANCE)
            Surface.SetTextColor(INV_GROUND_TEXT_COLOUR);



    /****************************************************************************************************/
    /****************************************************************************************************/
    /****************************************************************************************************
     * draw distances to next airspace
     ****************************************************************************************************/
    /****************************************************************************************************/
    /****************************************************************************************************/
    if (bValid) {

        /****************************************************************************************************
         * draw horizontal distance to next airspace
         ****************************************************************************************************/
        Surface.SetTextColor(Sideview_TextColor);
        Surface.SetBackgroundOpaque();
        const auto hfOldU = Surface.SelectObject(LK8InfoNormalFont);
        // horizontal distance
        line[0].x = CalcDistanceCoordinat(0, &sDia);
        line[0].y = CalcHeightCoordinat(GPSalt, &sDia);
        line[1].x = CalcDistanceCoordinat(iABS_AS_HorDistance, &sDia);
        line[1].y = line[0].y;
        #ifdef NO_DASH_LINES
        Surface.DrawLine(PEN_SOLID, ScreenThinSize, line[0], line[1], Sideview_TextColor, rc);
        #else
        Surface.DrawDashLine(THICK_LINE, line[0], line[1], Sideview_TextColor, rc);
        #endif
        if (iAS_HorDistance < 0) {
            line[0].y = CalcHeightCoordinat(GPSalt - (double) iAS_VertDistance, &sDia);
            line[1].y = line[0].y;

            #ifdef NO_DASH_LINES
            Surface.DrawLine(PEN_SOLID, ScreenThinSize, line[0], line[1], Sideview_TextColor, rc);
            #else
            Surface.DrawDashLine(THICK_LINE, line[0], line[1], Sideview_TextColor, rc);
            #endif
        }

        bool bLeft = false;
        if (line[0].x < line[1].x)
            bLeft = false;
        else
            bLeft = true;

        lk::strcpy(text, _T(" "));
        Units::FormatDistance(iABS_AS_HorDistance, text + 1, 7);
        Surface.GetTextSize(text, &tsize); 

        if ((GPSalt - sDia.fYMin /*-calc_terrainalt */) < 300)
            TxXPt.y = CalcHeightCoordinat(GPSalt, &sDia) - tsize.cy;
        else
            TxXPt.y = CalcHeightCoordinat(GPSalt, &sDia) + NIBLSCALE(3);


        if (tsize.cx > (line[1].x - line[0].x))
            TxXPt.x = CalcDistanceCoordinat(iABS_AS_HorDistance, &sDia) - tsize.cx - NIBLSCALE(3);
        else
            TxXPt.x = CalcDistanceCoordinat(iABS_AS_HorDistance / 2.0, &sDia) - tsize.cx / 2;
        Surface.DrawText(TxXPt.x, TxXPt.y, text);



        /****************************************************************************************************
         * draw vertical distance to next airspace
         ****************************************************************************************************/
        line[0].x = CalcDistanceCoordinat(iABS_AS_HorDistance, &sDia);
        line[0].y = CalcHeightCoordinat(GPSalt, &sDia);
        line[1].x = line[0].x;
        line[1].y = CalcHeightCoordinat(GPSalt - (double) iAS_VertDistance, &sDia);

        #ifdef NO_DASH_LINES
        Surface.DrawLine(PEN_SOLID, ScreenThinSize, line[0], line[1], Sideview_TextColor, rc);
        #else
        Surface.DrawDashLine(THICK_LINE, line[0], line[1], Sideview_TextColor, rc);
        #endif

        lk::strcpy(text, _T(" "));
        Units::FormatAltitude((double) abs(iAS_VertDistance), text + 1, 7);
        Surface.GetTextSize(text, &tsize);

        if (bLeft)
            TxYPt.x = CalcDistanceCoordinat(iABS_AS_HorDistance, &sDia) - tsize.cx - NIBLSCALE(3);
        else
            TxYPt.x = CalcDistanceCoordinat(iABS_AS_HorDistance, &sDia) + NIBLSCALE(5);
        if (abs(line[0].y - line[1].y) > tsize.cy)
            TxYPt.y = CalcHeightCoordinat(GPSalt - (double) iAS_VertDistance / 2.0, &sDia) - tsize.cy / 2;
        else
            TxYPt.y = min(line[0].y, line[1].y) - tsize.cy;
        Surface.DrawText(TxYPt.x, TxYPt.y, text);
        Surface.SelectObject(hfOldU);
    }

    /****************************************************************************************************
     * draw plane sideview at least
     ****************************************************************************************************/
    RenderPlaneSideview(Surface, 0.0, GPSalt, wpt_brg, &sDia);

    hfOldFnt = Surface.SelectObject(LK8InfoNormalFont/* Sender->GetFont()*/);

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
    Surface.SelectObject(hfOldFnt/* Sender->GetFont()*/);
    Surface.SetBackgroundTransparent();
    Surface.SelectObject(hfOldFnt/* Sender->GetFont()*/);
}
