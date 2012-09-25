/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStatistics.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "InfoBoxLayout.h"
#include "McReady.h"
#include "Atmosphere.h"
#include "RasterTerrain.h"
#include "LKInterface.h"
#include "RGB.h"
#include "Sideview.h"



double Statistics::yscale;
double Statistics::xscale;
double Statistics::y_min;
double Statistics::x_min;
double Statistics::x_max;
double Statistics::y_max;
bool   Statistics::unscaled_x;
bool   Statistics::unscaled_y;


static HPEN penThinSignal = NULL;

#define USESONAR 1
#ifdef USESONAR
#define SONAR_TEST
#endif

// These are globals, should start with Uppercase, and being non-static (local to this file)
// they should have a Sideview_ remembering us what are they used for. Otherwise use a class,
// which is exactly the same.

AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];
int   Sideview_iNoHandeldSpaces=0;
extern int   Sideview_asp_heading_task ;
long  iSonarLevel = 0;

#if USESONAR
bool  Sonar_IsEnabled = true;
#else
bool  Sonar_IsEnabled = false;
#endif
TCHAR Sideview_szNearAS[80];

extern COLORREF  Sideview_TextColor;


void Statistics::ResetScale() {
  unscaled_y = true;  
  unscaled_x = true;  
}



void Statistics::Reset() {
  ThermalAverage.Reset();
  Wind_x.Reset();
  Wind_y.Reset();
  Altitude.Reset();
  Altitude_Base.Reset();
  Altitude_Ceiling.Reset();
  Task_Speed.Reset();
  Altitude_Terrain.Reset();
  for(int j=0;j<MAXTASKPOINTS;j++) {
    LegStartTime[j] = -1;
  }
}


void Statistics::ScaleYFromData(const RECT rc, LeastSquares* lsdata) 
{
  if (!lsdata->sum_n) {
    return;
  }

  if (unscaled_y) {
    y_min = lsdata->y_min;
    y_max = lsdata->y_max;
    unscaled_y = false;
  } else {
    y_min = min(y_min,lsdata->y_min);
    y_max = max(y_max,lsdata->y_max);
  }

  if (lsdata->sum_n>1) {
    double y0, y1;
    y0 = lsdata->x_min*lsdata->m+lsdata->b;
    y1 = lsdata->x_max*lsdata->m+lsdata->b;
    y_min = min(y_min,min(y0,y1));
    y_max = max(y_max,max(y0,y1));
  }


  if (fabs(y_max - y_min) > 50){
    yscale = (y_max - y_min);
    if (yscale>0) {
      yscale = (rc.bottom-rc.top-BORDER_Y)/yscale;
    }
  } else {
    yscale = 2000;
  }
}


void Statistics::ScaleXFromData(const RECT rc, LeastSquares* lsdata) 
{
  if (!lsdata->sum_n) {
    return;
  }
  if (unscaled_x) {
    x_min = lsdata->x_min;
    x_max = lsdata->x_max;
    unscaled_x = false;
  } else {
    x_min = min(x_min,lsdata->x_min);
    x_max = max(x_max,lsdata->x_max);
  }

  xscale = (x_max-x_min);
  if (xscale>0) {
    xscale = (rc.right-rc.left-BORDER_X)/xscale;
  }
}


void Statistics::ScaleYFromValue(const RECT rc, const double value) 
{
  if (unscaled_y) {
    y_min = value;
    y_max = value;
    unscaled_y = false;
  } else {
    y_min = min(value, y_min);
    y_max = max(value, y_max);
  }

  yscale = (y_max - y_min);
  if (yscale>0) {
    yscale = (rc.bottom-rc.top-BORDER_Y)/yscale;
  }
}


void Statistics::ScaleXFromValue(const RECT rc, const double value) 
{
  if (unscaled_x) {
    x_min = value;
    x_max = value;
    unscaled_x = false;
  } else {
    x_min = min(value, x_min);
    x_max = max(value, x_max);
  }

  xscale = (x_max-x_min);
  if (xscale>0) {
    xscale = (rc.right-rc.left-BORDER_X)/xscale;
  }

}


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
    SelectObject(hdc, penThinSignal /*GetStockObject(BLACK_PEN)*/);
    MapWindow::_Polyline(hdc, line, 2, rc);
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


void Statistics::DrawLabel(HDC hdc, const RECT rc, const TCHAR *text, 
			   const double xv, const double yv) {

  SIZE tsize;
  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
  int x = (int)((xv-x_min)*xscale)+rc.left-tsize.cx/2+BORDER_X;
  int y = (int)((y_max-yv)*yscale)+rc.top-tsize.cy/2;
  SetBkMode(hdc, OPAQUE);
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


void Statistics::FormatTicText(TCHAR *text, const double val, const double step) {
  if (step<1.0) {
    _stprintf(text, TEXT("%.1f"), val);
  } else {
    _stprintf(text, TEXT("%.0f"), val);
  }
}


void Statistics::DrawXGrid(HDC hdc, const RECT rc, 
			   const double tic_step, 
			   const double zero,
                           const int Style, 
			   const double unit_step, bool draw_units) {

  if(INVERTCOLORS)
    SelectObject(hdc, GetStockObject(BLACK_PEN));


  POINT line[2];

  double xval;
  SIZE tsize;

  int xmin, ymin, xmax, ymax;
  if (!tic_step) return;
  LKASSERT(tic_step!=0);

  //  bool do_units = ((x_max-zero)/tic_step)<10;

  for (xval=zero; xval<= x_max; xval+= tic_step) {

    xmin = (int)((xval-x_min)*xscale)+rc.left+BORDER_X;
    ymin = rc.top;
    xmax = xmin;
    ymax = rc.bottom;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax-BORDER_Y;

    // STYLE_THINDASHPAPER
    if ((xval< x_max) 
        && (xmin>=rc.left+BORDER_X) && (xmin<=rc.right)) {
      StyleLine(hdc, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, xval*unit_step/tic_step, unit_step);

//	SetBkMode(hdc, OPAQUE);
	GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	ExtTextOut(hdc, xmin-tsize.cx/2, ymax-tsize.cy ,
		   ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);
	SetBkMode(hdc, TRANSPARENT);
      }
    }

  }

  for (xval=zero-tic_step; xval>= x_min; xval-= tic_step) {

    xmin = (int)((xval-x_min)*xscale)+rc.left+BORDER_X;
    ymin = rc.top;
    xmax = xmin;
    ymax = rc.bottom;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax-BORDER_Y;

    // STYLE_THINDASHPAPER

    if ((xval> x_min) 
        && (xmin>=rc.left+BORDER_X) && (xmin<=rc.right)) {

      StyleLine(hdc, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, xval*unit_step/tic_step, unit_step);
//	SetBkMode(hdc, OPAQUE);
	GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	ExtTextOut(hdc, xmin-tsize.cx/2, ymax-tsize.cy,
		   ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);
	SetBkMode(hdc, TRANSPARENT);
      }
    }

  }

}

void Statistics::DrawYGrid(HDC hdc, const RECT rc, 
			   const double tic_step, 
			   const double zero,
                           const int Style, 
			   const double unit_step, bool draw_units) {

  POINT line[2];
  SIZE tsize;
  double yval;

  if(INVERTCOLORS)
    SelectObject(hdc, GetStockObject(BLACK_PEN));


  int xmin, ymin, xmax, ymax;

  if (!tic_step) return;

  for (yval=zero; yval<= y_max; yval+= tic_step) {

    xmin = rc.left;
    ymin = (int)((y_max-yval)*yscale)+rc.top;
    xmax = rc.right;
    ymax = ymin;
    line[0].x = xmin+BORDER_X;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    // STYLE_THINDASHPAPER
    if ((yval< y_max) && 
        (ymin>=rc.top) && (ymin<=rc.bottom-BORDER_Y)) {

      StyleLine(hdc, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, yval*unit_step/tic_step, unit_step);
//	SetBkMode(hdc, OPAQUE);
	GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	ExtTextOut(hdc, xmin, ymin-tsize.cy/2,
		   ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);
//	SetBkMode(hdc, TRANSPARENT);
      }
    }
  }

  for (yval=zero-tic_step; yval>= y_min; yval-= tic_step) {

    xmin = rc.left;
    ymin = (int)((y_max-yval)*yscale)+rc.top;
    xmax = rc.right;
    ymax = ymin;
    line[0].x = xmin+BORDER_X;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    // STYLE_THINDASHPAPER
    if ((yval> y_min) &&
        (ymin>=rc.top) && (ymin<=rc.bottom-BORDER_Y)) {

      StyleLine(hdc, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, yval*unit_step/tic_step, unit_step);
//	SetBkMode(hdc, OPAQUE);
	GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	ExtTextOut(hdc, xmin, ymin-tsize.cy/2,
		   ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);
//	SetBkMode(hdc, TRANSPARENT);
      }
    }
  }
}




