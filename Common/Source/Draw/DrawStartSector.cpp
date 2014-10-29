/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKObjects.h"

extern LKColor taskcolor;

void MapWindow::DrawStartEndSector(LKSurface& Surface, const RECT& rc,
        const POINT &Start, const POINT &End, int Index,
        int Type, double Radius) {

    double tmp;
    LKPen oldpen;
    LKBrush oldbrush;

    switch (Type) {
        case 0: // CIRCLE
            tmp = Radius * zoom.ResScaleOverDistanceModify();
            oldpen = Surface.SelectObject(hpStartFinishThick);
            oldbrush = Surface.SelectObject(LKBrush_Hollow);
            Surface.Circle(WayPointList[Index].Screen.x,
                    WayPointList[Index].Screen.y, (int) tmp, rc, false, false);
            Surface.SelectObject(LKPen_Red_N1);
            Surface.Circle(WayPointList[Index].Screen.x,
                    WayPointList[Index].Screen.y, (int) tmp, rc, false, false);

            Surface.SelectObject(oldpen);
            Surface.SelectObject(oldbrush);
            break;
        case 1: // LINE
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(5), End, Start, taskcolor, rc);
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), End, Start, LKColor(255, 0, 0), rc);
            break;
        case 2: // SECTOR
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
                    Start, taskcolor, rc);
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
                    End, taskcolor, rc);
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
                    Start, LKColor(255, 0, 0), rc);
            Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
                    End, LKColor(255, 0, 0), rc);
            break;
    }

}
