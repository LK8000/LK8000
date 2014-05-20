/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"



extern HPEN penThinSignal;


void Statistics::StyleLine(HDC hdc, const POINT l1, const POINT l2,
                           const int Style, const RECT rc) {
  int minwidth = 1;
  minwidth = 3;
  POINT line[2];
  line[0] = l1;
  line[1] = l2;
  HPEN mpen ;
  HPEN oldpen;
  COLORREF COL;
  switch (Style) {
  case STYLE_BLUETHIN:
	COL = RGB(0,50,255);
	if(INVERTCOLORS)
	  COL = ChangeBrightness(COL,0.5);
    MapWindow::DrawDashLine(hdc, 
			    minwidth, 
			    l1, 
			    l2, 
			    COL, rc);
    break;
  case STYLE_REDTHICK:
	COL = RGB(250,50,50);
	if(INVERTCOLORS)
	  COL = ChangeBrightness(COL,0.7);
    MapWindow::DrawDashLine(hdc, minwidth,
			    l1,
			    l2,
			    COL, rc);
    break;

  case STYLE_GREENMEDIUM:
	  COL =   RGB(0,255,0);
	  if(INVERTCOLORS)
		COL = ChangeBrightness(COL,0.7);
	  line[0].x +=1;
	  line[1].x +=1;
      mpen = (HPEN)CreatePen(PS_SOLID, IBLSCALE(2),  COL);
      oldpen = (HPEN)SelectObject(hdc, (HPEN)mpen);
      MapWindow::_Polyline(hdc, line, 2, rc);
      SelectObject(hdc, oldpen);
      DeleteObject(mpen);
    break;

  case STYLE_GREENTHICK:
	  COL =   RGB(0,255,0);
	  if(INVERTCOLORS)
		COL = ChangeBrightness(COL,0.7);
	  line[0].x +=2;
	  line[1].x +=2;
      mpen = (HPEN)CreatePen(PS_SOLID, IBLSCALE(4),  COL);
      oldpen = (HPEN)SelectObject(hdc, (HPEN)mpen);
      MapWindow::_Polyline(hdc, line, 2, rc);
      SelectObject(hdc, oldpen);
      DeleteObject(mpen);
    break;

  case STYLE_ORANGETHICK:
	COL =  RGB(255,165,0);
	if(INVERTCOLORS)
	  COL = ChangeBrightness(COL,0.7);

	line[0].x +=2;
	line[1].x +=2;
    mpen = (HPEN)CreatePen(PS_SOLID, IBLSCALE(4),  COL);
    oldpen = (HPEN)SelectObject(hdc, (HPEN)mpen);
    MapWindow::_Polyline(hdc, line, 2, rc);
    SelectObject(hdc, oldpen);
    DeleteObject(mpen);
  break;

  case STYLE_ORANGETHIN:
	COL =  RGB(255,165,0);
	if(INVERTCOLORS)
	  COL = ChangeBrightness(COL,0.7);

	line[0].x +=2;
	line[1].x +=2;
  mpen = (HPEN)CreatePen(PS_SOLID, IBLSCALE(2),  COL);
  oldpen = (HPEN)SelectObject(hdc, (HPEN)mpen);
  MapWindow::_Polyline(hdc, line, 2, rc);
  SelectObject(hdc, oldpen);
  DeleteObject(mpen);
  break;
  case STYLE_DASHGREEN:
	COL = RGB(0,255,0);
	if(INVERTCOLORS)
	  COL = ChangeBrightness(COL,0.7);

    MapWindow::DrawDashLine(hdc, IBLSCALE(2),
			    line[0], 
			    line[1], 
			    COL, rc);
    break;
  case STYLE_MEDIUMBLACK:
    oldpen = (HPEN)SelectObject(hdc, penThinSignal /*GetStockObject(BLACK_PEN)*/);
    MapWindow::_Polyline(hdc, line, 2, rc);
    SelectObject(hdc, oldpen);
    break;
  case STYLE_THINDASHPAPER:
    MapWindow::DrawDashLine(hdc, 1, 
			    l1, 
			    l2, 
			    RGB(0x60,0x60,0x60), rc);    
    break;
  case STYLE_WHITETHICK:
	COL =  RGB_WHITE;
	if(INVERTCOLORS)
	  COL = ChangeBrightness(COL,0.3);


    MapWindow::DrawDashLine(hdc, 3, 
          l1, 
          l2, 
          COL, rc);
    break;

  default:
    break;
  }

}


