/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "ContestMgr.h"
#include "LKObjects.h"

CContestMgr::TType contestType = CContestMgr::TYPE_OLC_CLASSIC;

/*****************************************************************
 * Alpha Lima splitted RenderContest from Render Task for CC
 * adding FAI Sector display
 ****************************************************************/
void Statistics::RenderContest(HDC hdc, const RECT rc)
{
if(contestType == CContestMgr::TYPE_FAI_TRIANGLE)
   RenderFAIOptimizer(hdc, rc);
else
{
  unsigned int ui;
  double fXY_Scale = 1.0;


  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
  double  x1, y1, x2=0, y2=0;
  double lat_c, lon_c;

  ResetScale();


  CContestMgr::CResult result = CContestMgr::Instance().Result(contestType, true);
  const CPointGPSArray &points = result.PointArray();


  // find center

  CPointGPSArray trace;
  CContestMgr::Instance().Trace(trace);
  for(ui=0; ui<trace.size(); ui++)
  {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    ScaleYFromValue(rc, lat1);
    ScaleXFromValue(rc, lon1);
  }

  HFONT hfOldU = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
  lat_c = (y_max+y_min)/2;
  lon_c = (x_max+x_min)/2;


  // find scale
  ResetScale();

  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1-lon_c)*fastcosine(lat1);
  y1 = (lat1-lat_c);
  ScaleXFromValue(rc, x1*fXY_Scale);
  ScaleYFromValue(rc, y1*fXY_Scale);
  for(ui=0; ui<trace.size(); ui++)
  {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    x1 = (lon1-lon_c)*fastcosine(lat1);
    y1 = (lat1-lat_c);
    ScaleXFromValue(rc, x1*fXY_Scale);
    ScaleYFromValue(rc, y1*fXY_Scale);
  }

  ScaleMakeSquare(rc);



  // draw track
  for(ui=0; trace.size() && ui<trace.size()-1; ui++)
  {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    lat2 = trace[ui+1].Latitude();
    lon2 = trace[ui+1].Longitude();
    x1 = (lon1-lon_c)*fastcosine(lat1);
    y1 = (lat1-lat_c);
    x2 = (lon2-lon_c)*fastcosine(lat2);
    y2 = (lat2-lat_c);
    DrawLine(hdc, rc,  x1, y1, x2, y2, STYLE_MEDIUMBLACK);
  }
  // Draw aircraft on top
double  lat_p = GPS_INFO.Latitude;
double  lon_p = GPS_INFO.Longitude;
double  xp = (lon_p-lon_c)*fastcosine(lat_p);
double  yp = (lat_p-lat_c);


  if(result.Type() == contestType)
  {
    for(ui=0; ui<points.size()-1; ui++)
    {
      lat1 = points[ui].Latitude();
      lon1 = points[ui].Longitude();
      lat2 = points[ui+1].Latitude();
      lon2 = points[ui+1].Longitude();

      x1 = (lon1-lon_c)*fastcosine(lat1);
      y1 = (lat1-lat_c);
      x2 = (lon2-lon_c)*fastcosine(lat2);
      y2 = (lat2-lat_c);
      int style = STYLE_REDTHICK;
      if((result.Type() == CContestMgr::TYPE_OLC_FAI ||
          result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED) &&
         (ui==0 || ui==3))
      {
        // triangle start and finish
        style = STYLE_DASHGREEN;
      }
      else if(result.Predicted() &&
              (result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED ||
               ui == points.size() - 2))
      {
        // predicted edge
        style = STYLE_BLUETHIN;
      }

      if((result.Type() == CContestMgr::TYPE_FAI_3_TPS) ||// TYPE_FAI_3_TPS_PREDICTED
       (result.Type() == CContestMgr::TYPE_FAI_3_TPS_PREDICTED) )
      {
      }

      if((contestType != CContestMgr::TYPE_FAI_TRIANGLE) )
        DrawLine(hdc, rc, x1, y1, x2, y2, style);
    }


  if(result.Type() == CContestMgr::TYPE_OLC_FAI ||
     result.Type() == CContestMgr::TYPE_OLC_FAI_PREDICTED)
  {
    // draw the last edge of a triangle
    lat1 = points[1].Latitude();
    lon1 = points[1].Longitude();
    lat2 = points[3].Latitude();
    lon2 = points[3].Longitude();
    x1 = (lon1-lon_c)*fastcosine(lat1);
    y1 = (lat1-lat_c);
    x2 = (lon2-lon_c)*fastcosine(lat2);
    y2 = (lat2-lat_c);

    DrawLine(hdc, rc, x1, y1, x2, y2, result.Predicted() ? STYLE_BLUETHIN : STYLE_REDTHICK);

  }

  DrawXGrid(hdc, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
  DrawYGrid(hdc, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);

  SelectObject(hdc, hfOldU);
  SetTextColor(hdc, RGB_MAGENTA);
  SetBkMode(hdc, TRANSPARENT);
  DrawLabel(hdc, rc, TEXT("O"), xp, yp);
}
}
}





