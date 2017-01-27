/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"
#include "ScreenGeometry.h"

void MapWindow::DrawWindAtAircraft2(LKSurface& Surface, const POINT& Orig, const RECT& rc) {
  int i;
  POINT Start;
  TCHAR sTmp[12];
  static SIZE tsize = {0,0};

  if (DerivedDrawInfo.WindSpeed<1) {
    return; // JMW don't bother drawing it if not significant
  }

  if (tsize.cx == 0){

    const auto oldFont = Surface.SelectObject(MapWindowBoldFont);
    Surface.GetTextSize(TEXT("99"), &tsize);
    Surface.SelectObject(oldFont);
    tsize.cx = tsize.cx/2;
  }

  int wmag = iround(4.0*DerivedDrawInfo.WindSpeed);

  Start.y = Orig.y;
  Start.x = Orig.x;

#ifdef RESCALE_PIXEL
  int kx = tsize.cx/2;
#else
  int kx = tsize.cx/ScreenScale/2;
#endif

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
    if (RescalePixelSize(1)>1) {
      Tail[i].x = RescalePixelSize(Tail[i].x);
      Tail[i].y = RescalePixelSize(Tail[i].y);
    }
    protateshift(Tail[i], angle, Start.x, Start.y);
  }
  // optionally draw dashed line for wind arrow
#ifdef NO_DASH_LINE
  Surface.DrawLine(PEN_SOLID, ScreenThinSize, Tail[0], Tail[1], LKColor(0,0,0), rc);
#else
  Surface.DrawDashLine(ScreenThinSize, Tail[0], Tail[1], LKColor(0,0,0), rc);
#endif

  // Paint wind value only while circling
  if ( (mode.Is(Mode::MODE_CIRCLING)) ) {

    _stprintf(sTmp, _T("%d"), iround(DerivedDrawInfo.WindSpeed * SPEEDMODIFY));

    TextInBoxMode_t TextInBoxMode = {0};
    TextInBoxMode.AlligneCenter = true;   // { 16 | 32 }; // JMW test {2 | 16};
    TextInBoxMode.WhiteBorder = true;
    if (Arrow[5].y>=Arrow[6].y) {
      TextInBox(Surface, &rc, sTmp, Arrow[5].x-kx, Arrow[5].y, &TextInBoxMode);
    } else {
      TextInBox(Surface, &rc, sTmp, Arrow[6].x-kx, Arrow[6].y, &TextInBoxMode);
    }
  }
  const auto hpOld = Surface.SelectObject(LKPen_Black_N2);
  const auto hbOld = Surface.SelectObject(LKBrush_Grey);
  Surface.Polygon(Arrow,5);

  Surface.SelectObject(hbOld);
  Surface.SelectObject(hpOld);
}
