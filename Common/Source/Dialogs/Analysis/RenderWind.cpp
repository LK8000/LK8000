/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Asset.hpp"

// from Calculations.cpp
#include "windanalyser.h"
extern WindAnalyser *windanalyser;

void Statistics::RenderWind(LKSurface& Surface, const RECT& rc)
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
    DrawNoData(Surface, rc);
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

  DrawXGrid(Surface, rc, iround(Units::FromWindSpped(5)), 0, STYLE_THINDASHPAPER, 5.0, true);
  DrawYGrid(Surface, rc, iround(Units::FromAltitude(1000)), 0, STYLE_THINDASHPAPER, 1000.0, true);

  DrawLineGraph(Surface, rc, &windstats_mag,
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
    StyleLine(Surface, wv[0], wv[1], STYLE_MEDIUMBLACK, rc);

    dX = (mag*WINDVECTORMAG-5);
    dY = -3;
    rotate(dX,dY,angle);
    wv[2].x = (int)(wv[0].x + dX);
    wv[2].y = (int)(wv[0].y + dY);
    StyleLine(Surface, wv[1], wv[2], STYLE_MEDIUMBLACK, rc);

    dX = (mag*WINDVECTORMAG-5);
    dY = 3;
    rotate(dX,dY,angle);
    wv[3].x = (int)(wv[0].x + dX);
    wv[3].y = (int)(wv[0].y + dY);

    StyleLine(Surface, wv[1], wv[3], STYLE_MEDIUMBLACK, rc);

  }

  if(INVERTCOLORS || IsDithered())
    Surface.SetTextColor(RGB_DARKGREEN);
  else
    Surface.SetTextColor(RGB_GREEN);

  Surface.SetBackgroundOpaque();

  TCHAR text[80];
  _stprintf(text,TEXT(" v/%s "),Units::GetHorizontalSpeedName());
  DrawXLabel(Surface, rc, text);
  _stprintf(text,TEXT(" h/%s "),Units::GetAltitudeName());
  DrawYLabel(Surface, rc, text);

 // DrawXLabel(hdc, rc, TEXT("w"));
 // DrawYLabel(hdc, rc, TEXT("h"));

}
