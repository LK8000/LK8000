/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


void Statistics::ResetScale() {
  unscaled_y = true;
  unscaled_x = true;
}


void Statistics::ScaleYFromData(const RECT& rc, LeastSquares* lsdata)
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


void Statistics::ScaleXFromData(const RECT& rc, LeastSquares* lsdata)
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


void Statistics::ScaleYFromValue(const RECT& rc, const double value)
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


void Statistics::ScaleXFromValue(const RECT& rc, const double value)
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


void Statistics::ScaleMakeSquare(const RECT& rc) {
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






int Statistics::ScaleX(const RECT& rc, double x)
{
  return (int)((x-x_min)*xscale)+rc.left+BORDER_X;
}

int Statistics::ScaleY(const RECT& rc,double y)
{
  return(int)((y_max-y)*yscale)+rc.top;
}
