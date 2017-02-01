/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"
#include "LKObjects.h"
#include "ScreenGeometry.h"

void MapWindow::DrawCompass(LKSurface& Surface, const RECT& rc, const double angle)
{
  POINT Start;

    static double lastDisplayAngle = 9999.9;
    static int lastRcRight = 0, lastRcTop = 0;
    static POINT Arrow[5] = { {0,-11}, {-5,9}, {0,3}, {5,9}, {0,-11}};

    if (DoInit[MDI_COMPASS]) {
	lastRcRight=0;
	lastRcTop=0;
	DoInit[MDI_COMPASS]=false;
    }

    if (lastDisplayAngle != angle || lastRcRight != rc.right || lastRcTop != rc.top){

      Arrow[0].x  = 0;
      Arrow[0].y  = -11;
      Arrow[1].x  = -5;
      Arrow[1].y  = 9;
      Arrow[2].x  = 0;
      Arrow[2].y  = 3;
      Arrow[3].x  = 5;
      Arrow[3].y  = 9;
      Arrow[4].x  = 0;
      Arrow[4].y  = -11;

	  // no more clock, no need to have different compass position
	  Start.y = rc.top + NIBLSCALE(11); 
	  Start.x = rc.right - NIBLSCALE(11);

      // North arrow
      PolygonRotateShift(Arrow, 5, Start.x, Start.y, 
                         -angle);

      lastDisplayAngle = angle;
      lastRcRight = rc.right;
      lastRcTop = rc.top;
    }

    const auto hpOld = Surface.SelectObject(LKPen_Black_N2);
    const auto hbOld = Surface.SelectObject(LKBrush_White);
    Surface.Polygon(Arrow,5);

    Surface.SelectObject(LKPen_Black_N1);
    Surface.Polygon(Arrow,5);

    Surface.SelectObject(hbOld);
    Surface.SelectObject(hpOld);

}


