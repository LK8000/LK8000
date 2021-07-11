/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Atmosphere.h"
#include "Asset.hpp"

void Statistics::RenderTemperature(LKSurface& Surface, const RECT& rc)
{
  ResetScale();

  int i;
  float hmin= 10000;
  float hmax= -10000;
  float tmin= (float)CuSonde::maxGroundTemperature;
  float tmax= (float)CuSonde::maxGroundTemperature;

  // find range for scaling of graph
  for (i=0; i<CUSONDE_NUMLEVELS-1; i++) {
    if (CuSonde::cslevels[i].nmeasurements) {

      hmin = min(hmin, (float)i);
      hmax = max(hmax, (float)i);
      tmin = min(tmin, (float)min(CuSonde::cslevels[i].tempDry,
			   (double)min(CuSonde::cslevels[i].airTemp,
                                      (double)CuSonde::cslevels[i].dewpoint)));
      tmax = max(tmax, (float)max(CuSonde::cslevels[i].tempDry,
			   (double)max(CuSonde::cslevels[i].airTemp,
			       (double)CuSonde::cslevels[i].dewpoint)));
    }
  }

  if (hmin>= hmax) {
    DrawNoData(Surface, rc);
    return;
  }

  ScaleYFromValue(rc, hmin);
  ScaleYFromValue(rc, hmax);
  ScaleXFromValue(rc, tmin);
  ScaleXFromValue(rc, tmax);

  bool labelDry = false;
  bool labelAir = false;
  bool labelDew = false;

  int ipos = 0;

  for (i=0; i<CUSONDE_NUMLEVELS-1; i++) {

    if (CuSonde::cslevels[i].nmeasurements &&
	CuSonde::cslevels[i+1].nmeasurements) {

      ipos++;

      DrawLine(Surface, rc,
	       CuSonde::cslevels[i].tempDry, i,
	       CuSonde::cslevels[i+1].tempDry, (i+1),
	       STYLE_REDTHICK);

      DrawLine(Surface, rc,
	       CuSonde::cslevels[i].airTemp, i,
	       CuSonde::cslevels[i+1].airTemp, (i+1),
	       STYLE_MEDIUMBLACK);

      DrawLine(Surface, rc,
	       CuSonde::cslevels[i].dewpoint, i,
	       CuSonde::cslevels[i+1].dewpoint, i+1,
	       STYLE_BLUETHIN);

      if (ipos> 2) {
	if (!labelDry) {
	  DrawLabel(Surface, rc, TEXT("DALR"),
		    CuSonde::cslevels[i+1].tempDry, i);
	  labelDry = true;
	} else {
	  if (!labelAir) {
	    DrawLabel(Surface, rc, TEXT("Air"),
		      CuSonde::cslevels[i+1].airTemp, i);
	    labelAir = true;
	  } else {
	    if (!labelDew) {
	      DrawLabel(Surface, rc, TEXT("Dew"),
			CuSonde::cslevels[i+1].dewpoint, i);
	      labelDew = true;
	    }
	  }
	}
      }
    }
  }

  if(INVERTCOLORS || IsDithered())
    Surface.SetTextColor(RGB_DARKGREEN);
  else
    Surface.SetTextColor(RGB_GREEN);

  Surface.SetBackgroundOpaque();

  TCHAR text[80];
  _stprintf(text,TEXT(" T/%sC "), MsgToken(2179));
  DrawXLabel(Surface, rc, text);
  _stprintf(text,TEXT(" h/%s "),Units::GetAltitudeName());
  DrawYLabel(Surface, rc, text);


//  DrawXLabel(hdc, rc, TEXT("T")TEXT(DEG));
//  DrawYLabel(hdc, rc, TEXT("h"));
}
