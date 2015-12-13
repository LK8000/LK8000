/*
LK8000 Tactical Flight Computer - WWW.LK8000.IT
Released under GNU/GPL License v.2
See CREDITS.TXT file for authors and copyrights

$Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "AATDistance.h"
#include "DoInits.h"
#include "RGB.h"
#include "LKObjects.h"
#include "utils/2dpclip.h"
#include "CriticalSection.h"

extern int RenderFAISector (LKSurface& Surface, const RECT& rc , double lat1, double lon1, double lat2, double lon2, int iOpposite , const LKColor& fillcolor);
extern LKColor taskcolor;

//
// THIS FUNCTION IS THREAD SAFE, but not using optimized clipping
//
void MapWindow::DrawTaskPicto(LKSurface& Surface,int TaskIdx, const RECT& rc, double fScaleFact)
{
int center_x = (rc.right-rc.left)/2;
int center_y = (rc.bottom-rc.top)/2;
int SecType = SectorType;
int width = center_x-2;
const auto oldbrush = Surface.SelectObject(AATEnabled
                                           ? LKBrush_LightGrey
                                           : LKBrush_Hollow);
const auto oldpen = Surface.SelectObject(hpStartFinishThick);
int finish=0;

while( ValidTaskPoint(finish))
 finish++;
finish--;

if(center_y < width)
  width = center_y-2;

POINT startfinishline[2] = {{0,-width/ScreenScale},
                            {0,width/ScreenScale}};

POINT track[] = {
    {0,-width/5/ScreenScale},
    {width/2/ScreenScale,0},
    {0,width/5/ScreenScale},
    {0,-width/5/ScreenScale}
};
if(TaskIdx == finish)
{
  track[0].x = -width/2/ScreenScale; track[0].y= -width/5/ScreenScale;
  track[1].x = 0 ; track[1].y= 0;
  track[2].x = -width/2/ScreenScale ; track[2].y= width/5/ScreenScale;
  track[3] = track[0];
}

LockTaskData(); // protect from external task changes
double StartRadial = Task[TaskIdx].AATStartRadial;
double FinishRadial = Task[TaskIdx].AATFinishRadial;

if(TaskIdx==0)
{
  FinishRadial = Task[TaskIdx].AATStartRadial;
  StartRadial = Task[TaskIdx].AATFinishRadial;
}

double LineBrg;
double SecRadius;
GetTaskSectorParameter( TaskIdx, &SecType,&SecRadius);

    switch (SecType)
    {
        case CIRCLE:
            Surface.CircleNoCliping(
             center_x,
             center_y,
             width-2, rc, true);
            break;
        case SECTOR:
            Surface.Segment(
             center_x,
             center_y, width, rc,
             StartRadial,
             FinishRadial);
            break;
        case DAe:
            if (!AATEnabled) { // this Type exist only if not AAT task
                // JMW added german rules
                Surface.CircleNoCliping(
                 center_x,
                 center_y,
                 width/8, rc, true);

                Surface.Segment(
                 center_x,
                 center_y, width, rc,
                 StartRadial,
                 FinishRadial);
            }
            break;
       default:
       case LINE:
            if (TaskIdx == 0) {
                LineBrg = Task[TaskIdx].OutBound-90;
            } else if (TaskIdx == finish) {
                LineBrg = Task[TaskIdx].InBound-90;
            } else {
                LineBrg = Task[TaskIdx].Bisector;
            }
            threadsafePolygonRotateShift(startfinishline, 2, center_x, center_y, LineBrg);
            Surface.Polyline(startfinishline, 2);
            if ((TaskIdx == 0) || (TaskIdx == finish)) {
                threadsafePolygonRotateShift(track, array_size(track), center_x, center_y, LineBrg);
                Surface.Polygon(track, array_size(track));
            }
       break;
        case CONE:
            if (DoOptimizeRoute()) {

                int radius = width-2;
                Surface.CircleNoCliping(center_x, center_y, radius, rc, true);
                const auto prevPen = Surface.SelectObject(LK_BLACK_PEN);
                for( int i = 1; i < 4 && radius > (width/5); ++i) {
                    Surface.CircleNoCliping(center_x, center_y, radius -= width/5, rc, true);
                }
                Surface.SelectObject(prevPen);
            }
            break;
    }
UnlockTaskData();

Surface.SelectObject(oldpen);
Surface.SelectObject(oldbrush);
}




void MapWindow::DrawTask(LKSurface& Surface, const RECT& rc, const POINT &Orig_Aircraft) {
    int i;
    double tmp;

    LKColor whitecolor = RGB_WHITE;
    LKColor origcolor = Surface.SetTextColor(whitecolor);

    static short size_tasklines=0;

    if (DoInit[MDI_DRAWTASK]) {
switch (ScreenSize) {
case ss480x272:
case ss272x480:
case ss320x240:
case ss240x320:
size_tasklines=NIBLSCALE(4);
break;
default:
size_tasklines=NIBLSCALE(3);
break;
}
DoInit[MDI_DRAWTASK]=false;
    }

    if (WayPointList.empty()) return;

    const auto oldpen = Surface.SelectObject(hpStartFinishThick);
    const auto oldbrush = Surface.SelectObject(LKBrush_Hollow);

    LockTaskData(); // protect from external task changes

    for (i = 1; ValidTaskPoint(i); i++) {
        if (!ValidTaskPoint(i + 1)) { // final waypoint
            if (ActiveTaskPoint > 1 || !ValidTaskPoint(2)) {
                // only draw finish line when past the first
                // waypoint. FIXED 110307: or if task is with only 2 tps
                DrawStartEndSector(Surface, rc, Task[i].Start, Task[i].End, Task[i].Index, FinishLine, FinishRadius);
            }
        } else { // normal sector
            if (AATEnabled != TRUE) {
                //Surface.DrawLine(PEN_DASH, NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].Start, RGB_PETROL, rc);
                //Surface.DrawLine(PEN_DASH, NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].End, RGB_PETROL, rc);
         // DrawDashLine(hdc, size_tasklines, WayPointList[Task[i].Index].Screen, Task[i].Start, RGB_PETROL, rc);
         // DrawDashLine(hdc, size_tasklines, WayPointList[Task[i].Index].Screen, Task[i].End, RGB_PETROL, rc);
            }

            int Type = 0;
            double Radius = 0.;
            GetTaskSectorParameter(i, &Type, &Radius);
            switch (Type) {
                case CIRCLE:
                    tmp = Radius * zoom.ResScaleOverDistanceModify();
                    Surface.Circle(
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y,
                            (int) tmp, rc, false, false);
                    break;
                case SECTOR:
                    tmp = Radius * zoom.ResScaleOverDistanceModify();
                    Surface.Segment(
                            WayPointList[Task[i].Index].Screen.x,
                            WayPointList[Task[i].Index].Screen.y, (int) tmp, rc,
                            Task[i].AATStartRadial - DisplayAngle,
                            Task[i].AATFinishRadial - DisplayAngle);
                    break;
                case DAe:
                    if (!AATEnabled) { // this Type exist only if not AAT task
                        // JMW added german rules
                        tmp = 500 * zoom.ResScaleOverDistanceModify();
                        Surface.Circle(
                                WayPointList[Task[i].Index].Screen.x,
                                WayPointList[Task[i].Index].Screen.y,
                                (int) tmp, rc, false, false);

                        tmp = 10e3 * zoom.ResScaleOverDistanceModify();

                        Surface.Segment(
                                WayPointList[Task[i].Index].Screen.x,
                                WayPointList[Task[i].Index].Screen.y, (int) tmp, rc,
                                Task[i].AATStartRadial - DisplayAngle,
                                Task[i].AATFinishRadial - DisplayAngle);
                    }
                    break;
                case LINE:
                    if (!AATEnabled) { // this Type exist only if not AAT task
                    	if(ISGAAIRCRAFT) {
                    		POINT start,end;
                    		double rotation=AngleLimit360(Task[i].Bisector-DisplayAngle);
                    		int length=14*ScreenScale; //Make intermediate WP lines always of the same size independent by zoom level
                    		start.x=WayPointList[Task[i].Index].Screen.x+(long)(length*fastsine(rotation));
                    		start.y=WayPointList[Task[i].Index].Screen.y-(long)(length*fastcosine(rotation));
                    		rotation=Reciprocal(rotation);
                    		end.x=WayPointList[Task[i].Index].Screen.x+(long)(length*fastsine(rotation));
                    		end.y=WayPointList[Task[i].Index].Screen.y-(long)(length*fastcosine(rotation));
                    		Surface.DrawLine(PEN_SOLID, NIBLSCALE(3), start, end, taskcolor, rc);
                    	} else Surface.DrawLine(PEN_SOLID, NIBLSCALE(3), Task[i].Start, Task[i].End, taskcolor, rc);
                    }
                    break;
                case CONE:
                    tmp = Radius * zoom.ResScaleOverDistanceModify();
                    int center_x = WayPointList[Task[i].Index].Screen.x;
                    int center_y = WayPointList[Task[i].Index].Screen.y;
                    Surface.Circle(center_x, center_y, (int) tmp, rc, false, false);
                    const auto prevPen = Surface.SelectObject(hpTerrainLine);
                    for( int j = 1; j < 5 && tmp > 0; ++j) {
                        Surface.Circle(center_x, center_y, tmp -= NIBLSCALE(5), rc, true, true);
                    }
                    Surface.SelectObject(prevPen);
                    break;
            }

            if (AATEnabled && !DoOptimizeRoute()) {
                // ELSE HERE IS *** AAT ***
                // JMW added iso lines
                if ((i == ActiveTaskPoint) || (mode.Is(Mode::MODE_TARGET_PAN) && (i == TargetPanIndex))) {
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
                                Surface.DrawLine(PEN_SOLID, NIBLSCALE(2),
                                        TaskStats[i].IsoLine_Screen[j],
                                        TaskStats[i].IsoLine_Screen[j + 1],
                                        LKColor(0, 0, 255), rc);
                            }
                        }
                    }
                }
            }
        }
    }

    if ((ActiveTaskPoint < 2) && ValidTaskPoint(0) && ValidTaskPoint(1)) {
        DrawStartEndSector(Surface, rc, Task[0].Start, Task[0].End, Task[0].Index, StartLine, StartRadius);
        if (EnableMultipleStartPoints) {
            for (i = 0; i < MAXSTARTPOINTS; i++) {
                if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
                    DrawStartEndSector(Surface, rc, StartPoints[i].Start, StartPoints[i].End,
                            StartPoints[i].Index, StartLine, StartRadius);
                }
            }
        }
    }
    
    LKPen ArrowPen(PEN_SOLID, size_tasklines-NIBLSCALE(1), taskcolor);
    for (i = 0; ValidTaskPoint(i + 1); i++) {
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
            Surface.DrawDashLine(NIBLSCALE(1), // 091217
                    WayPointList[imin].Screen,
                    WayPointList[imax].Screen,
                    taskcolor, rc);
        } else {
            sct1 = WayPointList[imin].Screen;
            sct2 = WayPointList[imax].Screen;
        }

        if ((i >= ActiveTaskPoint && DoOptimizeRoute()) || !DoOptimizeRoute()) {
            POINT ClipPt1 = sct1, ClipPt2 = sct2;
            if(LKGeom::ClipLine(rc, ClipPt1, ClipPt2)) {
                DrawMulticolorDashLine(Surface, size_tasklines,
                        ClipPt1,
                        ClipPt2,
                        taskcolor, RGB_BLACK,rc);
                
                // draw small arrow along task direction
                POINT p_p;
                POINT Arrow[] = {
                    {6, 6},
                    {0, 0},
                    {-6, 6}
                };
                ScreenClosestPoint(sct1, sct2, Orig_Aircraft, &p_p, NIBLSCALE(25));
                PolygonRotateShift(Arrow, array_size(Arrow), p_p.x, p_p.y, bearing - DisplayAngle);

                const auto OldPen = Surface.SelectObject(ArrowPen);
                Surface.Polyline(Arrow, array_size(Arrow), rc);
                Surface.SelectObject(OldPen);                
            }
        }
    }
    
    // Draw DashLine From current position to Active TurnPoint center
    if(ValidTaskPoint(ActiveTaskPoint)) {
        POINT ptStart;
        LatLon2Screen(DrawInfo.Longitude, DrawInfo.Latitude, ptStart);
        Surface.DrawDashLine(NIBLSCALE(1),
                    ptStart,
                    WayPointList[Task[ActiveTaskPoint].Index].Screen,
                    taskcolor, rc);

    }

    {
        UnlockTaskData();
    }

    // restore original color
    Surface.SetTextColor(origcolor);
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);
}




void MapWindow::DrawTaskSectors(LKSurface& Surface, const RECT& rc) {
int Active = ActiveTaskPoint;
if(ValidTaskPoint(PanTaskEdit))
Active = PanTaskEdit;

CScopeLock LockTask(LockTaskData, UnlockTaskData);

    /*******************************************************************************************************/
