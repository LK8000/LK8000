/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Topology.h"


void MapWindow::DrawBitmapIn(LKSurface& Surface, const POINT &sc, const LKIcon& Icon) {
    if (!Icon) return; // don't draw Bitmap if no bitmap
    if (!PointVisible(sc)) return;
    Icon.Draw(Surface, sc.x - NIBLSCALE(5), sc.y - NIBLSCALE(5), NIBLSCALE(10), NIBLSCALE(10));
}

//
// Like DrawDashLine, but with two alternating colors
//
void MapWindow::DrawMulticolorDashLine(LKSurface& Surface, const int width,
			     const POINT& ptStart, const POINT& ptEnd, const LKColor& cr1, const LKColor& cr2,
			     const RECT& rc)
{
  int i;
  POINT pt[2];
  bool flipcol=false;
  int Offset = ((width - 1) / 2) + 1;
               // amount to shift 1st line to center
               // the group of lines properly

  //Create a dot pen
  LKPen hpDash1(PEN_DASH, 1, cr1);
  LKPen hpDash2(PEN_DASH, 1, cr2);
  const auto hpOld = Surface.SelectObject(hpDash1);

  pt[0] = ptStart;
  pt[1] = ptEnd;

  //increment on smallest variance
  if(abs(ptStart.x - ptEnd.x) < abs(ptStart.y - ptEnd.y)){
    pt[0].x -= Offset;
    pt[1].x -= Offset;
    for (i = 0; i < width; i++, flipcol=!flipcol){
      flipcol ?  Surface.SelectObject(hpDash2) : Surface.SelectObject(hpDash1);
      pt[0].x += 1;
      pt[1].x += 1;
      Surface.Polyline(pt, 2, rc);
    }
  } else {
    pt[0].y -= Offset;
    pt[1].y -= Offset;
    for (i = 0; i < width; i++, flipcol=!flipcol){
      flipcol ?  Surface.SelectObject(hpDash2) : Surface.SelectObject(hpDash1);
      pt[0].y += 1;
      pt[1].y += 1;
      Surface.Polyline(pt, 2, rc);
    }
  }

  Surface.SelectObject(hpOld);

}
