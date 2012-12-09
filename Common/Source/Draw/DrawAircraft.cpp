/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"
#include "Multimap.h"


void MapWindow::DrawAircraft(HDC hdc, const POINT Orig)
{

  if ( ISPARAGLIDER || ISCAR ) {

    #define NUMPARAPOINTS 3

    POINT Para[3] = {
      { 0,-5},
      {5,9},
      {-5,9}
    };

    int pi;
    HPEN hpPOld;
    HBRUSH hbPAircraftSolid; 
    HBRUSH hbPAircraftSolidBg;

    if (BlackScreen) {
      hbPAircraftSolid = LKBrush_LightCyan;
      hbPAircraftSolidBg = LKBrush_Blue;
    } else {
      hbPAircraftSolid = LKBrush_Blue;
      hbPAircraftSolidBg = LKBrush_Grey;
    }

    HBRUSH hbPOld = (HBRUSH)SelectObject(hdc, hbPAircraftSolidBg);
    hpPOld = (HPEN)SelectObject(hdc, LKPen_White_N3);
  
    PolygonRotateShift(Para, NUMPARAPOINTS, Orig.x+1, Orig.y+1,
                       DisplayAircraftAngle+
                       (DerivedDrawInfo.Heading-DrawInfo.TrackBearing));

    Polygon(hdc, Para, NUMPARAPOINTS);

    // draw it again so can get white border
    SelectObject(hdc, LKPen_Black_N1);
    SelectObject(hdc, hbPAircraftSolid);

    for(pi=0; pi<NUMPARAPOINTS; pi++)
      {
	Para[pi].x -= 1;  Para[pi].y -= 1;
      }

    Polygon(hdc, Para, NUMPARAPOINTS);

    SelectObject(hdc, hpPOld);
    SelectObject(hdc, hbPOld);

    return;
  }

  if ( ISGAAIRCRAFT ) {

#define NUMAIRCRAFTPOINTS 16

    POINT Aircraft[NUMAIRCRAFTPOINTS] = {
      { 1,-6},
      {2,-1},
      {15,0},
      {15,2},
      {1,2},
      {0,10},
      {4,11},
      {4,12},
      {-4,12},
      {-4,11},
      {0,10},
      {-1,2},
      {-15,2},
      {-15,0},
      {-2,-1},
      {-1,-6}
    };

    int i;
    HPEN hpOld;
    HBRUSH hbAircraftSolid; 
    HBRUSH hbAircraftSolidBg;

    hbAircraftSolid = LKBrush_Black;
    hbAircraftSolidBg = LKBrush_White;

    HBRUSH hbOld = (HBRUSH)SelectObject(hdc, hbAircraftSolidBg);
    hpOld = (HPEN)SelectObject(hdc, hpAircraft);
  
    PolygonRotateShift(Aircraft, NUMAIRCRAFTPOINTS, Orig.x+1, Orig.y+1,
                       DisplayAircraftAngle+
                       (DerivedDrawInfo.Heading-DrawInfo.TrackBearing));

    Polygon(hdc, Aircraft, NUMAIRCRAFTPOINTS);

    // draw it again so can get white border
    SelectObject(hdc, LKPen_White_N2);
    SelectObject(hdc, hbAircraftSolid);

    for(i=0; i<NUMAIRCRAFTPOINTS; i++)
      {
	Aircraft[i].x -= 1;  Aircraft[i].y -= 1;
      }

    Polygon(hdc, Aircraft, NUMAIRCRAFTPOINTS);

    SelectObject(hdc, hpOld);
    SelectObject(hdc, hbOld);
    
    return;

  }

      // GLIDER AICRAFT NORMAL ICON

      HPEN oldPen;
      POINT Aircraft[] = {
	{1, -5},
	{1, 0},
	{14, 0}, 
	{14, 1}, 
	{1, 1},
	{1, 8},
	{4, 8},
	{4, 9},
	{-3, 9},
	{-3, 8},
	{0, 8},
	{0, 1},
	{-13, 1}, 
	{-13, 0}, 
	{0, 0},
	{0, -5},
	{1, -5},
      };

      int n = sizeof(Aircraft)/sizeof(Aircraft[0]);

      double angle = DisplayAircraftAngle+
	(DerivedDrawInfo.Heading-DrawInfo.TrackBearing);

      PolygonRotateShift(Aircraft, n,
			 Orig.x-1, Orig.y, angle);

      oldPen = (HPEN)SelectObject(hdc, hpAircraft);
      Polygon(hdc, Aircraft, n);

      HBRUSH hbOld;
      hbOld = (HBRUSH)SelectObject(hdc, GetStockObject(BLACK_BRUSH));
      SelectObject(hdc, LKPen_White_N2);
      Polygon(hdc, Aircraft, n);

      SelectObject(hdc, oldPen);
      SelectObject(hdc, hbOld);


}


