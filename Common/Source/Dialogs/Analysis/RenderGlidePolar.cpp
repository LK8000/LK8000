/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"



void Statistics::RenderGlidePolar(LKSurface& Surface, const RECT& rc)
{
  int minSpeed = iround(GlidePolar::Vminsink*0.8);
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

  #if BUGSTOP
  LKASSERT(maxSpeed<MAXAVERAGECLIMBRATESIZE);
  #endif
  if (maxSpeed>=MAXAVERAGECLIMBRATESIZE) maxSpeed=MAXAVERAGECLIMBRATESIZE-1; // could be also without -1

  for (int i=minSpeed; i<maxSpeed; ++i) {
    
    sinkrate0 = GlidePolar::SinkRate(i);
    sinkrate1 = GlidePolar::SinkRate(i+1);

    DrawLine(Surface, rc,
             i, sinkrate0 , 
             i+1, sinkrate1, 
             STYLE_DASHGREEN);

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

  double ff = maxSpeed / max(1.0, CALCULATED_INFO.VMacCready);
  double sb = GlidePolar::SinkRate(CALCULATED_INFO.VMacCready);
  ff= (sb-MACCREADY)/max(1.0, CALCULATED_INFO.VMacCready);

  DrawLine(Surface, rc,
           0, MACCREADY, 
           maxSpeed,
           MACCREADY+ff*maxSpeed,
           STYLE_REDTHICK);

  if(INVERTCOLORS)
    Surface.SetTextColor(RGB_DARKGREEN);
  else
    Surface.SetTextColor(RGB_GREEN);
  #if (WINDOWSPC > 0)
  Surface.SetBackgroundOpaque();
  #endif
  TCHAR text[80];
  _stprintf(text,TEXT(" v/%s "),Units::GetHorizontalSpeedName());
  DrawXLabel(Surface, rc, text);
  _stprintf(text,TEXT(" w/%s "),Units::GetVerticalSpeedName());
  DrawYLabel(Surface, rc, text);




  if(INVERTCOLORS)
    Surface.SetTextColor(RGB_BLACK);
  else
    Surface.SetTextColor(RGB_WHITE);

  const auto hfOldU = Surface.SelectObject(LK8InfoNormalFont);
  extern void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR* ext);
  LK_tsplitpath(szPolarFile, (TCHAR*) NULL, (TCHAR*) NULL, text, (TCHAR*) NULL);

   Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(130), text, _tcslen(text));

  _stprintf(text,TEXT("%s %.0f kg"),  
            gettext(TEXT("_@M814_")), // Weight
	        GlidePolar::GetAUW());
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(110), text, _tcslen(text));

  _stprintf(text,TEXT("%s %.1f kg/m2"),  
	             gettext(TEXT("_@M821_")), // Wing load
	             GlidePolar::WingLoading);
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(90), text, _tcslen(text));

  _stprintf(text, TEXT("%s: %3.0f  @ %3.0f %s"),
		MsgToken(140), // Best LD
                  GlidePolar::bestld,
                  GlidePolar::Vbestld*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(70), text, _tcslen(text));

  _stprintf(text, TEXT("%s: %3.2f %s @ %3.0f %s"),
		MsgToken(437), // Min sink
                  GlidePolar::minsink*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),
                  GlidePolar::Vminsink*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  Surface.DrawText(rc.left+IBLSCALE(30), rc.bottom-IBLSCALE(50), text, _tcslen(text));

  Surface.SelectObject(hfOldU);
  Surface.SetBackgroundTransparent();
}


