/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "RGB.h"
#include "LKObjects.h"
#include "Task/TaskRendererMgr.h"

#ifdef HAVE_HATCHED_BRUSH
    constexpr uint8_t AlphaLevel = 255*35/100;
#else
    constexpr uint8_t AlphaLevel = 255*17/100;
#endif


void MapWindow::DrawTaskAAT(LKSurface& Surface, const RECT& rc) {

    if (WayPointList.empty()) return;
    if (!UseAATTarget()) return;

    LockTaskData(); // protect from external task changes
#ifdef USE_GDI
    /**********************************************/
    /* Check if not Validated Waypoint is visible */
    bool bDraw = false;
    int maxTp = 1;
    RECT rcDraw = (RECT){rc.right, rc.bottom, rc.left, rc.top};

    for (maxTp = std::max(1, ActiveTaskPoint); ValidTaskPoint(maxTp + 1); ++maxTp) {
        if (ValidTaskPoint(maxTp)) {

					const TaskRenderer* pItem = gTaskSectorRenderer.GetRenderer(maxTp);
					if(pItem) {
						PixelRect rect = pItem->GetScreenBounds();

						if (CheckRectOverlap(&rect, &rc)) {
							rcDraw.top = std::min(rect.top, rcDraw.top);
							rcDraw.bottom = std::max(rect.bottom, rcDraw.bottom);
							rcDraw.left = std::min(rect.left, rcDraw.left);
							rcDraw.right = std::max(rect.right, rcDraw.right);
							bDraw = true;
						}
					}
        }
    }
    /**********************************************/
#else
    const int maxTp = MAXTASKPOINTS;
#endif

#ifdef USE_GDI
    if (bDraw) { // Draw Only if one is Visible
        rcDraw.top = std::max(rc.top, rcDraw.top);
        rcDraw.bottom = std::min(rc.bottom, rcDraw.bottom);
        rcDraw.left = std::max(rc.left, rcDraw.left);
        rcDraw.right = std::min(rc.right, rcDraw.right);

        LKColor origcolor = TempSurface.SetTextColor(RGB_WHITE);

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
        TempSurface.SetBkColor(RGB_WHITE);

        LKSurface & AliasSurface = TempSurface;
#else
        LKSurface & AliasSurface = Surface;
        Surface.SelectObject(LKBrush(LKColor(255U,255U,0U).WithAlpha(AlphaLevel)));
#endif
        for (int i = maxTp - 1; i > std::max(0, ActiveTaskPoint - 1); i--) {
					if (ValidTaskPoint(i)) {
						const TaskRenderer* pItem = gTaskSectorRenderer.GetRenderer(i);
						if(pItem) {
							pItem->Draw(AliasSurface, rc, true);
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
    }
#endif

    UnlockTaskData();
}