/*****************************************************************
 * Alpha Lima splitted RenderContest from Render Task for CC
 * adding FAI Sector display
 ****************************************************************/
void Statistics::RenderFAIOptimizer(HDC hdc, const RECT rc)
{

unsigned int ui;
double fXY_Scale = 1.0;
double lat0 = 0;
double lon0 = 0;
double lat1 = 0;
double lon1 = 0;
double lat2 = 0;
double lon2 = 0;
double x0, y0, x1, y1, x2=0, y2=0;
double lat_c, lon_c;
double  lat_p = GPS_INFO.Latitude;
double  lon_p = GPS_INFO.Longitude;
BOOL bFlat = false;
#define DRAWPERCENT
#ifdef DRAWPERCENT
double fTotalPercent = 1.0;
#endif

ResetScale();


  CContestMgr::CResult result = CContestMgr::Instance().Result(contestType, true);
  const CPointGPSArray &points = result.PointArray();
  if(contestType == CContestMgr::TYPE_FAI_TRIANGLE)
     fXY_Scale = 1.5;

  // find center
  double fTotalDistance = result.Distance();
 if (fTotalDistance < 1) fTotalDistance =1;
  CPointGPSArray trace;
  CContestMgr::Instance().Trace(trace);
  for(ui=0; ui<trace.size(); ui++)
  {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    ScaleYFromValue(rc, lat1);
    ScaleXFromValue(rc, lon1);
  }


  HFONT hfOldU = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
  BOOL bFAITri =  CContestMgr::Instance().FAI();
  double fDist, fAngle;
  lat_c = (y_max+y_min)/2;
  lon_c = (x_max+x_min)/2;

  double  xp = (lon_p-lon_c)*fastcosine(lat_p);
  double  yp = (lat_p-lat_c);

  // find scale
  ResetScale();

  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1-lon_c)*fastcosine(lat1);
  y1 = (lat1-lat_c);
  ScaleXFromValue(rc, x1*fXY_Scale);
  ScaleYFromValue(rc, y1*fXY_Scale);
  for(ui=0; ui<trace.size(); ui++)
  {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    x1 = (lon1-lon_c)*fastcosine(lat1);
    y1 = (lat1-lat_c);
    ScaleXFromValue(rc, x1*fXY_Scale);
    ScaleYFromValue(rc, y1*fXY_Scale);
  }

  ScaleMakeSquare(rc);

  if(result.Type() == contestType)
  {
    for(ui=0; ui<points.size()-1; ui++)
    {
      lat1 = points[ui].Latitude();
      lon1 = points[ui].Longitude();
      lat2 = points[ui+1].Latitude();
      lon2 = points[ui+1].Longitude();

      x1 = (lon1-lon_c)*fastcosine(lat1);
      y1 = (lat1-lat_c);
      x2 = (lon2-lon_c)*fastcosine(lat2);
      y2 = (lat2-lat_c);
      DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);

      if( (ui <points.size()-2) && !bFlat && ((fDist / fTotalDistance ) > 0.05) )
      {
		if(fDist > 5000)
		{
		  COLORREF rgbCol = RGB_BLUE;
		  switch(ui)
		  {
			case 0: rgbCol = RGB_LIGHTYELLOW; break;
			case 1: rgbCol = RGB_LIGHTCYAN  ; break;
			case 2: rgbCol = RGB_LIGHTGREEN ; break;
			default:
			break;
		  }
		  RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,1, rgbCol );
		  RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,0, rgbCol );
		}
      }
      if((fDist / fTotalDistance ) > 0.45) /* prevent drawing almost same sectors */
    	bFlat = true;
    }

    // draw track
    for(ui=0; trace.size() && ui<trace.size()-1; ui++)
    {
      lat1 = trace[ui].Latitude();
      lon1 = trace[ui].Longitude();
      lat2 = trace[ui+1].Latitude();
      lon2 = trace[ui+1].Longitude();
      x1 = (lon1-lon_c)*fastcosine(lat1);
      y1 = (lat1-lat_c);
      x2 = (lon2-lon_c)*fastcosine(lat2);
      y2 = (lat2-lat_c);
      DrawLine(hdc, rc,  x1, y1, x2, y2, STYLE_MEDIUMBLACK);
    }



	for(ui=0; ui<points.size()-1; ui++)
	{
	  lat1 = points[ui].Latitude();
	  lon1 = points[ui].Longitude();
	  lat2 = points[ui+1].Latitude();
	  lon2 = points[ui+1].Longitude();

	  x1 = (lon1-lon_c)*fastcosine(lat1);
	  y1 = (lat1-lat_c);
	  x2 = (lon2-lon_c)*fastcosine(lat2);
	  y2 = (lat2-lat_c);
	  int style = STYLE_REDTHICK;

	  if((ui > 0) && (ui < 3))
	  {
		style = STYLE_BLUETHIN;
		DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);
	#ifdef DRAWPERCENT

		if((result.Distance()> 5000) && bFAITri)
		{
		  TCHAR text[180];
		  SIZE tsize;
		  fTotalPercent -= fDist/result.Distance();
		  _stprintf(text, TEXT("%3.1f%%"), (fDist/result.Distance()*100.0));
		  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
		  SetTextColor(hdc, RGB_BLUE);
	      ExtTextOut(hdc, ScaleX(rc, x1 +( x2-x1)/2)-tsize.cx/2,   ScaleY(rc,y1 + (y2-y1)/2), ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
	    }
	#endif

	    DrawLine(hdc, rc, x1, y1, x2, y2, style);
	  }
	}

    if(bFAITri)
    {
	  if(points.size() >3)
	  {

        if(ISPARAGLIDER)
        {
		  lat0 = CContestMgr::Instance().GetBestNearClosingPoint().Latitude();
		  lon0 = CContestMgr::Instance().GetBestNearClosingPoint().Longitude();
		  lat1 = CContestMgr::Instance().GetBestClosingPoint().Latitude();
		  lon1 = CContestMgr::Instance().GetBestClosingPoint().Longitude();
		  x1 = (lon0-lon_c)*fastcosine(lat0);
		  y1 = (lat0-lat_c);
		  x2 = (lon1-lon_c)*fastcosine(lat1);
		  y2 = (lat1-lat_c);
		  DrawLine(hdc, rc, x1, y1, x2, y2, STYLE_ORANGETHIN ); //result.Predicted() ? STYLE_BLUETHIN : STYLE_REDTHICK);
        }

	    lat1 = points[1].Latitude();
	    lon1 = points[1].Longitude();
	    lat2 = points[3].Latitude();
	    lon2 = points[3].Longitude();
	    x1 = (lon1-lon_c)*fastcosine(lat1);
	    y1 = (lat1-lat_c);
	    x2 = (lon2-lon_c)*fastcosine(lat2);
	    y2 = (lat2-lat_c);

	    DrawLine(hdc, rc, x1, y1, x2, y2, STYLE_THINDASHPAPER ); //result.Predicted() ? STYLE_BLUETHIN : STYLE_REDTHICK);
#ifdef DRAWPERCENT
	    TCHAR text[180];
	    SIZE tsize;
	    _stprintf(text, TEXT("%3.1f%%"), (fTotalPercent*100.0));
	    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
	    SetTextColor(hdc, RGB_LIGHTBLUE);
	    ExtTextOut(hdc, ScaleX(rc, x1 +( x2-x1)/2)-tsize.cx/2,   ScaleY(rc,y1 + (y2-y1)/2), ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
#endif

	    lat0 = CContestMgr::Instance().GetClosingPoint().Latitude();
	    lon0 = CContestMgr::Instance().GetClosingPoint().Longitude();
	    x0 = (lon0-lon_c)*fastcosine(lat0);
	    y0 = (lat0-lat_c);
	    DrawLine(hdc, rc, xp, yp, x0, y0, STYLE_REDTHICK);
	  }
    }
  }




DrawXGrid(hdc, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
DrawYGrid(hdc, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
SetBkMode(hdc, TRANSPARENT);
SetTextColor(hdc, RGB_MAGENTA);
DrawLabel(hdc, rc, TEXT("O"), xp, yp);
SelectObject(hdc, hfOldU);

}