#include "ContestMgr.h"
using std::min;
using std::max;

static CContestMgr::TType contestType = CContestMgr::TYPE_OLC_CLASSIC;


void Statistics::RenderBarograph(HDC hdc, const RECT rc)
{

  if (flightstats.Altitude.sum_n<2) {
    DrawNoData(hdc, rc);
    return;
  }

  ResetScale();

  ScaleXFromData(rc, &flightstats.Altitude);
  ScaleYFromData(rc, &flightstats.Altitude);
  ScaleYFromValue(rc, 0);
  ScaleXFromValue(rc, 1.2f*(flightstats.Altitude.x_min+1.0)); // in case no data
  ScaleXFromValue(rc, flightstats.Altitude.x_min); 

#if  (WINDOWSPC > 0)
  if(INVERTCOLORS)
#else
  if(ISCAR && INVERTCOLORS)
#endif
	RenderSky( hdc,   rc, SKY_HORIZON_COL , SKY_SPACE_COL, GC_NO_COLOR_STEPS );

  for(int j=1;j<MAXTASKPOINTS;j++) {
    if (ValidTaskPoint(j) && (flightstats.LegStartTime[j]>=0)) {
      double xx = 
        (flightstats.LegStartTime[j]-CALCULATED_INFO.TakeOffTime)/3600.0;
      if (xx>=0) {
        DrawLine(hdc, rc,
                 xx, y_min,
                 xx, y_max,
                 STYLE_REDTHICK);
      }
    }
  }

  HPEN   hpHorizonGround;
  HBRUSH hbHorizonGround;
  hpHorizonGround = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1), 
                                    GROUND_COLOUR);
  hbHorizonGround = (HBRUSH)CreateSolidBrush(GROUND_COLOUR);
  HPEN  oldPen = (HPEN) SelectObject(hdc, hpHorizonGround);
  HBRUSH oldBrush =(HBRUSH) SelectObject(hdc, hbHorizonGround);

  DrawFilledLineGraph(hdc, rc, &flightstats.Altitude_Terrain,
                GROUND_COLOUR);

  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);
  DeleteObject(hpHorizonGround);
  DeleteObject(hbHorizonGround);

  DrawXGrid(hdc, rc, 
            0.5, flightstats.Altitude.x_min,
            STYLE_THINDASHPAPER, 0.5, true);


  if(Units::GetUserInvAltitudeUnit() == unFeet) {
    DrawYGrid(hdc, rc, 500.0/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER, 500.0, true);
  } else {
    DrawYGrid(hdc, rc, 1000.0/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER, 1000.0, true);
  }
  DrawLineGraph(hdc, rc, &flightstats.Altitude,
                STYLE_MEDIUMBLACK);

  DrawTrend(hdc, rc, &flightstats.Altitude_Base, STYLE_BLUETHIN);

  DrawTrend(hdc, rc, &flightstats.Altitude_Ceiling, STYLE_BLUETHIN);

  DrawXLabel(hdc, rc, TEXT("t"));
  DrawYLabel(hdc, rc, TEXT("h"));

}


void Statistics::RenderSpeed(HDC hdc, const RECT rc)
{

  if ((flightstats.Task_Speed.sum_n<2)
      || !ValidTaskPoint(ActiveWayPoint)) {
    DrawNoData(hdc, rc);
    return;
  }

  ResetScale();

  ScaleXFromData(rc, &flightstats.Task_Speed);
  ScaleYFromData(rc, &flightstats.Task_Speed);
  ScaleYFromValue(rc, 0);
  ScaleXFromValue(rc, flightstats.Task_Speed.x_min+1.0); // in case no data
  ScaleXFromValue(rc, flightstats.Task_Speed.x_min); 

  for(int j=1;j<MAXTASKPOINTS;j++) {
    if (ValidTaskPoint(j) && (flightstats.LegStartTime[j]>=0)) {
      double xx = 
        (flightstats.LegStartTime[j]-CALCULATED_INFO.TaskStartTime)/3600.0;
      if (xx>=0) {
        DrawLine(hdc, rc,
                 xx, y_min,
                 xx, y_max,
                 STYLE_REDTHICK);
      }
    }
  }

  DrawXGrid(hdc, rc, 
            0.5, flightstats.Task_Speed.x_min,
            STYLE_THINDASHPAPER, 0.5, true);

 /* DrawYGrid(hdc, rc, 10/TASKSPEEDMODIFY, 0, STYLE_THINDASHPAPER,
            10, true);*/
  if(Units::GetUserHorizontalSpeedUnit() == unStatuteMilesPerHour) {
    DrawYGrid(hdc, rc, 5.0/TASKSPEEDMODIFY, 0, STYLE_THINDASHPAPER, 5.0, true);
  } else {
    DrawYGrid(hdc, rc, 10/TASKSPEEDMODIFY, 0, STYLE_THINDASHPAPER, 10, true);
  }

  DrawLineGraph(hdc, rc, &flightstats.Task_Speed,
                STYLE_MEDIUMBLACK);

  DrawTrend(hdc, rc, &flightstats.Task_Speed, STYLE_BLUETHIN);

  DrawXLabel(hdc, rc, TEXT("t"));
  DrawYLabel(hdc, rc, TEXT("V"));

}



void Statistics::RenderClimb(HDC hdc, const RECT rc) 
{

  if (flightstats.ThermalAverage.sum_n<1) {
    DrawNoData(hdc, rc);
    return;
  }
  HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
  ResetScale();
  ScaleYFromData(rc, &flightstats.ThermalAverage);
  ScaleYFromValue(rc, (MACCREADY+1.0));
  ScaleYFromValue(rc, 0);

  ScaleXFromValue(rc, -1);
  ScaleXFromValue(rc, flightstats.ThermalAverage.sum_n);

  if(Units::GetUserInvAltitudeUnit() == unFeet) {
    DrawYGrid(hdc, rc, 0.5/LIFTMODIFY, 0, STYLE_THINDASHPAPER, 0.5, true);
  } else {
    DrawYGrid(hdc, rc, 1.0/LIFTMODIFY, 0, STYLE_THINDASHPAPER, 1.0, true);
  }

  DrawBarChart(hdc, rc,
               &flightstats.ThermalAverage);

  DrawLine(hdc, rc,
           0, MACCREADY, 
           flightstats.ThermalAverage.sum_n,
           MACCREADY,
           STYLE_REDTHICK);

  DrawLabel(hdc, rc, TEXT("MC"), 
	    max(0.5, (double)flightstats.ThermalAverage.sum_n-1), MACCREADY);
  
  DrawTrendN(hdc, rc,
             &flightstats.ThermalAverage,
             STYLE_BLUETHIN);
  SelectObject(hdc, hfOld);
  DrawXLabel(hdc, rc, TEXT("n"));
  DrawYLabel(hdc, rc, TEXT("w"));

}



void Statistics::RenderGlidePolar(HDC hdc, const RECT rc)
{
  int i;

  ResetScale();
  ScaleYFromValue(rc, 0);
  ScaleYFromValue(rc, GlidePolar::SinkRateFast(0,(int)(SAFTEYSPEED-1))*1.1);
  ScaleXFromValue(rc, GlidePolar::Vminsink*0.8);
  ScaleXFromValue(rc, SAFTEYSPEED+2);



  DrawXGrid(hdc, rc, 
            10.0/SPEEDMODIFY, 0,
            STYLE_THINDASHPAPER, 10.0, true);
  DrawYGrid(hdc, rc, 
            1.0/LIFTMODIFY, 0,
            STYLE_THINDASHPAPER, 1.0, true);
  
  double sinkrate0, sinkrate1;
  double v0=0, v1;
  bool v0valid = false;
  int i0=0;

  for (i= GlidePolar::Vminsink; i< SAFTEYSPEED-1;
       i++) {
    
    sinkrate0 = GlidePolar::SinkRateFast(0,i);
    sinkrate1 = GlidePolar::SinkRateFast(0,i+1);

    DrawLine(hdc, rc,
             i, sinkrate0 , 
             i+1, sinkrate1, 
             STYLE_DASHGREEN);

    if (CALCULATED_INFO.AverageClimbRateN[i]>0) {
      v1= CALCULATED_INFO.AverageClimbRate[i]
        /CALCULATED_INFO.AverageClimbRateN[i];

      if (v0valid) {

        DrawLine(hdc, rc,
                 i0, v0 ,
                 i, v1,
                 STYLE_DASHGREEN
                   );


      }
     
      v0 = v1; i0 = i;
      v0valid = true;
    } 

  }

  double ff = SAFTEYSPEED/max(1.0, CALCULATED_INFO.VMacCready);
  double sb = GlidePolar::SinkRate(CALCULATED_INFO.VMacCready);
  ff= (sb-MACCREADY)/max(1.0, CALCULATED_INFO.VMacCready);

  DrawLine(hdc, rc,
           0, MACCREADY, 
           SAFTEYSPEED,
           MACCREADY+ff*SAFTEYSPEED,
           STYLE_REDTHICK);

  DrawXLabel(hdc, rc, TEXT("V"));
  DrawYLabel(hdc, rc, TEXT("w"));

  TCHAR text[80];
  SetBkMode(hdc, OPAQUE);
  if(INVERTCOLORS)
    SetTextColor(hdc,RGB(50,50,160));
  else
	SetTextColor(hdc,RGB_WHITE);
  HFONT hfOldU = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  extern void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext);
  LK_wsplitpath(szPolarFile, (WCHAR*) NULL, (WCHAR*) NULL, text, (WCHAR*) NULL);

   ExtTextOut(hdc, rc.left+IBLSCALE(30),
 	               rc.bottom-IBLSCALE(90),
 	               ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  _stprintf(text,TEXT("%s %.0f kg"),  
            gettext(TEXT("_@M814_")), // Weight
	        GlidePolar::GetAUW());
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(70),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  _stprintf(text,TEXT("%s %.1f kg/m2"),  
	             gettext(TEXT("_@M821_")), // Wing load
	             GlidePolar::WingLoading);
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(50),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  SelectObject(hdc, hfOldU);
  SetBkMode(hdc, TRANSPARENT);
}


