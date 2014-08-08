/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "RGB.h"

void MapWindow::DrawTaskAAT(HDC hdc, const RECT rc) {
    int i;
    double tmp1 = 0.0;

    if (!WayPointList) return;
    if (!AATEnabled) return;

    HPEN oldpen = 0;
    HBRUSH oldbrush = 0;
    HBITMAP oldbitmap = 0;

    LockTaskData(); // protect from external task changes
    /**********************************************/
    /* Check if not Validated Waypoint is visible */
    bool bDraw = false;
    int maxTp = 1;
    rectObj rcrect = (rectObj){(double) rc.left, (double) rc.top, (double) rc.right, (double) rc.bottom};
    RECT rcDraw = (RECT){rc.right, rc.bottom, rc.left, rc.top};

    for (maxTp = std::max(1, ActiveWayPoint); ValidTaskPoint(maxTp + 1); ++maxTp) {
        if (ValidTaskPoint(maxTp)) {
            int Type = 0;
            double Radius = 0.;
            GetTaskSectorParameter(maxTp, &Type, &Radius);
            switch (Type) {
                case CONE:
                case CIRCLE:
                    tmp1 = Task[maxTp].AATCircleRadius * zoom.ResScaleOverDistanceModify();
                    break;
                case SECTOR:
                    tmp1 = Task[maxTp].AATSectorRadius * zoom.ResScaleOverDistanceModify();
                    break;
                default:
                    tmp1 = 0.0;
                    break;
            }
        }

        LONG x = WayPointList[Task[maxTp].Index].Screen.x;
        LONG y = WayPointList[Task[maxTp].Index].Screen.y;
        rectObj rect = (rectObj){x - tmp1, y - tmp1, x + tmp1, y + tmp1};

        if (msRectOverlap(&rect, &rcrect) == MS_TRUE) {
            rcDraw.top = std::min((LONG) rect.miny, rcDraw.top);
            rcDraw.bottom = std::max((LONG) rect.maxy, rcDraw.bottom);
            rcDraw.left = std::min((LONG) rect.minx, rcDraw.left);
            rcDraw.right = std::max((LONG) rect.maxx, rcDraw.right);
            bDraw = true;
        }
    }
    /**********************************************/

    if (bDraw) { // Draw Only if one is Visible
        rcDraw.top = std::max(rc.top, rcDraw.top);
        rcDraw.bottom = std::min(rc.bottom, rcDraw.bottom);
        rcDraw.left = std::max(rc.left, rcDraw.left);
        rcDraw.right = std::min(rc.right, rcDraw.right);

        COLORREF whitecolor = RGB_WHITE;
        COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);

        oldbitmap = (HBITMAP) SelectObject(hDCTemp, (HBITMAP) hDrawBitMapTmp);

        oldpen = (HPEN) SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
        oldbrush = (HBRUSH) SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
        if(MapWindow::AlphaBlendSupported()) {
            // copy original bitmap into temp (for saving fully transparent areas)
            BitBlt(hDCTemp, rcDraw.left, rcDraw.top, 
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top, 
                    hdc, rcDraw.left, rcDraw.top, SRCCOPY);
        } else {
            Rectangle(hDCTemp, rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom);
        }
        
        SelectObject(hDCTemp, GetStockObject(NULL_PEN));
        SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[AATASK]]);

        // this color is used as the black bit
        SetTextColor(hDCTemp, Colours[iAirspaceColour[AATASK]]);
        // this color is the transparent bit
        SetBkColor(hDCTemp, whitecolor);

        for (i = maxTp - 1; i > std::max(0, ActiveWayPoint - 1); i--) {
        if (ValidTaskPoint(i)) {
            int Type = 0;
            double Radius = 0.;
            GetTaskSectorParameter(i, &Type, &Radius);

            switch (Type) {
                case CONE:
                case CIRCLE:
                    tmp1 = Radius * zoom.ResScaleOverDistanceModify();
                    Circle(hDCTemp,
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y,
                            (int) tmp1, rc, true, true);
                    break;
                case SECTOR:
                    tmp1 = Radius * zoom.ResScaleOverDistanceModify();
                    Segment(hDCTemp,
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y, (int) tmp1, rc,
                            Task[i].AATStartRadial - DisplayAngle,
                            Task[i].AATFinishRadial - DisplayAngle);
                    break;
            }
        }
        }

        // restore original color
        SetTextColor(hDCTemp, origcolor);
        SelectObject(hDCTemp, oldpen);
        SelectObject(hDCTemp, oldbrush);
        
        // Use AlphaBlend if supported !
        if(MapWindow::AlphaBlendSupported()) {
            MapWindow::DoAlphaBlend(hdc,rcDraw, hDCTemp,rcDraw,255*35/100);
        } else {
            TransparentBlt(hdc,
                    rcDraw.left, rcDraw.top,
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top,
                    hDCTemp,
                    rcDraw.left, rcDraw.top,
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top,
                    whitecolor
                    );
        }
        SelectObject(hDCTemp, oldbitmap);
	}
    {
        UnlockTaskData();
    }
}
