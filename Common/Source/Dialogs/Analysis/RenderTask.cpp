/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"


void Statistics::RenderTask(LKSurface& Surface, const RECT& rc, const bool olcmode)
{
  int i, j;
  unsigned int ui;
double fXY_Scale = 1.5;
  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
  double x1, y1, x2=0, y2=0;
  double lat_c, lon_c;
  double aatradius[MAXTASKPOINTS];

  // find center
  ResetScale();

  if ( (!ValidTaskPoint(0) || !ValidTaskPoint(1)) && !olcmode)
  {
	DrawNoData(Surface,rc);
	return;
  }

  for (i=0; i<MAXTASKPOINTS; i++)
  {
    aatradius[i]=0;
  }
  bool nowaypoints = true;

  for (i=0; i<MAXTASKPOINTS; i++)
  {
    if (ValidTaskPoint(i))
    {
      lat1 = WayPointList[Task[i].Index].Latitude;
      lon1 = WayPointList[Task[i].Index].Longitude;
      ScaleYFromValue(rc, lat1);
      ScaleXFromValue(rc, lon1);
      nowaypoints = false;
    }
  }

  if (nowaypoints )
  {
    DrawNoData(Surface, rc);
    return;
  }

  CPointGPSArray trace;
  CContestMgr::Instance().Trace(trace);
  for(ui=0; ui<trace.size(); ui++)
  {
    lat1 = trace[ui].Latitude();
    lon1 = trace[ui].Longitude();
    ScaleYFromValue(rc, lat1);
    ScaleXFromValue(rc, lon1);
  }
  const auto hfOldU = Surface.SelectObject(LK8InfoNormalFont);

  lat_c = (y_max+y_min)/2;
  lon_c = (x_max+x_min)/2;

  int nwps = 0;

  // find scale
  ResetScale();

  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1-lon_c)*fastcosine(lat1);
  y1 = (lat1-lat_c);
  ScaleXFromValue(rc, x1*fXY_Scale);
  ScaleYFromValue(rc, y1*fXY_Scale);



  for (i=0; i<MAXTASKPOINTS; i++)
  {
    if (ValidTaskPoint(i))
    {
      nwps++;
      lat1 = WayPointList[Task[i].Index].Latitude;
      lon1 = WayPointList[Task[i].Index].Longitude;
      x1 = (lon1-lon_c)*fastcosine(lat1);
      y1 = (lat1-lat_c);
      ScaleXFromValue(rc, x1*fXY_Scale);
      ScaleYFromValue(rc, y1*fXY_Scale);

      if (AATEnabled)
      {
	    double aatlat;
	    double aatlon;
	    double bearing;
	    double radius;

        if (ValidTaskPoint(i+1))
        {
          if (Task[i].AATType == SECTOR)
            radius = Task[i].AATSectorRadius;
          else
            radius = Task[i].AATCircleRadius;

          for (j=0; j<4; j++)
          {
            bearing = j*360.0/4;
            FindLatitudeLongitude(WayPointList[Task[i].Index].Latitude, WayPointList[Task[i].Index].Longitude, bearing, radius,  &aatlat, &aatlon);
            x1 = (aatlon-lon_c)*fastcosine(aatlat);
            y1 = (aatlat-lat_c);
            ScaleXFromValue(rc, x1);
            ScaleYFromValue(rc, y1);
            if (j==0)
            {
              aatradius[i] = fabs(aatlat-WayPointList[Task[i].Index].Latitude);
            }
          }
        }
        else
        {
          aatradius[i] = 0;
        }
      }
    }
  }


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


  // draw aat areas
    if (AATEnabled)
    {
      for (i=MAXTASKPOINTS-1; i>0; i--)
      {
	    if (ValidTaskPoint(i))
	    {
		  lat1 = WayPointList[Task[i-1].Index].Latitude;
		  lon1 = WayPointList[Task[i-1].Index].Longitude;
		  lat2 = WayPointList[Task[i].Index].Latitude;
		  lon2 = WayPointList[Task[i].Index].Longitude;
		  x1 = (lon1-lon_c)*fastcosine(lat1);
		  y1 = (lat1-lat_c);
		  x2 = (lon2-lon_c)*fastcosine(lat2);
		  y2 = (lat2-lat_c);

#ifdef HAVE_HATCHED_BRUSH 
		  Surface.SelectObject(MapWindow::GetAirspaceBrushByClass(AATASK));
#else
                  Surface.SelectObject(LKBrush_Yellow);
#endif
		  Surface.SelectObject(LK_WHITE_PEN);
		  if (Task[i].AATType == SECTOR)
		  {
			Surface.Segment((long)((x2-x_min)*xscale+rc.left+BORDER_X),(long)((y_max-y2)*yscale+rc.top),(long)(aatradius[i]*yscale),rc,	Task[i].AATStartRadial,	Task[i].AATFinishRadial);
		  }
		  else
		  {
	        Surface.Circle((long)((x2-x_min)*xscale+rc.left+BORDER_X), (long)((y_max-y2)*yscale+rc.top),  (long)(aatradius[i]*yscale), rc);
	      }
        }
      }
    }

  if (!AATEnabled)
  {
	for (i=MAXTASKPOINTS-1; i>0; i--)
	{
	  if (ValidTaskPoint(i) && ValidTaskPoint(i-1))
	  {
		lat1 = WayPointList[Task[i-1].Index].Latitude;
		lon1 = WayPointList[Task[i-1].Index].Longitude;
		if (!ValidTaskPoint(1) ) {
		  lat2 = GPS_INFO.Latitude;
		  lon2 = GPS_INFO.Longitude;
		}
		else
		{
		  lat2 = WayPointList[Task[i].Index].Latitude;
		  lon2 = WayPointList[Task[i].Index].Longitude;
		}
		x1 = (lon1-lon_c)*fastcosine(lat1);
		y1 = (lat1-lat_c);
		x2 = (lon2-lon_c)*fastcosine(lat2);
		y2 = (lat2-lat_c);

	//	DrawLine(hdc, rc, x1, y1, x2, y2, STYLE_DASHGREEN);
		if( ValidTaskPoint(4) && i <2)
			goto skip_FAI;
#ifndef UNDITHER
		RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,1, RGB_LIGHTYELLOW );
	    RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,0, RGB_LIGHTCYAN   );
#else
		RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,1, RGB_LIGHTGREY );
	    RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,0, RGB_GREY   );
