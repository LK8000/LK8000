/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Multimap.h"

// RETURNS Longitude, Latitude!
void MapWindow::OrigScreen2LatLon(const int &x, const int &y, double &X, double &Y) {
  int sx = x;
  int sy = y;
  irotate(sx, sy, DisplayAngle);
  Y= PanLatitude  - sy*zoom.InvDrawScale();
  X= PanLongitude + sx*invfastcosine(Y)*zoom.InvDrawScale();
}



void MapWindow::Screen2LatLon(const int &x, const int &y, double &X, double &Y) {
  int sx = x-(int)Orig_Screen.x;
  int sy = y-(int)Orig_Screen.y;
  irotate(sx, sy, DisplayAngle);
  Y= PanLatitude  - sy*zoom.InvDrawScale();
  X= PanLongitude + sx*invfastcosine(Y)*zoom.InvDrawScale();
}


//
// Called Sideview, but really we are talking about TOPVIEW!!
//
void MapWindow::SideviewScreen2LatLon(const int &x, const int &y, double &X, double &Y) {
  int sx = x-(int)Current_Multimap_TopOrig.x;
  int sy = y-(int)Current_Multimap_TopOrig.y;
  irotate(sx, sy, Current_Multimap_TopAngle);
  Y= PanLatitude  - sy*Current_Multimap_TopZoom;
  X= PanLongitude + sx*invfastcosine(Y)*Current_Multimap_TopZoom;
}


void MapWindow::LatLon2Screen(const double &lon, const double &lat, POINT &sc) {
  int Y = Real2Int((PanLatitude-lat)*zoom.DrawScale());
  int X = Real2Int((PanLongitude-lon)*fastcosine(lat)*zoom.DrawScale());
    
  irotate(X, Y, DisplayAngle);
    
  sc.x = Orig_Screen.x - X;
  sc.y = Orig_Screen.y + Y;
}


//
// This one is optimised for long polygons
//
void MapWindow::LatLon2Screen(pointObj *ptin, POINT *ptout, const int n, const int skip) {
  static double lastangle = -1;
  static int cost=1024, sint=0;
  const double mDisplayAngle = DisplayAngle;

  if(mDisplayAngle != lastangle) {
    lastangle = mDisplayAngle;
    int deg = DEG_TO_INT(AngleLimit360(mDisplayAngle));
    cost = ICOSTABLE[deg];
    sint = ISINETABLE[deg];
  }
  const int xxs = Orig_Screen.x*1024-512;
  const int yys = Orig_Screen.y*1024+512;
  const double mDrawScale = zoom.DrawScale();
  const double mPanLongitude = PanLongitude;
  const double mPanLatitude = PanLatitude;
  pointObj* p = ptin;
  const pointObj* ptend = ptin+n;

  while (p<ptend) {
    int Y = Real2Int((mPanLatitude-p->y)*mDrawScale);
    int X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)*mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    ptout++;

    p+= skip;
  }
}


#if TOPO_CLIPPING
//
// This one is optimised for long polygons reduces
//
int MapWindow::LatLon2ScreenCompr(pointObj *ptin, POINT *ptout, const int n, const int skip) {
  static double lastangle = -1;
  static int cost=1024, sint=0;
  const double mDisplayAngle = DisplayAngle;

  if(mDisplayAngle != lastangle) {
    lastangle = mDisplayAngle;
    int deg = DEG_TO_INT(AngleLimit360(mDisplayAngle));
    cost = ICOSTABLE[deg];
    sint = ISINETABLE[deg];
  }
  const int xxs = Orig_Screen.x*1024-512;
  const int yys = Orig_Screen.y*1024+512;
  const double mDrawScale = zoom.DrawScale();
  const double mPanLongitude = PanLongitude;
  const double mPanLatitude = PanLatitude;
  pointObj* p = ptin;
  const pointObj* ptend = ptin+n;
  pointObj oldpt ={-10,-10};
  int icnt=0;
  while (p<ptend) {
    int Y = Real2Int((mPanLatitude-p->y)*mDrawScale);
    int X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)*mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    if(( abs( ptout->x - oldpt.x) >5) || ( abs( ptout->y - oldpt.y) >5))
    {
      oldpt.x = ptout->x;
      oldpt.y = ptout->y;
      ptout++;
      icnt++;
    }
    p+= skip;
  }
  return icnt;
}
#endif


//
// Since many functions call LatLon2screen, and only a few need multimap checks for display angle,
// we separate functions to avoid slow downs. First used by CalculateScreenPositionsGroundline
//
void MapWindow::LatLon2ScreenMultimap(pointObj *ptin, POINT *ptout, const int n, const int skip) {
  static double lastangle = -1;
  static int cost=1024, sint=0;

  LKASSERT(Current_Multimap_TopZoom!=0);

  const double mDisplayAngle = Current_Multimap_TopAngle;
  if(mDisplayAngle != lastangle) {
	lastangle = mDisplayAngle;
	int deg = DEG_TO_INT(AngleLimit360(mDisplayAngle));
	cost = ICOSTABLE[deg];
	sint = ISINETABLE[deg];
  }

  const int xxs = Current_Multimap_TopOrig.x*1024-512;
  const int yys = Current_Multimap_TopOrig.y*1024+512;
  const double mDrawScale = Current_Multimap_TopZoom;
  const double mPanLongitude = PanLongitude;
  const double mPanLatitude = PanLatitude;
  pointObj* p = ptin;
  const pointObj* ptend = ptin+n;

  while (p<ptend) {
    int Y = Real2Int((mPanLatitude-p->y)/mDrawScale);
    int X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)/mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    ptout++;
    p+= skip;
  }
}

