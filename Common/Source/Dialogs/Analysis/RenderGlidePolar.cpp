/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"



void Statistics::RenderGlidePolar(HDC hdc, const RECT rc)
{
  int minSpeed = iround(GlidePolar::Vminsink*0.8);
  int maxSpeed = iround(SAFTEYSPEED*1.1);
  
  ResetScale();
  ScaleYFromValue(rc, 0);
  ScaleYFromValue(rc, GlidePolar::SinkRate(maxSpeed));
  ScaleXFromValue(rc, minSpeed);
  ScaleXFromValue(rc, maxSpeed);

  DrawXGrid(hdc, rc, 
            10.0/SPEEDMODIFY, 0,
            STYLE_THINDASHPAPER, 10.0, true);
  DrawYGrid(hdc, rc, 
            1.0/LIFTMODIFY, 0,
            STYLE_THINDASHPAPER, 1.0, true);
  
  double sinkrate0, sinkrate1;
  double v0=0, v1;
  bool v0valid = false;
  int i0=0;

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

  double ff = maxSpeed / max(1.0, CALCULATED_INFO.VMacCready);
  double sb = GlidePolar::SinkRate(CALCULATED_INFO.VMacCready);
  ff= (sb-MACCREADY)/max(1.0, CALCULATED_INFO.VMacCready);

  DrawLine(hdc, rc,
           0, MACCREADY, 
           maxSpeed,
           MACCREADY+ff*maxSpeed,
           STYLE_REDTHICK);

  DrawXLabel(hdc, rc, TEXT("V"));
  DrawYLabel(hdc, rc, TEXT("w"));

  TCHAR text[80];
  SetBkMode(hdc, OPAQUE);

  if(INVERTCOLORS)
    SetTextColor(hdc,RGB_BLACK);
  else
    SetTextColor(hdc,RGB_WHITE);

  HFONT hfOldU = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
  extern void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext);
  LK_wsplitpath(szPolarFile, (WCHAR*) NULL, (WCHAR*) NULL, text, (WCHAR*) NULL);

   ExtTextOut(hdc, rc.left+IBLSCALE(30),
 	               rc.bottom-IBLSCALE(130),
 	               ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  _stprintf(text,TEXT("%s %.0f kg"),  
            gettext(TEXT("_@M814_")), // Weight
	        GlidePolar::GetAUW());
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(110),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  _stprintf(text,TEXT("%s %.1f kg/m2"),  
	             gettext(TEXT("_@M821_")), // Wing load
	             GlidePolar::WingLoading);
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(90),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  _stprintf(text, TEXT("%s: %3.0f  @ %3.0f %s"),
		MsgToken(140), // Best LD
                  GlidePolar::bestld,
                  GlidePolar::Vbestld*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(70),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  _stprintf(text, TEXT("%s: %3.2f %s @ %3.0f %s"),
		MsgToken(437), // Min sink
                  GlidePolar::minsink*LIFTMODIFY,
                  Units::GetVerticalSpeedName(),
                  GlidePolar::Vminsink*SPEEDMODIFY,
                  Units::GetHorizontalSpeedName());
  ExtTextOut(hdc, rc.left+IBLSCALE(30), 
	              rc.bottom-IBLSCALE(50),
	              ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

  SelectObject(hdc, hfOldU);
  SetBkMode(hdc, TRANSPARENT);
}

