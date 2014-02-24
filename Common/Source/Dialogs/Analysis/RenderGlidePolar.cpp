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



void Statistics::RenderGlidePolar(HDC hdc, const RECT rc)
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

  DrawXGrid(hdc, rc,  gridtick/SPEEDMODIFY, 0, STYLE_THINDASHPAPER, gridtick, true);

  gridtick = 0.5;
  if((GlidePolar::SinkRate(minSpeed)-GlidePolar::SinkRate(maxSpeed))*LIFTMODIFY > 5)  {
	  gridtick = 1.0;  }
  if((GlidePolar::SinkRate(minSpeed)-GlidePolar::SinkRate(maxSpeed))*LIFTMODIFY > 20)  {
  	  gridtick = 5.0;  }
  DrawYGrid_cor(hdc, rc,  gridtick/LIFTMODIFY, 0, STYLE_THINDASHPAPER, gridtick, true);

  double sinkrate0, sinkrate1;
  double v0=0, v1;
  bool v0valid = false;
  int i0=0;

  #if BUGSTOP
  LKASSERT(maxSpeed<MAXAVERAGECLIMBRATESIZE);
  #endif
  if (maxSpeed>=MAXAVERAGECLIMBRATESIZE) maxSpeed=MAXAVERAGECLIMBRATESIZE-1; // could be also without -1

  // Draw Polar curve
  for (int i=minSpeed; i<maxSpeed; ++i) {
    
    sinkrate0 = GlidePolar::SinkRate(i);
    sinkrate1 = GlidePolar::SinkRate(i+1);

    DrawLine(hdc, rc,
             i, sinkrate0 , 
             i+1, sinkrate1, 
             STYLE_DASHGREEN);

    if (CALCULATED_INFO.AverageClimbRateN[i]>0) {
      v1= CALCULATED_INFO.AverageClimbRate[i]
        /CALCULATED_INFO.AverageClimbRateN[i];

      if (v0valid) {

        DrawLine(hdc, rc,
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

  DrawLine(hdc, rc,
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

      DrawLine(hdc, rc, 0, eqMC, maxSpeed, eqMC-(maxSpeed/ConeSlope), STYLE_BLUETHIN);

      SetTextColor(hdc,INVERTCOLORS?RGB_BLACK:RGB_WHITE);

      HFONT hfOldU = (HFONT)SelectObject(hdc, LK8InfoSmallFont);
      TCHAR text[3][80]; TCHAR value[3][80];
      _stprintf(text[0],TEXT("%s"), gettext(TEXT("_@M2175_"))); // Conical ESS
      _stprintf(text[1],TEXT("  %s"), gettext(TEXT("_@M2179_")));  // Speed Opt.
      _stprintf(text[2],TEXT("  %s"), gettext(TEXT("_@M2180_")));  // "Thermal Min"

      _stprintf(value[0],TEXT(" : %.1f"), ConeSlope); // Conical ESS
      _stprintf(value[1],TEXT(" : %.0f %s"), VOpt*SPEEDMODIFY, Units::GetHorizontalSpeedName()); // Speed
      _stprintf(value[2],TEXT(" : %.1f %s"), eqMC*LIFTMODIFY, Units::GetVerticalSpeedName()); // "Min sink"

      // Calc Size of text
      SIZE tsize = {0,0};
      SIZE vsize = {0,0};

      for(unsigned i = 0; i<array_size(text); ++i) {
        SIZE sizeTmp;
        GetTextExtentPoint(hdc, text[i], _tcslen(text[i]), &sizeTmp);
        tsize.cx = std::max(tsize.cx, sizeTmp.cx);
        tsize.cy = std::max(tsize.cy, sizeTmp.cy);

        GetTextExtentPoint(hdc, value[i], _tcslen(value[i]), &sizeTmp);
        vsize.cx = std::max(vsize.cx, sizeTmp.cx);
        vsize.cy = std::max(vsize.cy, sizeTmp.cy);

      }

      RECT blockR = {
          rc.right-tsize.cx-vsize.cx-IBLSCALE(4),
          rc.top,
          rc.right-IBLSCALE(4),
          rc.top+tsize.cy*(int)array_size(text)
      };

      ::FillRect(hdc, &blockR, INVERTCOLORS?LKBrush_White:LKBrush_Black);

      int OldBck = SetBkMode(hdc, OPAQUE);
      for(unsigned i = 0; i<array_size(text); ++i) {
        ExtTextOut(hdc, blockR.left,
                        blockR.top+tsize.cy*i,
                        ETO_OPAQUE, NULL, text[i], _tcslen(text[i]), NULL);
        ExtTextOut(hdc, blockR.left+tsize.cx,
                        rc.top+tsize.cy*i,
                        ETO_OPAQUE, NULL, value[i], _tcslen(value[i]), NULL);
      }
      SetBkMode(hdc, OldBck);
      SelectObject(hdc, hfOldU);
    }
  }

  if(INVERTCOLORS)
    SetTextColor(hdc,RGB_DARKGREEN);
  else
    SetTextColor(hdc,RGB_GREEN);
  SetBkMode(hdc, OPAQUE);
  TCHAR text[80];
  _stprintf(text,TEXT(" v/%s "),Units::GetHorizontalSpeedName());
  DrawXLabel(hdc, rc, text);
  _stprintf(text,TEXT(" w/%s "),Units::GetVerticalSpeedName());
  DrawYLabel(hdc, rc, text);




  if(INVERTCOLORS)
    SetTextColor(hdc,RGB_BLACK);
  else
    SetTextColor(hdc,RGB_WHITE);

  HFONT hfOldU = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  if( GlidePolar::WingArea>0.1 ) {
    _stprintf(text,TEXT("%s %.1f kg/m2"),
	             gettext(TEXT("_@M821_")), // Wing load
	             GlidePolar::WingLoading);
    ExtTextOut(hdc, rc.left+IBLSCALE(30),
	              rc.bottom-IBLSCALE(90),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
  }
  _stprintf(text, TEXT("%s: %3.1f  @ %3.0f %s"),
		MsgToken(140), // Best LD
                  GlidePolar::bestld,
                  GlidePolar::Vbestld()*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(70),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  _stprintf(text, TEXT("%s: %3.2f %s @ %3.0f %s"),
		MsgToken(437), // Min sink
                  GlidePolar::minsink*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),
                  GlidePolar::Vminsink()*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(50),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  SelectObject(hdc, hfOldU);
  SetBkMode(hdc, TRANSPARENT);
}


