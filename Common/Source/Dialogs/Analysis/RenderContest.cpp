/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "ContestMgr.h"

CContestMgr::TType contestType = CContestMgr::TYPE_OLC_CLASSIC;





/*****************************************************************
 * Alpha Lima splitted RenderContest from Render Task for CC
 * adding FAI Sector display
 ****************************************************************/
void Statistics::RenderContest(HDC hdc, const RECT rc)
{
  unsigned int ui;
  double fXY_Scale = 1.0;
  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
  double x1, y1, x2=0, y2=0;
  double lat_c, lon_c;

  ResetScale();
  CContestMgr::CResult result = CContestMgr::Instance().Result(contestType, true);

  if(result.Type() == CContestMgr::TYPE_FAI_TRIANGLE)
     fXY_Scale = 1.5;

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

  HFONT hfOldU = (HFONT)SelectObject(hdc, LK8InfoNormalFont);
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


  if(result.Type() == contestType)
  {
    const CPointGPSArray &points = result.PointArray();

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
        DrawLine(hdc, rc, x1, y1, x2, y2, style);
      }

      if((result.Type() == CContestMgr::TYPE_FAI_TRIANGLE))// TYPE_FAI_TRIANGLE
      {
        double fDist, fAngle;
        DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);
        if(ui < 2)
        if(fDist > 10000)
        { COLORREF rgbCol = RGB_BLUE;
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
        DrawLine(hdc, rc, x1, y1, x2, y2, style);
      }
    }

    if((result.Type() == CContestMgr::TYPE_FAI_TRIANGLE))// TYPE_FAI_TRIANGLE
    {
	  // draw track again 8over sectors
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
      if((result.Type() == CContestMgr::TYPE_FAI_TRIANGLE))// TYPE_FAI_TRIANGLE
      {
        double fDist, fAngle;
        DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);
        if(fDist > 10000)
        {
//    		RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,1, 0 );
//    	    RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,0, 0   );
        }
        DrawLine(hdc, rc, x1, y1, x2, y2, style);
      }
    }

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
  }

  DrawXGrid(hdc, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
  DrawYGrid(hdc, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);


  SelectObject(hdc, hfOldU);
  // Draw aircraft on top
  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1-lon_c)*fastcosine(lat1);
  y1 = (lat1-lat_c);
  SetBkMode(hdc, TRANSPARENT);
  DrawLabel(hdc, rc, TEXT("+"), x1, y1);
}


