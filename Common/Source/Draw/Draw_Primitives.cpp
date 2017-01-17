/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Topology.h"


void MapWindow::DrawBitmapIn(LKSurface& Surface, const POINT &sc, const LKIcon& Icon) {
    if (!Icon) return; // don't draw Bitmap if no bitmap
    if (!PointVisible(sc)) return;

    LKASSERT(Icon.GetSize().cx == 10);
    LKASSERT(Icon.GetSize().cy == 10);

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
  #ifdef GTL2
  int Offset = ((width - 1) / 2) + 1;
               // amount to shift 1st line to center
               // the group of lines properly
  #endif

  //Create a dot pen
  LKPen hpDash1(PEN_DASH, 1, cr1);
  LKPen hpDash2(PEN_DASH, 1, cr2);
  const auto hpOld = Surface.SelectObject(hpDash1);

  #ifdef GTL2
  pt[0] = ptStart;
  pt[1] = ptEnd;
  #else
  pt[0].x = ptStart.x;
  pt[0].y = ptStart.y;
  pt[1].x = ptEnd.x;
  pt[1].y = ptEnd.y;
  #endif

  //increment on smallest variance
  if(abs(ptStart.x - ptEnd.x) < abs(ptStart.y - ptEnd.y)){
    #ifdef GTL2
    pt[0].x -= Offset;
    pt[1].x -= Offset;
    #endif
    for (i = 0; i < width; i++, flipcol=!flipcol){
      flipcol ?  Surface.SelectObject(hpDash2) : Surface.SelectObject(hpDash1);
      pt[0].x += 1;
      pt[1].x += 1;
      Surface.Polyline(pt, 2, rc);
    }
  } else {
    #ifdef GTL2
    pt[0].y -= Offset;
    pt[1].y -= Offset;
    #endif
    for (i = 0; i < width; i++, flipcol=!flipcol){
      flipcol ?  Surface.SelectObject(hpDash2) : Surface.SelectObject(hpDash1);
      pt[0].y += 1;
      pt[1].y += 1;
      Surface.Polyline(pt, 2, rc);
    }
  }

  Surface.SelectObject(hpOld);

}