void Statistics::ScaleMakeSquare(const RECT rc) {
  if (y_max-y_min<=0) return;
  if (rc.bottom-rc.top-BORDER_Y<=0) return;
  double ar = ((double)(rc.right-rc.left-BORDER_X))
    /(rc.bottom-rc.top-BORDER_Y);
  double ard = (x_max-x_min)/(y_max-y_min);
  LKASSERT(ar!=0);
  double armod = ard/ar;
  double delta;

  if (armod<1.0) {
    // need to expand width
    delta = (x_max-x_min)*(1.0/armod-1.0);
    x_max += delta/2.0;
    x_min -= delta/2.0;
  } else {
    // need to expand height
    delta = (y_max-y_min)*(armod-1.0);
    y_max += delta/2.0;
    y_min -= delta/2.0;
  }
  // shrink both by 10%
  delta = (x_max-x_min)*(1.1-1.0);
  x_max += delta/2.0;
  x_min -= delta/2.0;
  delta = (y_max-y_min)*(1.1-1.0);
  y_max += delta/2.0;
  y_min -= delta/2.0;

  yscale = (rc.bottom-rc.top-BORDER_Y)/(y_max-y_min);
  xscale = (rc.right-rc.left-BORDER_X)/(x_max-x_min);
}


void Statistics::RenderTask(HDC hdc, const RECT rc, const bool olcmode)
{
  int i;

  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
  double x1, y1, x2=0, y2=0;
  double lat_c, lon_c;
  double aatradius[MAXTASKPOINTS];

  // find center
  ResetScale();

  if ( (!ValidTaskPoint(0) || !ValidTaskPoint(1)) && !olcmode) {
	DrawNoData(hdc,rc);
	return;
  }

  for (i=0; i<MAXTASKPOINTS; i++) {
    aatradius[i]=0;
  }
  bool nowaypoints = true;
  for (i=0; i<MAXTASKPOINTS; i++) {
    if (ValidTaskPoint(i)) {
      lat1 = WayPointList[Task[i].Index].Latitude;
      lon1 = WayPointList[Task[i].Index].Longitude;
      ScaleYFromValue(rc, lat1);
      ScaleXFromValue(rc, lon1);
      nowaypoints = false;
    }
  }
  if (nowaypoints && !olcmode) {
    DrawNoData(hdc, rc);
    return;
  }

  CPointGPSArray trace;
  CContestMgr::Instance().Trace(trace);
  for(unsigned i=0; i<trace.size(); i++) {
    lat1 = trace[i].Latitude();
    lon1 = trace[i].Longitude();
    ScaleYFromValue(rc, lat1);
    ScaleXFromValue(rc, lon1);
  }
  HFONT hfOldU = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  lat_c = (y_max+y_min)/2;
  lon_c = (x_max+x_min)/2;

  int nwps = 0;

  // find scale
  ResetScale();

  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1-lon_c)*fastcosine(lat1);
  y1 = (lat1-lat_c);
  ScaleXFromValue(rc, x1);
  ScaleYFromValue(rc, y1);

  if (olcmode) goto olcmode;	 // by Mazuk 18.4.2012

  for (i=0; i<MAXTASKPOINTS; i++) {
    if (ValidTaskPoint(i)) {
      nwps++;
      lat1 = WayPointList[Task[i].Index].Latitude;
      lon1 = WayPointList[Task[i].Index].Longitude;
      x1 = (lon1-lon_c)*fastcosine(lat1);
      y1 = (lat1-lat_c);
      ScaleXFromValue(rc, x1);
      ScaleYFromValue(rc, y1);

      if (AATEnabled) {
	double aatlat;
	double aatlon;
	double bearing;
	double radius;

        if (ValidTaskPoint(i+1)) {
          if (Task[i].AATType == SECTOR) {
            radius = Task[i].AATSectorRadius;
          } else {
            radius = Task[i].AATCircleRadius;
          }
          for (int j=0; j<4; j++) {
            bearing = j*360.0/4;
            
            FindLatitudeLongitude(WayPointList[Task[i].Index].Latitude,
                                  WayPointList[Task[i].Index].Longitude, 
                                  bearing, radius,
                                  &aatlat, &aatlon);
            x1 = (aatlon-lon_c)*fastcosine(aatlat);
            y1 = (aatlat-lat_c);
            ScaleXFromValue(rc, x1);
            ScaleYFromValue(rc, y1);
            if (j==0) {
              aatradius[i] = fabs(aatlat-WayPointList[Task[i].Index].Latitude);
            }
          }
        } else {
          aatradius[i] = 0;
        }
      }
    }
  }

