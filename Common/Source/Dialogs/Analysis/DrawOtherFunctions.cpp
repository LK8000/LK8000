/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "Sideview.h"


void Statistics::DrawLabel(LKSurface& Surface, const RECT& rc, const TCHAR *text,
			   const double xv, const double yv) {

  SIZE tsize;
  Surface.GetTextSize(text, _tcslen(text), &tsize);
  int x = (int)((xv-x_min)*xscale)+rc.left-tsize.cx/2+BORDER_X;
  int y = (int)((y_max-yv)*yscale)+rc.top-tsize.cy/2;
//  SetBkMode(hdc, OPAQUE);
  if(INVERTCOLORS)
    Surface.SelectObject(LK_BLACK_PEN);


  Surface.DrawText(x, y, text, _tcslen(text));
  Surface.SetBackgroundTransparent();
}


void Statistics::DrawNoData(LKSurface& Surface, const RECT& rc) {

  SIZE tsize;
  TCHAR text[80];
	// LKTOKEN  _@M470_ = "No data" 
  _stprintf(text,TEXT("%s"), gettext(TEXT("_@M470_")));
  Surface.GetTextSize(text, _tcslen(text), &tsize);
  int x = (int)(rc.left+rc.right-tsize.cx)/2;
  int y = (int)(rc.top+rc.bottom-tsize.cy)/2;
  #if (WINDOWSPC>0)
  Surface.SetBackgroundOpaque();
  #endif
  Surface.DrawText(x, y, text, _tcslen(text));
  Surface.SetBackgroundTransparent();
}



void Statistics::DrawXLabel(LKSurface& Surface, const RECT& rc, const TCHAR *text) {
  SIZE tsize;
  const auto hfOld = Surface.SelectObject(MapLabelFont);
  Surface.GetTextSize(text, _tcslen(text), &tsize);
  int x = rc.right-tsize.cx-IBLSCALE(3);
  int y = rc.bottom-tsize.cy;
  if(INVERTCOLORS)
    Surface.SelectObject(LK_BLACK_PEN);

  Surface.DrawText(x, y, text, _tcslen(text));
  Surface.SelectObject(hfOld);
}


void Statistics::DrawYLabel(LKSurface& Surface, const RECT& rc, const TCHAR *text) {
  SIZE tsize;
  const auto hfOld = Surface.SelectObject(MapLabelFont);
  Surface.GetTextSize(text, _tcslen(text), &tsize);
  int x = max(2,(int)rc.left-(int)tsize.cx);
  int y = rc.top;
  if(INVERTCOLORS)
    Surface.SelectObject(LK_BLACK_PEN);


  Surface.DrawText(x, y, text, _tcslen(text));
  Surface.SelectObject(hfOld);
}


void Statistics::DrawTrend(LKSurface& Surface, const RECT& rc, LeastSquares* lsdata,
			   const int Style) 
{
  if (lsdata->sum_n<2) {
    return;
  }

  if (unscaled_x || unscaled_y) {
    return;
  }

  double xmin, xmax, ymin, ymax;
  xmin = lsdata->x_min;
  xmax = lsdata->x_max;
  ymin = lsdata->x_min*lsdata->m+lsdata->b;
  ymax = lsdata->x_max*lsdata->m+lsdata->b;
  
  xmin = (int)((xmin-x_min)*xscale)+rc.left+BORDER_X;
  xmax = (int)((xmax-x_min)*xscale)+rc.left+BORDER_X;
  ymin = (int)((y_max-ymin)*yscale)+rc.top;
  ymax = (int)((y_max-ymax)*yscale)+rc.top;
  POINT line[2];
  line[0].x = (int)xmin;
  line[0].y = (int)ymin;
  line[1].x = (int)xmax;
  line[1].y = (int)ymax;

  StyleLine(Surface, line[0], line[1], Style, rc);

}


