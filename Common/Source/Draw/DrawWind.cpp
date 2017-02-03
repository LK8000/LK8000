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
  POINT Start;
  TCHAR sTmp[12];

  if (DerivedDrawInfo.WindSpeed<1) {
    return; // JMW don't bother drawing it if not significant
  }

  int wmag = iround(4.0*DerivedDrawInfo.WindSpeed);
  double angle = AngleLimit360(DerivedDrawInfo.WindBearing-DisplayAngle);

  Start.y = Orig.y;
  Start.x = Orig.x;

  POINT Arrow[] = { 
      {0,-20}, 
      {-6,-26-wmag}, {0,-20-wmag}, {6,-26-wmag}, 
      {0,-20}
  };
  PolygonRotateShift(Arrow, array_size(Arrow), Start.x, Start.y, angle);

  POINT Tail[] = {
      {0,-20}, 
      {0,-26-min(20,wmag)*3}
  };

  PolygonRotateShift(Tail, array_size(Tail), Start.x, Start.y, angle);
  
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
    TextInBoxMode.NoSetFont = true; // font alredy set for calc text size.
    
    auto oldFont = Surface.SelectObject(MapWaypointFont);
    SIZE tSize;
    Surface.GetTextSize(sTmp, &tSize);
    POINT pt = {
      IBLSCALE(8) + tSize.cx/2,
      IBLSCALE(-24)
    };  
    if (Arrow[1].y>=Arrow[3].y) {
        pt.x *= (-1);
    }
    
    protateshift(pt, angle, Start.x, Start.y);
    TextInBox(Surface, &rc, sTmp, pt.x, pt.y, &TextInBoxMode);
    Surface.SelectObject(oldFont);
  }
  const auto hpOld = Surface.SelectObject(LKPen_Black_N2);
  const auto hbOld = Surface.SelectObject(LKBrush_Grey);
  Surface.Polygon(Arrow,array_size(Arrow));

  Surface.SelectObject(hbOld);
  Surface.SelectObject(hpOld);
}