olcmode:
  for(unsigned i=0; i<trace.size(); i++) {
    lat1 = trace[i].Latitude();
    lon1 = trace[i].Longitude();
    x1 = (lon1-lon_c)*fastcosine(lat1);
    y1 = (lat1-lat_c);
    ScaleXFromValue(rc, x1);
    ScaleYFromValue(rc, y1);
  }

  ScaleMakeSquare(rc);

  DrawXGrid(hdc, rc, 
            1.0, 0,
            STYLE_THINDASHPAPER, 1.0, false);
  DrawYGrid(hdc, rc, 
            1.0, 0,
            STYLE_THINDASHPAPER, 1.0, false);

  // draw aat areas
  if (!olcmode) {
    if (AATEnabled) {
      for (i=MAXTASKPOINTS-1; i>0; i--) {
	if (ValidTaskPoint(i)) {
	  lat1 = WayPointList[Task[i-1].Index].Latitude;
	  lon1 = WayPointList[Task[i-1].Index].Longitude;
	  lat2 = WayPointList[Task[i].Index].Latitude;
	  lon2 = WayPointList[Task[i].Index].Longitude;
	  x1 = (lon1-lon_c)*fastcosine(lat1);
	  y1 = (lat1-lat_c);
	  x2 = (lon2-lon_c)*fastcosine(lat2);
	  y2 = (lat2-lat_c);
	  
	  SelectObject(hdc, 
		       MapWindow::GetAirspaceBrushByClass(AATASK));
	  SelectObject(hdc, GetStockObject(WHITE_PEN));
	  if (Task[i].AATType == SECTOR) {
	    Segment(hdc,
		    (long)((x2-x_min)*xscale+rc.left+BORDER_X),
		    (long)((y_max-y2)*yscale+rc.top),
		    (long)(aatradius[i]*yscale), 
		    rc, 
		    Task[i].AATStartRadial, 
		    Task[i].AATFinishRadial); 
	  } else {
	    Circle(hdc,
		   (long)((x2-x_min)*xscale+rc.left+BORDER_X),
		   (long)((y_max-y2)*yscale+rc.top),
		   (long)(aatradius[i]*yscale), 
		   rc);
	  }
	}
      }
    }
  }

  // draw track
  for(unsigned i=0; trace.size() && i<trace.size()-1; i++) {
    lat1 = trace[i].Latitude();
    lon1 = trace[i].Longitude();
    lat2 = trace[i+1].Latitude();
    lon2 = trace[i+1].Longitude();
    x1 = (lon1-lon_c)*fastcosine(lat1);
    y1 = (lat1-lat_c);
    x2 = (lon2-lon_c)*fastcosine(lat2);
    y2 = (lat2-lat_c);
    DrawLine(hdc, rc,
	     x1, y1, x2, y2,
	     STYLE_MEDIUMBLACK);
  }

  // draw task lines and labels

  if (!olcmode) {
    for (i=MAXTASKPOINTS-1; i>0; i--) {
      if (ValidTaskPoint(i) && ValidTaskPoint(i-1)) {
        lat1 = WayPointList[Task[i-1].Index].Latitude;
	lon1 = WayPointList[Task[i-1].Index].Longitude;
		if (!ValidTaskPoint(1) ) {
		  lat2 = GPS_INFO.Latitude;
		  lon2 = GPS_INFO.Longitude;
		} else {
		  lat2 = WayPointList[Task[i].Index].Latitude;
		  lon2 = WayPointList[Task[i].Index].Longitude;
		}
	x1 = (lon1-lon_c)*fastcosine(lat1);
	y1 = (lat1-lat_c);
	x2 = (lon2-lon_c)*fastcosine(lat2);
	y2 = (lat2-lat_c);
	
	DrawLine(hdc, rc,
		 x1, y1, x2, y2,
		 STYLE_BLUETHIN /* STYLE_DASHGREEN*/);
	
	TCHAR text[100];
	if ((i==nwps-1) && (Task[i].Index == Task[0].Index)) {
	  _stprintf(text,TEXT("%0d"),1);
	  DrawLabel(hdc, rc, text, x2, y2);
	} else {
	  _stprintf(text,TEXT("%0d"),i+1);
	  DrawLabel(hdc, rc, text, x2, y2);
	}
	
	if ((i==ActiveWayPoint)&&(!AATEnabled)) {
	  lat1 = GPS_INFO.Latitude;
	  lon1 = GPS_INFO.Longitude;
	  x1 = (lon1-lon_c)*fastcosine(lat1);
	  y1 = (lat1-lat_c);
	  DrawLine(hdc, rc,
		   x1, y1, x2, y2,
		   STYLE_REDTHICK);
	}


	
      }
    }

    // draw aat task line 
    
    if (AATEnabled) {
      for (i=MAXTASKPOINTS-1; i>0; i--) {
	if (ValidTaskPoint(i) && ValidTaskPoint(i-1)) {
          if (i==1) {
            lat1 = WayPointList[Task[i-1].Index].Latitude;
            lon1 = WayPointList[Task[i-1].Index].Longitude;
          } else {
            lat1 = Task[i-1].AATTargetLat;
            lon1 = Task[i-1].AATTargetLon;
          }
          lat2 = Task[i].AATTargetLat;
          lon2 = Task[i].AATTargetLon;

          /*	  
	  if (i==ActiveWayPoint) {
	    lat1 = GPS_INFO.Latitude;
	    lon1 = GPS_INFO.Longitude;
	  }
          */

	  x1 = (lon1-lon_c)*fastcosine(lat1);
	  y1 = (lat1-lat_c);
	  x2 = (lon2-lon_c)*fastcosine(lat2);
	  y2 = (lat2-lat_c);
	  
	  DrawLine(hdc, rc,
		   x1, y1, x2, y2,
		   STYLE_REDTHICK);
	}
      }
    }
  }
  
  if(olcmode) {
    CContestMgr::CResult result = CContestMgr::Instance().Result(contestType, true);
    if(result.Type() == contestType) {
      const CPointGPSArray &points = result.PointArray();
      for(unsigned i=0; i<points.size()-1; i++) {
        lat1 = points[i].Latitude();
        lon1 = points[i].Longitude();
        lat2 = points[i+1].Latitude();
        lon2 = points[i+1].Longitude();
        x1 = (lon1-lon_c)*fastcosine(lat1);
        y1 = (lat1-lat_c);
        x2 = (lon2-lon_c)*fastcosine(lat2);
        y2 = (lat2-lat_c);
        int style = STYLE_REDTHICK;
        if((result.Type() == CContestMgr::TYPE_OLC_FAI ||
            result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) &&
           (i==0 || i==3)) {
          // triangle start and finish
          style = STYLE_DASHGREEN;
        }
        else if(result.Predicted() &&
                (result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED ||
                 i == points.size() - 2)) {
          // predicted edge
          style = STYLE_BLUETHIN;
        }
        DrawLine(hdc, rc, x1, y1, x2, y2, style);
      }
      if(result.Type() == CContestMgr::TYPE_OLC_FAI ||
         result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) {
        // draw the last edge of a triangle
        lat1 = points[1].Latitude();
        lon1 = points[1].Longitude();
        lat2 = points[3].Latitude();
        lon2 = points[3].Longitude();
        x1 = (lon1-lon_c)*fastcosine(lat1);
        y1 = (lat1-lat_c);
        x2 = (lon2-lon_c)*fastcosine(lat2);
        y2 = (lat2-lat_c);
        DrawLine(hdc, rc, x1, y1, x2, y2, result.Predicted() ? STYLE_BLUETHIN : STYLE_REDTHICK);
      }
    }
  }

  SelectObject(hdc, hfOldU);
  // Draw aircraft on top
  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1-lon_c)*fastcosine(lat1);
  y1 = (lat1-lat_c);
  SetBkMode(hdc, TRANSPARENT);
  DrawLabel(hdc, rc, TEXT("+"), x1, y1);
}


void Statistics::RenderTemperature(HDC hdc, const RECT rc)
{
  ResetScale();

  int i;
  float hmin= 10000;
  float hmax= -10000;
  float tmin= (float)CuSonde::maxGroundTemperature;
  float tmax= (float)CuSonde::maxGroundTemperature;

  // find range for scaling of graph
  for (i=0; i<CUSONDE_NUMLEVELS-1; i++) {
    if (CuSonde::cslevels[i].nmeasurements) {

      hmin = min(hmin, (float)i);
      hmax = max(hmax, (float)i);
      tmin = min(tmin, (float)min(CuSonde::cslevels[i].tempDry,
			   (double)min(CuSonde::cslevels[i].airTemp,
                                      (double)CuSonde::cslevels[i].dewpoint)));
      tmax = max(tmax, (float)max(CuSonde::cslevels[i].tempDry,
			   (double)max(CuSonde::cslevels[i].airTemp,
			       (double)CuSonde::cslevels[i].dewpoint)));
    }
  }
  if (hmin>= hmax) {
    DrawNoData(hdc, rc);
    return;
  }

  ScaleYFromValue(rc, hmin);
  ScaleYFromValue(rc, hmax);
  ScaleXFromValue(rc, tmin);
  ScaleXFromValue(rc, tmax);

  bool labelDry = false;
  bool labelAir = false;
  bool labelDew = false;

  int ipos = 0;

  for (i=0; i<CUSONDE_NUMLEVELS-1; i++) {

    if (CuSonde::cslevels[i].nmeasurements &&
	CuSonde::cslevels[i+1].nmeasurements) {

      ipos++;

      DrawLine(hdc, rc,
	       CuSonde::cslevels[i].tempDry, i,
	       CuSonde::cslevels[i+1].tempDry, (i+1), 
	       STYLE_REDTHICK);

      DrawLine(hdc, rc,
	       CuSonde::cslevels[i].airTemp, i,
	       CuSonde::cslevels[i+1].airTemp, (i+1), 
	       STYLE_MEDIUMBLACK);

      DrawLine(hdc, rc,
	       CuSonde::cslevels[i].dewpoint, i,
	       CuSonde::cslevels[i+1].dewpoint, i+1, 
	       STYLE_BLUETHIN);

      if (ipos> 2) {
	if (!labelDry) {
	  DrawLabel(hdc, rc, TEXT("DALR"), 
		    CuSonde::cslevels[i+1].tempDry, i);
	  labelDry = true;
	} else {
	  if (!labelAir) {
	    DrawLabel(hdc, rc, TEXT("Air"), 
		      CuSonde::cslevels[i+1].airTemp, i);
	    labelAir = true;
	  } else {
	    if (!labelDew) {
	      DrawLabel(hdc, rc, TEXT("Dew"), 
			CuSonde::cslevels[i+1].dewpoint, i);
	      labelDew = true;
	    }
	  }
	}
      }
    }
  }

  DrawXLabel(hdc, rc, TEXT("T")TEXT(DEG));
  DrawYLabel(hdc, rc, TEXT("h"));
}


// from Calculations.cpp
#include "windanalyser.h"
extern WindAnalyser *windanalyser;

