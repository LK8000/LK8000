/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "DoInits.h"
#include "ScreenProjection.h"

void MapWindow::DrawWeatherStPicto(LKSurface& Surface, const RECT& rc, FANET_WEATHER* pWeather)
{
	POINT windsock[8];
	int cx = rc.right-rc.left;
	int cy = rc.bottom-rc.top;
	int x = rc.left + cx/2;
	int y = rc.top + cy/2;
	//static double zoomfact = (double)cy/NIBLSCALE(18);
	static double zoomfact = 0.17;

	const auto hpold = Surface.SelectObject(LKPen_Black_N1);
	const auto oldfont = Surface.SelectObject(LK8MapFont);	
    //Draw windsock
	windsock[0].x = 0;
	windsock[0].y = 80;
	windsock[1].x = -25;
	windsock[1].y = 30;
	windsock[2].x = 25;
	windsock[2].y = 30;
	windsock[3].x = 0;
	windsock[3].y = 80;
	for (int q=0; q < 4; q++)
	{
		windsock[q].x  = (windsock[q].x * zoomfact);
		windsock[q].y  = (windsock[q].y * zoomfact);
	}

    PolygonRotateShift(windsock, 4, x, y, AngleLimit360(pWeather->windDir + 180));
    Surface.SelectObject(LK_HOLLOW_BRUSH);
    Surface.SelectObject(LKPen_Black_N1);
    Surface.Polygon(windsock,4);

	//Draw red sock
	windsock[0].x = 25;
	windsock[0].y = 30;
	windsock[1].x = -25;
	windsock[1].y = 30;
	windsock[2].x = -20;
	windsock[2].y = -50;
	windsock[3].x = 20;
	windsock[3].y = -50;
	windsock[4].x = 25;
	windsock[4].y = 30;
	for (int q=0; q < 5; q++)
	{
		windsock[q].x  = (windsock[q].x * zoomfact);
		windsock[q].y  = (windsock[q].y * zoomfact);
	}

    PolygonRotateShift(windsock, 5, x, y, AngleLimit360(pWeather->windDir + 180));
    Surface.SelectObject(LKBrush_Red);
    Surface.SelectObject(LKPen_Black_N1);
    Surface.Polygon(windsock,5);

	//Draw white stripe 1
	windsock[0].x = 22;
	windsock[0].y = 14;
	windsock[1].x = -22;
	windsock[1].y = 14;
	windsock[2].x = -20;
	windsock[2].y = -2;
	windsock[3].x = 20;
	windsock[3].y = -2;
	windsock[4].x = 22;
	windsock[4].y = 14;
	for (int q=0; q < 5; q++)
	{
		windsock[q].x  = (windsock[q].x * zoomfact);
		windsock[q].y  = (windsock[q].y * zoomfact);
	}

    PolygonRotateShift(windsock, 5, x, y, AngleLimit360(pWeather->windDir + 180));
    Surface.SelectObject(LK_WHITE_BRUSH);
    Surface.SelectObject(LKPen_White_N1);
    Surface.Polygon(windsock,5);

	//Draw white stripe 2
	windsock[0].x = 19;
	windsock[0].y = -16;
	windsock[1].x = -19;
	windsock[1].y = -16;
	windsock[2].x = -17;
	windsock[2].y = -34;
	windsock[3].x = 17;
	windsock[3].y = -34;
	windsock[4].x = 19;
	windsock[4].y = -16;
	for (int q=0; q < 5; q++)
	{
		windsock[q].x  = (windsock[q].x * zoomfact);
		windsock[q].y  = (windsock[q].y * zoomfact);
	}

    PolygonRotateShift(windsock, 5, x, y, AngleLimit360(pWeather->windDir + 180));
    Surface.SelectObject(LK_WHITE_BRUSH);
    Surface.SelectObject(LKPen_White_N1);
    Surface.Polygon(windsock,5);
  Surface.SelectObject(oldfont);
  Surface.SelectObject(hpold);
}

