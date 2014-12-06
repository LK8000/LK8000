/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "RGB.h"
#include "LKObjects.h"

void MapWindow::DrawTaskAAT(LKSurface& Surface, const RECT& rc) {
    int i;
    double tmp1 = 0.0;

    if (WayPointList.empty()) return;
    if (!AATEnabled) return;

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

        LKColor whitecolor = RGB_WHITE;
        LKColor origcolor = hDCTempTask.SetTextColor(whitecolor);

        const auto oldpen = hDCTempTask.SelectObject(LK_WHITE_PEN);
        const auto oldbrush = hDCTempTask.SelectObject(LKBrush_White);
        if(LKSurface::AlphaBlendSupported()) {
            // copy original bitmap into temp (for saving fully transparent areas)
            hDCTempTask.Copy(rcDraw.left, rcDraw.top,
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top, 
                    Surface, rcDraw.left, rcDraw.top);
        } else {
            hDCTempTask.Rectangle(rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom);
        }
        
        hDCTempTask.SelectObject(LK_NULL_PEN);
#ifdef HAVE_HATCHED_BRUSH          
        hDCTempTask.SelectObject(hAirspaceBrushes[iAirspaceBrush[AATASK]]);
#else
#warning "TODO : maybe we need solid brush or that !"
#endif  
        // this color is used as the black bit
        hDCTempTask.SetTextColor(Colours[iAirspaceColour[AATASK]]);
        // this color is the transparent bit
        hDCTempTask.SetBkColor(whitecolor);

        for (i = maxTp - 1; i > std::max(0, ActiveWayPoint - 1); i--) {
        if (ValidTaskPoint(i)) {
            int Type = 0;
            double Radius = 0.;
            GetTaskSectorParameter(i, &Type, &Radius);

            switch (Type) {
                case CONE:
                case CIRCLE:
                    tmp1 = Radius * zoom.ResScaleOverDistanceModify();
                    hDCTempTask.Circle(
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y,
                            (int) tmp1, rc, true, true);
                    break;
                case SECTOR:
                    tmp1 = Radius * zoom.ResScaleOverDistanceModify();
                    hDCTempTask.Segment(
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y, (int) tmp1, rc,
                            Task[i].AATStartRadial - DisplayAngle,
                            Task[i].AATFinishRadial - DisplayAngle);
                    break;
            }
        }
        }

        // restore original color
        hDCTempTask.SetTextColor(origcolor);
        hDCTempTask.SelectObject(oldpen);
        hDCTempTask.SelectObject(oldbrush);
        
        if(!Surface.AlphaBlend(rcDraw, hDCTempTask,rcDraw, 255*35/100)) {
            // if AlphaBlend is not supported, use TransparentBld
            Surface.TransparentCopy(
                    rcDraw.left, rcDraw.top,
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top,
                    hDCTempTask,
                    rcDraw.left, rcDraw.top,
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top,
                    whitecolor
                    );
        }
	}
    {
        UnlockTaskData();
    }
}