void Statistics::RenderWind(HDC hdc, const RECT rc) 
{
  int numsteps=10;
  int i;
  double h;
  Vector wind;
  bool found=true;
  double mag;

  LeastSquares windstats_mag;

  if (flightstats.Altitude_Ceiling.y_max
      -flightstats.Altitude_Ceiling.y_min<=10) {
    DrawNoData(hdc, rc);
    return;
  }

  for (i=0; i<numsteps ; i++) {

    h = (flightstats.Altitude_Ceiling.y_max-flightstats.Altitude_Base.y_min)*
      i/(double)(numsteps-1)+flightstats.Altitude_Base.y_min; 

    wind = windanalyser->windstore.getWind(GPS_INFO.Time, h, &found);
    mag = sqrt(wind.x*wind.x+wind.y*wind.y);

    windstats_mag.least_squares_update(mag, h);

  }

  //

  ResetScale();

  ScaleXFromData(rc, &windstats_mag);
  ScaleXFromValue(rc, 0);
  ScaleXFromValue(rc, 10.0);

  ScaleYFromData(rc, &windstats_mag);

  DrawXGrid(hdc, rc, 5/SPEEDMODIFY, 0, STYLE_THINDASHPAPER, 5.0, true);
  DrawYGrid(hdc, rc, 1000/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER,
            1000.0, true);

  DrawLineGraph(hdc, rc, &windstats_mag,
                STYLE_MEDIUMBLACK);

#define WINDVECTORMAG 25

  numsteps = (int)((rc.bottom-rc.top)/WINDVECTORMAG)-1;

  // draw direction vectors

  POINT wv[4];
  double dX, dY;
  double angle;
  double hfact;
  for (i=0; i<numsteps ; i++) {
    hfact = (i+1)/(double)(numsteps+1);
    h = (flightstats.Altitude_Ceiling.y_max-flightstats.Altitude_Base.y_min)*
      hfact+flightstats.Altitude_Base.y_min;

    wind = windanalyser->windstore.getWind(GPS_INFO.Time, h, &found);
    if (windstats_mag.x_max == 0)
      windstats_mag.x_max=1;  // prevent /0 problems
    wind.x /= windstats_mag.x_max;
    wind.y /= windstats_mag.x_max;
    mag = sqrt(wind.x*wind.x+wind.y*wind.y);
    if (mag<= 0.0) continue;

    angle = atan2(wind.x,-wind.y)*RAD_TO_DEG;

    wv[0].y = (int)((1-hfact)*(rc.bottom-rc.top-BORDER_Y))+rc.top;
    wv[0].x = (rc.right+rc.left-BORDER_X)/2+BORDER_X;

    dX = (mag*WINDVECTORMAG);
    dY = 0;
    rotate(dX,dY,angle);
    wv[1].x = (int)(wv[0].x + dX);
    wv[1].y = (int)(wv[0].y + dY);
    StyleLine(hdc, wv[0], wv[1], STYLE_MEDIUMBLACK, rc);

    dX = (mag*WINDVECTORMAG-5);
    dY = -3;
    rotate(dX,dY,angle);
    wv[2].x = (int)(wv[0].x + dX);
    wv[2].y = (int)(wv[0].y + dY);
    StyleLine(hdc, wv[1], wv[2], STYLE_MEDIUMBLACK, rc);

    dX = (mag*WINDVECTORMAG-5);
    dY = 3;
    rotate(dX,dY,angle);
    wv[3].x = (int)(wv[0].x + dX);
    wv[3].y = (int)(wv[0].y + dY);

    StyleLine(hdc, wv[1], wv[3], STYLE_MEDIUMBLACK, rc);

  }

  DrawXLabel(hdc, rc, TEXT("w"));
  DrawYLabel(hdc, rc, TEXT("h"));

}



static int page=0;
static WndForm *wf=NULL;
static WndOwnerDrawFrame *wGrid=NULL;
static WndOwnerDrawFrame *wInfo=NULL;
static WndButton *wCalc=NULL;

static void SetCalcCaption(const TCHAR* caption) {
  if (wCalc) {
    wCalc->SetCaption(gettext(caption));
  }
}






static void OnAnalysisPaint(WindowControl * Sender, HDC hDC){

  RECT  rcgfx;
  HFONT hfOld;

  CopyRect(&rcgfx, Sender->GetBoundRect());
  hfOld = (HFONT)SelectObject(hDC,LK8PanelUnitFont/* Sender->GetFont()*/);

  SetBkMode(hDC, TRANSPARENT);
  SetTextColor(hDC, Sender->GetForeColor());

  if(INVERTCOLORS)
  {
    Sender->SetBackColor(SKY_HORIZON_COL);
    Sideview_TextColor = INV_GROUND_TEXT_COLOUR;
  }
  else
    Sideview_TextColor = RGB_WHITE;

  SetTextColor(hDC, Sideview_TextColor);

  switch (page) {
  case ANALYSIS_PAGE_BAROGRAPH:
    SetCalcCaption(gettext(TEXT("_@M885_"))); // Settings
    Statistics::RenderBarograph(hDC, rcgfx);
    break;
  case ANALYSIS_PAGE_CLIMB:
    SetCalcCaption(gettext(TEXT("_@M886_"))); // Task calc
    Statistics::RenderClimb(hDC, rcgfx);
    break;
  case ANALYSIS_PAGE_WIND:
    SetCalcCaption(gettext(TEXT("_@M887_"))); // Set wind
    Statistics::RenderWind(hDC, rcgfx);
    break;
  case ANALYSIS_PAGE_POLAR:
    SetCalcCaption(gettext(TEXT("_@M885_"))); // Settings
    Statistics::RenderGlidePolar(hDC, rcgfx);
    break;
  case ANALYSIS_PAGE_TEMPTRACE:
    SetCalcCaption(gettext(TEXT("_@M885_"))); // Settings
    Statistics::RenderTemperature(hDC, rcgfx);
    break;
  case ANALYSIS_PAGE_TASK:
    SetCalcCaption(gettext(TEXT("_@M886_"))); // Task calc
    LockTaskData();
    Statistics::RenderTask(hDC, rcgfx, false);
    UnlockTaskData();
    break;
  case ANALYSIS_PAGE_CONTEST:
    SetCalcCaption(gettext(TEXT("_@M1451_"))); // Change
    LockTaskData();
    Statistics::RenderTask(hDC, rcgfx, true);
    UnlockTaskData();
    break;

#ifdef DDDDD
  case ANALYSIS_PAGE_AIRSPACE:
#if USESONAR
    if(Sideview_asp_heading_task !=2)
      SetCalcCaption(gettext(TEXT("_@M888_"))); // Warnings
    else
    {
      if(Sonar_IsEnabled)
        SetCalcCaption(gettext(TEXT("_@M1294_"))); //  _@M1294_ "Sonar Off"
      else
        SetCalcCaption(gettext(TEXT("_@M1293_"))); //  _@M1293_ "Sonar On"
    }
#else
      SetCalcCaption(gettext(TEXT("_@M888_"))); // Warnings
#endif
      MapWindow::RenderAirspace(hDC, rcgfx);
    break;
#endif

  case ANALYSIS_PAGE_TASK_SPEED:
    SetCalcCaption(gettext(TEXT("_@M886_"))); // Task calc
    LockTaskData();
    Statistics::RenderSpeed(hDC, rcgfx);
    UnlockTaskData();
    break;
  default:
    // should never get here!
    break;
  }
  SelectObject(hDC, hfOld);

}



