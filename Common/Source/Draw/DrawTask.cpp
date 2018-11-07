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
#include "ScreenProjection.h"
#include "NavFunctions.h"
#include "Task/TaskRendererCircle.h"
#include "Task/TaskRendererSector.h"
#include "Task/TaskRendererDae.h"
#include "Task/TaskRendererLine.h"
#include "Task/TaskRendererMgr.h"
#include "ScreenGeometry.h"
#include "LKObjects.h"
#include "DrawFAIOpti.h"

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
const auto oldpen = Surface.SelectObject(hpStartFinishThin);
int finish=0;

while( ValidTaskPoint(finish))
 finish++;
finish--;

if(center_y < width)
  width = center_y-2;

POINT startfinishline[2] = {{0,-width},
                            {0,width}};

POINT track[] = {
    {0,-width/5},
    {width/2,0},
    {0,width/5},
    {0,-width/5}
};
if(TaskIdx == finish)
{
  track[0].x = -width/2; track[0].y= -width/5;
  track[1].x = 0 ; track[1].y= 0;
  track[2].x = -width/2 ; track[2].y= width/5;
  track[3] = track[0];
}

LockTaskData(); // protect from external task changes
double StartRadial = Task[TaskIdx].AATStartRadial;
double FinishRadial = Task[TaskIdx].AATFinishRadial;

double LineBrg;
double SecRadius;
GetTaskSectorParameter( TaskIdx, &SecType,&SecRadius);

    switch (SecType)
    {
        case ESS_CIRCLE:
        case CIRCLE:
            Surface.DrawCircle(center_x, center_y, width-2, true);
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
                Surface.DrawCircle(center_x, center_y, width/8, true);

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
            protateshift(startfinishline[0], LineBrg, center_x, center_y);
            protateshift(startfinishline[1], LineBrg, center_x, center_y);
            Surface.Polyline(startfinishline, 2);
            if ((TaskIdx == 0) || (TaskIdx == finish)) {
                for(POINT& pt : track) {
                    protateshift(pt, LineBrg, center_x, center_y);
                }
                Surface.Polygon(track, array_size(track));
            }
       break;
        case CONE:
            if (DoOptimizeRoute()) {

                int radius = width-2;
                Surface.DrawCircle(center_x, center_y, radius, true);
                const auto prevPen = Surface.SelectObject(LK_BLACK_PEN);
                for( int i = 1; i < 4 && radius > (width/5); ++i) {
                    Surface.DrawCircle(center_x, center_y, radius -= width/5, false);
                }
                Surface.SelectObject(prevPen);
            }
            break;
    }
UnlockTaskData();

Surface.SelectObject(oldpen);
Surface.SelectObject(oldbrush);
}




