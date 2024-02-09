/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"
#include "Asset.hpp"


void Statistics::RenderBarograph(LKSurface& Surface, const RECT& rc)
{

  if (flightstats.Altitude.sum_n<2) {
    DrawNoData(Surface, rc);
    return;
  }

  ResetScale();

  ScaleXFromData(rc, &flightstats.Altitude);
  ScaleYFromData(rc, &flightstats.Altitude);
  ScaleYFromValue(rc, 0);
  ScaleXFromValue(rc, 1.2f*(flightstats.Altitude.x_min+1.0)); // in case no data
  ScaleXFromValue(rc, flightstats.Altitude.x_min);

  if((ISCAR && INVERTCOLORS)  || IsDithered())
	RenderSky( Surface,   rc, SKY_HORIZON_COL , SKY_SPACE_COL, GC_NO_COLOR_STEPS );

  for(int j=1;j<MAXTASKPOINTS;j++) {
    if (ValidTaskPoint(j) && (flightstats.LegStartTime[j]>=0)) {
      double xx =
        (flightstats.LegStartTime[j]-CALCULATED_INFO.TakeOffTime)/3600.0;
      if (xx>=0) {
        DrawLine(Surface, rc,
                 xx, y_min,
                 xx, y_max,
                 STYLE_REDTHICK);
      }
    }
  }

  LKPen hpHorizonGround(PEN_SOLID, IBLSCALE(1), GROUND_COLOUR);
  LKBrush hbHorizonGround(GROUND_COLOUR);
  const auto oldPen = Surface.SelectObject(hpHorizonGround);
  const auto oldBrush = Surface.SelectObject(hbHorizonGround);

  DrawFilledLineGraph(Surface, rc, &flightstats.Altitude_Terrain, GROUND_COLOUR);

  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldBrush);

  DrawXGrid(Surface, rc,
            0.5, flightstats.Altitude.x_min,
            STYLE_THINDASHPAPER, 0.5, true);

  RECT rci = rc;
  rci.top += BORDER_Y;

  if(Units::GetAlternateAltitudeUnit() == unFeet) {
    DrawYGrid(Surface, rci, Units::FromAltitude(500.0), 0, STYLE_THINDASHPAPER, 500.0, true);
  } else {
    DrawYGrid(Surface, rci, Units::FromAltitude(1000.0), 0, STYLE_THINDASHPAPER, 1000.0, true);
  }
  DrawLineGraph(Surface, rc, &flightstats.Altitude,
                STYLE_MEDIUMBLACK);

  DrawTrend(Surface, rc, &flightstats.Altitude_Base, STYLE_BLUETHIN);

  DrawTrend(Surface, rc, &flightstats.Altitude_Ceiling, STYLE_BLUETHIN);

  if(INVERTCOLORS || IsDithered())
    Surface.SetTextColor(RGB_DARKGREEN);
  else
    Surface.SetTextColor(RGB_GREEN);

  TCHAR text[80];
  DrawXLabel(Surface, rc, TEXT(" t/h "));
  _stprintf(text,TEXT(" h/%s "),Units::GetAltitudeName());
  DrawYLabel(Surface, rc, text);


//  DrawYLabel(hdc, rc, TEXT("h"));

}