static void Update(void){
  TCHAR sTmp[1000];

  TCHAR szPolarName[80];
  extern void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext);
  LK_wsplitpath(szPolarFile, (WCHAR*) NULL, (WCHAR*) NULL, szPolarName, (WCHAR*) NULL);

  //  WndProperty *wp;

  // Hide airspace heading switch button by default if not on ASP page
  if (page != ANALYSIS_PAGE_AIRSPACE) {
    WndButton *wb = (WndButton *)wf->FindByName(TEXT("cmdAspBear"));
    if(wb) {
      wb->SetVisible(false);
    }
  }


  switch(page){
    case ANALYSIS_PAGE_BAROGRAPH:
      _stprintf(sTmp, TEXT("%s: %s"),
	// LKTOKEN  _@M93_ = "Analysis" 
                gettext(TEXT("_@M93_")), 
	// LKTOKEN  _@M127_ = "Barograph" 
                gettext(TEXT("_@M127_")));
      wf->SetCaption(sTmp);
      if (flightstats.Altitude_Ceiling.sum_n<2) {
        _stprintf(sTmp, TEXT("\0"));
      } else if (flightstats.Altitude_Ceiling.sum_n<4) {
        _stprintf(sTmp, TEXT("%s:\r\n  %.0f-%.0f %s"),
	// LKTOKEN  _@M823_ = "Working band" 
                  gettext(TEXT("_@M823_")),
                  flightstats.Altitude_Base.y_ave*ALTITUDEMODIFY,
                  flightstats.Altitude_Ceiling.y_ave*ALTITUDEMODIFY,
                  Units::GetAltitudeName());
        
      } else {

        _stprintf(sTmp, TEXT("%s:\r\n  %.0f-%.0f %s\r\n\r\n%s:\r\n  %.0f %s/hr"),
	// LKTOKEN  _@M823_ = "Working band" 
                  gettext(TEXT("_@M823_")),
                  flightstats.Altitude_Base.y_ave*ALTITUDEMODIFY,
                  flightstats.Altitude_Ceiling.y_ave*ALTITUDEMODIFY,
                  Units::GetAltitudeName(),
	// LKTOKEN  _@M165_ = "Ceiling trend" 
                  gettext(TEXT("_@M165_")),
                  flightstats.Altitude_Ceiling.m*ALTITUDEMODIFY,
                  Units::GetAltitudeName());
      }
      wInfo->SetCaption(sTmp);

    break;
    case ANALYSIS_PAGE_CLIMB:
      _stprintf(sTmp, TEXT("%s: %s"), 
	// LKTOKEN  _@M93_ = "Analysis" 
                gettext(TEXT("_@M93_")),
	// LKTOKEN  _@M182_ = "Climb" 
                gettext(TEXT("_@M182_")));
      wf->SetCaption(sTmp);

      if (flightstats.ThermalAverage.sum_n==0) {
        _stprintf(sTmp, TEXT("\0"));
      } else if (flightstats.ThermalAverage.sum_n==1) {
        _stprintf(sTmp, TEXT("%s:\r\n  %3.1f %s"),
	// LKTOKEN  _@M116_ = "Av climb" 
                  gettext(TEXT("_@M116_")),
                  flightstats.ThermalAverage.y_ave*LIFTMODIFY,
                  Units::GetVerticalSpeedName()
                  );
      } else {
        _stprintf(sTmp, TEXT("%s:\r\n  %3.1f %s\r\n\r\n%s:\r\n  %3.2f %s"),
	// LKTOKEN  _@M116_ = "Av climb" 
                  gettext(TEXT("_@M116_")),
                  flightstats.ThermalAverage.y_ave*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),                    
	// LKTOKEN  _@M181_ = "Climb trend" 
                  gettext(TEXT("_@M181_")),
                  flightstats.ThermalAverage.m*LIFTMODIFY,
                  Units::GetVerticalSpeedName()
                  );
      }

      wInfo->SetCaption(sTmp);

    break;
    case ANALYSIS_PAGE_WIND:
      _stprintf(sTmp, TEXT("%s: %s"), 
	// LKTOKEN  _@M93_ = "Analysis" 
                gettext(TEXT("_@M93_")),
	// LKTOKEN  _@M820_ = "Wind at Altitude" 
                gettext(TEXT("_@M820_")));
      wf->SetCaption(sTmp);
      _stprintf(sTmp, TEXT(" "));
      wInfo->SetCaption(sTmp);
    break;
    case ANALYSIS_PAGE_POLAR:
      _stprintf(sTmp, TEXT("%s: %s %s (%s %3.0f kg)"),
	// LKTOKEN  _@M93_ = "Analysis" 
                gettext(TEXT("_@M93_")),
	// LKTOKEN  _@M325_ = "Glide Polar"
                szPolarName,
                gettext(TEXT("_@M325_")),
                gettext(TEXT("_@M889_")), // Mass
                GlidePolar::GetAUW());
      wf->SetCaption(sTmp);
      if (ScreenLandscape) {
        _stprintf(sTmp, TEXT("%s:\r\n  %3.0f\r\n  @ %3.0f %s\r\n\r\n%s:\r\n%3.2f %s\r\n  @ %3.0f %s"),
	// LKTOKEN  _@M140_ = "Best LD" 
                  gettext(TEXT("_@M140_")),
                  GlidePolar::bestld,
                  GlidePolar::Vbestld*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName(),
	// LKTOKEN  _@M437_ = "Min sink" 
                  gettext(TEXT("_@M437_")),
                  GlidePolar::minsink*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),
                  GlidePolar::Vminsink*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName()
                  );
      } else {
        _stprintf(sTmp, TEXT("%s:\r\n  %3.0f @ %3.0f %s\r\n%s:\r\n  %3.2f %s @ %3.0f %s"),
	// LKTOKEN  _@M140_ = "Best LD" 
                  gettext(TEXT("_@M140_")),
                  GlidePolar::bestld,
                  GlidePolar::Vbestld*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName(),
	// LKTOKEN  _@M437_ = "Min sink" 
                  gettext(TEXT("_@M437_")),
                  GlidePolar::minsink*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),
                  GlidePolar::Vminsink*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
      }
      wInfo->SetCaption(sTmp);
    break;
  case ANALYSIS_PAGE_TEMPTRACE:
    _stprintf(sTmp, TEXT("%s: %s"), 
	// LKTOKEN  _@M93_ = "Analysis" 
              gettext(TEXT("_@M93_")),
	// LKTOKEN  _@M701_ = "Temp trace" 
              gettext(TEXT("_@M701_")));
    wf->SetCaption(sTmp);

    _stprintf(sTmp, TEXT("%s:\r\n  %5.0f %s\r\n\r\n%s:\r\n  %5.0f %s\r\n"),
	// LKTOKEN  _@M714_ = "Thermal height" 
	      gettext(TEXT("_@M714_")),
	      CuSonde::thermalHeight*ALTITUDEMODIFY,
	      Units::GetAltitudeName(),
	// LKTOKEN  _@M187_ = "Cloud base" 
	      gettext(TEXT("_@M187_")),
	      CuSonde::cloudBase*ALTITUDEMODIFY,
	      Units::GetAltitudeName());

    wInfo->SetCaption(sTmp);
    break;
  case ANALYSIS_PAGE_TASK_SPEED:
    _stprintf(sTmp, TEXT("%s: %s"), 
	// LKTOKEN  _@M93_ = "Analysis" 
              gettext(TEXT("_@M93_")),
	// LKTOKEN  _@M697_ = "Task speed" 
              gettext(TEXT("_@M697_")));
    wf->SetCaption(sTmp);
    wInfo->SetCaption(TEXT(""));
    break;
  case ANALYSIS_PAGE_TASK:
    _stprintf(sTmp, TEXT("%s: %s"), 
	// LKTOKEN  _@M93_ = "Analysis" 
              gettext(TEXT("_@M93_")),
	// LKTOKEN  _@M699_ = "Task" 
              gettext(TEXT("_@M699_")));
    wf->SetCaption(sTmp);

    RefreshTaskStatistics();

    if (!ValidTaskPoint(ActiveWayPoint)) {
	// LKTOKEN  _@M476_ = "No task" 
      _stprintf(sTmp, gettext(TEXT("_@M476_")));
    } else {
      TCHAR timetext1[100];
      TCHAR timetext2[100];
      if (AATEnabled) {
        Units::TimeToText(timetext1, (int)CALCULATED_INFO.TaskTimeToGo);
        Units::TimeToText(timetext2, (int)CALCULATED_INFO.AATTimeToGo);
       
        if (ScreenLandscape) {
          _stprintf(sTmp, 
                    TEXT("%s:\r\n  %s\r\n%s:\r\n  %s\r\n%s:\r\n  %5.0f %s\r\n%s%.0f %s\r\n"), // 100429
	// LKTOKEN  _@M698_ = "Task to go" 
                    gettext(TEXT("_@M698_")),
                    timetext1,
	// LKTOKEN  _@M42_ = "AAT to go" 
                    gettext(TEXT("_@M42_")),
                    timetext2,
	// LKTOKEN  _@M242_ = "Dist to go" 
                    gettext(TEXT("_@M242_")),
                    DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance,
                    Units::GetDistanceName(),
	// LKTOKEN  _@M626_ = "Sp " 
                    gettext(TEXT("_@M626_")),
                    TASKSPEEDMODIFY*CALCULATED_INFO.AATTargetSpeed,
                    Units::GetTaskSpeedName()		
                    );
        } else {
          _stprintf(sTmp, 
                    TEXT("%s: %s\r\n%s: %s\r\n%s: %5.0f %s\r\n%s: %5.0f %s\r\n"),
	// LKTOKEN  _@M698_ = "Task to go" 
                    gettext(TEXT("_@M698_")),
                    timetext1,
	// LKTOKEN  _@M42_ = "AAT to go" 
                    gettext(TEXT("_@M42_")),
                    timetext2,
	// LKTOKEN  _@M242_ = "Dist to go" 
                    gettext(TEXT("_@M242_")),
                    DISTANCEMODIFY*CALCULATED_INFO.AATTargetDistance,
                    Units::GetDistanceName(),
	// LKTOKEN  _@M681_ = "Targ.speed" 
                    gettext(TEXT("_@M681_")),
                    TASKSPEEDMODIFY*CALCULATED_INFO.AATTargetSpeed,
                    Units::GetTaskSpeedName()		
                    );
        }
      } else {
        Units::TimeToText(timetext1, (int)CALCULATED_INFO.TaskTimeToGo);
        _stprintf(sTmp, TEXT("%s: %s\r\n%s: %5.0f %s\r\n"),
	// LKTOKEN  _@M698_ = "Task to go" 
                  gettext(TEXT("_@M698_")),
                  timetext1,
	// LKTOKEN  _@M242_ = "Dist to go" 
                  gettext(TEXT("_@M242_")),
                  DISTANCEMODIFY*CALCULATED_INFO.TaskDistanceToGo,
                  Units::GetDistanceName());
      }
    } 
    wInfo->SetCaption(sTmp);
    break;
  case ANALYSIS_PAGE_CONTEST:
    _stprintf(sTmp, TEXT("%s: %s - %s"), 
              // LKTOKEN  _@M93_ = "Analysis" 
              gettext(TEXT("_@M93_")),
              // LKTOKEN  _@M1450_ = "Contest" 
              gettext(TEXT("_@M1450_")),
              CContestMgr::TypeToString(contestType));
    wf->SetCaption(sTmp);
    
    {
      CContestMgr::CResult result = CContestMgr::Instance().Result(contestType, false);
      if(result.Type() == contestType) {
        TCHAR distStr[50];
        _stprintf(distStr, _T("%.1f %s\r\n"),
                  DISTANCEMODIFY * result.Distance(),
                  Units::GetDistanceName());
        
        TCHAR speedStr[50];
        _stprintf(speedStr, TEXT("%.1f %s\r\n"),
                  TASKSPEEDMODIFY * result.Speed(),
                  Units::GetTaskSpeedName());
        
        TCHAR timeTempStr[50];
        Units::TimeToText(timeTempStr, result.Duration());
        TCHAR timeStr[50];
        _stprintf(timeStr, _T("%s\r\n"), timeTempStr);
        
        TCHAR scoreStr[50] = _T("");
        if(result.Type() != CContestMgr::TYPE_FAI_3_TPS &&
           result.Type() != CContestMgr::TYPE_FAI_3_TPS_PREDICTED)
          _stprintf(scoreStr, TEXT("%.2f pt\r\n"), result.Score());
        
        TCHAR plusStr[50] = _T("");
        if(result.Type() == CContestMgr::TYPE_OLC_CLASSIC ||
           result.Type() == CContestMgr::TYPE_OLC_CLASSIC_PREDICTED ||
           result.Type() == CContestMgr::TYPE_OLC_FAI ||
           result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) {
          CContestMgr::TType type = (result.Type() == CContestMgr::TYPE_OLC_CLASSIC_PREDICTED ||
                                     result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) ?
            CContestMgr::TYPE_OLC_PLUS_PREDICTED : CContestMgr::TYPE_OLC_PLUS;
          CContestMgr::CResult resultPlus = CContestMgr::Instance().Result(type, false);
          if(ScreenLandscape)
            _stprintf(plusStr, TEXT("\r\n%s:\r\n%.2f pt"),
                      CContestMgr::TypeToString(type),
                      resultPlus.Score());
          else
            _stprintf(plusStr, TEXT("\r\n%s: %.2f pt"),
                      CContestMgr::TypeToString(type),
                      resultPlus.Score());
        }
        
        _stprintf(sTmp, _T("%s%s%s%s%s"), distStr, speedStr, timeStr, scoreStr, plusStr);
      }
      else {
        _stprintf(sTmp, TEXT("%s\r\n"),
                  // LKTOKEN  _@M477_ = "No valid path" 
                  gettext(TEXT("_@M477_")));
      }
      wInfo->SetCaption(sTmp);
    }

    break;
  case ANALYSIS_PAGE_AIRSPACE:
	switch (Sideview_asp_heading_task )
	{
	case 1:
	   _stprintf(sTmp, TEXT("%s: %s %s"),
	  // LKTOKEN  _@M93_ = "Analysis"
	                gettext(TEXT("_@M93_")),
	  //_@M1289_ "Next WP"
	                gettext(TEXT("_@M1289_")),
	  // LKTOKEN   _@M1295_ "Sideview"
	                gettext(TEXT("_@M1295_"))
	   );
	break;
	case 2:
      _stprintf(sTmp, TEXT("%s: %s %s"),
	  // LKTOKEN  _@M93_ = "Analysis"
              gettext(TEXT("_@M93_")),
	  // LKTOKEN  _@M1292_ = "Nearest Airspace"
              gettext(TEXT("_@M1292_")),
              // LKTOKEN   _@M1295_ "Sideview"
              gettext(TEXT("_@M1295_"))
              );
    break;
	default:
	case 0:
      _stprintf(sTmp, TEXT("%s: %s %s"),
	  // LKTOKEN  _@M93_ = "Analysis"
              gettext(TEXT("_@M93_")),
              //_@M1287_ "Heading"
              gettext(TEXT("_@M1287_")),
              // LKTOKEN   _@M1295_ "Sideview"
              gettext(TEXT("_@M1295_"))
              );
     break;
	}

    wf->SetCaption(sTmp);
    WndButton *wb = (WndButton *)wf->FindByName(TEXT("cmdAspBear"));
    int overindex = GetOvertargetIndex();
    TCHAR ovtname[LKSIZEBUFFERLARGE];
    if(wb) {
      wb->SetVisible(true);

      switch (Sideview_asp_heading_task)
      {
        case 0:
          wb->SetCaption(gettext(TEXT("_@M1289_")));                               //_@M1289_ "Next WP"
          wInfo->SetCaption(gettext(TEXT("_@M1290_")));                            //_@M1290_ "Showing towards heading"
        break;
        case 1:
          wb->SetCaption(gettext(TEXT("_@M1291_")));                               //_@M1291_ "Near AS"
          if (overindex>=0)
          {
            GetOvertargetName(ovtname);
            _stprintf(sTmp, TEXT("%s: %s"), gettext(TEXT("_@M1288_")), ovtname);                //_@M1288_ "Showing towards next waypoint"
            wInfo->SetCaption(sTmp);
          }
          else
          {
            _stprintf(sTmp, TEXT("%s: %s"), gettext(TEXT("_@M1288_")), gettext(TEXT("_@M479_")));                    //_@M1288_ "Showing towards next waypoint"  _@M479_ "None"
            wInfo->SetCaption(sTmp);
          }

        break;
        case 2:
          wb->SetCaption(gettext(TEXT("_@M1287_")));                               //_@M1287_ "Heading"                              //_@M1287_ "Heading"
          _stprintf(sTmp, TEXT("%s"), Sideview_szNearAS );                  //"Showing nearest airspace"
          wInfo->SetCaption(sTmp);//

        break;
      }
    }
    break;
  }

  wGrid->SetVisible(page<MAXPAGE+1);

  if (wGrid != NULL)
    wGrid->Redraw();

}

