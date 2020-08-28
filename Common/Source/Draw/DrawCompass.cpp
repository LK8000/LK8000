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
    static POINT ArrowL[4] = { {0,-11}, {-5,9}, {0,3}, /*{5,9},*/ {0,-11}};
    static POINT ArrowR[4] = { {0,-11}, { 5,9}, {0,3}, /*{5,9},*/ {0,-11}};
    if (DoInit[MDI_COMPASS]) {
	lastRcRight=0;
	lastRcTop=0;
	DoInit[MDI_COMPASS]=false;
    }
   // no more clock, no need to have different compass position
	Start.y = rc.top + NIBLSCALE(11); 
	Start.x = rc.right - NIBLSCALE(11);
    if (lastDisplayAngle != angle || lastRcRight != rc.right || lastRcTop != rc.top){

      ArrowL[0].x  = 0;
      ArrowL[0].y  = -11;
      ArrowL[1].x  = -5;
      ArrowL[1].y  = 9;
      ArrowL[2].x  = 0;
      ArrowL[2].y  = 3;
      ArrowL[3].x  = 0;
      ArrowL[3].y  = -11;

      ArrowR[0].x  = 0;
      ArrowR[0].y  = -11;
      ArrowR[1].x  = 5;
      ArrowR[1].y  = 9;
      ArrowR[2].x  = 0;
      ArrowR[2].y  = 3;
      ArrowR[3].x  = 0;
      ArrowR[3].y  = -11;
	// no more clock, no need to have different compass position
	Start.y = rc.top + NIBLSCALE(11); 
	Start.x = rc.right - NIBLSCALE(11);

      // North arrow
      PolygonRotateShift(ArrowL, 4, Start.x, Start.y, 
                         -angle);
      PolygonRotateShift(ArrowR, 4, Start.x, Start.y, 
                         -angle);
      lastDisplayAngle = angle;
      lastRcRight = rc.right;
      lastRcTop = rc.top;
    }

    const auto hpOld = Surface.SelectObject(LKPen_Black_N0);
    LKBrush ArrowBrush;  ArrowBrush.Create(OverColorRef);
    const auto hbOld = Surface.SelectObject(ArrowBrush);
    Surface.Polygon(ArrowL,4);
    
    static LKColor ShaddowCol = RGB_GREY;
    static LKColor OverlayColor  = RGB_GREY;
    if(OverlayColor != OverColorRef) 
    {
      ShaddowCol = ShaddowCol.MixColors(OverColorRef, 0.6);   
      OverlayColor = OverColorRef;
    }
    LKBrush ShaddowArrowBrush; 
    ShaddowArrowBrush.Create(ShaddowCol); 
    Surface.SelectObject(ShaddowArrowBrush);
    
    Surface.Polygon(ArrowR,4);
   
    Surface.SelectObject(hbOld);
    Surface.SelectObject(hpOld);
    ShaddowArrowBrush.Release();

}


