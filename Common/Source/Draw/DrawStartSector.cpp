/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

extern COLORREF taskcolor;


void MapWindow::DrawStartSector(HDC hdc, const RECT rc, 
                                POINT &Start,
                                POINT &End, int Index) {
  double tmp;
  HPEN oldpen;

  if(StartLine) {
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
              Start, taskcolor, rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
              End, taskcolor, rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
              Start, RGB(255,0,0), rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
              End, RGB(255,0,0), rc);
  } else {
    tmp = StartRadius*zoom.ResScaleOverDistanceModify();
    oldpen=(HPEN)SelectObject(hdc, hpStartFinishThick);
    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Circle(hdc,
           WayPointList[Index].Screen.x,
           WayPointList[Index].Screen.y,(int)tmp, rc, false, false);
    SelectObject(hdc, hpStartFinishThin);
    Circle(hdc,
           WayPointList[Index].Screen.x,
           WayPointList[Index].Screen.y,(int)tmp, rc, false, false);

    SelectObject(hdc,oldpen);
  }

}