static void NextPage(int Step){
  page += Step;

LockFlightData(); /* skip Temperature page if no OAT available */
  if(page == ANALYSIS_PAGE_TEMPTRACE)
    if(GPS_INFO.TemperatureAvailable!=TRUE)
      page += Step;
UnlockFlightData();

  if (page > MAXPAGE)
    page = 0;
  if (page < 0)
    page = MAXPAGE;
  Update();
}


static void OnNextClicked(WindowControl * Sender){
	(void)Sender;

  NextPage(+1);
}

static void OnPrevClicked(WindowControl * Sender){
	(void)Sender;
  NextPage(-1);
}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;

  wf->SetModalResult(mrOK);
}


static int FormKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
  (void)Sender; (void)lParam;

  if (wGrid->GetFocused())
    return(0);
  
  switch(wParam & 0xffff){
    case VK_LEFT:
    case '6':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdPrev")))->GetHandle());
      NextPage(-1);
      //((WndButton *)wf->FindByName(TEXT("cmdPrev")))->SetFocused(true, NULL);
    return(0);
    case VK_RIGHT:
    case '7':
      SetFocus(((WndButton *)wf->FindByName(TEXT("cmdNext")))->GetHandle());
      NextPage(+1);
      //((WndButton *)wf->FindByName(TEXT("cmdNext")))->SetFocused(true, NULL);
    return(0);
  }
  return(1);
}

