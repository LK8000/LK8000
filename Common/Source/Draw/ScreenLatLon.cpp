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
int MapWindow::LatLon2Screen(const pointObj *ptin, int nIn, POINT *ptout, int nOut, int skip) {
  static double lastangle = -1;
  static int cost=1024, sint=0;
  const double mDisplayAngle = DisplayAngle;

  assert(nOut >= ((nIn/skip)+((nIn%skip)?1:0))); // Out buffer to small !
  
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
  const pointObj* p = ptin;
  const pointObj* ptend = ptin+nIn;

  POINT* ptout_start = ptout;
  POINT* const ptout_end = ptout+nOut;
  
  while (p < (ptend-1) && ptout<ptout_end) {
    long long Y = Real2Int((mPanLatitude-p->y)*mDrawScale);
    long long X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)*mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    ptout++;

    p+= skip;
  }
  if(p >= (ptend-1) && ptout<ptout_end) {
    p = (ptend-1);
    long long Y = Real2Int((mPanLatitude-p->y)*mDrawScale);
    long long X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)*mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    ptout++;
  }

  return std::distance(ptout_start, ptout);
}


//
// Since many functions call LatLon2screen, and only a few need multimap checks for display angle,
// we separate functions to avoid slow downs. First used by CalculateScreenPositionsGroundline
//
int MapWindow::LatLon2ScreenMultimap(const pointObj *ptin, int nIn, POINT *ptout, int nOut, int skip) {
  static double lastangle = -1;
  static int cost=1024, sint=0;
  
  assert(nOut >= ((nIn/skip)+((nIn%skip)?1:0))); // Out buffer to small !
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
  const pointObj* p = ptin;
  const pointObj* const ptend = ptin+nIn;

  POINT* ptout_start = ptout;
  POINT* const ptout_end = ptout+nOut;
  
  while (p < (ptend-1) && ptout<ptout_end) {
    int Y = Real2Int((mPanLatitude-p->y)/mDrawScale);
    int X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)/mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    ptout++;
    p+= skip;
  }
  if(p >= (ptend-1) && ptout<ptout_end) {
    p = (ptend-1);
    int Y = Real2Int((mPanLatitude-p->y)/mDrawScale);
    int X = Real2Int((mPanLongitude-p->x)*fastcosine(p->y)/mDrawScale);
    ptout->x = (xxs-X*cost + Y*sint)/1024;
    ptout->y = (Y*cost + X*sint + yys)/1024;
    ptout++;
  }
  return std::distance(ptout_start, ptout);  
}

