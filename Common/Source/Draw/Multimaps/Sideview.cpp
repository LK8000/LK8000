/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "Sideview.h"


extern double fSplitFact;
extern LKColor Sideview_TextColor;

LKColor Sideview_TextColor = RGB_WHITE;
using std::min;
using std::max;

int Sideview_asp_heading_task = 0;

void SetMMNorthUp(int iPage, int iVal) {
    BUGSTOP_LKASSERT(iPage < NUMBER_OF_SHARED_MULTIMAPS);
    if (iPage < 0) iPage = 0;
    if (iPage >= NUMBER_OF_SHARED_MULTIMAPS) iPage = NUMBER_OF_SHARED_MULTIMAPS - 1;
    MMNorthUp_Runtime[iPage] = iVal;
}

int GetMMNorthUp(int iPage) {
    if (iPage < 0) iPage = 0;
    if (iPage >= NUMBER_OF_SHARED_MULTIMAPS) iPage = NUMBER_OF_SHARED_MULTIMAPS - 1;
    return MMNorthUp_Runtime[iPage];
}

int SetSplitScreenSize(int iPercent) {
    LKASSERT(iPercent >= 0 && iPercent <= 100);
    int iOld = (int) (fSplitFact * 100.0);
#if 0
    if (iPercent >= 90)
	iPercent =90;
#endif
    if (iPercent == 100)
        fSplitFact = 1;
    else
        fSplitFact = (double) (iPercent % 100) / 100.0f;

    return iOld;
}

int IncSideviewPage(void) {
    Sideview_asp_heading_task++;
    if (Sideview_asp_heading_task >= NO_SIDEVIEW_PAGES)
        Sideview_asp_heading_task = 0;
    return Sideview_asp_heading_task;
}

int DecSideviewPage(void) {
    Sideview_asp_heading_task--;
    if (Sideview_asp_heading_task < 0)
        Sideview_asp_heading_task = NO_SIDEVIEW_PAGES - 1;
    return Sideview_asp_heading_task;
}

int GetSideviewPage(void) {
    return Sideview_asp_heading_task;
}

int SetSideviewPage(int i) {
    int oldPage = Sideview_asp_heading_task;
    Sideview_asp_heading_task = i;

    return oldPage;
}

void DrawWindRoseDirection(LKSurface& Surface, double fAngle, int x, int y) {
    BOOL bInvCol = true; //INVERTCOLORS
    const TCHAR* text = AngleToWindRose(fAngle);

    Surface.SetBackgroundTransparent();
    if (bInvCol)
        Surface.SetTextColor(RGB_BLACK);
    else
        Surface.SetTextColor(RGB_WHITE);

    SIZE tsize;
    Surface.GetTextSize(text, &tsize);
    Surface.DrawText(x - tsize.cx / 2, y - tsize.cy / 2, text);

    return;
}

void DrawSelectionFrame(LKSurface& Surface, const RECT& rc) {
    Surface.SetBackgroundTransparent();
    RECT rci = rc;
#define SHRINK 1
    rci.left += 1;
    rci.top -= 1;
    rci.right -= 2;
    rci.bottom -= 2;
    int iSize = NIBLSCALE(2);
    LKColor col = RGB_BLACK;

    Surface.DrawLine(PEN_SOLID, iSize, (POINT) {
        rci.left, rci.top}, (POINT) {
        rci.left, rci.bottom
    }, col, rci);

    Surface.DrawLine(PEN_SOLID, iSize, (POINT) {
        rci.left, rci.bottom}, (POINT) {
        rci.right, rci.bottom
    }, col, rci);

    Surface.DrawLine(PEN_SOLID, iSize, (POINT) {
        rci.right, rci.bottom}, (POINT) {
        rci.right, rci.top
    }, col, rci);

    Surface.DrawLine(PEN_SOLID, iSize, (POINT) {
        rci.right, rci.top}, (POINT) {
        rci.left, rci.top
    }, col, rci);

    col = RGB_YELLOW;

#ifdef NO_DASH_LINES
    Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), (POINT) {
        rci.left, rci.top}, (POINT) {
        rci.left, rci.bottom
    }, col, rci);

    Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), (POINT) {
        rci.left, rci.bottom}, (POINT) {
        rci.right, rci.bottom
    }, col, rci);

    Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), (POINT) {
        rci.right, rci.bottom}, (POINT) {
        rci.right, rci.top
    }, col, rci);

    Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), (POINT) {
        rci.right, rci.top}, (POINT) {
        rci.left, rci.top
    }, col, rci);

#else
    Surface.DrawDashLine(iSize, (POINT) {
        rci.left, rci.top}, (POINT) {
        rci.left, rci.bottom
    }, col, rci);

    Surface.DrawDashLine(iSize, (POINT) {
        rci.left, rci.bottom}, (POINT) {
        rci.right, rci.bottom
    }, col, rci);

    Surface.DrawDashLine(iSize, (POINT) {
        rci.right, rci.bottom}, (POINT) {
        rci.right, rci.top
    }, col, rci);

    Surface.DrawDashLine(iSize, (POINT) {
        rci.right, rci.top}, (POINT) {
        rci.left, rci.top
    }, col, rci);
#endif

}

int CalcHeightCoordinatOutbound(double fHeight, DiagrammStruct* psDia) {
    RECT rc = psDia->rc;
    int y0 = rc.bottom; //-BORDER_Y;

    //  fMaxAltToday = 3300;
    //  double hmin = max(0.0, alt-2300);
    //  double hmax = max(fMaxAltToday, alt+1000);
    double hmin = psDia->fYMin;
    double hmax = psDia->fYMax;
    if (hmax == hmin) hmax++; // RECOVER DIVISION BY ZERO!
    double gfh = (fHeight - hmin) / (hmax - hmin);
    int yPos = (int) (gfh * (rc.top - rc.bottom) + y0) - 1;
    return yPos;
}

int CalcHeightCoordinat(double fHeight, DiagrammStruct* psDia) {
    int yPos;
    RECT rc = psDia->rc;
    yPos = CalcHeightCoordinatOutbound(fHeight, psDia);
    if (yPos < rc.top)
        yPos = rc.top;
    if (yPos > rc.bottom)
        yPos = rc.bottom;
    return yPos;

}

int CalcDistanceCoordinat(double fDist, DiagrammStruct* psDia) {
    RECT rc = psDia->rc;
    if (psDia->fXMax == psDia->fXMin) psDia->fXMax++; // RECOVER DIVISION BY ZERO!
    double xscale = (double) (rc.right) / (psDia->fXMax - psDia->fXMin);
    int xPos = (int) ((fDist - psDia->fXMin) * xscale) + rc.left;
    return xPos;

}