int TaskPoints =0;
while(ValidTaskPoint(TaskPoints))
TaskPoints++;
if(TaskPoints < 2)
return;
if(TaskPoints > 5)
return;
int a=0, b=1;

if(TaskPoints ==3)
{
  switch (Active)
  {
    case 0: a = 0; b = 1; break;
    case 1: a = 0; b = 1; break;
    case 2: a = 1; b = 2; break;
  }
}

if(TaskPoints ==4)
{
  switch (Active)
  {
    case 0: a = 1; b = 2; break;
    case 1: a = 2; b = 0; break;
    case 2: a = 0; b = 1; break;
    case 3: a = 1; b = 2; break;
  }
}

if(TaskPoints ==5)
{
  switch (Active)
  {
    case 0: a = 3; b = 1; break;
    case 1: a = 2; b = 3; break;
    case 2: a = 3; b = 1; break;
    case 3: a = 1; b = 2; break;
    case 4: a = 3; b = 1; break;
  }
}



double	lat1 = WayPointList[Task[a].Index].Latitude;
double	lon1 = WayPointList[Task[a].Index].Longitude;
double	lat2 = WayPointList[Task[b].Index].Latitude;
double	lon2 = WayPointList[Task[b].Index].Longitude;

RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, 1, RGB_YELLOW );
RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, 0, RGB_CYAN );



/*******************************************************************************************************/


}
