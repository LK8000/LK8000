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

#ifdef HAVE_HATCHED_BRUSH          
    constexpr uint8_t AlphaLevel = 255*35/100;
#else
    constexpr uint8_t AlphaLevel = 255*17/100;
#endif  
        
        
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

    for (maxTp = std::max(1, ActiveTaskPoint); ValidTaskPoint(maxTp + 1); ++maxTp) {
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

        PixelScalar x = WayPointList[Task[maxTp].Index].Screen.x;
        PixelScalar y = WayPointList[Task[maxTp].Index].Screen.y;
        rectObj rect = (rectObj){x - tmp1, y - tmp1, x + tmp1, y + tmp1};

        if (msRectOverlap(&rect, &rcrect) == MS_TRUE) {
            rcDraw.top = std::min((PixelScalar)rect.miny, rcDraw.top);
            rcDraw.bottom = std::max((PixelScalar) rect.maxy, rcDraw.bottom);
            rcDraw.left = std::min((PixelScalar) rect.minx, rcDraw.left);
            rcDraw.right = std::max((PixelScalar) rect.maxx, rcDraw.right);
            bDraw = true;
        }
    }
    /**********************************************/

    if (bDraw) { // Draw Only if one is Visible
#ifdef USE_GDI
        rcDraw.top = std::max(rc.top, rcDraw.top);
        rcDraw.bottom = std::min(rc.bottom, rcDraw.bottom);
        rcDraw.left = std::max(rc.left, rcDraw.left);
        rcDraw.right = std::min(rc.right, rcDraw.right);


        LKColor whitecolor = RGB_WHITE;
        LKColor origcolor = TempSurface.SetTextColor(whitecolor);

        const auto oldpen = TempSurface.SelectObject(LK_WHITE_PEN);
        const auto oldbrush = TempSurface.SelectObject(LKBrush_White);
        if(LKSurface::AlphaBlendSupported()) {
            // copy original bitmap into temp (for saving fully transparent areas)
            TempSurface.Copy(rcDraw.left, rcDraw.top,
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top, 
                    Surface, rcDraw.left, rcDraw.top);
        } else {
            TempSurface.Rectangle(rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom);
        }
        
        TempSurface.SelectObject(LK_NULL_PEN);
#ifdef HAVE_HATCHED_BRUSH          
        TempSurface.SelectObject(hAirspaceBrushes[iAirspaceBrush[AATASK]]);
#else
        TempSurface.SelectObject(LKBrush_Yellow);
#endif  
        // this color is used as the black bit
        TempSurface.SetTextColor(Colours[iAirspaceColour[AATASK]]);
        // this color is the transparent bit
        TempSurface.SetBkColor(whitecolor);
        
        LKSurface & AliasSurface = TempSurface;
#else
        LKSurface & AliasSurface = Surface;
        Surface.SelectObject(LKBrush(LKColor(255U,255U,0U).WithAlpha(AlphaLevel)));
#endif
        for (i = maxTp - 1; i > std::max(0, ActiveTaskPoint - 1); i--) {
        if (ValidTaskPoint(i)) {
            int Type = 0;
            double Radius = 0.;
            GetTaskSectorParameter(i, &Type, &Radius);

            switch (Type) {
                case CONE:
                case CIRCLE:
                    tmp1 = Radius * zoom.ResScaleOverDistanceModify();
                    AliasSurface.Circle(
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y,
                            (int) tmp1, rc, true, true);
                    break;
                case SECTOR:
                    tmp1 = Radius * zoom.ResScaleOverDistanceModify();
                    AliasSurface.Segment(
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y, (int) tmp1, rc,
                            Task[i].AATStartRadial - DisplayAngle,
                            Task[i].AATFinishRadial - DisplayAngle);
                    break;
            }
        }
        }
#ifdef USE_GDI
        // restore original color
        TempSurface.SetTextColor(origcolor);
        TempSurface.SelectObject(oldpen);
        TempSurface.SelectObject(oldbrush);
        
        if(!Surface.AlphaBlend(rcDraw, TempSurface,rcDraw, AlphaLevel)) {
            // if AlphaBlend is not supported, use TransparentBld
            Surface.TransparentCopy(
                    rcDraw.left, rcDraw.top,
                    rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top,
                    TempSurface,
                    rcDraw.left, rcDraw.top);
        }
#endif
	}
    
    {
        UnlockTaskData();
    }
}
