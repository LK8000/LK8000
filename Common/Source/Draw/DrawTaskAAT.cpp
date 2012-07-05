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
  
  LockTaskData();  // protect from external task changes
#ifdef HAVEEXCEPTIONS
  __try{
#endif

    COLORREF whitecolor = RGB_WHITE;
    COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);

    SelectObject(hDCTemp, (HBITMAP)hDrawBitMapTmp);

    oldpen=(HPEN)SelectObject(hDCTemp, GetStockObject(WHITE_PEN));
    SelectObject(hDCTemp, GetStockObject(WHITE_BRUSH));
    Rectangle(hDCTemp,rc.left,rc.top,rc.right,rc.bottom);
    
    for(i=MAXTASKPOINTS-2;i>0;i--)
      {
	if(ValidTaskPoint(i) && ValidTaskPoint(i+1)) {
	  if(Task[i].AATType == CIRCLE)
	    {
	      tmp = Task[i].AATCircleRadius*zoom.ResScaleOverDistanceModify();
          
	      // this color is used as the black bit
	      SetTextColor(hDCTemp, 
			   Colours[iAirspaceColour[AATASK]]);
          
	      // this color is the transparent bit
	      SetBkColor(hDCTemp, 
			 whitecolor);

	      if (i<ActiveWayPoint) {
		SelectObject(hDCTemp, GetStockObject(HOLLOW_BRUSH));
	      } else {
		SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[AATASK]]);
	      }

	      SelectObject(hDCTemp, hpStartFinishThick);
          
	      Circle(hDCTemp,
		     WayPointList[Task[i].Index].Screen.x,
		     WayPointList[Task[i].Index].Screen.y,
		     (int)tmp, rc, true, true); 
	    }
	  else
	    {
          
	      // this color is used as the black bit
	      SetTextColor(hDCTemp, 
			   Colours[iAirspaceColour[AATASK]]);
          
	      // this color is the transparent bit
	      SetBkColor(hDCTemp, 
			 whitecolor);
          
	      if (i<ActiveWayPoint) {
		SelectObject(hDCTemp, GetStockObject(HOLLOW_BRUSH));
	      } else {
		SelectObject(hDCTemp, hAirspaceBrushes[iAirspaceBrush[AATASK]]);
	      }
	      SelectObject(hDCTemp, hpStartFinishThick);
          
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
  
#ifdef HAVEEXCEPTIONS
  }__finally
#endif
     {
       UnlockTaskData();
     }
}


