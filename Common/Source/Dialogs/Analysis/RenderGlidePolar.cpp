/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "utils/stl_utils.h"
#include "LKObjects.h"
#include "Asset.hpp"


void Statistics::RenderGlidePolar(LKSurface& Surface, const RECT& rc)
{
  int minSpeed = iround(GlidePolar::Vminsink()*0.6);
  int maxSpeed = iround(SAFTEYSPEED*1.1);

  ResetScale();
  ScaleYFromValue(rc, 0);
  ScaleYFromValue(rc, GlidePolar::SinkRate(maxSpeed));
  ScaleXFromValue(rc, minSpeed);
  ScaleXFromValue(rc, maxSpeed);
  double gridtick = 20;
  if( Units::ToHorizontalSpeed(maxSpeed-minSpeed) < 150)  {
	  gridtick = 10;
  }

  DrawXGrid(Surface, rc,  Units::FromHorizontalSpeed(gridtick), 0, STYLE_THINDASHPAPER, gridtick, true);

  gridtick = 0.5;
  if(Units::ToVerticalSpeed(GlidePolar::SinkRate(minSpeed)-GlidePolar::SinkRate(maxSpeed)) > 5)  {
	  gridtick = 1.0;
  }
  if(Units::ToVerticalSpeed(GlidePolar::SinkRate(minSpeed)-GlidePolar::SinkRate(maxSpeed)) > 20)  {
	  gridtick = 5.0;
  }
  DrawYGrid_cor(Surface, rc,  Units::FromVerticalSpeed(gridtick), 0, STYLE_THINDASHPAPER, gridtick, true);

  double sinkrate0, sinkrate1;
  double v0=0, v1;
  bool v0valid = false;
  int i0=0;

  BUGSTOP_LKASSERT(maxSpeed<MAXAVERAGECLIMBRATESIZE);
  if (maxSpeed>=MAXAVERAGECLIMBRATESIZE) maxSpeed=MAXAVERAGECLIMBRATESIZE-1; // could be also without -1

  // Draw Polar curve
  for (int i=minSpeed; i<maxSpeed; ++i) {

    sinkrate0 = GlidePolar::SinkRate(i);
    sinkrate1 = GlidePolar::SinkRate(i+1);

    DrawLine(Surface, rc,
             i, sinkrate0 ,
             i+1, sinkrate1,
             STYLE_GREENMEDIUM); // ex STYLE_DASHGREEN

    if (CALCULATED_INFO.AverageClimbRateN[i]>0) {
      v1= CALCULATED_INFO.AverageClimbRate[i]
        /CALCULATED_INFO.AverageClimbRateN[i];

      if (v0valid) {

        DrawLine(Surface, rc,
                 i0, v0 ,
                 i, v1,
                 STYLE_DASHGREEN
                   );


      }

      v0 = v1; i0 = i;
      v0valid = true;
    }
  }

  // Draw Current MC Glide Slope
  double sb = GlidePolar::SinkRate(CALCULATED_INFO.VMacCready);
  double ff= (sb-MACCREADY)/max(1.0, CALCULATED_INFO.VMacCready);

  DrawLine(Surface, rc,
           0, MACCREADY,
           maxSpeed,
           MACCREADY+ff*maxSpeed,
           STYLE_REDTHICK);

  if(INVERTCOLORS || IsDithered())
    Surface.SetTextColor(RGB_DARKGREEN);
  else
    Surface.SetTextColor(RGB_GREEN);

  Surface.SetBackgroundOpaque();
  TCHAR text[80];
  _stprintf(text,TEXT(" v/%s "),Units::GetHorizontalSpeedName());
  DrawXLabel(Surface, rc, text);
  _stprintf(text,TEXT(" w/%s "),Units::GetVerticalSpeedName());
  DrawYLabel(Surface, rc, text);




  if(INVERTCOLORS || IsDithered())
    Surface.SetTextColor(RGB_BLACK);
  else
    Surface.SetTextColor(RGB_WHITE);

  auto hfOldU = Surface.SelectObject(LK8InfoNormalFont);
  if( GlidePolar::WingArea>0.1 ) {
    _stprintf(text,TEXT("%s %.1f kg/m2"),
	             MsgToken<821>(), // Wing load
	             GlidePolar::WingLoading);
    Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(90), text);
  }
  _stprintf(text, TEXT("%s: %3.1f  @ %3.0f %s"),
		MsgToken<140>(), // Best LD
                  GlidePolar::bestld,
                  Units::ToHorizontalSpeed(GlidePolar::Vbestld()),
                  Units::GetHorizontalSpeedName());
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(70), text);

  _stprintf(text, TEXT("%s: %3.2f %s @ %3.0f %s"),
                  MsgToken<437>(), // Min sink
                  Units::ToVerticalSpeed(GlidePolar::minsink),
                  Units::GetVerticalSpeedName(),
                  Units::ToHorizontalSpeed(GlidePolar::Vminsink()),
                  Units::GetHorizontalSpeedName());
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(50), text);

  Surface.SelectObject(hfOldU);
  Surface.SetBackgroundTransparent();
}
