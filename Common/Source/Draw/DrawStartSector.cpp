/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

extern COLORREF taskcolor;


void MapWindow::DrawStartEndSector(HDC hdc, const RECT& rc, 
                                    const POINT &Start, const POINT &End, int Index, 
                                    int Type, double Radius) {
    double tmp;
  HPEN oldpen;
  HBRUSH oldbrush;

  switch (Type) {
  case 0: // CIRCLE
    tmp = Radius * zoom.ResScaleOverDistanceModify();
    oldpen = (HPEN) SelectObject(hdc, hpStartFinishThick);
    oldbrush = (HBRUSH) SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    Circle(hdc, WayPointList[Index].Screen.x,
        		WayPointList[Index].Screen.y, (int) tmp, rc, false, false);
    SelectObject(hdc, hpStartFinishThin);
    Circle(hdc, WayPointList[Index].Screen.x,
    			WayPointList[Index].Screen.y, (int) tmp, rc, false, false);

    SelectObject(hdc, oldpen);
    SelectObject(hdc, oldbrush);
    break;
  case 1: // LINE
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), End, Start, taskcolor, rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), End, Start, RGB(255, 0, 0), rc);
    break;
  case 2: // SECTOR
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
              Start, taskcolor, rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(5), WayPointList[Index].Screen,
              End, taskcolor, rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
              Start, RGB(255,0,0), rc);
    _DrawLine(hdc, PS_SOLID, NIBLSCALE(1), WayPointList[Index].Screen,
              End, RGB(255,0,0), rc);
    break;
  }

}


