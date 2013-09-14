/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
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

extern int GetTaskSectorParameter(int TskIdx, int *SecType, double *SecRadius);
extern int RenderFAISector (HDC hdc, const RECT rc , double lat1, double lon1, double lat2, double lon2, int iOpposite , COLORREF fillcolor);
extern COLORREF taskcolor;

void  MapWindow::DrawTaskPicto(HDC hdc,int TaskIdx, RECT rc, double fScaleFact)
{
#ifdef PICTORIALS
int center_x = (rc.right-rc.left)/2;
int center_y = (rc.bottom-rc.top)/2;
int SecType = 	SectorType;
int width = center_x-2;
HPEN oldpen = 0;
HBRUSH oldbrush = 0;
if(AATEnabled)
  oldbrush = (HBRUSH) SelectObject(hdc, LKBrush_LightGrey);
else
  oldbrush = (HBRUSH) SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

oldpen = (HPEN) SelectObject(hdc, hpStartFinishThick);
int finish=0;

while( ValidTaskPoint(finish))
 finish++;
finish--;

if(center_y < width)
  width = center_y-2;

fScaleFact /= (2500.0);

width = (int)((double)width*(fScaleFact));


POINT startfinishline[2] = {{0,-width/2},
                            {0,+width/2}};

POINT track[3] = {{0,-width/10},
		          {width/4,0},
                  {0,width/10}};
if(TaskIdx == finish)
{
	track[0].x = -width/4 ; track[0].y= -width/10;
	track[1].x = 0        ; track[1].y= 0;
	track[2].x = -width/4 ; track[2].y= width/10;
}

LockTaskData(); // protect from external task changes
double StartRadial = Task[TaskIdx].AATStartRadial;
double FinishRadial = Task[TaskIdx].AATFinishRadial;


double SecRadius;
GetTaskSectorParameter( TaskIdx, &SecType,&SecRadius);

    switch (SecType)
    {
        case CIRCLE:
            Circle(hdc,
            		center_x,
            		center_y,
            		width-2, rc, true, true);
            break;
        case SECTOR:
            Segment(hdc,
            		center_x,
            		center_y, width, rc,
            		StartRadial,
            		FinishRadial);
            break;
        case DAe:
            if (!AATEnabled) { // this Type exist only if not AAT task
                // JMW added german rules
                Circle(hdc,
                		center_x,
                		center_y,
                		width/8, rc, false, true);

                Segment(hdc,
                		center_x,
                		center_y, width, rc,
                		StartRadial,
                		FinishRadial);
            }
            break;
       case LINE:
       default:
   	     PolygonRotateShift(startfinishline, 2,  center_x, center_y,  Task[TaskIdx].AATStartRadial);
   	   	 Polygon(hdc,startfinishline ,2 );
    	 if((TaskIdx == 0) || (TaskIdx == finish))
    	 {
    	   PolygonRotateShift(track, 3,  center_x, center_y,  Task[TaskIdx].AATStartRadial);
    	   Polygon(hdc,track ,3 );
    	 }
       break;
    }
UnlockTaskData();

SelectObject(hdc, oldpen);
SelectObject(hdc, oldbrush);
#endif
}




void MapWindow::DrawTask(HDC hdc, RECT rc, const POINT &Orig_Aircraft) {
    int i;
    double tmp;

    COLORREF whitecolor = RGB_WHITE;
    COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);
    HPEN oldpen = 0;
    HBRUSH oldbrush = 0;

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

    if (!WayPointList) return;

    oldpen = (HPEN) SelectObject(hdc, hpStartFinishThick);
    oldbrush = (HBRUSH) SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

    LockTaskData(); // protect from external task changes

    for (i = 1; ValidTaskPoint(i); i++) {
        if (!ValidTaskPoint(i + 1)) { // final waypoint
            if (ActiveWayPoint > 1 || !ValidTaskPoint(2)) {
                // only draw finish line when past the first
                // waypoint. FIXED 110307: or if task is with only 2 tps
                DrawStartEndSector(hdc, rc, Task[i].Start, Task[i].End, Task[i].Index, FinishLine, FinishRadius);
            }
        } else { // normal sector
            if (AATEnabled != TRUE) {
                //_DrawLine(hdc, PS_DASH, NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].Start, RGB_PETROL, rc);
                //_DrawLine(hdc, PS_DASH, NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].End, RGB_PETROL, rc);
         //       DrawDashLine(hdc,  size_tasklines, WayPointList[Task[i].Index].Screen, Task[i].Start, RGB_PETROL, rc);
         //       DrawDashLine(hdc,  size_tasklines, WayPointList[Task[i].Index].Screen, Task[i].End, RGB_PETROL, rc);
            }

            int Type = SectorType;
            double Radius = SectorRadius;
            if (AATEnabled) {
                Type = Task[i].AATType;
                Radius = (Type==CIRCLE) ? Task[i].AATCircleRadius : Task[i].AATSectorRadius;
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
            DrawDashLine(hdc, NIBLSCALE(1), // 091217
                    WayPointList[imin].Screen,
                    WayPointList[imax].Screen,
                    taskcolor, rc);
        } else {
            sct1 = WayPointList[imin].Screen;
            sct2 = WayPointList[imax].Screen;
        }

        if ((i >= ActiveWayPoint && DoOptimizeRoute()) || !DoOptimizeRoute()) {
            POINT ClipPt1 = sct1, ClipPt2 = sct2;
            if(LKGeom::ClipLine((POINT) {rc.left, rc.top}, (POINT) {rc.right, rc.bottom}, ClipPt1, ClipPt2)) {
                DrawMulticolorDashLine(hdc, size_tasklines,
                        sct1,
                        sct2,
                        taskcolor, RGB_BLACK,rc);
                
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

                _DrawLine(hdc, PS_SOLID, size_tasklines-NIBLSCALE(1), Arrow[0], p_p, taskcolor, rc);
                _DrawLine(hdc, PS_SOLID, size_tasklines-NIBLSCALE(1), Arrow[1], p_p, taskcolor, rc);
            }
        }
    }
    
    // Draw DashLine From current position to Active TurnPoint center
    if(ValidTaskPoint(ActiveWayPoint)) {
        POINT ptStart;
        LatLon2Screen(DrawInfo.Longitude, DrawInfo.Latitude, ptStart);
        DrawDashLine(hdc, NIBLSCALE(1),
                    ptStart,
                    WayPointList[Task[ActiveWayPoint].Index].Screen,
                    taskcolor, rc);        

    }

    {
        UnlockTaskData();
    }

    // restore original color
    SetTextColor(hDCTemp, origcolor);
    SelectObject(hdc, oldpen);
    SelectObject(hdc, oldbrush);
}




void MapWindow::DrawTaskSectors(HDC hdc, RECT rc) {
int Active =  ActiveWayPoint;
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

RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, 1, RGB_YELLOW );
RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, 0, RGB_CYAN );



/*******************************************************************************************************/


}