void MapWindow::DrawTask(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT &Orig_Aircraft) {

#ifdef HAVE_GLES
    typedef FloatPoint ScreenPoint;
#else
    typedef RasterPoint ScreenPoint;
#endif

    const GeoToScreen<ScreenPoint> ToScreen(_Proj);


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

    /**
     * Drawing Task :
     *   - Draw Task Sector Except Start Point
     *        (draw finish sector only after validate the first waypoint or if task is with only 2 tps)
     *   - AAT Task only : Draw Isoline of Active Turnpoint
     *   - Draw all Start Turnpoint only if Active Turmpoint is First.
     *   - Build Polyline connecting all turn point center
     *   - Build Line from current position to Active TurnPoint
     *   - if(AAT or optimized) {
     *          - Draw Thin Dash Polyline using polyline 
     *          - Draw Thin Dash line form current position to Active Turn point
     *          - replace Polyline by another one Connecting All AAT Target point
     *      }
     *   - Draw Multicolor DashLine using Polyline
     *   - Draw Arrow along multicolor DashLine
     */





	const auto oldpen = Surface.SelectObject(hpStartFinishThin);
	const auto oldbrush = Surface.SelectObject(LKBrush_Hollow);

	LockTaskData(); // protect from external task changes

    
    // Draw All Task Sector Except first
	for (int i = 1; ValidTaskPoint(i); i++) {
		const TaskRenderer* pItem = gTaskSectorRenderer.GetRenderer(i);
		if(pItem) {
			if (!ValidTaskPoint(i + 1)) { // final waypoint
				if (ActiveTaskPoint > 1 || !ValidTaskPoint(2)) {
					// only draw finish line when past the first
					// waypoint. FIXED 110307: or if task is with only 2 tps
					Surface.SelectObject(hpStartFinishThick);
					pItem->Draw(Surface, rc, false);

					Surface.SelectObject(LKPen_Red_N1);
					pItem->Draw(Surface, rc, false);
				}
			} else { // normal sector

				if(SectorType == LINE && !AATEnabled && ISGAAIRCRAFT) { // this Type exist only if not AAT task
					double rotation=AngleLimit360(Task[i].Bisector-DisplayAngle);
					const int length=IBLSCALE(14); //Make intermediate WP lines always of the same size independent by zoom level

                    const auto& wpt = WayPointList[Task[i].Index];

                    const ScreenPoint Center = ToScreen(wpt.Latitude, wpt.Longitude);
                    const ScreenPoint Start = {
                            static_cast<ScreenPoint::scalar_type>(Center.x + (length*fastsine(rotation))),
                            static_cast<ScreenPoint::scalar_type>(Center.y - (length*fastcosine(rotation)))
                    };
					rotation=Reciprocal(rotation);
                    const ScreenPoint End = {
                            static_cast<ScreenPoint::scalar_type>(Center.x + (length*fastsine(rotation))),
                            static_cast<ScreenPoint::scalar_type>(Center.y - (length*fastcosine(rotation)))
                    };
					Surface.DrawLine(PEN_SOLID, IBLSCALE(3), Start, End, taskcolor, rc);
				} else {
					Surface.SelectObject(hpStartFinishThin);
					pItem->Draw(Surface, rc, false);
				}
			}
		}
	}

    // Draw Iso Line
	if (AATEnabled && !DoOptimizeRoute()) {
		// ELSE HERE IS *** AAT ***
		// JMW added iso lines

		// JMW 20080616 flash arc line if very close to target
		static bool flip = false;
		if (DerivedDrawInfo.WaypointDistance < AATCloseDistance()*2.0) {
			flip = !flip;
		} else {
			flip = true;
		}

		if (flip) {
			if(ValidTaskPoint(ActiveTaskPoint)) {
				TASKSTATS_POINT& StatPt =  TaskStats[ActiveTaskPoint];
				for (int j = 0; j < MAXISOLINES - 1; j++) {
					if (StatPt.IsoLine_valid[j] && StatPt.IsoLine_valid[j + 1]) {
						Surface.DrawLine(PEN_SOLID, IBLSCALE(2),
									StatPt.IsoLine_Screen[j],
									StatPt.IsoLine_Screen[j + 1],
									LKColor(0, 0, 255), rc);
					}
				}
			}
			if ((mode.Is(Mode::MODE_TARGET_PAN) && ValidTaskPoint(TargetPanIndex))) {
				TASKSTATS_POINT& StatPt =  TaskStats[TargetPanIndex];
				for (int j = 0; j < MAXISOLINES - 1; j++) {
					if (StatPt.IsoLine_valid[j] && StatPt.IsoLine_valid[j + 1]) {
						Surface.DrawLine(PEN_SOLID, IBLSCALE(2),
									StatPt.IsoLine_Screen[j],
									StatPt.IsoLine_Screen[j + 1],
									LKColor(0, 0, 255), rc);
					}
				}
			}
		}
	}

    // Draw Start Turnpoint
	if ((ActiveTaskPoint < 2) && ValidTaskPoint(0) && ValidTaskPoint(1)) {

		const TaskRenderer* pTaskItem = gTaskSectorRenderer.GetRenderer(0);
		assert(pTaskItem);
		if(pTaskItem) {
			Surface.SelectObject(hpStartFinishThick);
			pTaskItem->Draw(Surface, rc, false);

			Surface.SelectObject(LKPen_Red_N1);
			pTaskItem->Draw(Surface, rc, false);
		}

		if (EnableMultipleStartPoints) {
			for (int i = 0; i < MAXSTARTPOINTS; i++) {
				if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
					const TaskRenderer* pStartItem = gStartSectorRenderer.GetRenderer(i);
					assert(pStartItem);
					if(pStartItem) {
						Surface.SelectObject(hpStartFinishThick);
						pStartItem->Draw(Surface, rc, false);

						Surface.SelectObject(LKPen_Red_N1);
						pStartItem->Draw(Surface, rc, false);
					}
				}
			}
		}
	}
    
    // Build Polyline connecting Center
    typedef std::vector<ScreenPoint> polyline_t;
    typedef std::array<ScreenPoint,2> line_t;

    polyline_t task_polyline; // make it static for save memory Alloc/Free ( don't forget to clear in this case )

    for(unsigned i = 0; ValidTaskPointFast(i); ++i ) {
        const WAYPOINT& wpt = WayPointList[Task[i].Index];
        task_polyline.push_back(ToScreen(wpt.Latitude, wpt.Longitude));
    }
    
    if(AATEnabled) {
        
#ifdef NO_DASH_LINES
        LKPen ThinPen(PEN_SOLID, ScreenThinSize, taskcolor);
        Surface.SelectObject(ThinPen);
        Surface.Polyline(task_polyline.data(), task_polyline.size(), rc);
#else
        Surface.DrawDashPoly(NIBLSCALE(1), taskcolor, task_polyline.data(), task_polyline.size(), rc);
#endif

        if((unsigned)ActiveTaskPoint < task_polyline.size()) {
            // Draw DashLine From current position to Active TurnPoint center
            const line_t to_next_center = {{(ToScreen(DrawInfo.Latitude, DrawInfo.Longitude)), (task_polyline[ActiveTaskPoint])}};

#ifdef NO_DASH_LINES
            Surface.Polyline(to_next_center.data(), to_next_center.size(), rc);

#else
            Surface.DrawDashPoly(NIBLSCALE(1), taskcolor, to_next_center.data(), to_next_center.size(), rc);
#endif
        }
        
        // replace Polyline by another one Connecting All AAT Target point 
        task_polyline.clear();
        for(unsigned i = 0; ValidTaskPointFast(i); ++i ) {
            const TASK_POINT& tpt = Task[i];
            task_polyline.push_back(ToScreen(tpt.AATTargetLat, tpt.AATTargetLon));
        }        
    }


