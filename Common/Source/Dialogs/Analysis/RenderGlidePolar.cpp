/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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
  double gridtick;
  gridtick = 20;
  if( (maxSpeed-minSpeed)*SPEEDMODIFY < 150)  {
	gridtick = 10;  }

  DrawXGrid(Surface, rc,  gridtick/SPEEDMODIFY, 0, STYLE_THINDASHPAPER, gridtick, true);

  gridtick = 0.5;
  if((GlidePolar::SinkRate(minSpeed)-GlidePolar::SinkRate(maxSpeed))*LIFTMODIFY > 5)  {
	  gridtick = 1.0;  }
  if((GlidePolar::SinkRate(minSpeed)-GlidePolar::SinkRate(maxSpeed))*LIFTMODIFY > 20)  {
	  gridtick = 5.0;  }
  DrawYGrid_cor(Surface, rc,  gridtick/LIFTMODIFY, 0, STYLE_THINDASHPAPER, gridtick, true);

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

  if(DoOptimizeRoute()) {
    // Draw Conical ESS Best Glide Slope
    double ConeSlope = 0.;

    // Find Cone Slope :
    LockTaskData();
    for(unsigned i = 0; ValidTaskPoint(i); ++i) {
        int Type;
        GetTaskSectorParameter(i, &Type, NULL);
        if(Type == CONE) {
          ConeSlope = Task[i].PGConeSlope;
          break;
        }
    }
    UnlockTaskData();

    if(ConeSlope>0.) {
      double VOpt = GlidePolar::FindSpeedForSlope(ConeSlope);
      double eqMC = GlidePolar::EquMC(VOpt);

      DrawLine(Surface, rc, 0, eqMC, maxSpeed, eqMC-(maxSpeed/ConeSlope), STYLE_BLUETHIN);

      Surface.SetTextColor((INVERTCOLORS || IsDithered())?RGB_BLACK:RGB_WHITE);

      auto hfOldU = Surface.SelectObject(LK8InfoSmallFont);
      TCHAR text[3][80]; TCHAR value[3][80];
      _stprintf(text[0],TEXT("%s"), MsgToken(2175)); // Conical ESS
      _stprintf(text[1],TEXT("  %s"), MsgToken(2187));  // Speed Opt.
      _stprintf(text[2],TEXT("  %s"), MsgToken(2188));  // "Thermal Min"

      _stprintf(value[0],TEXT(" : %.1f"), ConeSlope); // Conical ESS
      _stprintf(value[1],TEXT(" : %.0f %s"), VOpt*SPEEDMODIFY, Units::GetHorizontalSpeedName()); // Speed
      _stprintf(value[2],TEXT(" : %.1f %s"), eqMC*LIFTMODIFY, Units::GetVerticalSpeedName()); // "Min sink"

      // Calc Size of text
      SIZE tsize = {0,0};
      SIZE vsize = {0,0};

      for(unsigned i = 0; i<std::size(text); ++i) {
        SIZE sizeTmp;
        Surface.GetTextSize(text[i], &sizeTmp);
        tsize.cx = std::max(tsize.cx, sizeTmp.cx);
        tsize.cy = std::max(tsize.cy, sizeTmp.cy);

        Surface.GetTextSize(value[i], &sizeTmp);
        vsize.cx = std::max(vsize.cx, sizeTmp.cx);
        vsize.cy = std::max(vsize.cy, sizeTmp.cy);

      }

      RECT blockR = {
          rc.right-tsize.cx-vsize.cx-IBLSCALE(4),
          rc.top,
          rc.right-IBLSCALE(4),
          rc.top+tsize.cy*(int)std::size(text)
      };

      Surface.FillRect(&blockR, (INVERTCOLORS || IsDithered())?LKBrush_White:LKBrush_Black);

      for(unsigned i = 0; i<std::size(text); ++i) {
        Surface.DrawText(blockR.left, blockR.top+tsize.cy*i, text[i]);
        Surface.DrawText(blockR.left+tsize.cx, rc.top+tsize.cy*i, value[i]);
      }
      Surface.SelectObject(hfOldU);
    }
  }

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
	             MsgToken(821), // Wing load
	             GlidePolar::WingLoading);
    Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(90), text);
  }
  _stprintf(text, TEXT("%s: %3.1f  @ %3.0f %s"),
		MsgToken(140), // Best LD
                  GlidePolar::bestld,
                  GlidePolar::Vbestld()*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(70), text);

  _stprintf(text, TEXT("%s: %3.2f %s @ %3.0f %s"),
		MsgToken(437), // Min sink
                  GlidePolar::minsink*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),
                  GlidePolar::Vminsink()*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(50), text);

  Surface.SelectObject(hfOldU);
  Surface.SetBackgroundTransparent();
}
