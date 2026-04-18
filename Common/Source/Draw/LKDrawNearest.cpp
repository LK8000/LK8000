/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: LKDrawNearest.cpp,v 1.2 2010/12/24 12:45:49 root Exp root $
 */

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "InputEvents.h"
#include "ScreenGeometry.h"
#include "Asset.hpp"
#include "Util/TruncateString.hpp"
#include "Calc/ThermalHistory.h"
#include "utils/printf.h"

extern bool CheckLandableReachableTerrainNew(NMEA_INFO *Basic, DERIVED_INFO *Calculated, double LegToGo, double LegBearing);

namespace {
    
bool ValidAirspace(int i) {
    if (i < 0 || i > MAXNEARAIRSPACES) return false;
    return LKAirspaces[i].Valid;
}

LKColor LineColor() {
  return IsDithered()
            ? (INVERTCOLORS ? RGB_WHITE : RGB_BLACK)
            : (INVERTCOLORS ? RGB_GREEN : RGB_DARKGREEN);
}

} // namespace

// shortcuts
#define MSMCOMMONS   (curmapspace==MSM_LANDABLE||curmapspace==MSM_AIRPORTS||curmapspace==MSM_NEARTPS|| \
                      curmapspace==MSM_COMMON||curmapspace==MSM_RECENT)
#define MSMAIRSPACES (curmapspace==MSM_AIRSPACES)
#define MSMTHERMALS  (curmapspace==MSM_THERMALS)
#define MSMTRAFFIC  (curmapspace==MSM_TRAFFIC)

void MapWindow::DrawNearest(LKSurface& Surface, const RECT& rc) {

    static TCHAR Buffer1[MAXNEAREST][MAXNUMPAGES][24];
    static TCHAR Buffer2[MAXNEAREST][MAXNUMPAGES][12];
    static TCHAR Buffer3[MAXNEAREST][MAXNUMPAGES][12];
    static TCHAR Buffer4[MAXNEAREST][MAXNUMPAGES][12];
    static TCHAR Buffer5[MAXNEAREST][MAXNUMPAGES][12];

    static unsigned short Column0[MSM_TOP + 1], Column1[MSM_TOP + 1], Column2[MSM_TOP + 1];
    static unsigned short Column3[MSM_TOP + 1], Column4[MSM_TOP + 1], Column5[MSM_TOP + 1];
    static unsigned short hColumn2, hColumn3, hColumn4, hColumn5;
    static RECT s_sortBox[6];
    static POINT p1, p2;
    static unsigned short s_rawspace, s_maxnlname[MSM_TOP + 1], lincr, left, right, bottom;
    static FontReference bigFont, bigItalicFont;
    static bool usetwolines = 0;
    static bool compact_headers = 0;
    static uint16_t numraws = 0;

    // Vertical and horizontal spaces
#define INTERRAW	1
#define HEADRAW	NIBLSCALE(6)

    RECT invsel;
    TCHAR Buffer[LKSIZEBUFFERLARGE];
    TCHAR text[30];
    double value;
    LKColor rcolor;
    short curmapspace = MapSpaceMode;


    if (DoInit[MDI_DRAWNEAREST]) {

        SIZE K1TextSize[MSM_TOP + 1], K2TextSize[MSM_TOP + 1], K3TextSize[MSM_TOP + 1], K4TextSize[MSM_TOP + 1];
        SIZE InfoTextSize, InfoNumberSize;
        SIZE phdrTextSize;
        unsigned short max_name[MSM_TOP + 1], min_name[MSM_TOP + 1];
        unsigned short ratio1_threshold[MSM_TOP + 1], ratio2_threshold[MSM_TOP + 1];

        // Init statics
        for (int n = 0; n <= MSM_TOP; n++) {
            max_name[n] = 0, min_name[n] = 0;
            ratio1_threshold[n] = 0, ratio2_threshold[n] = 0;
            s_maxnlname[n] = 0;
            K1TextSize[n] = {0, 0};
            K2TextSize[n] = {0, 0};
            K3TextSize[n] = {0, 0};
            K4TextSize[n] = {0, 0};
            Column0[n] = 0;
            Column1[n] = 0;
            Column2[n] = 0;
            Column3[n] = 0;
            Column4[n] = 0;
            Column5[n] = 0;
        }

        usetwolines = (ScreenLandscape ? false : UseTwoLines);
        compact_headers = (ScreenGeometry == SCREEN_GEOMETRY_43) || !ScreenLandscape;

        bigFont = (usetwolines ? LK8InfoBig2LFont : LK8InfoBigFont);

        bigItalicFont = (usetwolines ? LK8InfoBigItalic2LFont : LK8InfoBigItalicFont);

        // Set screen borders to avoid writing on extreme pixels
        left = rc.left + NIBLSCALE(4);
        right = rc.right - NIBLSCALE(4);
        bottom = rc.bottom - BottomSize;

        Surface.SelectObject(bigFont);

        // We want an average size of an alphabet letter and number
        Surface.GetTextSize(_T("ALPHAROMEO"), &InfoTextSize);
        InfoTextSize.cx /= 10;

        Surface.GetTextSize(_T("0123456789"), &InfoNumberSize);
        InfoNumberSize.cx /= 10;

        //
        // SPECIAL TYPES, FIRST
        //

#define LKASP_TYPE_LEN  4
        // ASP TYPE
        Surface.GetTextSize(_T("CTRA"), &K1TextSize[MSM_AIRSPACES]);

        // Flags can be SFE, three chars
        Surface.GetTextSize(_T("SFE"), &K4TextSize[MSM_AIRSPACES]);

        // Thermal average
        Surface.GetTextSize(_T("+55.5"), &K3TextSize[MSM_THERMALS]);
        K3TextSize[MSM_TRAFFIC] = K3TextSize[MSM_THERMALS];

        //
        // COMMON TYPES (Distance, Bearing, Altitude Diff,  Efficiency)
        //

        // DISTANCE
        //
        lk::strcpy(Buffer, _T("555.5"));
        if (usetwolines) {
            _tcscat(Buffer, _T(" "));
            _tcscat(Buffer, Units::GetDistanceName());
        }
        // #warning "to much call with same input"
        // We do like this for better code understanding
        Surface.GetTextSize(Buffer, &K1TextSize[MSM_LANDABLE]);
        Surface.GetTextSize(Buffer, &K1TextSize[MSM_AIRPORTS]);
        Surface.GetTextSize(Buffer, &K1TextSize[MSM_NEARTPS]);
        Surface.GetTextSize(Buffer, &K1TextSize[MSM_COMMON]);
        Surface.GetTextSize(Buffer, &K1TextSize[MSM_RECENT]);
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_AIRSPACES]);
        Surface.GetTextSize(Buffer, &K1TextSize[MSM_THERMALS]);
        Surface.GetTextSize(Buffer, &K1TextSize[MSM_TRAFFIC]);


        // BEARING
        //
        lk::strcpy(Buffer, _T("<325"));
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_LANDABLE]);
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_AIRPORTS]);
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_NEARTPS]);
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_COMMON]);
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_RECENT]);
        Surface.GetTextSize(Buffer, &K3TextSize[MSM_AIRSPACES]);
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_THERMALS]);
        Surface.GetTextSize(Buffer, &K2TextSize[MSM_TRAFFIC]);


        // REQUIRED EFFICIENCY (max 200)
        //
        lk::strcpy(Buffer, _T("255"));
        Surface.GetTextSize(Buffer, &K3TextSize[MSM_LANDABLE]);
        Surface.GetTextSize(Buffer, &K3TextSize[MSM_AIRPORTS]);
        Surface.GetTextSize(Buffer, &K3TextSize[MSM_NEARTPS]);
        Surface.GetTextSize(Buffer, &K3TextSize[MSM_COMMON]);
        Surface.GetTextSize(Buffer, &K3TextSize[MSM_RECENT]);


        // REQUIRED ALTITUDE DIFFERENCE
        //
        if (Units::GetAltitudeUnit() == unFeet) {
          lk::strcpy(Buffer, _T("-99999"));
        }
        else {
          lk::strcpy(Buffer, _T("-9999"));
        }
        if (usetwolines) {
          _tcscat(Buffer, _T(" "));
          _tcscat(Buffer, Units::GetAltitudeName());
        }
        Surface.GetTextSize(Buffer, &K4TextSize[MSM_LANDABLE]);
        Surface.GetTextSize(Buffer, &K4TextSize[MSM_AIRPORTS]);
        Surface.GetTextSize(Buffer, &K4TextSize[MSM_NEARTPS]);
        Surface.GetTextSize(Buffer, &K4TextSize[MSM_COMMON]);
        Surface.GetTextSize(Buffer, &K4TextSize[MSM_RECENT]);
        Surface.GetTextSize(Buffer, &K4TextSize[MSM_THERMALS]);
        Surface.GetTextSize(Buffer, &K4TextSize[MSM_TRAFFIC]); // we dont use +-


        Surface.SelectObject(LK8PanelMediumFont);
        Surface.GetTextSize(_T("4.4"), &phdrTextSize);

        // Col0 is where APTS 1/3 can be written, after ModeIndex:Curtype
        Column0[MSM_LANDABLE] = rc.left + phdrTextSize.cx + LEFTLIMITER + NIBLSCALE(5);
        Column1[MSM_LANDABLE] = left;
        Column5[MSM_LANDABLE] = right;

        // Copy values - identical for 0,1,5
        Column0[MSM_AIRPORTS] = Column0[MSM_LANDABLE];
        Column0[MSM_NEARTPS] = Column0[MSM_LANDABLE];
        Column0[MSM_COMMON] = Column0[MSM_LANDABLE];
        Column0[MSM_RECENT] = Column0[MSM_LANDABLE];
        Column0[MSM_AIRSPACES] = Column0[MSM_LANDABLE];
        Column0[MSM_THERMALS] = Column0[MSM_LANDABLE];
        Column0[MSM_TRAFFIC] = Column0[MSM_LANDABLE];

        Column1[MSM_AIRPORTS] = Column1[MSM_LANDABLE];
        Column1[MSM_NEARTPS] = Column1[MSM_LANDABLE];
        Column1[MSM_COMMON] = Column1[MSM_LANDABLE];
        Column1[MSM_RECENT] = Column1[MSM_LANDABLE];
        Column1[MSM_AIRSPACES] = Column1[MSM_LANDABLE];
        Column1[MSM_THERMALS] = Column1[MSM_LANDABLE];
        Column1[MSM_TRAFFIC] = Column1[MSM_LANDABLE];

        Column5[MSM_AIRPORTS] = Column5[MSM_LANDABLE];
        Column5[MSM_NEARTPS] = Column5[MSM_LANDABLE];
        Column5[MSM_COMMON] = Column5[MSM_LANDABLE];
        Column5[MSM_RECENT] = Column5[MSM_LANDABLE];
        Column5[MSM_AIRSPACES] = Column5[MSM_LANDABLE];
        Column5[MSM_THERMALS] = Column5[MSM_LANDABLE];
        Column5[MSM_TRAFFIC] = Column5[MSM_LANDABLE];
        //

        max_name[MSM_LANDABLE] = MAXNLNAME;
        max_name[MSM_AIRPORTS] = MAXNLNAME;
        max_name[MSM_NEARTPS] = MAXNLNAME;
        max_name[MSM_COMMON] = MAXNLNAME;
        max_name[MSM_RECENT] = MAXNLNAME;
        max_name[MSM_AIRSPACES] = 18;
        max_name[MSM_THERMALS] = 8; // 7 is enough
        max_name[MSM_TRAFFIC] = MAXFLARMNAME;

        // So, how long we want the name, minimally? The remaining space will be
        // equally divided for spacing the other items on the row, and increasing
        // as well the name size at the same time.
