/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "MapWindow.h"
#include "LKObjects.h"
#include "Bitmaps.h"
#include "DoInits.h"



// This is painting traffic icons on the screen.
void MapWindow::LKDrawFLARMTraffic(HDC hDC, RECT rc, POINT Orig_Aircraft) {

  if (!EnableFLARMMap) return;

  if (!DrawInfo.FLARM_Available) return;

  // init scaled coords for traffic icon
  static short scaler[5];
  static short tscaler=0;
  if (DoInit[MDI_DRAWFLARMTRAFFIC]) {

	switch (ScreenSize) {
		case ss480x640:
		case ss480x800:
		case ss896x672:
		case ss800x480:
		case ss640x480:
			scaler[0]=-1*(NIBLSCALE(4)-2);
			scaler[1]=NIBLSCALE(5)-2;
			scaler[2]=-1*(NIBLSCALE(6)-2);
			scaler[3]=NIBLSCALE(4)-2;
			scaler[4]=NIBLSCALE(2)-2;
			tscaler=NIBLSCALE(7)-2;
			break;
		case ss240x320:
		case ss272x480:
		case ss320x240:
		case ss480x272:
		case ss720x408:
		case ss480x234:
		case ss400x240:
			scaler[0]=-1*(NIBLSCALE(8)-2);
			scaler[1]=NIBLSCALE(10)-2;
			scaler[2]=-1*(NIBLSCALE(12)-2);
			scaler[3]=NIBLSCALE(8)-2;
			scaler[4]=NIBLSCALE(4)-2;
			tscaler=NIBLSCALE(13)-2;
			break;
		default:
			scaler[0]=-1*NIBLSCALE(4);
			scaler[1]=NIBLSCALE(5);
			scaler[2]=-1*NIBLSCALE(6);
			scaler[3]=NIBLSCALE(4);
			scaler[4]=NIBLSCALE(2);
			tscaler=NIBLSCALE(7);
			break;
	}


	DoInit[MDI_DRAWFLARMTRAFFIC]=false;
  }

  HPEN hpold;
  HPEN thinBlackPen = LKPen_Black_N1;
  POINT Arrow[5];

  TCHAR lbuffer[50];

  hpold = (HPEN)SelectObject(hDC, thinBlackPen);

  int i;
  int painted=0;

//  double dX, dY;

  TextInBoxMode_t displaymode = {0};
  displaymode.NoSetFont = 1;

  #if 0
  double screenrange = GetApproxScreenRange();
  double scalefact = screenrange/6000.0; // FIX 100820
  #endif

  HBRUSH redBrush = LKBrush_Red;
  HBRUSH yellowBrush = LKBrush_Yellow;
  HBRUSH greenBrush = LKBrush_Green;
  HFONT  oldfont = (HFONT)SelectObject(hDC, LK8MapFont);

  for (i=0,painted=0; i<FLARM_MAX_TRAFFIC; i++) {

	// limit to 10 icons map traffic
	if (painted>=10) {
		i=FLARM_MAX_TRAFFIC;
		continue;
	}

	if ( (DrawInfo.FLARM_Traffic[i].ID !=0) && (DrawInfo.FLARM_Traffic[i].Status != LKT_ZOMBIE) ) {

		painted++;

		double target_lon;
		double target_lat;

		target_lon = DrawInfo.FLARM_Traffic[i].Longitude;
		target_lat = DrawInfo.FLARM_Traffic[i].Latitude;

		#if (0) // No scaling, wrong
		if ((EnableFLARMMap==2)&&(scalefact>1.0)) {

			double distance;
			double bearing;

			DistanceBearing(DrawInfo.Latitude, DrawInfo.Longitude, target_lat, target_lon, &distance, &bearing);
			FindLatitudeLongitude(DrawInfo.Latitude, DrawInfo.Longitude, bearing, distance*scalefact, &target_lat, &target_lon);
		}
		#endif
      
		POINT sc, sc_name, sc_av;
		LatLon2Screen(target_lon, target_lat, sc);

		sc_name = sc;

		sc_name.y -= NIBLSCALE(16);
		sc_av = sc_name;

		wsprintf(lbuffer,_T(""));
		if (DrawInfo.FLARM_Traffic[i].Cn && DrawInfo.FLARM_Traffic[i].Cn[0]!=_T('?')) { // 100322
			_tcscat(lbuffer,DrawInfo.FLARM_Traffic[i].Cn);
		}
		if (DrawInfo.FLARM_Traffic[i].Average30s>=0.1) {
			if (_tcslen(lbuffer) >0)
				_stprintf(lbuffer,_T("%s:%.1f"),lbuffer,LIFTMODIFY*DrawInfo.FLARM_Traffic[i].Average30s);
			else
				_stprintf(lbuffer,_T("%.1f"),LIFTMODIFY*DrawInfo.FLARM_Traffic[i].Average30s);
		}

		displaymode.Border=1;

		if (_tcslen(lbuffer)>0)
		TextInBox(hDC, lbuffer, sc.x+tscaler, sc.y+tscaler, 0, &displaymode, false);

		// red circle
		if ((DrawInfo.FLARM_Traffic[i].AlarmLevel>0) && (DrawInfo.FLARM_Traffic[i].AlarmLevel<4)) {
			DrawBitmapIn(hDC, sc, hFLARMTraffic,true);
		}
#if 0
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
#endif

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

		switch (DrawInfo.FLARM_Traffic[i].Status) { // 100321
			case LKT_GHOST:
				SelectObject(hDC, yellowBrush);
				break;
			case LKT_ZOMBIE:
				SelectObject(hDC, redBrush);
				break;
			default:
				SelectObject(hDC, greenBrush);
				break;
		}

		PolygonRotateShift(Arrow, 5, sc.x, sc.y, DrawInfo.FLARM_Traffic[i].TrackBearing - DisplayAngle);
		Polygon(hDC,Arrow,5);

	}
  }

  SelectObject(hDC, oldfont);
  SelectObject(hDC, hpold);

}


