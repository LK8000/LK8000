/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"

void MapWindow::DrawWindAtAircraft2(LKSurface& Surface, const POINT& Orig, const RECT& rc) {
  int i;
  POINT Start;
  TCHAR sTmp[12];
  static SIZE tsize = {0,0};
  
  if (DerivedDrawInfo.WindSpeed<1) {
    return; // JMW don't bother drawing it if not significant
  }
  
  if (tsize.cx == 0){

    LKFont oldFont = Surface.SelectObject(MapWindowBoldFont);
    Surface.GetTextSize(TEXT("99"), 2, &tsize);
    Surface.SelectObject(oldFont);
    tsize.cx = tsize.cx/2;
  }

  LKPen hpOld = Surface.SelectObject(LKPen_Black_N2);
  LKBrush hbOld = Surface.SelectObject(LKBrush_Grey);
  
  int wmag = iround(4.0*DerivedDrawInfo.WindSpeed);
  
  Start.y = Orig.y;
  Start.x = Orig.x;

  int kx = tsize.cx/ScreenScale/2;

  POINT Arrow[7] = { {0,-20}, {-6,-26}, {0,-20}, 
                     {6,-26}, {0,-20}, 
                     {8+kx, -24}, 
                     {-8-kx, -24}};

  for (i=1;i<4;i++)
    Arrow[i].y -= wmag;

  PolygonRotateShift(Arrow, 7, Start.x, Start.y, 
		     DerivedDrawInfo.WindBearing-DisplayAngle);

  //
  // Draw Wind Arrow
  //
  POINT Tail[2] = {{0,-20}, {0,-26-min(20,wmag)*3}};
  double angle = AngleLimit360(DerivedDrawInfo.WindBearing-DisplayAngle);
  for(i=0; i<2; i++) {
    if (ScreenScale>1) {
      Tail[i].x *= ScreenScale;
      Tail[i].y *= ScreenScale;
    }
    protateshift(Tail[i], angle, Start.x, Start.y);
  }
  // optionally draw dashed line for wind arrow
  Surface.DrawLine(PEN_DASH, 1, Tail[0], Tail[1], LKColor(0,0,0), rc);

  // Paint wind value only while circling
  if ( (mode.Is(Mode::MODE_CIRCLING)) ) {

    _stprintf(sTmp, _T("%d"), iround(DerivedDrawInfo.WindSpeed * SPEEDMODIFY));

    TextInBoxMode_t TextInBoxMode = {0};
    TextInBoxMode.AlligneCenter = true;   // { 16 | 32 }; // JMW test {2 | 16};
    TextInBoxMode.WhiteBorder = true;
    if (Arrow[5].y>=Arrow[6].y) {
      TextInBox(Surface, &rc, sTmp, Arrow[5].x-kx, Arrow[5].y, 0, &TextInBoxMode);
    } else {
      TextInBox(Surface, &rc, sTmp, Arrow[6].x-kx, Arrow[6].y, 0, &TextInBoxMode);
    }
  }
  Surface.Polygon(Arrow,5);


  Surface.SelectObject(hbOld);
  Surface.SelectObject(hpOld);
}