#define MINNAME_COMMON_CONDITION (ScreenLandscape?8:4)
        min_name[MSM_LANDABLE] = MINNAME_COMMON_CONDITION;
        min_name[MSM_AIRPORTS] = MINNAME_COMMON_CONDITION;
        min_name[MSM_NEARTPS] = MINNAME_COMMON_CONDITION;
        min_name[MSM_COMMON] = MINNAME_COMMON_CONDITION;
        min_name[MSM_RECENT] = MINNAME_COMMON_CONDITION;
        min_name[MSM_THERMALS] = (ScreenLandscape ? 6 : 2);
        min_name[MSM_AIRSPACES] = (ScreenLandscape ? 15 : 6);
        min_name[MSM_TRAFFIC] = (ScreenLandscape ? 6 : 2);

        ratio1_threshold[MSM_LANDABLE] = 10;
        ratio2_threshold[MSM_LANDABLE] = 11;
        ratio1_threshold[MSM_AIRPORTS] = ratio1_threshold[MSM_LANDABLE];
        ratio2_threshold[MSM_AIRPORTS] = ratio2_threshold[MSM_LANDABLE];
        ratio1_threshold[MSM_NEARTPS] = ratio1_threshold[MSM_LANDABLE];
        ratio2_threshold[MSM_NEARTPS] = ratio2_threshold[MSM_LANDABLE];
        ratio1_threshold[MSM_COMMON] = ratio1_threshold[MSM_LANDABLE];
        ratio2_threshold[MSM_COMMON] = ratio2_threshold[MSM_LANDABLE];
        ratio1_threshold[MSM_RECENT] = ratio1_threshold[MSM_LANDABLE];
        ratio2_threshold[MSM_RECENT] = ratio2_threshold[MSM_LANDABLE];

        ratio1_threshold[MSM_AIRSPACES] = 15;
        ratio2_threshold[MSM_AIRSPACES] = 18;
        ratio1_threshold[MSM_THERMALS] = 8; // unused really
        ratio2_threshold[MSM_THERMALS] = 9;
        ratio1_threshold[MSM_TRAFFIC] = 8; // unused really
        ratio2_threshold[MSM_TRAFFIC] = 9;


        // CALCULATE THE BEST POSSIBLE SIZE FOR THE ITEM NAME
        //
        if (usetwolines) {
            // TODO: CALCULATE AVAILABLE SPACE AND SIZE CORRECTLY
            for (int n = 0; n < MSM_TOP; n++) {
                if (!max_name[n]) continue;
                Column4[n] = right;
                Column3[n] = Column4[n] - K4TextSize[n].cx - InfoNumberSize.cx;
                Column2[n] = Column3[n] - K3TextSize[n].cx - InfoNumberSize.cx;
                int spare = (Column2[n] - K2TextSize[n].cx - InfoNumberSize.cx - left) / 3;
                if (spare > 0) {
                    Column4[n] -= spare;
                    Column3[n] -= (spare * 2);
                    Column2[n] -= (spare * 3);
                }
                s_maxnlname[n] = max_name[n];
            }
            lincr = 2;
        } else {
            for (int n = 0; n < MSM_TOP; n++) {
                if (!max_name[n]) continue;
                Column5[n] = right;
                Column4[n] = Column5[n] - K4TextSize[n].cx - InfoNumberSize.cx;
                Column3[n] = Column4[n] - K3TextSize[n].cx - InfoNumberSize.cx;
                Column2[n] = Column3[n] - K2TextSize[n].cx - InfoNumberSize.cx;

                int spare = (Column2[n] - K1TextSize[n].cx - left - InfoNumberSize.cx);
                if (spare < 1) {
                    TestLog(_T("... NEAREST: mapspace=%d spare invalid, =%d"), n, spare);
                }
                if (spare < 0) spare = 0; // no problem, the user chose a font too big
                // the available space for printing the name after minimally spacing the other items.
                // Result will be perfectly readable, but spacing between items is much welcome.
                int available_size = ceil(spare / (float) InfoTextSize.cx);

                unsigned short size_name = 0; // the calculated size for name, in progress

                // if the available space is not enough for min_name, we take it all.
                if (available_size <= min_name[n]) {
                    //  0 or 1 chars do not make a name. We force 2 and good luck with visibility.
                    if (available_size < 2) {
                        TestLog(_T("... NEAREST: mapspace=%d not enough space=%d for item names"), n, available_size);
                        size_name = 2;
                    } else {
                        size_name = available_size;
                    }
                } else {
                    available_size -= min_name[n];
                    size_name = min_name[n];
                    short ratio = 2;
                    while (available_size > ratio) {
                        available_size -= (ratio + 1);
                        size_name++;
                        if (size_name == ratio1_threshold[n]) ratio = 3;
                        if (size_name == ratio2_threshold[n]) ratio = 4;
                    }
                }
                // Check that we dont go beyond a max
                s_maxnlname[n] = size_name > max_name[n] ? max_name[n] : size_name;
                int advance = spare - (size_name * InfoTextSize.cx);
                if (advance > 3) {
                    float fadv = advance / 4.0;
                    Column4[n] -= ceil(fadv);
                    Column3[n] -= ceil(fadv * 2);
                    Column2[n] -= ceil(fadv * 3);
                }
            } // for all mapspaces
            lincr = 1;
        }

        // Vertical alignement

        Surface.SelectObject(LK8InfoNearestFont);
        Surface.GetTextSize(_T("M"), &phdrTextSize);

        TopSize = rc.top + HEADRAW * 2 + phdrTextSize.cy;
        p1.x = rc.left;
        p1.y = TopSize;
        p2.x = rc.right;
        p2.y = p1.y;
        if (!ScreenLandscape) {
            TopSize += HEADRAW;
            numraws = (bottom - TopSize) / (InfoTextSize.cy + (INTERRAW * 2));
        } else {
            TopSize += HEADRAW / 2;
            numraws = (bottom - TopSize) / (InfoTextSize.cy + INTERRAW);
        }
        if (usetwolines) {
            ;
            numraws /= 2;
            numraws *= 2;
        } // make it even number
        if (numraws > MAXNEAREST) numraws = MAXNEAREST;

        s_rawspace = (InfoTextSize.cy + INTERRAW);

        // center in available vertical space
        int cs = (bottom - TopSize) - (numraws * (InfoTextSize.cy + INTERRAW) + INTERRAW);

        if ((cs > (numraws - 1) && (numraws > 1))) {
            s_rawspace += cs / (numraws - 1);
            s_rawspace -= NIBLSCALE(1); // adjust rounding errors
            TopSize += (cs - ((cs / (numraws - 1))*(numraws - 1))) / 2 - 1;
        } else {
            TopSize += cs / 2 - 1;
        }

