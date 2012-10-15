/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "RGB.h"


void MapWindow::DrawTaskAAT(HDC hdc, const RECT rc)
{
  int i;
  double tmp;

  if (!WayPointList) return;
  if (!AATEnabled) return;

  HPEN oldpen=0;
  HBRUSH oldbrush=0;
  HBITMAP oldbitmap=0;
  
  LockTaskData();  // protect from external task changes
  /**********************************************/
  /* Check if not Validated Waypoint is visible */
  bool bDraw = false;
  int maxTp = 1;
  rectObj rcrect = (rectObj){(double)rc.left,(double)rc.top,(double)rc.right,(double)rc.bottom};
  
  for(maxTp = std::max(1, ActiveWayPoint); ValidTaskPoint(maxTp+1); ++maxTp) {
      if(!bDraw) {
        double tmp = Task[maxTp].AATCircleRadius*zoom.ResScaleOverDistanceModify();

        LONG x = WayPointList[Task[maxTp].Index].Screen.x;
        LONG y = WayPointList[Task[maxTp].Index].Screen.y;
        rectObj rect = (rectObj){x-tmp, y-tmp, x+tmp, y+tmp};

        if (msRectOverlap(&rect, &rcrect) == MS_TRUE) {
            bDraw=true;
        }
      }
  }
  /**********************************************/

  if(bDraw) { // Draw Only if one is Visible

    COLORREF whitecolor = RGB_WHITE;
    COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);

    oldbitmap = (HBITMAP)SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);

    oldpen=(HPEN)SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
    oldbrush=(HBRUSH)SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
    Rectangle(hDCTemp,rc.left,rc.top,rc.right,rc.bottom);
  
    // this color is used as the black bit
    SetTextColor(hDCTemp, Colours[iAirspaceColour[AATASK]]);
    // this color is the transparent bit
    SetBkColor(hDCTemp, whitecolor);
   	SelectObject(hDCTemp, hpStartFinishThick);
          
    for(i=maxTp-1;i>std::max(0, ActiveWayPoint-1);i--)
      {
	if(ValidTaskPoint(i)) {
	  if(Task[i].AATType == CIRCLE)
	    {
	      tmp = Task[i].AATCircleRadius*zoom.ResScaleOverDistanceModify();

	      if (i<ActiveWayPoint) {
		SelectObject(hDCTemp, GetStockObject(HOLLOW_BRUSH));
	      } else {
		SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[AATASK]]);
	      }

	      Circle(hDCTemp,
		     WayPointList[Task[i].Index].Screen.x,
		     WayPointList[Task[i].Index].Screen.y,
		     (int)tmp, rc, true, true); 
	    }
	  else
	    {
          
	      if (i<ActiveWayPoint) {
		SelectObject(hDCTemp, GetStockObject(HOLLOW_BRUSH));
	      } else {
		SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[AATASK]]);
	      }
          
	      tmp = Task[i].AATSectorRadius*zoom.ResScaleOverDistanceModify();
          
	      Segment(hDCTemp,
		      WayPointList[Task[i].Index].Screen.x,
		      WayPointList[Task[i].Index].Screen.y,(int)tmp, rc, 
		      Task[i].AATStartRadial-DisplayAngle, 
		      Task[i].AATFinishRadial-DisplayAngle); 
          
	      DrawSolidLine(hDCTemp,
			    WayPointList[Task[i].Index].Screen, Task[i].AATStart,
			    rc);
	      DrawSolidLine(hDCTemp,
			    WayPointList[Task[i].Index].Screen, Task[i].AATFinish,
			    rc);

	    }

	}
      }

    // restore original color
    SetTextColor(hDCTemp, origcolor);
    SelectObject(hDCTemp, oldpen);
    SelectObject(hDCTemp, oldbrush);
    
#if (WINDOWSPC<1)
    TransparentImage(hdc,
		     rc.left,rc.top,
		     rc.right-rc.left,rc.bottom-rc.top,
		     hDCTemp,
		     rc.left,rc.top,
		     rc.right-rc.left,rc.bottom-rc.top,
		     whitecolor
		     );

#else
    TransparentBlt(hdc,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   hDCTemp,
                   rc.left,rc.top,
                   rc.right-rc.left,rc.bottom-rc.top,
                   whitecolor
                   );
#endif
     SelectObject(hDCTemp, oldbitmap);
  }  
     {
       UnlockTaskData();
     }
                   

}


