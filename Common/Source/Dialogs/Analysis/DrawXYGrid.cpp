/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Asset.hpp"

void Statistics::DrawXGrid(LKSurface& Surface, const RECT& rc,
			   const double tic_step,
			   const double zero,
                           const int Style,
			   const double unit_step, bool draw_units) {

  if(INVERTCOLORS || IsDithered())
    Surface.SelectObject(LK_BLACK_PEN);


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
      StyleLine(Surface, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, xval*unit_step/tic_step, unit_step);

//	SetBkMode(hdc, OPAQUE);
	Surface.GetTextSize(unit_text, &tsize);
	Surface.SetBackgroundOpaque();
	Surface.DrawText(xmin-tsize.cx/2, ymax-tsize.cy, unit_text);
	Surface.SetBackgroundTransparent();
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

      StyleLine(Surface, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, xval*unit_step/tic_step, unit_step);
//	SetBkMode(hdc, OPAQUE);
	Surface.GetTextSize(unit_text, &tsize);
	Surface.SetBackgroundOpaque();
	Surface.DrawText(xmin-tsize.cx/2, ymax-tsize.cy, unit_text);
	Surface.SetBackgroundTransparent();
      }
    }

  }

}



void Statistics::DrawYGrid(LKSurface& Surface, const RECT& rc,
			   const double tic_step,
			   const double zero,
                           const int Style,
			   const double unit_step, bool draw_units) {

  POINT line[2];
  SIZE tsize;
  double yval;

  if(INVERTCOLORS || IsDithered())
    Surface.SelectObject(LK_BLACK_PEN);


  int xmin, ymin, xmax, ymax;

  if (!tic_step) return;

  for (yval=zero; yval<= y_max; yval+= tic_step) {

    xmin = rc.left;
    ymin = (int)((y_max-yval)*yscale)+rc.top -BORDER_Y;
    xmax = rc.right;
    ymax = ymin;
    line[0].x = xmin+BORDER_X;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    // STYLE_THINDASHPAPER
    if ((yval< y_max) &&
        (ymin>=rc.top) && (ymin<=rc.bottom-BORDER_Y)) {

      StyleLine(Surface, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, yval*unit_step/tic_step, unit_step);
	Surface.GetTextSize(unit_text, &tsize);
	Surface.SetBackgroundOpaque();
	Surface.DrawText(xmin, ymin-tsize.cy/2, unit_text);
    Surface.SetBackgroundTransparent();
      }
    }
  }

  for (yval=zero-tic_step; yval>= y_min; yval-= tic_step) {

    xmin = rc.left;
    ymin = (int)((y_max-yval)*yscale)+rc.top-BORDER_Y;
    xmax = rc.right;
    ymax = ymin;
    line[0].x = xmin+BORDER_X;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    // STYLE_THINDASHPAPER
    if ((yval> y_min) &&
        (ymin>=rc.top) && (ymin<=rc.bottom-BORDER_Y)) {

      StyleLine(Surface, line[0], line[1], Style, rc);

      if (draw_units) {
	TCHAR unit_text[MAX_PATH];
	FormatTicText(unit_text, yval*unit_step/tic_step, unit_step);
	Surface.GetTextSize(unit_text, &tsize);
    Surface.SetBackgroundOpaque();
	Surface.DrawText(xmin, ymin-tsize.cy/2, unit_text);
    Surface.SetBackgroundTransparent();
      }
    }
  }
}



void Statistics::DrawYGrid_cor(LKSurface& Surface, const RECT& rc,
			   const double tic_step,
			   const double zero,
                           const int Style,
			   const double unit_step, bool draw_units) {

  POINT line[2];
  SIZE tsize;
  double yval;

  if(INVERTCOLORS || IsDithered())
    Surface.SelectObject(LK_BLACK_PEN);


  int xmin, ymin, xmax, ymax;

  if (!tic_step) return;

  for (yval=zero; yval<= y_max; yval+= tic_step) {

    xmin = rc.left;
    ymin = (int)((y_max-yval)*yscale)+rc.top ;
    ymin = (int)((-yval)*yscale)+rc.top -BORDER_Y;
    xmax = rc.right;
    ymax = ymin;
    line[0].x = xmin+BORDER_X;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    // STYLE_THINDASHPAPER
    if ((yval< y_max) &&
        (ymin>=rc.top) && (ymin<=rc.bottom)) {

      StyleLine(Surface, line[0], line[1], Style, rc);

      if (draw_units) {
        TCHAR unit_text[MAX_PATH];
        FormatTicText(unit_text, yval*unit_step/tic_step, unit_step);
        Surface.GetTextSize(unit_text, &tsize);
        Surface.SetBackgroundOpaque();
        Surface.DrawText(xmin, ymin-tsize.cy/2, unit_text);
        Surface.SetBackgroundTransparent();
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
        (ymin>=rc.top) && (ymin<=rc.bottom)) {

      StyleLine(Surface, line[0], line[1], Style, rc);

      if (draw_units) {
        TCHAR unit_text[MAX_PATH];
        FormatTicText(unit_text, yval*unit_step/tic_step, unit_step);
        Surface.GetTextSize(unit_text, &tsize);
        Surface.SetBackgroundOpaque();
        Surface.DrawText(xmin, ymin-tsize.cy/2, unit_text);
        Surface.SetBackgroundTransparent();
      }
    }
  }
}
