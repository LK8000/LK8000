/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Topology.h"


void MapWindow::DrawBitmapX(const HDC hdc, const int x, const int y,
                            const int sizex, const int sizey,
                            const HDC source,
                            const int offsetx, const int offsety,
                            const DWORD mode, const bool autostretch) {

  if (autostretch && ScreenScale>1) {
    StretchBlt(hdc, x, y, 
               IBLSCALE(sizex), 
               IBLSCALE(sizey), 
               source,
               offsetx, offsety, sizex, sizey,
               mode);
  } else {
    BitBlt(hdc, x, y, sizex, sizey, source, offsetx, offsety, mode); 
  }
}



void MapWindow::DrawBitmapIn(const HDC hdc, const POINT &sc, const HBITMAP h, const bool autostretch) {
  if (!PointVisible(sc)) return;

  SelectObject(hDCTemp, h);

  DrawBitmapX(hdc,
              sc.x-NIBLSCALE(5),
              sc.y-NIBLSCALE(5),
              10,10,
	      hDCTemp,0,0,SRCPAINT, autostretch);
  DrawBitmapX(hdc,
              sc.x-NIBLSCALE(5),
              sc.y-NIBLSCALE(5),
              10,10,
              hDCTemp,10,0,SRCAND, autostretch);
}



void MapWindow::_Polyline(HDC hdc, POINT* pt, const int npoints, 
			  const RECT rc) {
#ifdef BUG_IN_CLIPPING
  ClipPolygon(hdc, pt, npoints, rc, false);
  //VENTA2
#elif defined(PNA)
  if (needclipping==true)
    ClipPolygon(hdc, pt, npoints, rc, false);
  else
    Polyline(hdc, pt, npoints);
#else
  Polyline(hdc, pt, npoints);
#endif
}



void MapWindow::DrawSolidLine(const HDC& hdc, const POINT &ptStart, 
                              const POINT &ptEnd, const RECT rc)
{
  POINT pt[2];
  
  pt[0].x= ptStart.x;
  pt[0].y= ptStart.y;
  pt[1].x= ptEnd.x;
  pt[1].y= ptEnd.y;

  _Polyline(hdc, pt, 2, rc);
} 



void MapWindow::_DrawLine(HDC hdc, const int PenStyle, const int width, 
			  const POINT ptStart, const POINT ptEnd, 
			  const COLORREF cr, 
			  const RECT rc) {

  HPEN hpDash,hpOld;
  POINT pt[2];
  //Create a dot pen
  hpDash = (HPEN)CreatePen(PenStyle, width, cr);
  hpOld = (HPEN)SelectObject(hdc, hpDash);

  pt[0].x = ptStart.x;
  pt[0].y = ptStart.y;
  pt[1].x = ptEnd.x;
  pt[1].y = ptEnd.y;

  _Polyline(hdc, pt, 2, rc);

  SelectObject(hdc, hpOld);
  DeleteObject((HPEN)hpDash);
}




void MapWindow::DrawDashLine(HDC hdc, const int width, 
			     const POINT ptStart, const POINT ptEnd, const COLORREF cr,
			     const RECT rc)
{
  int i;
  HPEN hpDash,hpOld;
  POINT pt[2];
  #ifdef GTL2
  int Offset = ((width - 1) / 2) + 1;
               // amount to shift 1st line to center
               // the group of lines properly
  #endif

  //Create a dot pen
  hpDash = (HPEN)CreatePen(PS_DASH, 1, cr);
  hpOld = (HPEN)SelectObject(hdc, hpDash);

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
    for (i = 0; i < width; i++){
      pt[0].x += 1;
      pt[1].x += 1;     
      _Polyline(hdc, pt, 2, rc);
    }   
  } else {
    #ifdef GTL2
    pt[0].y -= Offset;
    pt[1].y -= Offset;
    #endif
    for (i = 0; i < width; i++){
      pt[0].y += 1;
      pt[1].y += 1;     
      _Polyline(hdc, pt, 2, rc);
    }   
  }
  
  SelectObject(hdc, hpOld);
  DeleteObject((HPEN)hpDash);
  
} 



#ifdef GTL2
void MapWindow::DrawDashPoly(HDC hdc, const int width, const COLORREF color,
                             POINT* pt, const int npoints, const RECT rc)
{
  int Segment;

  for (Segment = 1; Segment < npoints; Segment++) {
    DrawDashLine(hdc, width, pt[Segment-1], pt[Segment], color, rc);
  }
}
#endif