#endif
	    skip_FAI:
		DrawLine(Surface, rc, x1, y1, x2, y2, STYLE_DASHGREEN);
		Surface.Segment((long)((x2-x_min)*xscale+rc.left+BORDER_X),(long)((y_max-y2)*yscale+rc.top),(long)(aatradius[i]*yscale),rc,	Task[i].AATStartRadial,	Task[i].AATFinishRadial);
	  }
	}

	if(	 ValidTaskPoint(1) && ValidTaskPoint(3))
	{
	  lat1 = WayPointList[Task[3].Index].Latitude;
	  lon1 = WayPointList[Task[3].Index].Longitude;
	  lat2 = WayPointList[Task[1].Index].Latitude;
	  lon2 = WayPointList[Task[1].Index].Longitude;
          #ifndef UNDITHER
	  RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,1, RGB_LIGHTYELLOW );
	  RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,0, RGB_LIGHTCYAN   );
          #else
	  RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,1, RGB_LIGHTGREY );
	  RenderFAISector ( Surface, rc, lat1, lon1, lat2, lon2, lat_c, lon_c,0, RGB_GREY   );
          #endif
	}
  }
	// draw task lines and label
	for (i=MAXTASKPOINTS-1; i>0; i--)
	{
	  if (ValidTaskPoint(i) && ValidTaskPoint(i-1))
	  {
		lat1 = WayPointList[Task[i-1].Index].Latitude;
		lon1 = WayPointList[Task[i-1].Index].Longitude;
		if (!ValidTaskPoint(1) ) {
		  lat2 = GPS_INFO.Latitude;
		  lon2 = GPS_INFO.Longitude;
		}
		else
		{
		  lat2 = WayPointList[Task[i].Index].Latitude;
		  lon2 = WayPointList[Task[i].Index].Longitude;
		}
		x1 = (lon1-lon_c)*fastcosine(lat1);
		y1 = (lat1-lat_c);
		x2 = (lon2-lon_c)*fastcosine(lat2);
		y2 = (lat2-lat_c);

		DrawLine(Surface, rc, x1, y1, x2, y2, STYLE_BLUETHIN);
                #if (WINDOWSPC>0)
	    Surface.SetBackgroundOpaque();
                #endif
		TCHAR text[100];
		 Surface.SetTextColor(RGB_BLUE);
/*
		if ((i==nwps-1) && (Task[i].Index == Task[0].Index))
		{
		  _stprintf(text,TEXT("%0d"),1);
		  DrawLabel(hdc, rc, text, x1+(x2-x1)/2, y1+(y2-y1)/2);
		}
		else */
		{
		  _stprintf(text,TEXT("%0d"),i);
		  DrawLabel(Surface, rc, text, x1+(x2-x1)/2, y1+(y2-y1)/2);
		}

		if ((i==ActiveWayPoint)&&(!AATEnabled))
		{
		  lat1 = GPS_INFO.Latitude;
		  lon1 = GPS_INFO.Longitude;
		  x1 = (lon1-lon_c)*fastcosine(lat1);
		  y1 = (lat1-lat_c);
		  DrawLine(Surface, rc, x1, y1, x2, y2,  STYLE_REDTHICK);
		}
	  }
	}
	
	// draw aat task line
	
	if (AATEnabled)
	{
	  for (i=MAXTASKPOINTS-1; i>0; i--)
	  {
		if (ValidTaskPoint(i) && ValidTaskPoint(i-1))
		{
		  if (i==1)
		  {
			lat1 = WayPointList[Task[i-1].Index].Latitude;
			lon1 = WayPointList[Task[i-1].Index].Longitude;
		  }
		  else
		  {
			lat1 = Task[i-1].AATTargetLat;
			lon1 = Task[i-1].AATTargetLon;
		  }
		  lat2 = Task[i].AATTargetLat;
		  lon2 = Task[i].AATTargetLon;
	
		  x1 = (lon1-lon_c)*fastcosine(lat1);
		  y1 = (lat1-lat_c);
		  x2 = (lon2-lon_c)*fastcosine(lat2);
		  y2 = (lat2-lat_c);

		  DrawLine(Surface, rc,   x1, y1, x2, y2,  STYLE_REDTHICK);
		}
	  }
	}

	  DrawXGrid(Surface, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);
	  DrawYGrid(Surface, rc, 1.0, 0, STYLE_THINDASHPAPER, 1.0, false);


  Surface.SelectObject(hfOldU);
  // Draw aircraft on top
  lat1 = GPS_INFO.Latitude;
  lon1 = GPS_INFO.Longitude;
  x1 = (lon1-lon_c)*fastcosine(lat1);
  y1 = (lat1-lat_c);
  Surface.SetBackgroundTransparent();
  DrawLabel(Surface, rc, TEXT("+"), x1, y1);
}

