/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "DoInits.h"
#include "FlarmRadar.h"
#include "ScreenProjection.h"

extern const LKBrush * variobrush[NO_VARIO_COLORS];

// This is painting traffic icons on the screen.
void MapWindow::LKDrawFLARMTraffic(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT& Orig_Aircraft) {

  if (!EnableFLARMMap) return;

  if (!DrawInfo.FLARM_Available) return;

  // init scaled coords for traffic icon
static int	iCircleSize = 7;
static int	iRectangleSize = 4;
  static short tscaler=0;
  if (DoInit[MDI_DRAWFLARMTRAFFIC]) {

	switch (ScreenSize) {
		case ss480x640:
		case ss480x800:
		case ss800x480:
		case ss640x480:
			iCircleSize = 9;
			iRectangleSize = 5;
			tscaler=NIBLSCALE(7)-2;
			break;
		case ss240x320:
		case ss272x480:
		case ss320x240:
		case ss480x272:
		case ss480x234:
		case ss400x240:
			iCircleSize = 7;
			iRectangleSize = 4;
			tscaler=NIBLSCALE(13)-2;
			break;
		default:
			iCircleSize = 7;
			iRectangleSize = 4;
			tscaler=NIBLSCALE(7);
			break;
	}


	DoInit[MDI_DRAWFLARMTRAFFIC]=false;
  }

  POINT Arrow[5];

  TCHAR lbuffer[50];

  const auto hpold = Surface.SelectObject(LKPen_Black_N1);

  int i;
  int painted=0;

//  double dX, dY;

  TextInBoxMode_t displaymode = {};
  displaymode.NoSetFont = 1;

  #if 0
  double screenrange = GetApproxScreenRange();
  double scalefact = screenrange/6000.0; // FIX 100820
  #endif




  const auto oldfont = Surface.SelectObject(LK8MapFont);

  for (i=0,painted=0; i<FLARM_MAX_TRAFFIC; i++) {

	// limit to 10 icons map traffic
	if (painted>=10) {
		i=FLARM_MAX_TRAFFIC;
		continue;
	}

	if ( (DrawInfo.FLARM_Traffic[i].ID !=0) && (DrawInfo.FLARM_Traffic[i].Status != LKT_ZOMBIE) ) {


		double target_lon;
		double target_lat;

		target_lon = DrawInfo.FLARM_Traffic[i].Longitude;
		target_lat = DrawInfo.FLARM_Traffic[i].Latitude;

                if (!PointVisible(target_lon, target_lat)) {
                   // StartupStore(_T("... This traffic is not visible on map with current zoom%s"),NEWLINE);
                   continue;
                }

		painted++;

		#if (0) // No scaling, wrong
		if ((EnableFLARMMap==2)&&(scalefact>1.0)) {

			double distance;
			double bearing;

			DistanceBearing(DrawInfo.Latitude, DrawInfo.Longitude, target_lat, target_lon, &distance, &bearing);
			FindLatitudeLongitude(DrawInfo.Latitude, DrawInfo.Longitude, bearing, distance*scalefact, &target_lat, &target_lon);
		}
		#endif
      
		POINT sc, sc_name, sc_av;
		sc = _Proj.ToRasterPoint(target_lat, target_lon);

		sc_name = sc;

		sc_name.y -= NIBLSCALE(16);
		sc_av = sc_name;

		_tcscpy(lbuffer,_T(""));
		if (DrawInfo.FLARM_Traffic[i].Cn[0]!=_T('?')) { // 100322
			_tcscat(lbuffer,DrawInfo.FLARM_Traffic[i].Cn);
		}
		if (DrawInfo.FLARM_Traffic[i].Average30s>=0.1) {
                  size_t len = _tcslen(lbuffer);
                  if (len > 0)
                    _stprintf(lbuffer + len,_T(":%.1f"),LIFTMODIFY*DrawInfo.FLARM_Traffic[i].Average30s);
                  else
                    _stprintf(lbuffer,_T("%.1f"),LIFTMODIFY*DrawInfo.FLARM_Traffic[i].Average30s);
		}

		displaymode.Border=1;

		if (_tcslen(lbuffer)>0)
		TextInBox(Surface, &rc, lbuffer, sc.x+tscaler, sc.y+tscaler, &displaymode, false);

		// red circle
		if ((DrawInfo.FLARM_Traffic[i].AlarmLevel>0) && (DrawInfo.FLARM_Traffic[i].AlarmLevel<4)) {
			DrawBitmapIn(Surface, sc, hFLARMTraffic);
		}
#if 1 // 1
		Arrow[0].x = -4;
		Arrow[0].y = 5;
		Arrow[1].x = 0;
		Arrow[1].y = -6;
		Arrow[2].x = 4;
		Arrow[2].y = 5;
		Arrow[3].x = 0;
		Arrow[3].y = 2;
		Arrow[4].x = -4;
		Arrow[4].y = 5;

		for (int q=0; q < 5; q++)
		{
			Arrow[q].x  = (LONG) ((double)Arrow[q].x * 1.7);
			Arrow[q].y  = (LONG) ((double)Arrow[q].y * 1.7);
		}
#else

		Arrow[0].x = scaler[0];
		Arrow[0].y = scaler[1];
		Arrow[1].x = 0;
		Arrow[1].y = scaler[2];
		Arrow[2].x = scaler[3];
		Arrow[2].y = scaler[1];
		Arrow[3].x = 0;
		Arrow[3].y = scaler[4];
		Arrow[4].x = scaler[0];
		Arrow[4].y = scaler[1];
#endif


/*
		switch (DrawInfo.FLARM_Traffic[i].Status) { // 100321
			case LKT_GHOST:
				Surface.SelectObject(yellowBrush);
				break;
			case LKT_ZOMBIE:
				Surface.SelectObject(redBrush);
				break;
			default:
				Surface.SelectObject(greenBrush);
				break;
		}
*/
		  /*************************************************************************
		   * calculate climb color
		   *************************************************************************/

		  int iVarioIdx = (int)(2*DrawInfo.FLARM_Traffic[i].Average30s  -0.5)+NO_VARIO_COLORS/2;
		  if(iVarioIdx < 0) iVarioIdx =0;
		  if(iVarioIdx >= NO_VARIO_COLORS) iVarioIdx =NO_VARIO_COLORS-1;
		  Surface.SelectObject(*variobrush[iVarioIdx]);

		  switch (DrawInfo.FLARM_Traffic[i].Status) { // 100321
			case LKT_GHOST:
				Surface.Rectangle(sc.x-iRectangleSize,  sc.y-iRectangleSize,sc.x+iRectangleSize, sc.y+iRectangleSize);
				break;
			case LKT_ZOMBIE:
				Surface.DrawCircle(sc.x,  sc.x, iCircleSize, rc, true );
				break;
			default:
				PolygonRotateShift(Arrow, 5, sc.x, sc.y, DrawInfo.FLARM_Traffic[i].TrackBearing - DisplayAngle);
				Surface.Polygon(Arrow,5);
				break;
		  }

	}
  }

  Surface.SelectObject(oldfont);
  Surface.SelectObject(hpold);

}