// This is painting traffic icons on the screen.
void MapWindow::LKDrawFanetData(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT& Orig_Aircraft) {

  // init scaled coords for traffic icon
//static int	iCircleSize = 7;
//static int	iRectangleSize = 4;
  static short tscaler=0;
  if (DoInit[MDI_DRAWFANETDATA]) {

	switch (ScreenSize) {
		case ss480x640:
		case ss480x800:
		case ss800x480:
		case ss640x480:
			tscaler=NIBLSCALE(7)-2;
			break;
		case ss240x320:
		case ss272x480:
		case ss320x240:
		case ss480x272:
		case ss480x234:
		case ss400x240:
			tscaler=NIBLSCALE(13)-2;
			break;
		default:
			tscaler=NIBLSCALE(7);
			break;
	}
	for (int i = 0;i < MAXFANETWEATHER;i++){
		DrawInfo.FANET_Weather[i].Time_Fix = 0; //reset all Data
	}
	DoInit[MDI_DRAWFANETDATA]=false;
  }

  //POINT Arrow[5];
  POINT windsock[8];

  TCHAR lbuffer[50];

  const auto hpold = Surface.SelectObject(LKPen_Black_N1);

  TextInBoxMode_t displaymode = {};
  displaymode.NoSetFont = 1;
  const auto oldfont = Surface.SelectObject(LK8MapFont);

//  double dX, dY;
  for (int i = 0;i < MAXFANETWEATHER;i++){
    if (DrawInfo.FANET_Weather[i].Time_Fix == 0){
		continue; //nothing to display
	}
	double target_lon = DrawInfo.FANET_Weather[i].Longitude;
	double target_lat = DrawInfo.FANET_Weather[i].Latitude;
	
	if (!PointVisible(target_lon, target_lat)) {
		continue; //point not visible
	}
	POINT sc, sc_name, sc_av;
	sc = _Proj.ToRasterPoint(target_lat, target_lon );
	sc_name = sc;

	sc_name.y -= NIBLSCALE(16);
	sc_av = sc_name;

	_stprintf(lbuffer, _T("%d|%d"), 
					(int)round(DrawInfo.FANET_Weather[i].windSpeed*SPEEDMODIFY), 
					(int)round(DrawInfo.FANET_Weather[i].windGust*SPEEDMODIFY));
	displaymode.Border=1;
	TextInBox(Surface, &rc, lbuffer, sc.x+tscaler, sc.y+tscaler , &displaymode, false);

    //Draw windsock
	//Draw arrow
	windsock[0].x = 0;
	windsock[0].y = 80;
	windsock[1].x = -25;
	windsock[1].y = 30;
	windsock[2].x = 25;
	windsock[2].y = 30;
	windsock[3].x = 0;
	windsock[3].y = 80;
	for (int q=0; q < 4; q++)
	{
		windsock[q].x  = (windsock[q].x * 0.17);
		windsock[q].y  = (windsock[q].y * 0.17);
	}

    PolygonRotateShift(windsock, 4, sc.x, sc.y, DrawInfo.FANET_Weather[i].windDir + 180 - DisplayAngle);
    Surface.SelectObject(LK_HOLLOW_BRUSH);
    Surface.SelectObject(LKPen_Black_N1);
    Surface.Polygon(windsock,4);

	//Draw red sock
	windsock[0].x = 25;
	windsock[0].y = 30;
	windsock[1].x = -25;
	windsock[1].y = 30;
	windsock[2].x = -20;
	windsock[2].y = -50;
	windsock[3].x = 20;
	windsock[3].y = -50;
	windsock[4].x = 25;
	windsock[4].y = 30;
	for (int q=0; q < 5; q++)
	{
		windsock[q].x  = (windsock[q].x * 0.17);
		windsock[q].y  = (windsock[q].y * 0.17);
	}

    PolygonRotateShift(windsock, 5, sc.x, sc.y, DrawInfo.FANET_Weather[i].windDir + 180 - DisplayAngle);
    Surface.SelectObject(LKBrush_Red);
    Surface.SelectObject(LKPen_Black_N1);
    Surface.Polygon(windsock,5);

	//Draw white stripe 1
	windsock[0].x = 22;
	windsock[0].y = 14;
	windsock[1].x = -22;
	windsock[1].y = 14;
	windsock[2].x = -20;
	windsock[2].y = -2;
	windsock[3].x = 20;
	windsock[3].y = -2;
	windsock[4].x = 22;
	windsock[4].y = 14;
	for (int q=0; q < 5; q++)
	{
		windsock[q].x  = (windsock[q].x * 0.17);
		windsock[q].y  = (windsock[q].y * 0.17);
	}

    PolygonRotateShift(windsock, 5, sc.x, sc.y, DrawInfo.FANET_Weather[i].windDir + 180 - DisplayAngle);
    Surface.SelectObject(LK_WHITE_BRUSH);
    Surface.SelectObject(LKPen_White_N1);
    Surface.Polygon(windsock,5);

	//Draw white stripe 2
	windsock[0].x = 19;
	windsock[0].y = -16;
	windsock[1].x = -19;
	windsock[1].y = -16;
	windsock[2].x = -17;
	windsock[2].y = -34;
	windsock[3].x = 17;
	windsock[3].y = -34;
	windsock[4].x = 19;
	windsock[4].y = -16;
	for (int q=0; q < 5; q++)
	{
		windsock[q].x  = (windsock[q].x * 0.17);
		windsock[q].y  = (windsock[q].y * 0.17);
	}

    PolygonRotateShift(windsock, 5, sc.x, sc.y, DrawInfo.FANET_Weather[i].windDir + 180 - DisplayAngle);
    Surface.SelectObject(LK_WHITE_BRUSH);
    Surface.SelectObject(LKPen_White_N1);
    Surface.Polygon(windsock,5);
  }

  Surface.SelectObject(oldfont);
  Surface.SelectObject(hpold);

}


