/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "Sideview.h"


void Statistics::DrawLabel(HDC hdc, const RECT rc, const TCHAR *text, 
			   const double xv, const double yv) {

  SIZE tsize;
  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
  int x = (int)((xv-x_min)*xscale)+rc.left-tsize.cx/2+BORDER_X;
  int y = (int)((y_max-yv)*yscale)+rc.top-tsize.cy/2;
//  SetBkMode(hdc, OPAQUE);
  if(INVERTCOLORS)
    SelectObject(hdc, GetStockObject(BLACK_PEN));


  ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
  SetBkMode(hdc, TRANSPARENT);
}


void Statistics::DrawNoData(HDC hdc, RECT rc) {

  SIZE tsize;
  TCHAR text[80];
	// LKTOKEN  _@M470_ = "No data" 
  _stprintf(text,TEXT("%s"), gettext(TEXT("_@M470_")));
  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
  int x = (int)(rc.left+rc.right-tsize.cx)/2;
  int y = (int)(rc.top+rc.bottom-tsize.cy)/2;
  SetBkMode(hdc, OPAQUE);
  ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
  SetBkMode(hdc, TRANSPARENT);
}



void Statistics::DrawXLabel(HDC hdc, const RECT rc, const TCHAR *text) {
  SIZE tsize;
  HFONT hfOld = (HFONT)SelectObject(hdc, MapLabelFont);
  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
  int x = rc.right-tsize.cx-IBLSCALE(3);
  int y = rc.bottom-tsize.cy;
  if(INVERTCOLORS)
    SelectObject(hdc, GetStockObject(BLACK_PEN));

  ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
  SelectObject(hdc, hfOld);
}


void Statistics::DrawYLabel(HDC hdc, const RECT rc, const TCHAR *text) {
  SIZE tsize;
  HFONT hfOld = (HFONT)SelectObject(hdc, MapLabelFont);
  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
  int x = max(2,(int)rc.left-(int)tsize.cx);
  int y = rc.top;
  if(INVERTCOLORS)
    SelectObject(hdc, GetStockObject(BLACK_PEN));


  ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
  SelectObject(hdc, hfOld);
}


void Statistics::DrawTrend(HDC hdc, const RECT rc, LeastSquares* lsdata, 
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

  StyleLine(hdc, line[0], line[1], Style, rc);

}


void Statistics::DrawTrendN(HDC hdc, const RECT rc, 
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

  StyleLine(hdc, line[0], line[1], Style, rc);

}



void Statistics::DrawLine(HDC hdc, const RECT rc, 
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

  StyleLine(hdc, line[0], line[1], Style, rc);

}


void Statistics::DrawBarChart(HDC hdc, const RECT rc, LeastSquares* lsdata) {
  int i;
COLORREF Col;
  if (unscaled_x || unscaled_y) {
    return;
  }

if(INVERTCOLORS)
  Col = ChangeBrightness(RGB_GREEN, 0.5);
else
  Col = RGB_WHITE;

  HPEN    hpBar = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1), Col);
  HBRUSH  hbBar = (HBRUSH)CreateSolidBrush(Col);
  HPEN   oldpen   = (HPEN)   SelectObject(hdc, hpBar);
  HBRUSH oldbrush = (HBRUSH) SelectObject(hdc, hbBar);


  int xmin, ymin, xmax, ymax;

  for (i= 0; i<lsdata->sum_n; i++) {
    xmin = (int)((i+1+0.2)*xscale)+rc.left+BORDER_X;
    ymin = (int)((y_max-y_min)*yscale)+rc.top;
    xmax = (int)((i+1+0.8)*xscale)+rc.left+BORDER_X;
    ymax = (int)((y_max-lsdata->ystore[i])*yscale)+rc.top;
    Rectangle(hdc, 
              xmin, 
              ymin,
              xmax,
              ymax);
  }

  SelectObject(hdc, oldpen);
  SelectObject(hdc, oldbrush);
  DeleteObject (hpBar);
  DeleteObject (hbBar);
}


void Statistics::DrawFilledLineGraph(HDC hdc, const RECT rc, 
				     LeastSquares* lsdata,
				     const COLORREF color) {

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
    Polygon(hdc, line, 4);
  }
}



void Statistics::DrawLineGraph(HDC hdc, RECT rc, LeastSquares* lsdata,
                               int Style) {

  POINT line[2];

  for (int i=0; i<lsdata->sum_n-1; i++) {
    line[0].x = (int)((lsdata->xstore[i]-x_min)*xscale)+rc.left+BORDER_X;
    line[0].y = (int)((y_max-lsdata->ystore[i])*yscale)+rc.top;
    line[1].x = (int)((lsdata->xstore[i+1]-x_min)*xscale)+rc.left+BORDER_X;
    line[1].y = (int)((y_max-lsdata->ystore[i+1])*yscale)+rc.top;

    // STYLE_DASHGREEN
    // STYLE_MEDIUMBLACK
    StyleLine(hdc, line[0], line[1], Style, rc);
  }
}


