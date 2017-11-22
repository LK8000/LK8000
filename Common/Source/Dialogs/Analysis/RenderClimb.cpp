/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Asset.hpp"

void Statistics::RenderClimb(LKSurface& Surface, const RECT& rc)
{

  if (flightstats.ThermalAverage.sum_n<1) {
    DrawNoData(Surface, rc);
    return;
  }
  const auto hfOld = Surface.SelectObject(LK8PanelUnitFont);
  ResetScale();
  ScaleYFromData(rc, &flightstats.ThermalAverage);
  ScaleYFromValue(rc, (MACCREADY+1.0));
  ScaleYFromValue(rc, 0);

  ScaleXFromValue(rc, -1);
  ScaleXFromValue(rc, flightstats.ThermalAverage.sum_n);

  RECT rci = rc;
  rci.top += BORDER_Y;
  if(Units::GetUserInvAltitudeUnit() == unFeet) {
    DrawYGrid(Surface, rci, Units::ToSysVerticalSpeed(0.5), 0, STYLE_THINDASHPAPER, 0.5, true);
  } else {
    DrawYGrid(Surface, rci, Units::ToSysVerticalSpeed(1.0), 0, STYLE_THINDASHPAPER, 1.0, true);
  }

  DrawBarChart(Surface, rc, &flightstats.ThermalAverage);

  DrawLine(Surface, rc,
           0, MACCREADY,
           flightstats.ThermalAverage.sum_n,
           MACCREADY,
           STYLE_REDTHICK);

  DrawLabel(Surface, rc, TEXT("MC"),
	    max(0.5, (double)flightstats.ThermalAverage.sum_n-1), MACCREADY);

  DrawTrendN(Surface, rc,
             &flightstats.ThermalAverage,
             STYLE_BLUETHIN);
  Surface.SelectObject(hfOld);
  if(INVERTCOLORS || IsDithered())
    Surface.SetTextColor(RGB_DARKGREEN);
  else
    Surface.SetTextColor(RGB_GREEN);

  #if (WINDOWSPC>0)
  Surface.SetBackgroundOpaque();
  #endif
  TCHAR text[80];

  DrawXLabel(Surface, rc, TEXT("n"));
  _stprintf(text,TEXT(" v/%s "),Units::GetVerticalSpeedName());
  DrawYLabel(Surface, rc, text);

}
