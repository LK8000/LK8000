/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


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

  RECT rci = rc;
  rci.top += BORDER_Y;
  if(Units::GetUserInvAltitudeUnit() == unFeet) {
    DrawYGrid(hdc, rci, 0.5/LIFTMODIFY, 0, STYLE_THINDASHPAPER, 0.5, true);
  } else {
    DrawYGrid(hdc, rci, 1.0/LIFTMODIFY, 0, STYLE_THINDASHPAPER, 1.0, true);
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
  if(INVERTCOLORS)
    SetTextColor(hdc,RGB_DARKGREEN);
  else
    SetTextColor(hdc,RGB_GREEN);
  SetBkMode(hdc, OPAQUE);
  TCHAR text[80];

  DrawXLabel(hdc, rc, TEXT("n"));
  _stprintf(text,TEXT(" v/%s "),Units::GetVerticalSpeedName());
  DrawYLabel(hdc, rc, text);

}



