/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "AATDistance.h"
#include "RGB.h"


extern COLORREF taskcolor;

void MapWindow::DrawTask(HDC hdc, RECT rc, const POINT &Orig_Aircraft) {
    int i;
    double tmp;

    COLORREF whitecolor = RGB_WHITE;
    COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);
    HPEN oldpen = 0;
    HBRUSH oldbrush = 0;

    if (!WayPointList) return;

    oldpen = (HPEN) SelectObject(hdc, hpStartFinishThick);
    oldbrush = (HBRUSH) SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

    LockTaskData(); // protect from external task changes

    if ((ActiveWayPoint < 2) && ValidTaskPoint(0) && ValidTaskPoint(1)) {
        DrawStartEndSector(hdc, rc, Task[0].Start, Task[0].End, Task[0].Index, StartLine, StartRadius);
        if (EnableMultipleStartPoints) {
            for (i = 0; i < MAXSTARTPOINTS; i++) {
                if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
                    DrawStartEndSector(hdc, rc, StartPoints[i].Start, StartPoints[i].End,
                            StartPoints[i].Index, StartLine, StartRadius);
                }
            }
        }
    }

    for (i = 1; ValidTaskPoint(i); i++) {
        if (!ValidTaskPoint(i + 1)) { // final waypoint
            if (ActiveWayPoint > 1 || !ValidTaskPoint(2)) {
                // only draw finish line when past the first
                // waypoint. FIXED 110307: or if task is with only 2 tps
                DrawStartEndSector(hdc, rc, Task[i].Start, Task[i].End, Task[i].Index, FinishLine, FinishRadius);
            }
        } else { // normal sector
            if (AATEnabled != TRUE) {
                _DrawLine(hdc, PS_DASH, NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].Start, RGB_PETROL, rc);
                _DrawLine(hdc, PS_DASH, NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].End, RGB_PETROL, rc);
            }

            int Type = SectorType;
            double Radius = SectorRadius;
            if (AATEnabled) {
                Type = Task[i].AATType;
                Radius = Task[i].AATCircleRadius;
            }

            switch (Type) {
                case CIRCLE:
                    tmp = Radius * zoom.ResScaleOverDistanceModify();
                    Circle(hdc,
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y,
                            (int) tmp, rc, false, false);
                    break;
                case SECTOR:
                    tmp = Radius * zoom.ResScaleOverDistanceModify();
                    Segment(hdc,
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y, (int) tmp, rc,
                            Task[i].AATStartRadial - DisplayAngle,
                            Task[i].AATFinishRadial - DisplayAngle);
                    break;
                case 2:
                    if (!AATEnabled) { // this Type exist only if not AAT task
                        // JMW added german rules
                        tmp = 500 * zoom.ResScaleOverDistanceModify();
                        Circle(hdc,
                                WayPointList[Task[i].Index].Screen.x,
                                WayPointList[Task[i].Index].Screen.y,
                                (int) tmp, rc, false, false);

                        tmp = 10e3 * zoom.ResScaleOverDistanceModify();

                        Segment(hdc,
                                WayPointList[Task[i].Index].Screen.x,
                                WayPointList[Task[i].Index].Screen.y, (int) tmp, rc,
                                Task[i].AATStartRadial - DisplayAngle,
                                Task[i].AATFinishRadial - DisplayAngle);
                    }
                    break;
            }

            if (AATEnabled && !DoOptimizeRoute()) {
                // ELSE HERE IS   *** AAT ***
                // JMW added iso lines
                if ((i == ActiveWayPoint) || (mode.Is(Mode::MODE_TARGET_PAN) && (i == TargetPanIndex))) {
                    // JMW 20080616 flash arc line if very close to target
                    static bool flip = false;

                    if (DerivedDrawInfo.WaypointDistance < AATCloseDistance()*2.0) {
                        flip = !flip;
                    } else {
                        flip = true;
                    }
                    if (flip) {
                        for (int j = 0; j < MAXISOLINES - 1; j++) {
                            if (TaskStats[i].IsoLine_valid[j]
                                    && TaskStats[i].IsoLine_valid[j + 1]) {
                                _DrawLine(hdc, PS_SOLID, NIBLSCALE(2),
                                        TaskStats[i].IsoLine_Screen[j],
                                        TaskStats[i].IsoLine_Screen[j + 1],
                                        RGB(0, 0, 255), rc);
                            }
                        }
                    }
                }
            }
        }
    }

    for (i = 0; ValidTaskPoint(i + 1); i++) {
        bool is_first = (Task[i].Index < Task[i + 1].Index);
        int imin = min(Task[i].Index, Task[i + 1].Index);
        int imax = max(Task[i].Index, Task[i + 1].Index);
        // JMW AAT!
        double bearing = Task[i].OutBound;
        POINT sct1, sct2;
        if (AATEnabled) {
            LatLon2Screen(Task[i].AATTargetLon,
                    Task[i].AATTargetLat,
                    sct1);
            LatLon2Screen(Task[i + 1].AATTargetLon,
                    Task[i + 1].AATTargetLat,
                    sct2);
            DistanceBearing(Task[i].AATTargetLat,
                    Task[i].AATTargetLon,
                    Task[i + 1].AATTargetLat,
                    Task[i + 1].AATTargetLon,
                    NULL, &bearing);

            // draw nominal track line
            DrawDashLine(hdc, NIBLSCALE(1), // 091217
                    WayPointList[imin].Screen,
                    WayPointList[imax].Screen,
                    taskcolor, rc);
        } else {
            sct1 = WayPointList[Task[i].Index].Screen;
            sct2 = WayPointList[Task[i + 1].Index].Screen;
        }

        if ((i >= ActiveWayPoint && DoOptimizeRoute()) || !DoOptimizeRoute()) {
            if (is_first) {
                DrawDashLine(hdc, NIBLSCALE(3),
                        sct1,
                        sct2,
                        taskcolor, rc);
            } else {
                DrawDashLine(hdc, NIBLSCALE(3),
                        sct2,
                        sct1,
                        taskcolor, rc);
            }

            // draw small arrow along task direction
            POINT p_p;
            POINT Arrow[2] = {
                {6, 6},
                {-6, 6}
            };
            ScreenClosestPoint(sct1, sct2,
                    Orig_Aircraft, &p_p, NIBLSCALE(25));
            PolygonRotateShift(Arrow, 2, p_p.x, p_p.y,
                    bearing - DisplayAngle);

            _DrawLine(hdc, PS_SOLID, NIBLSCALE(2), Arrow[0], p_p, taskcolor, rc);
            _DrawLine(hdc, PS_SOLID, NIBLSCALE(2), Arrow[1], p_p, taskcolor, rc);
        }
    }

    {
        UnlockTaskData();
    }

    // restore original color
    SetTextColor(hDCTemp, origcolor);
    SelectObject(hdc, oldpen);
    SelectObject(hdc, oldbrush);
}
