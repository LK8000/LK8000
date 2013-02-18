/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"



void Statistics::RenderBarograph(HDC hdc, const RECT rc)
{

  if (flightstats.Altitude.sum_n<2) {
    DrawNoData(hdc, rc);
    return;
  }

  ResetScale();

 // ScaleXFromData(rc, &flightstats.Altitude);
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

  RECT rci = rc;
  rci.top += BORDER_Y;

  if(Units::GetUserInvAltitudeUnit() == unFeet) {
    DrawYGrid(hdc, rci, 500.0/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER, 500.0, true);
  } else {
    DrawYGrid(hdc, rci, 1000.0/ALTITUDEMODIFY, 0, STYLE_THINDASHPAPER, 1000.0, true);
  }
  DrawLineGraph(hdc, rc, &flightstats.Altitude,
                STYLE_MEDIUMBLACK);

  DrawTrend(hdc, rc, &flightstats.Altitude_Base, STYLE_BLUETHIN);

  DrawTrend(hdc, rc, &flightstats.Altitude_Ceiling, STYLE_BLUETHIN);

  DrawXLabel(hdc, rc, TEXT("t"));
  DrawYLabel(hdc, rc, TEXT("h"));

}
