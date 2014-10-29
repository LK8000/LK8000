/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"
#include "Multimap.h"


void MapWindow::DrawAircraft(LKSurface& Surface, const POINT& Orig)
{

  if ( ISPARAGLIDER || ISCAR ) {

    #define NUMPARAPOINTS 3

    POINT Para[3] = {
      { 0,-5},
      {5,9},
      {-5,9}
    };

    int pi;
    LKPen hpPOld;
    LKBrush hbPAircraftSolid; 
    LKBrush hbPAircraftSolidBg;

    if (BlackScreen) {
      hbPAircraftSolid = LKBrush_LightCyan;
      hbPAircraftSolidBg = LKBrush_Blue;
    } else {
      hbPAircraftSolid = LKBrush_Blue;
      hbPAircraftSolidBg = LKBrush_Grey;
    }

    LKBrush hbPOld = Surface.SelectObject(hbPAircraftSolidBg);
    hpPOld = Surface.SelectObject(LKPen_White_N3);
  
    PolygonRotateShift(Para, NUMPARAPOINTS, Orig.x+1, Orig.y+1,
                       DisplayAircraftAngle+
                       (DerivedDrawInfo.Heading-DrawInfo.TrackBearing));

    Surface.Polygon(Para, NUMPARAPOINTS);

    // draw it again so can get white border
    Surface.SelectObject(LKPen_Black_N1);
    Surface.SelectObject(hbPAircraftSolid);

    for(pi=0; pi<NUMPARAPOINTS; pi++)
      {
	Para[pi].x -= 1;  Para[pi].y -= 1;
      }

    Surface.Polygon(Para, NUMPARAPOINTS);

    Surface.SelectObject(hpPOld);
    Surface.SelectObject(hbPOld);

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
    LKPen hpOld;
    LKBrush hbAircraftSolid; 
    LKBrush hbAircraftSolidBg;

    hbAircraftSolid = LKBrush_Black;
    hbAircraftSolidBg = LKBrush_White;

    LKBrush hbOld = Surface.SelectObject(hbAircraftSolidBg);
    hpOld = Surface.SelectObject(hpAircraft);
  
    PolygonRotateShift(Aircraft, NUMAIRCRAFTPOINTS, Orig.x+1, Orig.y+1,
                       DisplayAircraftAngle+
                       (DerivedDrawInfo.Heading-DrawInfo.TrackBearing));

    Surface.Polygon(Aircraft, NUMAIRCRAFTPOINTS);

    // draw it again so can get white border
    Surface.SelectObject(LKPen_White_N2);
    Surface.SelectObject(hbAircraftSolid);

    for(i=0; i<NUMAIRCRAFTPOINTS; i++)
      {
	Aircraft[i].x -= 1;  Aircraft[i].y -= 1;
      }

    Surface.Polygon(Aircraft, NUMAIRCRAFTPOINTS);

    Surface.SelectObject(hpOld);
    Surface.SelectObject(hbOld);
    
    return;

  }

      // GLIDER AICRAFT NORMAL ICON

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

      LKPen oldPen = Surface.SelectObject(hpAircraft);
      Surface.Polygon(Aircraft, n);

      LKBrush hbOld = Surface.SelectObject(LKBrush_Black);
      Surface.SelectObject(LKPen_White_N2);
      Surface.Polygon(Aircraft, n);

      Surface.SelectObject(oldPen);
      Surface.SelectObject(hbOld);


}


