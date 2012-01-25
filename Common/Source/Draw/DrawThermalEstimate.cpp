/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id#
*/

#include "externs.h"
#include "MapWindow.h"
#include "Bitmaps.h"
#include "LKObjects.h"


//
// Draw circles and gadgets for thermals
//
void MapWindow::DrawThermalEstimate(HDC hdc, const RECT rc) {
  POINT screen;
  HPEN oldPen;
  if (!EnableThermalLocator) return;

  if (mode.Is(Mode::MODE_CIRCLING)) {
	if (DerivedDrawInfo.ThermalEstimate_R>0) {
		LatLon2Screen(DerivedDrawInfo.ThermalEstimate_Longitude, DerivedDrawInfo.ThermalEstimate_Latitude, screen);
		DrawBitmapIn(hdc, screen, hBmpThermalSource,true);

		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		double tradius;
		if (ISPARAGLIDER)
			tradius=50;
		else
			tradius=100;
			
		oldPen=(HPEN)SelectObject(hdc, LKPen_White_N3); 
		Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc);
		SelectObject(hdc, hpAircraftBorder); 
		Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify())+NIBLSCALE(2), rc);
		Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc);

		/* 101219 This would display circles around the simulated thermal, but people is confused.
		if (SIMMODE && (ThLatitude>1 && ThLongitude>1)) { // there's a thermal to show
			if ((counter==5 || counter==6|| counter==7)) {
				LatLon2Screen(ThLongitude, ThLatitude, screen);
				SelectObject(hdc, hSnailPens[7]);  
				Circle(hdc, screen.x, screen.y, (int)(ThermalRadius*zoom.ResScaleOverDistanceModify()), rc); 
				SelectObject(hdc, hSnailPens[7]); 
				Circle(hdc, screen.x, screen.y, (int)((ThermalRadius+SinkRadius)*zoom.ResScaleOverDistanceModify()), rc); 
			}
			if (++counter>=60) counter=0;
		}
 		*/

		SelectObject(hdc,oldPen);
	}
  } else {
	if (zoom.RealScale() <= 4) {
		for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
			if (DerivedDrawInfo.ThermalSources[i].Visible) {
				DrawBitmapIn(hdc, DerivedDrawInfo.ThermalSources[i].Screen, hBmpThermalSource,true);
			}
		}
	}
  }
}



//
// Paint a circle around thermal multitarget
// Called only during map mode L>
//
void MapWindow::DrawThermalEstimateMultitarget(HDC hdc, const RECT rc) {

  POINT screen;
  HPEN  oldPen;
  int idx=0;

  // do not mix old and new thermals 
  if (mode.Is(Mode::MODE_CIRCLING))
    return; 

  // draw only when visible , at high zoom level
  if ( MapWindow::zoom.RealScale() >1 ) return;

  idx=GetThermalMultitarget();
  // no L> target destination
  if (idx <0)
    return; 

  LatLon2Screen( ThermalHistory[idx].Longitude, ThermalHistory[idx].Latitude, screen);

  //DrawBitmapIn(hdc, screen, hBmpThermalSource);

  SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
  double tradius;
  if (ISPARAGLIDER)
     tradius=100;
  else
     tradius=200;

  oldPen=(HPEN)SelectObject(hdc, LKPen_White_N3);

  Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc);
  SelectObject(hdc, hpAircraftBorder);
  Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify())+NIBLSCALE(2), rc);
  Circle(hdc, screen.x, screen.y, (int)(tradius*zoom.ResScaleOverDistanceModify()), rc);

  SelectObject(hdc,oldPen);


}