#define HMARGIN NIBLSCALE(2) // left and right margins for header selection

        //
        // HEADER SORTBOXES, used by VirtualKeys
        //
        Surface.SelectObject(LK8InfoNearestFont);
        if (ScreenLandscape)
            Surface.GetTextSize(_T("MMMM 3/3"), &phdrTextSize);
        else
            Surface.GetTextSize(_T("MMM 3/3"), &phdrTextSize);

        s_sortBox[0].left = Column0[MSM_LANDABLE] - NIBLSCALE(1);
        s_sortBox[0].right = Column0[MSM_LANDABLE] + phdrTextSize.cx;
        s_sortBox[0].top = rc.top + 2;
        s_sortBox[0].bottom = p1.y - NIBLSCALE(1);
        ;
        SortBoxX[MSM_LANDABLE][0] = s_sortBox[0].right;
        SortBoxX[MSM_AIRPORTS][0] = s_sortBox[0].right;
        SortBoxX[MSM_NEARTPS][0] = s_sortBox[0].right;
        SortBoxX[MSM_COMMON][0] = s_sortBox[0].right;
        SortBoxX[MSM_RECENT][0] = s_sortBox[0].right;
        SortBoxX[MSM_AIRSPACES][0] = s_sortBox[0].right;
        SortBoxX[MSM_THERMALS][0] = s_sortBox[0].right;
        SortBoxX[MSM_TRAFFIC][0] = s_sortBox[0].right;

        int headerspacing = (right - s_sortBox[0].right) / 4; // used only for monoline portrait

        s_sortBox[1].left = s_sortBox[0].right + HMARGIN;
        s_sortBox[1].right = s_sortBox[0].right + headerspacing - HMARGIN;
        s_sortBox[1].top = rc.top + 2;
        s_sortBox[1].bottom = p1.y - NIBLSCALE(1);
        SortBoxX[MSM_LANDABLE][1] = s_sortBox[1].right;
        SortBoxX[MSM_AIRPORTS][1] = s_sortBox[1].right;
        SortBoxX[MSM_NEARTPS][1] = s_sortBox[1].right;
        SortBoxX[MSM_COMMON][1] = s_sortBox[1].right;
        SortBoxX[MSM_RECENT][1] = s_sortBox[1].right;
        SortBoxX[MSM_AIRSPACES][1] = s_sortBox[1].right;
        SortBoxX[MSM_THERMALS][1] = s_sortBox[1].right;
        SortBoxX[MSM_TRAFFIC][1] = s_sortBox[1].right;

        s_sortBox[2].left = s_sortBox[1].right + HMARGIN;
        s_sortBox[2].right = s_sortBox[1].right + headerspacing;
        s_sortBox[2].top = rc.top + 2;
        s_sortBox[2].bottom = p1.y - NIBLSCALE(1);
        SortBoxX[MSM_LANDABLE][2] = s_sortBox[2].right;
        SortBoxX[MSM_AIRPORTS][2] = s_sortBox[2].right;
        SortBoxX[MSM_NEARTPS][2] = s_sortBox[2].right;
        SortBoxX[MSM_COMMON][2] = s_sortBox[2].right;
        SortBoxX[MSM_RECENT][2] = s_sortBox[2].right;
        SortBoxX[MSM_AIRSPACES][2] = s_sortBox[2].right;
        SortBoxX[MSM_THERMALS][2] = s_sortBox[2].right;
        SortBoxX[MSM_TRAFFIC][2] = s_sortBox[2].right;

        s_sortBox[3].left = s_sortBox[2].right + HMARGIN;
        s_sortBox[3].right = s_sortBox[2].right + headerspacing;
        s_sortBox[3].top = rc.top + 2;
        s_sortBox[3].bottom = p1.y - NIBLSCALE(1);
        SortBoxX[MSM_LANDABLE][3] = s_sortBox[3].right;
        SortBoxX[MSM_AIRPORTS][3] = s_sortBox[3].right;
        SortBoxX[MSM_NEARTPS][3] = s_sortBox[3].right;
        SortBoxX[MSM_COMMON][3] = s_sortBox[3].right;
        SortBoxX[MSM_RECENT][3] = s_sortBox[3].right;
        SortBoxX[MSM_AIRSPACES][3] = s_sortBox[3].right;
        SortBoxX[MSM_THERMALS][3] = s_sortBox[3].right;
        SortBoxX[MSM_TRAFFIC][3] = s_sortBox[3].right;

        s_sortBox[4].left = s_sortBox[3].right + HMARGIN;
        s_sortBox[4].right = right;
        s_sortBox[4].top = rc.top + 2;
        s_sortBox[4].bottom = p1.y - NIBLSCALE(1);
        SortBoxX[MSM_LANDABLE][4] = s_sortBox[4].right;
        SortBoxX[MSM_AIRPORTS][4] = s_sortBox[4].right;
        SortBoxX[MSM_NEARTPS][4] = s_sortBox[4].right;
        SortBoxX[MSM_COMMON][4] = s_sortBox[4].right;
        SortBoxX[MSM_RECENT][4] = s_sortBox[4].right;
        SortBoxX[MSM_AIRSPACES][4] = s_sortBox[4].right;
        SortBoxX[MSM_THERMALS][4] = s_sortBox[4].right;
        SortBoxX[MSM_TRAFFIC][4] = s_sortBox[4].right;

        SortBoxY[MSM_LANDABLE] = p1.y;
        SortBoxY[MSM_AIRPORTS] = p1.y;
        SortBoxY[MSM_NEARTPS] = p1.y;
        SortBoxY[MSM_COMMON] = p1.y;
        SortBoxY[MSM_RECENT] = p1.y;
        SortBoxY[MSM_AIRSPACES] = p1.y;
        SortBoxY[MSM_THERMALS] = p1.y;
        SortBoxY[MSM_TRAFFIC] = p1.y;

        Numpages = roundupdivision(MAXNEAREST*lincr, numraws);

        if (Numpages > MAXNUMPAGES) Numpages = MAXNUMPAGES;
        else if (Numpages < 1) Numpages = 1;

        SelectedRaw[MSM_AIRPORTS] = 0;
        SelectedPage[MSM_AIRPORTS] = 0;
        SelectedRaw[MSM_LANDABLE] = 0;
        SelectedPage[MSM_LANDABLE] = 0;
        SelectedRaw[MSM_NEARTPS] = 0;
        SelectedPage[MSM_NEARTPS] = 0;
        SelectedRaw[MSM_COMMON] = 0;
        SelectedPage[MSM_COMMON] = 0;
        SelectedRaw[MSM_RECENT] = 0;
        SelectedPage[MSM_RECENT] = 0;
        SelectedRaw[MSM_AIRSPACES] = 0;
        SelectedPage[MSM_AIRSPACES] = 0;
        SelectedRaw[MSM_THERMALS] = 0;
        SelectedPage[MSM_THERMALS] = 0;
        SelectedRaw[MSM_TRAFFIC] = 0;
        SelectedPage[MSM_TRAFFIC] = 0;

        hColumn2 = s_sortBox[1].right - NIBLSCALE(2);
        hColumn3 = s_sortBox[2].right - NIBLSCALE(2);
        hColumn4 = s_sortBox[3].right - NIBLSCALE(2);
        hColumn5 = s_sortBox[4].right - NIBLSCALE(2);

        DoInit[MDI_DRAWNEAREST] = false;
        return;
    } // doinit

    int* pSortedNumber = nullptr;
    int* pSortedIndex = nullptr;
    const TCHAR* headertoken[5];
    double *pLastDoNearest;
    bool *pNearestDataReady;

    switch (curmapspace) {
        case MSM_LANDABLE:
            pSortedIndex = SortedLandableIndex;
            pSortedNumber = &SortedNumber;
            pLastDoNearest = &LastDoNearest;
            pNearestDataReady = &NearestDataReady;
            headertoken[0] = ScreenLandscape ? MsgToken<1312>() : MsgToken<1311>(); // LKTOKEN _@M1311_ "LND" _@M1312_ "LNDB"
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>(); // LKTOKEN _@M1300_ "Dist"  _@M1304_ "Distance"
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>(); // LKTOKEN _@M1301_ "Dir" LKTOKEN _@M1305_ "Direction"
            headertoken[3] = compact_headers ? MsgToken<1302>() : MsgToken<1306>(); // LKTOKEN _@M1302_ "rEff"  LKTOKEN _@M1306_ "ReqEff"
            // In v5 using 1308 for landscape compact_headers. Now simplified because compact_headers is also for portrait
            // LKTOKEN _@M1303_ "AltA" LKTOKEN _@M1307_ "AltArr" LKTOKEN _@M1308_ "Arriv"
            headertoken[4] = compact_headers ? MsgToken<1303>() : MsgToken<1307>();
            break;
        case MSM_AIRPORTS:
            pSortedIndex = SortedAirportIndex;
            pSortedNumber = &SortedNumber;
            pLastDoNearest = &LastDoNearest;
            pNearestDataReady = &NearestDataReady;
            headertoken[0] = ScreenLandscape ? MsgToken<1314>() : MsgToken<1313>(); // LKTOKEN _@M1313_ "APT"  _@M1314_ "APTS"
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>();
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>();
            headertoken[3] = compact_headers ? MsgToken<1302>() : MsgToken<1306>();
            headertoken[4] = compact_headers ? MsgToken<1303>() : MsgToken<1307>();
            break;
        case MSM_NEARTPS:
            pSortedIndex = SortedTurnpointIndex;
            pSortedNumber = &SortedNumber;
            pLastDoNearest = &LastDoNearest;
            pNearestDataReady = &NearestDataReady;
            headertoken[0] = MsgToken<1315>(); // LKTOKEN _@M1315_ "TPS"
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>();
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>();
            headertoken[3] = compact_headers ? MsgToken<1302>() : MsgToken<1306>();
            headertoken[4] = compact_headers ? MsgToken<1303>() : MsgToken<1307>();
            break;
        case MSM_COMMON:
            pSortedIndex = CommonIndex;
            pSortedNumber = &CommonNumber;
            pLastDoNearest = &LastDoCommon;
            pNearestDataReady = &CommonDataReady;
            headertoken[0] = MsgToken<1309>(); // LKTOKEN _@M1309_ "COMN"
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>();
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>();
            headertoken[3] = compact_headers ? MsgToken<1302>() : MsgToken<1306>();
            headertoken[4] = compact_headers ? MsgToken<1303>() : MsgToken<1307>();
            break;
        case MSM_RECENT:
            pSortedIndex = RecentIndex;
            pSortedNumber = &RecentNumber;
            pLastDoNearest = &LastDoCommon;
            pNearestDataReady = &RecentDataReady;
            headertoken[0] = MsgToken<1310>(); // LKTOKEN _@M1310_ "HIST"
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>();
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>();
            headertoken[3] = compact_headers ? MsgToken<1302>() : MsgToken<1306>();
            headertoken[4] = compact_headers ? MsgToken<1303>() : MsgToken<1307>();
            break;
        case MSM_AIRSPACES:
            pSortedIndex = nullptr;
            pSortedNumber = &LKNumAirspaces;
            pLastDoNearest = NULL;
            pNearestDataReady = NULL;
            headertoken[0] = MsgToken<1642>(); // LKTOKEN _@M1642_ "ASP"
            headertoken[1] = MsgToken<752>(); // Type
            headertoken[2] = MsgToken<1300>(); // Dist
            headertoken[3] = MsgToken<1301>(); // Dir
            headertoken[4] = MsgToken<2186>(); // *
            break;
        case MSM_THERMALS:
            pSortedIndex = LKSortedThermals.data();
            pSortedNumber = &LKNumThermals;
            pLastDoNearest = &LastDoThermalH;
            pNearestDataReady = NULL;
            headertoken[0] = MsgToken<1670>(); //  "THR"
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>();
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>();
            headertoken[3] = MsgToken<1673>(); // Avg
            headertoken[4] = compact_headers ? MsgToken<1303>() : MsgToken<1307>();
            break;
        case MSM_TRAFFIC:
            pSortedIndex = LKSortedTraffic;
            pSortedNumber = &LKNumTraffic;
            pLastDoNearest = &LastDoTraffic;
            pNearestDataReady = NULL;
            headertoken[0] = MsgToken<1331>(); // TRF
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>();
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>();
            headertoken[3] = MsgToken<1673>(); // Avg
            headertoken[4] = MsgToken<1334>(); // Alt
            break;
        default:
            pSortedIndex = CommonIndex;
            pSortedNumber = &CommonNumber;
            pLastDoNearest = &LastDoCommon;
            pNearestDataReady = &RecentDataReady;
            headertoken[0] = MsgToken<266>(); // LKTOKEN _@M266_ "Error"
            headertoken[1] = compact_headers ? MsgToken<1300>() : MsgToken<1304>();
            headertoken[2] = compact_headers ? MsgToken<1301>() : MsgToken<1305>();
            headertoken[3] = compact_headers ? MsgToken<1302>() : MsgToken<1306>();
            headertoken[4] = compact_headers ? MsgToken<1303>() : MsgToken<1307>();
            break;
    }

    if (!pSortedNumber) {
        return;
    }

    if (MSMTHERMALS) DoThermalHistory(&DrawInfo, &DerivedDrawInfo);
    if (MSMTRAFFIC) DoTraffic(&DrawInfo, &DerivedDrawInfo);

    Numpages = roundupdivision(*pSortedNumber * lincr, numraws);
    if (Numpages > MAXNUMPAGES) {
        Numpages = MAXNUMPAGES;
    }
    else if (Numpages < 1) {
        Numpages = 1;
    }

    short curpage = SelectedPage[curmapspace];
    if (curpage < 0 || curpage >= MAXNUMPAGES) { // TODO also >Numpages
        // DoStatusMessage(_T("ERR-091 curpage invalid!"));  // selection while waiting for data ready
        SelectedPage[curmapspace] = 0;
        LKevent = LKEVENT_NONE;
        return;
    }

    switch (LKevent) {
        case LKEVENT_NONE:
            break;
        case LKEVENT_ENTER:
            LKevent = LKEVENT_NONE;
            {
              short i = SelectedRaw[curmapspace] + (curpage * numraws / lincr);
              if (pSortedIndex) {
                i = pSortedIndex[i];
              }

              if (MSMCOMMONS) {
                if (!ValidWayPoint(i)) {
                  break;
                }
                /*
                 * we can't show dialog from Draw thread
                 * instead, new event is queued, dialog will be popup by main thread
                 */
                InputEvents::processPopupDetails(im_waypoint{static_cast<size_t>(i)});
                // SetModeType(LKMODE_MAP,MP_MOVING); EXperimental OFF 101219
              }
              if (MSMAIRSPACES) {
                if (ValidAirspace(i)) {
                  auto pAsp = LKAirspaces[i].Pointer.lock();
                  if (pAsp) {
                    InputEvents::processPopupDetails(im_airspace{pAsp});
                  }
                }
              }

              if (MSMTHERMALS) {
                if ((i < 0) || (i >= MAXTHISTORY)) {
                  break;
                }
                InputEvents::processPopupDetails(im_thermal{static_cast<size_t>(i)});
              }
              if (MSMTRAFFIC) {
                if ((i < 0) || (i >= MAXTRAFFIC) || (LKTraffic[i].RadioId <= 0)) {
                  break;
                }
                InputEvents::processPopupDetails(im_flarm{i});
              }
            }
            LKevent = LKEVENT_NONE;
            return;
            break;
        case LKEVENT_DOWN:
            if (++SelectedRaw[curmapspace] >= numraws) {
                SelectedRaw[curmapspace] = 0;
                if (++SelectedPage[curmapspace] >= Numpages) {
                    SelectedPage[curmapspace] = 0;
                }
                curpage = SelectedPage[curmapspace];
            }

            if (curpage * numraws + SelectedRaw[curmapspace] >= MAXNEAREST) {
                SelectedRaw[curmapspace] = 0;
                curpage = SelectedPage[curmapspace] = 0;
            }

            if (MSMCOMMONS) *pLastDoNearest = DrawInfo.Time + PAGINGTIMEOUT - 1.0;
            if (MSMTHERMALS) *pLastDoNearest = DrawInfo.Time + PAGINGTIMEOUT - 1.0;
            if (MSMTRAFFIC) *pLastDoNearest = DrawInfo.Time + PAGINGTIMEOUT - 1.0;
            break;
        case LKEVENT_UP:
            if (--SelectedRaw[curmapspace] < 0) {
                SelectedRaw[curmapspace] = numraws - 1;
                if (--SelectedPage[curmapspace] < 0) {
                    SelectedPage[curmapspace] = (Numpages - 1);
                }
                curpage = SelectedPage[curmapspace];
            }

            if (MSMCOMMONS) *pLastDoNearest = DrawInfo.Time + PAGINGTIMEOUT - 1.0;
            if (MSMTHERMALS) *pLastDoNearest = DrawInfo.Time + PAGINGTIMEOUT - 1.0;
            if (MSMTRAFFIC) *pLastDoNearest = DrawInfo.Time + PAGINGTIMEOUT - 1.0;
            break;
        case LKEVENT_PAGEUP:
            LKevent = LKEVENT_NONE;
            break;
        case LKEVENT_PAGEDOWN:
            LKevent = LKEVENT_NONE;
            break;
        case LKEVENT_NEWRUN:
            for (int i = 0; i < MAXNEAREST; i++) {
                for (int k = 0; k < MAXNUMPAGES; k++) {
                    lk::strcpy(Buffer1[i][k], _T("------------")); // 12 chars
                    lk::strcpy(Buffer2[i][k], _T("----"));
                    lk::strcpy(Buffer3[i][k], _T("----"));
                    lk::strcpy(Buffer4[i][k], _T("----"));
                    lk::strcpy(Buffer5[i][k], _T("----"));
                }
            }
            break;
        case LKEVENT_NEWPAGE:
            break;
        default:
            LKevent = LKEVENT_NONE;
            break;
    }

    Surface.DrawLine(PEN_SOLID, ScreenThinSize, p1, p2, LineColor(), rc);
    Surface.SelectObject(LK8InfoNearestFont); // Heading line

    //
    // DRAW HEADER ROW WITH SORTBOXES
    //

    short cursortbox;
    // Highlighted header sorting selection. Not available for commons.
    if (curmapspace == MSM_COMMON || curmapspace == MSM_RECENT)
        cursortbox = 99; // hack
    else {
        cursortbox = SortedMode[curmapspace];
        Surface.FillRect(&s_sortBox[cursortbox],
                       !IsDithered() ? (INVERTCOLORS ? LKBrush_LightGreen : LKBrush_DarkGreen) :
                       (INVERTCOLORS ? LKBrush_White : LKBrush_Black));
    }

    // PAGE INDEX, example: 2.1
    //
    Surface.SelectObject(LK8PanelMediumFont);
    lk::snprintf(Buffer, TEXT("%d.%d"), ModeIndex, CURTYPE + 1);
    LKWriteText(Surface,
              Buffer,
              rc.left + LEFTLIMITER,
              rc.top + TOPLIMITER,
              WTMODE_NORMAL,
              WTALIGN_LEFT,
              !IsDithered() ? RGB_LIGHTGREEN : RGB_WHITE,
              false);


    LKColor tmpcolor = (cursortbox == 0) ? RGB_BLACK : RGB_LIGHTGREEN;;
    if (IsDithered()) {
      tmpcolor = (cursortbox == 0) ? RGB_BLACK : RGB_WHITE;
    }

    Surface.SelectObject(LK8InfoNearestFont);

    lk::snprintf(Buffer, TEXT("%s %d/%d"), headertoken[0], curpage + 1, Numpages);

    LKWriteText(Surface, Buffer, Column0[curmapspace], rc.top + HEADRAW - NIBLSCALE(1), WTMODE_NORMAL, WTALIGN_LEFT, tmpcolor, false);
    if (cursortbox == 99) cursortbox = 0;

    tmpcolor = cursortbox == 1 ? RGB_BLACK : RGB_WHITE;
    LKWriteText(Surface,  headertoken[1], hColumn2, rc.top + HEADRAW, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);

    tmpcolor = cursortbox == 2 ? RGB_BLACK : RGB_WHITE;
    LKWriteText(Surface, headertoken[2], hColumn3, rc.top + HEADRAW, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);

    tmpcolor = cursortbox == 3 ? RGB_BLACK : RGB_WHITE;
    LKWriteText(Surface, headertoken[3], hColumn4, rc.top + HEADRAW, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);

    tmpcolor = cursortbox == 4 ? RGB_BLACK : RGB_WHITE;
    LKWriteText(Surface, headertoken[4], hColumn5, rc.top + HEADRAW, WTMODE_NORMAL, WTALIGN_RIGHT, tmpcolor, false);

    Surface.SelectObject(bigFont); // Text font for Nearest

    // try to reduce conflicts, as task thread could change it while we are using it here.
    // so we copy it and clear it here once forever in this run
    bool ndr = false;
    if (MSMCOMMONS) {
        ndr = *pNearestDataReady;
        *pNearestDataReady = false;
    }
    if (MSMAIRSPACES) {
        ndr = DoAirspaces(&DrawInfo, &DerivedDrawInfo);
    }

    uint16_t drawn_items_onpage = 0;

    for (uint16_t i = 0, j = 0; i < numraws; j++, i += lincr) {
        uint16_t iRaw = TopSize + (s_rawspace * i);
        uint16_t curraw = (curpage * numraws);
        if (usetwolines) {
            curraw /= 2;
            curraw += j;
        } else {
            curraw += i;
        }
        if (curraw >= MAXNEAREST) {
            break;
        }
        size_t rli = curraw;
        if (pSortedIndex) {
            if (curraw < (*pSortedNumber)) {
                rli = pSortedIndex[curraw];
            }
            else {
                rli = ~0;
            }
        }

        if (!ndr) {
            if (MSMCOMMONS) goto _KeepOldCommonsValues;
            if (MSMAIRSPACES) goto _KeepOldAirspacesValues;
        }

        if (MSMCOMMONS) {
            if (ValidWayPoint(rli)) {
                lk::strcpy(Buffer, WayPointList[rli].Name, s_maxnlname[curmapspace]);
                CharUpper(Buffer);
                lk::strcpy(Buffer1[i][curpage], Buffer);

                value = Units::ToDistance(WayPointCalc[rli].Distance);
                if (usetwolines) {
                  lk::snprintf(Buffer2[i][curpage], TEXT("%0.1lf %s"), value, Units::GetDistanceName());
                }
                else {
                  lk::snprintf(Buffer2[i][curpage], TEXT("%0.1lf"), value);
                }

                if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
                  value = AngleLimit180(WayPointCalc[rli].Bearing - DrawInfo.TrackBearing);

                  if (value > 1) {
                    lk::snprintf(Buffer3[i][curpage], TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
                  }
                  else if (value < -1) {
                    lk::snprintf(Buffer3[i][curpage], TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
                  }
                  else {
                    lk::snprintf(Buffer3[i][curpage], TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
                  }
                }
                else {
                  lk::snprintf(Buffer3[i][curpage], TEXT("%2.0f%s"), WayPointCalc[rli].Bearing,
                               MsgToken<2179>());  // 101219
                }

                value = WayPointCalc[rli].GR;
                if (value < 1 || value >= MAXEFFICIENCYSHOW) {
                  lk::strcpy(Buffer4[i][curpage], _T("----"));
                }
                else {
                  if (value > 99) {
                    lk::snprintf(text, _T("%.0f"), value);
                  }
                  else {
                    lk::snprintf(text, _T("%.1f"), value);
                  }
                  lk::strcpy(Buffer4[i][curpage], text);
                }

                value = Units::ToAltitude(WayPointCalc[rli].AltArriv[AltArrivMode]);
                if (value < -9999 || value > 9999) {
                  lk::strcpy(text, _T("----"));
                }
                else {
                  lk::snprintf(text, _T("%+.0f"), value);
                }
                if (usetwolines) {
                  lk::snprintf(Buffer5[i][curpage], TEXT("%s %s"), text, Units::GetAltitudeName());
                }
                else {
                  lk::snprintf(Buffer5[i][curpage], TEXT("%s"), text);
                }
            }
            else {  // !Valid
              lk::strcpy(Buffer1[i][curpage], _T("------------"));
              lk::strcpy(Buffer2[i][curpage], _T("----"));
              lk::strcpy(Buffer3[i][curpage], _T("----"));
              lk::strcpy(Buffer4[i][curpage], _T("----"));
              lk::strcpy(Buffer5[i][curpage], _T("----"));
            }

_KeepOldCommonsValues:

            if (ValidWayPoint(rli)) {

                drawn_items_onpage++;

                if (WayPointCalc[rli].IsOutlanding) {
                    rcolor = RGB_LIGHTYELLOW;
                    Surface.SelectObject(bigItalicFont);
                } else {
                    rcolor = RGB_WHITE;
                    Surface.SelectObject(bigFont);
                }

                // 120601 extend search for tps, missing reachable status
                // If we are listing tps, and the current tp has a positive arrival altitude,
                // then check if it is really unreachable because we dont calculate tps for that.
                // Unless they are in a task, common, alternates, of course.
                if (curmapspace == MSM_NEARTPS) {
                    if (WayPointCalc[rli].AltArriv[AltArrivMode] > 0) {
                        if (CheckLandableReachableTerrainNew(&DrawInfo, &DerivedDrawInfo,
                                WayPointCalc[rli].Distance, WayPointCalc[rli].Bearing)) {
                            rcolor = RGB_WHITE;
                        } else {
                            rcolor = RGB_LIGHTRED;
                        }
                    } else {
                        rcolor = RGB_LIGHTRED;
                    }
                } else // old stuff as usual
                    if ((WayPointCalc[rli].VGR == 3) || (!WayPointList[rli].Reachable)) {

                    rcolor = RGB_LIGHTRED;
                }


            } else {
                rcolor = RGB_GREY;
            }
        } // MSMCOMMONS

        if (MSMAIRSPACES) {
            if (ValidAirspace(rli)) {

                //
                // AIRSPACE NAME
                //
                lk::strcpy(Buffer, LKAirspaces[rli].Name, s_maxnlname[curmapspace]);
                CharUpper(Buffer);
                lk::strcpy(Buffer1[i][curpage], Buffer);


                //
                // AIRSPACE TYPE
                //
                lk::strcpy(Buffer, LKAirspaces[rli].Type, LKASP_TYPE_LEN);
                CharUpper(Buffer);
                lk::strcpy(Buffer2[i][curpage], Buffer);


                //
                // AIRSPACE DISTANCE
                //
                switch (LKAirspaces[rli].WarningLevel) {
                    case awYellow:
                        value = Units::ToDistance(LKAirspaces[rli].Distance);
                        if (usetwolines)
                            lk::snprintf(Buffer3[i][curpage], TEXT("%0.1lf%s!"), value, Units::GetDistanceName());
                        else
                            lk::snprintf(Buffer3[i][curpage], TEXT("%0.1lf!"), value);
                        break;
                    case awRed:
                        lk::strcpy(Buffer3[i][curpage], TEXT("IN"));
                        break;
                    default:
                        value = Units::ToDistance(LKAirspaces[rli].Distance);
                        if (usetwolines)
                            lk::snprintf(Buffer3[i][curpage], TEXT("%0.1lf%s"), value, Units::GetDistanceName());
                        else
                            lk::snprintf(Buffer3[i][curpage], TEXT("%0.1lf"), value);
                        break;
                }

                //
                // AIRSPACE BEARING DIFFERENCE, OR BEARING IF CIRCLING
                //
                if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
                    value = AngleLimit180(LKAirspaces[rli].Bearing - DrawInfo.TrackBearing);

                    if (value > 1) {
                      lk::snprintf(Buffer4[i][curpage], TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
                    }
                    else if (value < -1) {
                      lk::snprintf(Buffer4[i][curpage], TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
                    }
                    else {
                      lk::snprintf(Buffer4[i][curpage], TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
                    }
                }
                else {
                  lk::snprintf(Buffer4[i][curpage], TEXT("%2.0f%s"), LKAirspaces[rli].Bearing, MsgToken<2179>());
                }

                //
                // AIRSPACE FLAGS
                //
                TCHAR aspflags[5];
                lk::snprintf(aspflags, _T("%s%s%s"), LKAirspaces[rli].Selected ? _T("S") : _T(""),
                          LKAirspaces[rli].Flyzone ? _T("F") : _T("  "), LKAirspaces[rli].Enabled ? _T("E") : _T("D"));

                if (!ScreenLandscape && usetwolines) {
                  lk::snprintf(Buffer5[i][curpage], TEXT("*%s"), aspflags);
                }
                else {
                  lk::snprintf(Buffer5[i][curpage], TEXT("%s"), aspflags);
                }
            }
            else {
              lk::strcpy(Buffer1[i][curpage], _T("------------"));  // max 12
              lk::strcpy(Buffer2[i][curpage], _T("----"));
              lk::strcpy(Buffer3[i][curpage], _T("----"));
              lk::strcpy(Buffer4[i][curpage], _T("----"));
              lk::strcpy(Buffer5[i][curpage], _T("  "));
            }

_KeepOldAirspacesValues:

            if (ValidAirspace(rli)) {

                drawn_items_onpage++;

                switch (LKAirspaces[rli].WarningLevel) {
                    case awYellow:
                        rcolor = RGB_LIGHTYELLOW;
                        Surface.SelectObject(bigItalicFont);
                        break;
                    case awRed:
                        rcolor = RGB_LIGHTRED;
                        Surface.SelectObject(bigItalicFont);
                        break;
                    case awNone:
                    default:
                        rcolor = RGB_WHITE;
                        Surface.SelectObject(bigFont);
                        break;
                }
            } else {
                rcolor = RGB_GREY;
            }
        } // MSMAIRSPACES

        if (MSMTHERMALS) {
        	if (rli < CopyThermalHistory.size()) {
                const auto& thermal = CopyThermalHistory[rli];

                TCHAR* pOut = Buffer1[i][curpage];
                if (IsThermalMultitarget(rli)) {
                    lk::strcpy(pOut, _T(">"), std::size(Buffer1[i][curpage]));
                    pOut += _tcslen(pOut);
                }
                CopyTruncateString(pOut, s_maxnlname[curmapspace], thermal.Name.c_str());

                // Distance
                value = Units::ToDistance(thermal.Distance);
                if (usetwolines) {
                  lk::snprintf(Buffer2[i][curpage], TEXT("%0.1lf %s"), value, Units::GetDistanceName());
                }
                else {
                  lk::snprintf(Buffer2[i][curpage], TEXT("%0.1lf"), value);
                }

                // relative bearing

                if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
                  value = AngleLimit180(thermal.Bearing - DrawInfo.TrackBearing);
                  if (value > 1) {
                    lk::snprintf(Buffer3[i][curpage], TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
                  }
                  else if (value < -1) {
                    lk::snprintf(Buffer3[i][curpage], TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
                  }
                  else {
                    lk::snprintf(Buffer3[i][curpage], TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
                  }
                }
                else {
                  lk::snprintf(Buffer3[i][curpage], _T("%2.0f%s"), thermal.Bearing, MsgToken<2179>());
                }

                // Average lift
                value = Units::ToVerticalSpeed(thermal.Lift);
                if (value < -99 || value > 99) {
                  lk::strcpy(Buffer4[i][curpage], _T("----"));
                }
                else {
                  lk::snprintf(Buffer4[i][curpage], _T("%+.1f"), value);
                }

                // Altitude
                value = Units::ToAltitude(thermal.Arrival);
                if (value < -1000 || value > 45000) {
                  lk::strcpy(Buffer5[i][curpage], _T("----"));
                }
                else {
                  if (usetwolines) {
                    lk::snprintf(Buffer5[i][curpage], TEXT("%.0f %s"), value, Units::GetAltitudeName());
                  }
                  else {
                    lk::snprintf(Buffer5[i][curpage], TEXT("%.0f"), value);
                  }
                }
            }
            else {
                // Empty thermals, fill in all empty data and maybe break loop
                lk::strcpy(Buffer1[i][curpage], _T("------------"));
                lk::strcpy(Buffer2[i][curpage], _T("----"));
                lk::strcpy(Buffer3[i][curpage], _T("----"));
                lk::strcpy(Buffer4[i][curpage], _T("----"));
                lk::strcpy(Buffer5[i][curpage], _T("----"));
            }

        	if (rli < CopyThermalHistory.size()) {
                const auto& thermal = CopyThermalHistory[rli];
                drawn_items_onpage++;
                if (thermal.Arrival >= 0) {
                    rcolor = RGB_WHITE;
                    Surface.SelectObject(bigFont);
                } else {
                    rcolor = RGB_LIGHTRED;
                    Surface.SelectObject(bigItalicFont);
                }
            } else {
                rcolor = RGB_GREY;
            }
        } // MSMTHERMALS

        if (MSMTRAFFIC) {
            if (rli < std::size(LKTraffic) && (LKTraffic[rli].RadioId > 0)) {

                // Traffic name
                int wlen = _tcslen(LKTraffic[rli].Name);

                // if name is unknown then it is a '?'
                if (wlen == 1) {
                    lk::snprintf(Buffer, _T("%06x"), (unsigned) LKTraffic[rli].RadioId);
                    Buffer[s_maxnlname[curmapspace]] = '\0';
                } else {
                    // if XY I-ABCD  doesnt fit..
                    if ((wlen + 3) > s_maxnlname[curmapspace]) {
                      lk::strcpy(Buffer, LKTraffic[rli].Name, s_maxnlname[curmapspace]);
                    }
                    else {
                      unsigned short cnlen = _tcslen(LKTraffic[rli].Cn);
                      // if cn is XY create XY I-ABCD
                      if (cnlen == 1 || cnlen == 2) {
                        lk::strcpy(Buffer, LKTraffic[rli].Cn);
                        _tcscat(Buffer, _T(" "));
                        _tcscat(Buffer, LKTraffic[rli].Name);
                        // for safety
                        Buffer[s_maxnlname[curmapspace]] = '\0';
                      }
                      else {
                        // else use only long name
                        lk::strcpy(Buffer, LKTraffic[rli].Name, wlen);
                      }
                    }
                    CharUpper(Buffer);
                }

                TCHAR* pOut = Buffer1[i][curpage];
                if (LKTraffic[rli].Locked) {
                    lk::strcpy(pOut, _T("*"), std::size(Buffer1[i][curpage]));
                    pOut += _tcslen(pOut);
                }
                CopyTruncateString(pOut, s_maxnlname[curmapspace], Buffer);

#ifdef DEBUG_LKT_DRAWTRAFFIC
                StartupStore(_T(".. Traffic[%d] Name=<%s> Id=<%0x> Status=%d Named:<%s>\n"), rli, LKTraffic[rli].Name,
                        LKTraffic[rli].ID, LKTraffic[rli].Status, Buffer);
#endif

                // Distance
                value = Units::ToDistance(LKTraffic[rli].Distance);
                if (usetwolines) {
                  lk::snprintf(Buffer2[i][curpage], TEXT("%0.1lf %s"), value, Units::GetDistanceName());
                }
                else {
                  lk::snprintf(Buffer2[i][curpage], TEXT("%0.1lf"), value);
                }

                // relative bearing

                if (!MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
                    value = AngleLimit180(LKTraffic[rli].Bearing - DrawInfo.TrackBearing);

                    if (value > 1) {
                      lk::snprintf(Buffer3[i][curpage], TEXT("%2.0f%s%s"), value, MsgToken<2179>(), MsgToken<2183>());
                    }
                    else if (value < -1) {
                      lk::snprintf(Buffer3[i][curpage], TEXT("%s%2.0f%s"), MsgToken<2182>(), -value, MsgToken<2179>());
                    }
                    else {
                      lk::snprintf(Buffer3[i][curpage], TEXT("%s%s"), MsgToken<2182>(), MsgToken<2183>());
                    }
                }
                else {
                  lk::snprintf(Buffer3[i][curpage], _T("%2.0f%s"), LKTraffic[rli].Bearing, MsgToken<2179>());
                }

                // Vario
                value = Units::ToVerticalSpeed(LKTraffic[rli].Average30s);
                if (value<-6 || value > 6)
                    lk::strcpy(Buffer4[i][curpage], _T("----"));
                else {
                    lk::snprintf(Buffer4[i][curpage], _T("%+.1f"), value);
                }

                // Altitude
                value = Units::ToAltitude(LKTraffic[rli].Altitude);
                if (value < -1000 || value > 45000) {
                  lk::strcpy(Buffer5[i][curpage], _T("----"));
                }
                else {
                  if (usetwolines) {
                    lk::snprintf(Buffer5[i][curpage], TEXT("%.0f %s"), value, Units::GetAltitudeName());
                  }
                  else {
                    lk::snprintf(Buffer5[i][curpage], TEXT("%.0f"), value);
                  }
                }
            } else {
                // Empty traffic, fill in all empty data and maybe break loop
                lk::strcpy(Buffer1[i][curpage], _T("------------"));
                lk::strcpy(Buffer2[i][curpage], _T("----"));
                lk::strcpy(Buffer3[i][curpage], _T("----"));
                lk::strcpy(Buffer4[i][curpage], _T("----"));
                lk::strcpy(Buffer5[i][curpage], _T("----"));
            }


            if (rli < std::size(LKTraffic) && (LKTraffic[rli].RadioId > 0)) {
                drawn_items_onpage++;
                if (LKTraffic[rli].Status == LKT_REAL) {
                    rcolor = RGB_WHITE;
                    Surface.SelectObject(bigFont);
                } else {
                    if (LKTraffic[rli].Status == LKT_GHOST) {
                        rcolor = RGB_LIGHTYELLOW;
                    } else {
                        rcolor = RGB_LIGHTRED;
                    }
                    Surface.SelectObject(bigItalicFont);
                }
            } else {
                rcolor = RGB_GREY;
            }
        } // MSMTRAFFIC

        if (!usetwolines) {
            LKWriteText(Surface, Buffer1[i][curpage], Column1[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
            LKWriteText(Surface, Buffer2[i][curpage], Column2[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface, Buffer3[i][curpage], Column3[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface, Buffer4[i][curpage], Column4[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface, Buffer5[i][curpage], Column5[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
        } else {
            LKWriteText(Surface, Buffer1[i][curpage], Column1[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_LEFT, rcolor, false);
            LKWriteText(Surface, Buffer2[i][curpage], Column5[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            iRaw += s_rawspace;
            LKWriteText(Surface, Buffer3[i][curpage], Column2[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface, Buffer4[i][curpage], Column3[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
            LKWriteText(Surface, Buffer5[i][curpage], Column4[curmapspace], iRaw, WTMODE_NORMAL, WTALIGN_RIGHT, rcolor, false);
        }

    } // for loop


    if (LKevent == LKEVENT_NEWRUN || LKevent == LKEVENT_NEWPAGE) {
        LKevent = LKEVENT_NONE;
        return;
    }

    // BOXOUT SELECTED ITEM
    //
    if (drawn_items_onpage > 0) {

        if (SelectedRaw[curmapspace] < 0 || SelectedRaw[curmapspace]>(numraws - 1)) {
            return;
        }
        // Avoid boxing and selecting nonexistent items
        // selectedraw starts from 0, drawnitems from 1...
        // In this case we set the first one, or last one, assuming we are rotating forward or backward
        if (SelectedRaw[curmapspace] >= drawn_items_onpage) {
            if (LKevent == LKEVENT_DOWN) {
                SelectedRaw[curmapspace] = 0;
                SelectedPage[curmapspace] = 0;
            }
            else if (LKevent == LKEVENT_UP) {
                SelectedRaw[curmapspace] = drawn_items_onpage - 1;
            }
            else {
                // Here we are recovering a selection problem caused by a delay while switching.
                // DoStatusMessage(_T("Cant find valid raw")); // not needed anymore
                SelectedRaw[curmapspace] = 0;
            }
        }


        invsel.left = left;
        invsel.right = right;
        if (usetwolines) {
            invsel.top = TopSize + (s_rawspace * SelectedRaw[curmapspace] * lincr);
        } else {
            invsel.top = TopSize + (s_rawspace * SelectedRaw[curmapspace]) + NIBLSCALE(2);
        }
        invsel.bottom = TopSize + (s_rawspace * (SelectedRaw[curmapspace] * lincr + 1)) - NIBLSCALE(1);
        if (usetwolines) invsel.bottom += s_rawspace;

#ifdef __linux__
        invsel.top -= (HEADRAW / 2 - NIBLSCALE(2));
        invsel.bottom -= NIBLSCALE(1); // interline
#endif

        // HIGHLIGHT user selection. Bordering will not be the same, under the sun not visible!
        // TO BE CHECKED CAREFULLY
#ifdef ENABLE_OPENGL

        LKPen SelectBorder(PEN_SOLID, NIBLSCALE(1), LineColor());

        Surface.SelectObject(SelectBorder);
        Surface.SelectObject(LK_HOLLOW_BRUSH);
        invsel.left -= NIBLSCALE(2); invsel.right += NIBLSCALE(2);
        Surface.RoundRect(invsel, 5, 5);
#else
        Surface.InvertRect(invsel);
#endif

    }

    LKevent = LKEVENT_NONE;
    return;
}
