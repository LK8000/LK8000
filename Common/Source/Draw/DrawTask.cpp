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



void MapWindow::DrawTask(HDC hdc, RECT rc, const POINT &Orig_Aircraft)
{
  int i;
  double tmp;

  COLORREF whitecolor = RGB_WHITE;
  COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);
  HPEN oldpen=0;

  if (!WayPointList) return;

  oldpen=(HPEN)SelectObject(hdc, hpStartFinishThick);

  LockTaskData();  // protect from external task changes

    if(ValidTaskPoint(0) && ValidTaskPoint(1) && (ActiveWayPoint<2))
      {
	DrawStartSector(hdc,rc, Task[0].Start, Task[0].End, Task[0].Index);
	if (EnableMultipleStartPoints) {
	  for (i=0; i<MAXSTARTPOINTS; i++) {
	    if (StartPoints[i].Active && ValidWayPoint(StartPoints[i].Index)) {
	      DrawStartSector(hdc,rc, 
			      StartPoints[i].Start, 
			      StartPoints[i].End, StartPoints[i].Index);
	    }
	  }
	}
      }
  
    for(i=1;i<MAXTASKPOINTS-1;i++) {

      if(ValidTaskPoint(i) && !ValidTaskPoint(i+1)) { // final waypoint
	if (ActiveWayPoint>1 || !ValidTaskPoint(2)) { 
	  // only draw finish line when past the first
	  // waypoint. FIXED 110307: or if task is with only 2 tps
	  if(FinishLine) {
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(3), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].Start, taskcolor, rc);
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(3), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].End, taskcolor, rc);
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].Start, RGB(255,0,0), rc);
	    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), 
		      WayPointList[Task[i].Index].Screen,
		      Task[i].End, RGB(255,0,0), rc);
	  } else {
	    tmp = FinishRadius*zoom.ResScaleOverDistanceModify(); 
	    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	    SelectObject(hdc, hpStartFinishThick);
	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 
	    SelectObject(hdc, hpStartFinishThin);
	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 
	  }        
	}
      } // final waypoint
      // DRAW TASK SECTORS
      if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) { // normal sector
	if(AATEnabled != TRUE) {
	  _DrawLine(hdc, PS_DASH,NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].Start, RGB_PETROL, rc);
	  _DrawLine(hdc, PS_DASH,NIBLSCALE(3), WayPointList[Task[i].Index].Screen, Task[i].End, RGB_PETROL, rc);

	  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH)); 
	  SelectObject(hdc, hpStartFinishThick);
	  if(SectorType== 0) {
	    tmp = SectorRadius*zoom.ResScaleOverDistanceModify();

	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 

	  }
	  // FAI SECTOR
	  if(SectorType==1) {
	    tmp = SectorRadius*zoom.ResScaleOverDistanceModify();

	    Segment(hdc,
		    WayPointList[Task[i].Index].Screen.x,
		    WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		    Task[i].AATStartRadial-DisplayAngle, 
		    Task[i].AATFinishRadial-DisplayAngle); 

	  }
	  if(SectorType== 2) {
	    // JMW added german rules
	    tmp = 500*zoom.ResScaleOverDistanceModify();
	    Circle(hdc,
		   WayPointList[Task[i].Index].Screen.x,
		   WayPointList[Task[i].Index].Screen.y,
		   (int)tmp, rc, false, false); 

	    tmp = 10e3*zoom.ResScaleOverDistanceModify();
          
	    Segment(hdc,
		    WayPointList[Task[i].Index].Screen.x,
		    WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		    Task[i].AATStartRadial-DisplayAngle, 
		    Task[i].AATFinishRadial-DisplayAngle); 

	  }
	} else {
		// ELSE HERE IS   *** AAT ***
	  // JMW added iso lines
	  if ((i==ActiveWayPoint) || (mode.Is(Mode::MODE_TARGET_PAN) && (i==TargetPanIndex))) {
	    // JMW 20080616 flash arc line if very close to target
	    static bool flip = false;
	  
	    if (DerivedDrawInfo.WaypointDistance<AATCloseDistance()*2.0) {
	      flip = !flip;
	    } else {
	      flip = true;
	    }
	    if (flip) {
	      for (int j=0; j<MAXISOLINES-1; j++) {
		if (TaskStats[i].IsoLine_valid[j] 
		    && TaskStats[i].IsoLine_valid[j+1]) {
		  _DrawLine(hdc, PS_SOLID, NIBLSCALE(2), 
			    TaskStats[i].IsoLine_Screen[j], 
			    TaskStats[i].IsoLine_Screen[j+1],
			    RGB(0,0,255), rc);
		}
	      }
	    }
	  }
	}
      }
    }

    for(i=0;i<MAXTASKPOINTS-1;i++) {
      if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
	bool is_first = (Task[i].Index < Task[i+1].Index);
	int imin = min(Task[i].Index,Task[i+1].Index);
	int imax = max(Task[i].Index,Task[i+1].Index);
	// JMW AAT!
	double bearing = Task[i].OutBound;
	POINT sct1, sct2;
	if (AATEnabled) {
	  LatLon2Screen(Task[i].AATTargetLon, 
			Task[i].AATTargetLat, 
			sct1);
	  LatLon2Screen(Task[i+1].AATTargetLon, 
			Task[i+1].AATTargetLat, 
			sct2);
	  DistanceBearing(Task[i].AATTargetLat,
			  Task[i].AATTargetLon,
			  Task[i+1].AATTargetLat,
			  Task[i+1].AATTargetLon,
			  NULL, &bearing);

	  // draw nominal track line
	  DrawDashLine(hdc, NIBLSCALE(1),   // 091217
		       WayPointList[imin].Screen, 
		       WayPointList[imax].Screen, 
		       taskcolor, rc);
	} else {
	  sct1 = WayPointList[Task[i].Index].Screen;
	  sct2 = WayPointList[Task[i+1].Index].Screen;
	}

	if( (DoOptimizeRoute() && i >= ActiveWayPoint) || !DoOptimizeRoute() ){
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
		POINT Arrow[2] = { {6,6}, {-6,6} };
		ScreenClosestPoint(sct1, sct2, 
				   Orig_Aircraft, &p_p, NIBLSCALE(25));
		PolygonRotateShift(Arrow, 2, p_p.x, p_p.y, 
				   bearing-DisplayAngle);

		_DrawLine(hdc, PS_SOLID, NIBLSCALE(2), Arrow[0], p_p, taskcolor, rc);
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(2), Arrow[1], p_p, taskcolor, rc);
	}
      }
    }
     {
       UnlockTaskData();
     }

  // restore original color
  SetTextColor(hDCTemp, origcolor);
  SelectObject(hdc, oldpen);

}


