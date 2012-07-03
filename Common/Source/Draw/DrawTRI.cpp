/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"


#ifndef LKCOMPETITION
//
// Turn Rate Indicator
//
void MapWindow::DrawAcceleration(HDC hDC, const RECT rc)
{
	double ScaleX= (rc.right - rc.left)/2/5;
	double ScaleY= (rc.top - rc.bottom)/2/5;
	double ScaleZ= (rc.top - rc.bottom)/2/20;
	POINT Pos;
	Pos.x = (rc.right - rc.left)/2 +(int)( GPS_INFO.AccelY * ScaleX);
	Pos.y = (rc.bottom   - rc.top)/2+ (int)(GPS_INFO.AccelZ * ScaleY);

	HPEN    oldPen   = (HPEN) SelectObject(hDC, GetStockObject(BLACK_PEN));
	HBRUSH  oldBrush = (HBRUSH)SelectObject(hDC, LKBrush_Red);

	  Circle( hDC, Pos.x, Pos.y-10, 20+ (int)(ScaleZ*GPS_INFO.AccelX),  rc,true, true);

	SelectObject(hDC, oldBrush);
	SelectObject(hDC, oldPen);
}

void MapWindow::DrawTRI(HDC hDC, const RECT rc)
{
  POINT Start;
  
  static short top=(((rc.bottom-BottomSize-(rc.top+TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)- (rc.top+TOPLIMITER);

  Start.y = ((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(10);
  Start.x = (rc.right - rc.left)/2;

  HPEN		hpBlack;
  HBRUSH	hbBlack;
  HPEN		hpWhite;
  HBRUSH	hbWhite;
  HPEN		hpBorder;
  HBRUSH	hbBorder;
  HPEN   hpOld;
  HBRUSH hbOld;

  // gauge size radius
  static int radius = NIBLSCALE(65);
  static int planesize = radius-NIBLSCALE(10);
  // planebody
  static int planeradius = NIBLSCALE(6);
  static int tailsize = planesize/4+NIBLSCALE(2);
  static int innerradius = radius - NIBLSCALE(8);
  static POINT d00[2][2],d15[2][4],d30[2][4], d45[2][4], d60[2][4];
  TCHAR Buffer[LKSIZEBUFFERVALUE];
  double beta=0.0;
  bool disabled=false;

  if (DoInit[MDI_DRAWTRI]) {

  top=(((rc.bottom-BottomSize-(rc.top+TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)- (rc.top+TOPLIMITER);
  radius = NIBLSCALE(65);
  planesize = radius-NIBLSCALE(10);
  planeradius = NIBLSCALE(6);
  tailsize = planesize/4+NIBLSCALE(2);
  innerradius = radius - NIBLSCALE(8);

  // [a][b]  a=0 external circle a=1 inner circle  b=1-4

  d00[0][0].x= Start.x - radius;
  d00[0][0].y= Start.y;
  d00[1][0].x= Start.x - innerradius;
  d00[1][0].y= Start.y;
  d00[0][1].x= Start.x + radius;
  d00[0][1].y= Start.y;
  d00[1][1].x= Start.x + innerradius;
  d00[1][1].y= Start.y;

  d15[0][0].x= Start.x - (long) (radius*fastcosine(15.0));
  d15[0][0].y= Start.y + (long) (radius*fastsine(15.0));
  d15[1][0].x= Start.x - (long) (innerradius*fastcosine(15.0));
  d15[1][0].y= Start.y + (long) (innerradius*fastsine(15.0));
  d15[0][1].x= Start.x - (long) (radius*fastcosine(15.0));
  d15[0][1].y= Start.y - (long) (radius*fastsine(15.0));
  d15[1][1].x= Start.x - (long) (innerradius*fastcosine(15.0));
  d15[1][1].y= Start.y - (long) (innerradius*fastsine(15.0));
  d15[0][2].x= Start.x + (long) (radius*fastcosine(15.0));
  d15[0][2].y= Start.y + (long) (radius*fastsine(15.0));
  d15[1][2].x= Start.x + (long) (innerradius*fastcosine(15.0));
  d15[1][2].y= Start.y + (long) (innerradius*fastsine(15.0));
  d15[0][3].x= Start.x + (long) (radius*fastcosine(15.0));
  d15[0][3].y= Start.y - (long) (radius*fastsine(15.0));
  d15[1][3].x= Start.x + (long) (innerradius*fastcosine(15.0));
  d15[1][3].y= Start.y - (long) (innerradius*fastsine(15.0));

  d30[0][0].x= Start.x - (long) (radius*fastcosine(30.0));
  d30[0][0].y= Start.y + (long) (radius*fastsine(30.0));
  d30[1][0].x= Start.x - (long) (innerradius*fastcosine(30.0));
  d30[1][0].y= Start.y + (long) (innerradius*fastsine(30.0));
  d30[0][1].x= Start.x - (long) (radius*fastcosine(30.0));
  d30[0][1].y= Start.y - (long) (radius*fastsine(30.0));
  d30[1][1].x= Start.x - (long) (innerradius*fastcosine(30.0));
  d30[1][1].y= Start.y - (long) (innerradius*fastsine(30.0));
  d30[0][2].x= Start.x + (long) (radius*fastcosine(30.0));
  d30[0][2].y= Start.y + (long) (radius*fastsine(30.0));
  d30[1][2].x= Start.x + (long) (innerradius*fastcosine(30.0));
  d30[1][2].y= Start.y + (long) (innerradius*fastsine(30.0));
  d30[0][3].x= Start.x + (long) (radius*fastcosine(30.0));
  d30[0][3].y= Start.y - (long) (radius*fastsine(30.0));
  d30[1][3].x= Start.x + (long) (innerradius*fastcosine(30.0));
  d30[1][3].y= Start.y - (long) (innerradius*fastsine(30.0));

  d45[0][0].x= Start.x - (long) (radius*fastcosine(45.0));
  d45[0][0].y= Start.y + (long) (radius*fastsine(45.0));
  d45[1][0].x= Start.x - (long) (innerradius*fastcosine(45.0));
  d45[1][0].y= Start.y + (long) (innerradius*fastsine(45.0));
  d45[0][1].x= Start.x - (long) (radius*fastcosine(45.0));
  d45[0][1].y= Start.y - (long) (radius*fastsine(45.0));
  d45[1][1].x= Start.x - (long) (innerradius*fastcosine(45.0));
  d45[1][1].y= Start.y - (long) (innerradius*fastsine(45.0));
  d45[0][2].x= Start.x + (long) (radius*fastcosine(45.0));
  d45[0][2].y= Start.y + (long) (radius*fastsine(45.0));
  d45[1][2].x= Start.x + (long) (innerradius*fastcosine(45.0));
  d45[1][2].y= Start.y + (long) (innerradius*fastsine(45.0));
  d45[0][3].x= Start.x + (long) (radius*fastcosine(45.0));
  d45[0][3].y= Start.y - (long) (radius*fastsine(45.0));
  d45[1][3].x= Start.x + (long) (innerradius*fastcosine(45.0));
  d45[1][3].y= Start.y - (long) (innerradius*fastsine(45.0));

  d60[0][0].x= Start.x - (long) (radius*fastcosine(60.0));
  d60[0][0].y= Start.y + (long) (radius*fastsine(60.0));
  d60[1][0].x= Start.x - (long) (innerradius*fastcosine(60.0));
  d60[1][0].y= Start.y + (long) (innerradius*fastsine(60.0));
  d60[0][1].x= Start.x - (long) (radius*fastcosine(60.0));
  d60[0][1].y= Start.y - (long) (radius*fastsine(60.0));
  d60[1][1].x= Start.x - (long) (innerradius*fastcosine(60.0));
  d60[1][1].y= Start.y - (long) (innerradius*fastsine(60.0));
  d60[0][2].x= Start.x + (long) (radius*fastcosine(60.0));
  d60[0][2].y= Start.y + (long) (radius*fastsine(60.0));
  d60[1][2].x= Start.x + (long) (innerradius*fastcosine(60.0));
  d60[1][2].y= Start.y + (long) (innerradius*fastsine(60.0));
  d60[0][3].x= Start.x + (long) (radius*fastcosine(60.0));
  d60[0][3].y= Start.y - (long) (radius*fastsine(60.0));
  d60[1][3].x= Start.x + (long) (innerradius*fastcosine(60.0));
  d60[1][3].y= Start.y - (long) (innerradius*fastsine(60.0));

  DoInit[MDI_DRAWTRI]=false;
  } // end dirty hack doinit

  //if (!CALCULATED_INFO.Flying) {
  // speed is in m/s
  if (GPS_INFO.Speed <5.5) disabled=true; 

  if (disabled) {
	hpBlack = LKPen_Grey_N1;
	hbBlack = LKBrush_Grey;
  } else {
	hpBlack = LKPen_Black_N1;
	hbBlack = LKBrush_Black;
  	beta = DerivedDrawInfo.BankAngle;
  }

  if(GPS_INFO.GyroscopeAvailable){
	  beta = GPS_INFO.Roll;
  }

  hpWhite = LKPen_White_N1;
  hbWhite = LKBrush_White;
  hpBorder = LKPen_Grey_N2;
  hbBorder = LKBrush_Grey;

  hpOld = (HPEN)SelectObject(hDC, hpWhite);
  hbOld = (HBRUSH)SelectObject(hDC, hbWhite);
  Circle(hDC, Start.x, Start.y, radius, rc, false, true );
  SelectObject(hDC, hpBorder);
  SelectObject(hDC, hbBorder);
  Circle(hDC, Start.x, Start.y, radius+NIBLSCALE(2), rc, false, false );

  SelectObject(hDC, hpBlack);
  SelectObject(hDC, hbBlack); 
  Circle(hDC, Start.x, Start.y, planeradius, rc, false, true );

  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d00[0][0], d00[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d00[0][1], d00[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][0], d15[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][1], d15[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][2], d15[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][3], d15[1][3], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][0], d30[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][1], d30[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][2], d30[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][3], d30[1][3], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][0], d45[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][1], d45[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][2], d45[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][3], d45[1][3], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][0], d60[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][1], d60[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][2], d60[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][3], d60[1][3], RGB_BLUE,rc);

  POINT a1, a2;

  a1.x = Start.x - (long) ( planesize * fastcosine(beta));
  a1.y = Start.y - (long) ( planesize * fastsine(beta));
  a2.x = Start.x + (long) ( planesize * fastcosine(beta));
  a2.y = Start.y + (long) ( planesize * fastsine(beta));
    if (disabled) 
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
    else
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc); 

  a1.x = Start.x;
  a1.y = Start.y;
  a2.x = Start.x + (long) ( tailsize * fastsine(beta));
  a2.y = Start.y - (long) ( tailsize * fastcosine(beta));
  if (disabled) 
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
  else
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc);

  SelectObject(hDC, LK8TitleFont);
  int bankindy=Start.y-radius/2;
#ifndef __MINGW32__
  if (beta > 1)
	_stprintf(Buffer, TEXT("%2.0f\xB0"), beta);
  else if (beta < -1)
	_stprintf(Buffer, TEXT("%2.0f\xB0"), -beta);
  else
	_tcscpy(Buffer, TEXT("--"));
#else
  if (beta > 1)
	_stprintf(Buffer, TEXT("%2.0f°"), beta);
  else if (beta < -1)
	_stprintf(Buffer, TEXT("%2.0f°"), -beta);
  else
	_tcscpy(Buffer, TEXT("--"));
#endif

  LKWriteText(hDC, Buffer, Start.x , bankindy, 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_BLUE, false);

//  MapDirty = true;
//  if (!disabled) MapWindow::RefreshMap();


  SelectObject(hDC, hbOld);
  SelectObject(hDC, hpOld);
  if(GPS_INFO.AccelerationAvailable)
  {
    DrawAcceleration( hDC,   rc);
    MapWindow::RequestFastRefresh();
  }
}

#else
	// LK COMPETITION VERSION HAS NO TRI 
void MapWindow::DrawTRI(HDC hDC, const RECT rc) {
	return;
}
#endif
