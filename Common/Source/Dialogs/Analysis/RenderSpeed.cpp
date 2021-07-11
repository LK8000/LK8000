/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Asset.hpp"

void Statistics::RenderSpeed(LKSurface& Surface, const RECT& rc)
{

  if ((flightstats.Task_Speed.sum_n<2)
      || !ValidTaskPoint(ActiveTaskPoint)) {
    DrawNoData(Surface, rc);
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
        DrawLine(Surface, rc,
                 xx, y_min,
                 xx, y_max,
                 STYLE_REDTHICK);
      }
    }
  }

  DrawXGrid(Surface, rc,
            0.5, flightstats.Task_Speed.x_min,
            STYLE_THINDASHPAPER, 0.5, true);

 /* DrawYGrid(hdc, rc, 10/TASKSPEEDMODIFY, 0, STYLE_THINDASHPAPER,
            10, true);*/
  if(Units::GetUserHorizontalSpeedUnit() == unStatuteMilesPerHour) {
    DrawYGrid(Surface, rc, 5.0/TASKSPEEDMODIFY, 0, STYLE_THINDASHPAPER, 5.0, true);
  } else {
    DrawYGrid(Surface, rc, 10/TASKSPEEDMODIFY, 0, STYLE_THINDASHPAPER, 10, true);
  }

  DrawLineGraph(Surface, rc, &flightstats.Task_Speed,STYLE_MEDIUMBLACK);

  DrawTrend(Surface, rc, &flightstats.Task_Speed, STYLE_BLUETHIN);


  if(INVERTCOLORS || IsDithered())
    Surface.SetTextColor(RGB_DARKGREEN);
  else
    Surface.SetTextColor(RGB_GREEN);

  Surface.SetBackgroundOpaque();

  TCHAR text[80];
  DrawXLabel(Surface, rc, TEXT(" t/h "));
  _stprintf(text,TEXT(" v/%s "),Units::GetHorizontalSpeedName());
  DrawYLabel(Surface, rc, text);


//  DrawXLabel(hdc, rc, TEXT("t"));
//  DrawYLabel(hdc, rc, TEXT("V"));

}
