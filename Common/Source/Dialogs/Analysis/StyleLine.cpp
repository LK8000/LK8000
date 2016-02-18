/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"



extern LKPen penThinSignal;


void Statistics::StyleLine(LKSurface& Surface, const POINT& l1, const POINT& l2,
                           const int Style, const RECT& rc) {
  int minwidth = 1;
  minwidth = 3;
  POINT line[2];
  line[0] = l1;
  line[1] = l2;
  LKPen mpen;
  LKSurface::OldPen oldpen;
  LKColor COL;
  switch (Style) {
  case STYLE_BLUETHIN:
	COL = LKColor(0,50,255);
	if(INVERTCOLORS || IsDithered())
	  COL = COL.ChangeBrightness(0.5);
    Surface.DrawDashLine(minwidth, l1, l2, COL, rc);
    break;
  case STYLE_REDTHICK:
	COL = LKColor(250,50,50);
	if(INVERTCOLORS || IsDithered())
	  COL = COL.ChangeBrightness(0.7);
    Surface.DrawDashLine(minwidth, l1, l2, COL, rc);
    break;

  case STYLE_GREENMEDIUM:
	  COL =   LKColor(0,255,0);
	  if(INVERTCOLORS || IsDithered())
		COL = COL.ChangeBrightness(0.7);
	  line[0].x +=1;
	  line[1].x +=1;
      mpen.Create(PEN_SOLID, IBLSCALE(2),  COL);
      oldpen = Surface.SelectObject(mpen);
      Surface.Polyline(line, 2, rc);
      Surface.SelectObject(oldpen);
      mpen.Release();
    break;

  case STYLE_GREENTHICK:
	  COL =   LKColor(0,255,0);
	  if(INVERTCOLORS || IsDithered())
		COL = COL.ChangeBrightness(0.7);
	  line[0].x +=2;
	  line[1].x +=2;
      mpen.Create(PEN_SOLID, IBLSCALE(4),  COL);
      oldpen = Surface.SelectObject(mpen);
      Surface.Polyline(line, 2, rc);
      Surface.SelectObject(oldpen);
      mpen.Release();
    break;

  case STYLE_ORANGETHICK:
	COL =  LKColor(255,165,0);
	if(INVERTCOLORS || IsDithered())
	  COL = COL.ChangeBrightness(0.7);

	line[0].x +=2;
	line[1].x +=2;
    mpen.Create(PEN_SOLID, IBLSCALE(4),  COL);
    oldpen = Surface.SelectObject(mpen);
    Surface.Polyline(line, 2, rc);
    Surface.SelectObject(oldpen);
    mpen.Release();
  break;

  case STYLE_ORANGETHIN:
	COL =  LKColor(255,165,0);
	if(INVERTCOLORS || IsDithered())
	  COL = COL.ChangeBrightness(0.7);

	line[0].x +=2;
	line[1].x +=2;
        mpen.Create(PEN_SOLID, IBLSCALE(2),  COL);
        oldpen = Surface.SelectObject(mpen);
        Surface.Polyline(line, 2, rc);
        Surface.SelectObject(oldpen);
        mpen.Release();
  break;
  case STYLE_DASHGREEN:
	COL = LKColor(0,255,0);
	if(INVERTCOLORS || IsDithered())
	  COL = COL.ChangeBrightness(0.7);

    Surface.DrawDashLine(IBLSCALE(2),line[0], line[1], COL, rc);
    break;
  case STYLE_MEDIUMBLACK:
    oldpen = Surface.SelectObject(penThinSignal);
    Surface.Polyline(line, 2, rc);
    Surface.SelectObject(oldpen);
    break;
  case STYLE_THINDASHPAPER:
    Surface.DrawDashLine(1, l1, l2, LKColor(0x60,0x60,0x60), rc);
    break;
  case STYLE_WHITETHICK:
	COL =  RGB_WHITE;
	if(INVERTCOLORS || IsDithered())
	  COL = COL.ChangeBrightness(0.3);

    Surface.DrawDashLine(3, l1, l2, COL, rc);
    break;

  default:
    break;
  }

}