#ifdef NO_DASH_LINES
    /**
     * This is faster way to draw Task polyline
     *  instead for have same look of V6.0 we use slow "DrawMulticolorDashLine"
     * TODO : use OpenGL Textured Line instead.
     */
     LKPen TaskPen(PEN_SOLID, size_tasklines, taskcolor);
     Surface.SelectObject(TaskPen);
     Surface.Polyline(task_polyline.data(), task_polyline.size(), rc);
#endif

    LKPen ArrowPen(PEN_SOLID, size_tasklines-IBLSCALE(1), taskcolor);
    LKBrush ArrowBrush(taskcolor);

    for (unsigned i = ActiveTaskPoint; i < task_polyline.size()-1; i++) {
        ScreenPoint sct1 = task_polyline[i];
        ScreenPoint sct2 = task_polyline[i+1];
                
        if(LKGeom::ClipLine(rc, sct1, sct2)) {
            // draw small arrow along task direction

            const RasterPoint Pt1(sct1.x, sct1.y);
            const RasterPoint Pt2(sct2.x, sct2.y);

#ifndef NO_DASH_LINES
            // TODO : remove after implement OpenGL Textured Line
            DrawMulticolorDashLine(Surface, size_tasklines, Pt1, Pt2, taskcolor, RGB_BLACK, rc);
#endif

            RasterPoint p_p;
            RasterPoint Arrow[] = {
                {8, 0},
                {-2, -5},
                {0, 0},
                {-2, 5},
                {8, 0}
            };
            
            const ScreenPoint Vect = sct2 - sct1;
            const double angle = (atan2(Vect.y,Vect.x)*RAD_TO_DEG);

            ScreenClosestPoint(Pt1, Pt2, Orig_Aircraft, &p_p, NIBLSCALE(25));
            PolygonRotateShift(Arrow, array_size(Arrow), p_p.x, p_p.y, angle);

            Surface.SelectObject(ArrowBrush);
            Surface.SelectObject(ArrowPen);

            Surface.Polygon(Arrow, array_size(Arrow), rc);
        }
    }

    UnlockTaskData();

    // restore original color
    Surface.SetTextColor(origcolor);
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);
}




void MapWindow::DrawTaskSectors(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) {
int Active = ActiveTaskPoint;
if(ValidTaskPoint(PanTaskEdit))
Active = PanTaskEdit;
static FAI_Sector FAI_TaskSectorCache[2];
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


float fZoom = MapWindow::zoom.RealScale() ;

double         fTic = 100;
if(fZoom > 50) fTic = 100; else
if(fZoom > 20) fTic = 50;  else
if(fZoom > 10) fTic = 25;  else fTic = 25;
if( DISTANCEMODIFY > 0.0)
  fTic =  fTic/ DISTANCEMODIFY;

double	lat1 = WayPointList[Task[a].Index].Latitude;
double	lon1 = WayPointList[Task[a].Index].Longitude;
double	lat2 = WayPointList[Task[b].Index].Latitude;
double	lon2 = WayPointList[Task[b].Index].Longitude;

FAI_TaskSectorCache[0].CalcSectorCache(lat1,  lon1,  lat2,  lon2, fTic, 0);
FAI_TaskSectorCache[0].DrawFAISector ( Surface, rc, _Proj, RGB_YELLOW );

FAI_TaskSectorCache[1].CalcSectorCache(lat1,  lon1,  lat2,  lon2, fTic, 1);
FAI_TaskSectorCache[1].DrawFAISector ( Surface, rc, _Proj, RGB_CYAN );


/*******************************************************************************************************/


}
