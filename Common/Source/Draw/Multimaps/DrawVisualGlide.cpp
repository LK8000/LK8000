/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Multimap.h"
#include "Sideview.h"
#include "LKObjects.h"
#include "RGB.h"

void MapWindow::DrawVisualGlide(HDC hdc, DiagrammStruct* pDia) {

  #if BUGSTOP
  LKASSERT(Current_Multimap_SizeY<SIZE4);
  #endif
  if (Current_Multimap_SizeY==SIZE4) return;

  DiagrammStruct mDia=*pDia;

  RECT vrc;
  vrc.left=0;
  vrc.right=ScreenSizeX;
  vrc.bottom=ScreenSizeY-BottomSize;
  if (Current_Multimap_SizeY==SIZE0) // Full screen?
	vrc.top=0;
  else
	vrc.top=mDia.rc.bottom;
  #if 0
  StartupStore(_T("fxMinMax=%f,%f  fyMinMax=%f,%f rcTLBR=%d,%d %d,%d\n"),
	mDia.fXMin,mDia.fXMax,mDia.fYMin,mDia.fYMax,
	mDia.rc.top, mDia.rc.left, mDia.rc.bottom,mDia.rc.right);
  #endif
  StartupStore(_T("VG AREA: %d,%d %d,%d\n"),vrc.top,vrc.left,vrc.bottom,vrc.right);

  //HBRUSH oldBrush=(HBRUSH) SelectObject(hdc,GetStockObject(WHITE_BRUSH));
  HPEN   oldPen  =(HPEN)   SelectObject(hdc, GetStockObject(BLACK_PEN));

  HBRUSH brush_back;
  if (!INVERTCOLORS) {
    brush_back = LKBrush_Black;
  } else {
    brush_back = LKBrush_Nlight;
  }

  //Rectangle(hdc, vrc.left, vrc.top, vrc.right. vrc.bottom);
  //FillRect(hdc, &vrc, brush_back);

  POINT center, p1, p2;
  center.x=(vrc.bottom-vrc.top)/2;
  center.y=(vrc.right-vrc.left)/2;
  unsigned int wScreen=vrc.right-vrc.left;
  unsigned int hScreen=vrc.bottom-vrc.top;

  SetBkMode(hdc,TRANSPARENT);
  p1.x=vrc.left; p1.y=center.y;
  p2.x=vrc.right; p2.y=center.y;
  SelectObject(hdc, LKPen_Black_N2);
  DrawSolidLine(hdc, p1, p2, vrc);


//_end:
  //SelectObject(hdc,oldBrush); 
  SelectObject(hdc,oldPen); 
  return;
}