void Statistics::DrawTrendN(LKSurface& Surface, const RECT& rc,
			    LeastSquares* lsdata, 
                            const int Style) 
{
  if (lsdata->sum_n<2) {
    return;
  }

  if (unscaled_x || unscaled_y) {
    return;
  }

  double xmin, xmax, ymin, ymax;
  xmin = 0.5;
  xmax = lsdata->sum_n+0.5;
  ymin = lsdata->x_min*lsdata->m+lsdata->b;
  ymax = lsdata->x_max*lsdata->m+lsdata->b;
  
  xmin = (int)((xmin)*xscale)+rc.left+BORDER_X;
  xmax = (int)((xmax)*xscale)+rc.left+BORDER_X;
  ymin = (int)((y_max-ymin)*yscale)+rc.top;
  ymax = (int)((y_max-ymax)*yscale)+rc.top;
  POINT line[2];
  line[0].x = (int)xmin;
  line[0].y = (int)ymin;
  line[1].x = (int)xmax;
  line[1].y = (int)ymax;

  StyleLine(Surface, line[0], line[1], Style, rc);

}



void Statistics::DrawLine(LKSurface& Surface, const RECT& rc,
			  const double xmin, const double ymin,
                          const double xmax, const double ymax,
                          const int Style) {

  if (unscaled_x || unscaled_y) {
    return;
  }
  POINT line[2];
  line[0].x = (int)((xmin-x_min)*xscale)+rc.left+BORDER_X;
  line[0].y = (int)((y_max-ymin)*yscale)+rc.top;
  line[1].x = (int)((xmax-x_min)*xscale)+rc.left+BORDER_X;
  line[1].y = (int)((y_max-ymax)*yscale)+rc.top;

  StyleLine(Surface, line[0], line[1], Style, rc);

}


void Statistics::DrawBarChart(LKSurface& Surface, const RECT& rc, LeastSquares* lsdata) {
  int i;
LKColor Col;
  if (unscaled_x || unscaled_y) {
    return;
  }

if(INVERTCOLORS)
  Col = RGB_GREEN.ChangeBrightness(0.5);
else
  Col = RGB_WHITE;

  LKPen hpBar(PEN_SOLID, IBLSCALE(1), Col);
  LKBrush hbBar(Col);
  const auto oldpen = Surface.SelectObject(hpBar);
  const auto oldbrush = Surface.SelectObject(hbBar);


  int xmin, ymin, xmax, ymax;

  for (i= 0; i<lsdata->sum_n; i++) {
    xmin = (int)((i+1+0.2)*xscale)+rc.left+BORDER_X;
    ymin = (int)((y_max-y_min)*yscale)+rc.top;
    xmax = (int)((i+1+0.8)*xscale)+rc.left+BORDER_X;
    ymax = (int)((y_max-lsdata->ystore[i])*yscale)+rc.top;
    Surface.Rectangle(xmin, ymin, xmax, ymax);
  }

  Surface.SelectObject(oldpen);
  Surface.SelectObject(oldbrush);
}


void Statistics::DrawFilledLineGraph(LKSurface& Surface, const RECT& rc,
				     LeastSquares* lsdata,
				     const LKColor& color) {

  POINT line[4];

  for (int i=0; i<lsdata->sum_n-1; i++) {
    line[0].x = (int)((lsdata->xstore[i]-x_min)*xscale)+rc.left+BORDER_X;
    line[0].y = (int)((y_max-lsdata->ystore[i])*yscale)+rc.top;
    line[1].x = (int)((lsdata->xstore[i+1]-x_min)*xscale)+rc.left+BORDER_X;
    line[1].y = (int)((y_max-lsdata->ystore[i+1])*yscale)+rc.top;
    line[2].x = line[1].x;
    line[2].y = rc.bottom-BORDER_Y;
    line[3].x = line[0].x;
    line[3].y = rc.bottom-BORDER_Y;
    Surface.Polygon(line, 4);
  }
}



void Statistics::DrawLineGraph(LKSurface& Surface, const RECT& rc, LeastSquares* lsdata,
                               int Style) {

  POINT line[2];

  for (int i=0; i<lsdata->sum_n-1; i++) {
    line[0].x = (int)((lsdata->xstore[i]-x_min)*xscale)+rc.left+BORDER_X;
    line[0].y = (int)((y_max-lsdata->ystore[i])*yscale)+rc.top;
    line[1].x = (int)((lsdata->xstore[i+1]-x_min)*xscale)+rc.left+BORDER_X;
    line[1].y = (int)((y_max-lsdata->ystore[i+1])*yscale)+rc.top;

    // STYLE_DASHGREEN
    // STYLE_MEDIUMBLACK
    StyleLine(Surface, line[0], line[1], Style, rc);
  }
}