static void OnCalcClicked(WindowControl * Sender, 
			  WndListFrame::ListInfo_t *ListInfo){
  (void)ListInfo;
  (void)Sender;
  if (page==ANALYSIS_PAGE_BAROGRAPH) {
    dlgBasicSettingsShowModal();
  }
  if (page==ANALYSIS_PAGE_CLIMB) {
    wf->SetVisible(false);
    dlgTaskCalculatorShowModal();
    wf->SetVisible(true);
  }
  if (page==ANALYSIS_PAGE_WIND) {
    dlgWindSettingsShowModal();
  }
  if (page==ANALYSIS_PAGE_POLAR) {
    dlgBasicSettingsShowModal();
  }
  if (page==ANALYSIS_PAGE_TEMPTRACE) {
    dlgBasicSettingsShowModal();
  }
  if ((page==ANALYSIS_PAGE_TASK) || (page==ANALYSIS_PAGE_TASK_SPEED)) {
    wf->SetVisible(false);
    dlgTaskCalculatorShowModal();
    wf->SetVisible(true);
  }
  if (page==ANALYSIS_PAGE_CONTEST) {
    // Rotate presented contest
    switch(contestType) {
    case CContestMgr::TYPE_OLC_CLASSIC:
      contestType = CContestMgr::TYPE_OLC_FAI;
      break;
    case CContestMgr::TYPE_OLC_FAI:
      contestType = CContestMgr::TYPE_OLC_CLASSIC_PREDICTED;
      break;
    case CContestMgr::TYPE_OLC_CLASSIC_PREDICTED:
      contestType = CContestMgr::TYPE_OLC_FAI_PREDICTED;
      break;
    case CContestMgr::TYPE_OLC_FAI_PREDICTED:
      contestType = CContestMgr::TYPE_OLC_LEAGUE;
      break;
    case CContestMgr::TYPE_OLC_LEAGUE:
      contestType = CContestMgr::TYPE_FAI_3_TPS;
      break;
    case CContestMgr::TYPE_FAI_3_TPS:
      contestType = CContestMgr::TYPE_FAI_3_TPS_PREDICTED;
      break;
    case CContestMgr::TYPE_FAI_3_TPS_PREDICTED:
      contestType = CContestMgr::TYPE_OLC_CLASSIC;
      break;
    default:
      contestType = CContestMgr::TYPE_OLC_CLASSIC;
    }
  }
  if (page==ANALYSIS_PAGE_AIRSPACE) {
	#if USESONAR
	  if(Sideview_asp_heading_task != 2)
	    dlgAirspaceWarningParamsShowModal();
	  else
		Sonar_IsEnabled = !Sonar_IsEnabled;
	#else
	    dlgAirspaceWarningParamsShowModal();
	#endif
  }
  Update();
}

static void OnAspBearClicked(WindowControl * Sender){
  (void)Sender;
//  Sideview_asp_heading_task++;
//  Sideview_asp_heading_task %=3;
    Update();
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnAnalysisPaint),
  DeclareCallBackEntry(OnNextClicked),
  DeclareCallBackEntry(OnPrevClicked),
  DeclareCallBackEntry(OnCalcClicked),
  DeclareCallBackEntry(OnAspBearClicked),
  DeclareCallBackEntry(NULL)
};

#ifdef RRRRR
static int OnTimerNotify(WindowControl *Sender)
{
  static short i=0;


static unsigned long lSonarCnt = 0;

   lSonarCnt++;

   if(Sideview_asp_heading_task== 2)
     if((iSonarLevel >=0) && (iSonarLevel < 10))
      if( lSonarCnt > (unsigned)sSonarLevel[iSonarLevel].iSoundDelay)
		{
		  lSonarCnt = 0;
                  // StartupStore(_T("... level=%d PLAY <%s>\n"),iSonarLevel,&sSonarLevel[iSonarLevel].szSoundFilename);
		  LKSound((TCHAR*) &(sSonarLevel[iSonarLevel].szSoundFilename));
		}


  if(i++ % 2 == 0) // run once per second
    return 0;

  Update();
  return 0;
}
#endif



#define ms  (1)
#define sec (1000*ms)
#define mnt (60*sec)
#define hr  (60*mnt)
#define day (24*hr)
long GetTimeMs(void)
{
  SYSTEMTIME sysTime;
  long lTime=0;

  ::GetSystemTime(&sysTime);
  lTime =   sysTime.wHour         * hr +
	        sysTime.wMinute       * mnt+
	        sysTime.wSecond       * sec+
	        sysTime.wMilliseconds * ms   ;

  return lTime;
}

static int TouchKeyDown(WindowControl * Sender, WPARAM wParam, LPARAM lParam){
        (void)lParam;
        (void)wParam;

 int X = LOWORD(lParam);
 int Y = HIWORD(lParam);
 int k;
/*
 static unsigned long LastClick =0;
 static int iLast_X =0;
 static int iLast_Y =0;
unsigned long ulTimeNow = GetTimeMs();
if(ulTimeNow - LastClick > 1000*ms)
{
	LastClick = ulTimeNow;
	return 0;
}

 if(ulTimeNow - LastClick > 400*ms)
 {
  if(abs(Y-iLast_Y) < 50)
   if(abs(X-iLast_X) < 50)
   {
   if (X<180) return 1;
   if (TouchContext< TCX_PROC_UP) {
     #ifndef DISABLEAUDIO
     if (EnableSoundModes) PlayResource(TEXT("IDR_WAV_CLICK"));
     #endif
     NextPage(+1);
     return 0;
   }
  }
 }

 iLast_X = X;
 iLast_Y = Y;
 LastClick = ulTimeNow;
*/


#define ULLI_ASPSELECTION
#ifdef ULLI_ASPSELECTION


if(page ==ANALYSIS_PAGE_AIRSPACE)
{
  if (TouchContext ==  TCX_PROC_UP)
  {
	 for (k=0 ; k <= Sideview_iNoHandeldSpaces; k++)
	 {
	   if( Sideview_pHandeled[k].psAS != NULL)
	   {
		 if (PtInRect(X,Y,Sideview_pHandeled[k].rc ))
		 {
	       if (EnableSoundModes)PlayResource(TEXT("IDR_WAV_BTONE4"));
           dlgAirspaceDetails(Sideview_pHandeled[k].psAS);       // dlgA
		 }
	   }
	 }
   }


}
#endif // ULLI


 return 1;
}









void dlgAnalysisShowModal(int inpage){
static bool entered = false;


if(inpage == ANALYSIS_PAGE_NEAR_AIRSPACE)
{
  inpage = ANALYSIS_PAGE_AIRSPACE;
  Sideview_asp_heading_task = 2;
  page = ANALYSIS_PAGE_AIRSPACE;
 // Update();
}



if (entered == true) /* prevent re entrance */
	return;

  wf=NULL;
  wGrid=NULL;
  wInfo=NULL;
  wCalc=NULL;
 entered = true;
  if (!ScreenLandscape) {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAnalysis_L.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_ANALYSIS_L"));
  } else  {
    char filename[MAX_PATH];
    LocalPathS(filename, TEXT("dlgAnalysis.xml"));
    wf = dlgLoadFromXML(CallBackTable, 
                        filename, 
                        hWndMainWindow,
                        TEXT("IDR_XML_ANALYSIS"));
  }

  if (!wf) return;

  COLORREF COL = RGB(50,243,45);
  if(INVERTCOLORS)
	COL = ChangeBrightness(COL,0.7);
  penThinSignal = CreatePen(PS_SOLID, IBLSCALE(2) , COL);

  wf->SetLButtonUpNotify(TouchKeyDown);
  wf->SetKeyDownNotify(FormKeyDown);

  wGrid = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmGrid"));
  wInfo = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmInfo"));
  
  wCalc = ((WndButton *)wf->FindByName(TEXT("cmdCalc")));

  WndButton *wClose = (WndButton *)wf->FindByName(TEXT("cmdClose"));
  if(wClose) {
    wClose->SetOnClickNotify(OnCloseClicked);
  }

  /*
    Does Not Work, Because wf->GetHeigth() Is WindowHeight and ClientRect Is Calculated By OnPaint
    Why WndForm Do Not Use NonClientArea for Border and Caption ?
  */
  /*
  WndFrame *wBtFrm = (WndFrame*)wf->FindByName(TEXT("frmButton"));
  if(wBtFrm) {
    wBtFrm->SetTop(wf->GetHeigth()- wBtFrm->GetHeight());
    if(wInfo) {
      wInfo->SetHeight(wBtFrm->GetTop()-wInfo->GetTop());
    }
  }
  */

  if(wGrid) {
    wGrid->SetWidth( wf->GetWidth() - wGrid->GetLeft()-6);
  }

//  wf->SetTimerNotify(OnTimerNotify);

  Update();

  if (inpage!=ANALYSYS_PAGE_DEFAULT) page=inpage;

  wf->ShowModal();

  delete wf;

  wf = NULL;

  DeleteObject(penThinSignal);


  MapWindow::RequestFastRefresh();
  FullScreen();
  entered = false;
}


