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

// #define USE_AHRS 1 // still to be tested in v5.0 

#ifndef LKCOMPETITION
//
// Turn Rate Indicator
//
void MapWindow::DrawAcceleration(HDC hDC, const RECT rc)
{
  const double ScaleX = (rc.right - rc.left)/10;
  const double ScaleY = (rc.top - rc.bottom)/10;
  const double ScaleZ = (rc.top - rc.bottom)/20;
  POINT Pos;
  Pos.x = (rc.right - rc.left)/2 + (int)(DrawInfo.AccelY * ScaleX);
  Pos.y = (rc.bottom - rc.top)/2 - (int)((DrawInfo.AccelZ - 1) * ScaleY);
  const double radius = NIBLSCALE(15) + (int)(DrawInfo.AccelX * ScaleZ);
  
  const HPEN    oldPen   = (HPEN) SelectObject(hDC, GetStockObject(BLACK_PEN));
  const HBRUSH  oldBrush = (HBRUSH)SelectObject(hDC, LKBrush_Red);
  Circle(hDC, Pos.x, Pos.y - (int)(radius/2), (int)radius, rc, true, true);
  
  SelectObject(hDC, oldBrush);
  SelectObject(hDC, oldPen);
}

void MapWindow::DrawTRI(HDC hDC, const RECT rc)
{
	#if USE_AHRS
	if (DrawInfo.GyroscopeAvailable)
	  return DrawAHRS( hDC,   rc);
        #endif

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
  Start.y = ((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(10);
  Start.x = (rc.right - rc.left)/2;
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
  if(DrawInfo.Speed <5.5 && !DrawInfo.GyroscopeAvailable)
    disabled=true; 

  if (disabled) {
	hpBlack = LKPen_Grey_N1;
	hbBlack = LKBrush_Grey;
  } else {
	hpBlack = LKPen_Black_N1;
	hbBlack = LKBrush_Black;
        beta = DrawInfo.GyroscopeAvailable ? DrawInfo.Roll : DerivedDrawInfo.BankAngle;
  }

  hpWhite = LKPen_White_N1;
  hbWhite = LKBrush_White;
  hpBorder = LKPen_Grey_N2;
  hbBorder = LKBrush_Grey;

  hpOld = (HPEN)SelectObject(hDC, hpWhite);
  hbOld = (HBRUSH)SelectObject(hDC, hbWhite);
  Circle(hDC, Start.x, Start.y, radius, rc, false, true );

  if(DrawInfo.AccelerationAvailable)
    DrawAcceleration(hDC, rc);

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
  const double beta_sine = fastsine(beta);
  const double beta_cosine = fastcosine(beta);
  a1.x = Start.x - (long)(planesize * beta_cosine);
  a1.y = Start.y - (long)(planesize * beta_sine);
  a2.x = Start.x + (long)(planesize * beta_cosine);
  a2.y = Start.y + (long)(planesize * beta_sine);
    if (disabled) 
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
    else
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc); 

  a1.x = Start.x;
  a1.y = Start.y;
  a2.x = Start.x + (long)(tailsize * beta_sine);
  a2.y = Start.y - (long)(tailsize * beta_cosine);
  if (disabled) 
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
  else
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc);

  SelectObject(hDC, LK8TitleFont);
  int bankindy=Start.y-radius/2;

  if (beta > 1)
	_stprintf(Buffer, TEXT("%2.0f%s"), beta, gettext(_T("_@M2179_")));
  else if (beta < -1)
	_stprintf(Buffer, TEXT("%2.0f%s"), -beta, gettext(_T("_@M2179_")));
  else
	_tcscpy(Buffer, TEXT("--"));

  LKWriteText(hDC, Buffer, Start.x , bankindy, 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_BLUE, false);

//  MapDirty = true;
//  if (!disabled) MapWindow::RefreshMap();


  SelectObject(hDC, hbOld);
  SelectObject(hDC, hpOld);
}


#if USE_AHRS
void MapWindow::DrawCompassRose(HDC hDC, const RECT rc, double direction)
{
	POINT Needle[4];
	SIZE sz = {rc.right-rc.left, rc.bottom-rc.top};
	POINT Center = {rc.left + sz.cx/2,rc.top + sz.cy/2};
int cx = ((rc.right-rc.left))/5/10; ///10/NIBLSCALE(1);
int cy = (int)((double)(rc.bottom-rc.top)*0.9/5.0);///NIBLSCALE(1);
/*
    if(hCompassPic != NULL)
    {
      BITMAP bmp;
      GetObject(hCompassPic, sizeof(BITMAP), (LPSTR)&bmp);

      SelectObject(hDCTemp,hCompassPic);
  	  StretchBlt(hDC,    rc.left, rc.top, sz.cx , sz.cy  , hDCTemp,0, 0, bmp.bmWidth, bmp.bmHeight,  SRCCOPY); //SRCPAINT
    }
*/
	Needle[0].x = 0 ;
	Needle[0].y = cy;
	Needle[1].x = -cx;
	Needle[1].y = 0;
	Needle[2].x = cx;
	Needle[2].y = 0;
	Needle[3] = Needle[0];

	PolygonRotateShift(Needle, 4,  Center.x-1, Center.y, (direction));
	const HPEN   oldPen   = (HPEN) SelectObject(hDC, LKPen_Red_N1);
	const HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, LKBrush_Red);
	Polygon(hDC,Needle  ,4 );


	Needle[0].x = 0 ;
	Needle[0].y = -cy;
	Needle[1].x = -cx;
	Needle[1].y = 0;
	Needle[2].x = cx;
	Needle[2].y = 0;
	Needle[3] = Needle[0];

	SelectObject(hDC, LKBrush_Blue);
	SelectObject(hDC, LKPen_Blue_N1);
	PolygonRotateShift(Needle, 4, Center.x, Center.y,  (direction));
	Polygon(hDC,Needle  ,4 );

    SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
}

void MapWindow::DrawAHRS(HDC hDC, const RECT rc)
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
  static int radius = NIBLSCALE(75);
  static int planesize = radius-NIBLSCALE(10);
  // planebody
  //static int planeradius = NIBLSCALE(6);
  //static int tailsize = planesize/4+NIBLSCALE(2);
  static int innerradius = radius - NIBLSCALE(8);
#define GC_NO_CIRCLE_PTS 90
  static POINT d00[2][2],d15[2][4],d30[2][4], d45[2][4], d60[2][4], circle[GC_NO_CIRCLE_PTS];
  TCHAR Buffer[LKSIZEBUFFERVALUE];
  double beta=0.0;
  bool disabled=false;

  if (DoInit[MDI_DRAWTRI])
  {

  top=(((rc.bottom-BottomSize-(rc.top+TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)- (rc.top+TOPLIMITER);
  planesize = radius-NIBLSCALE(10);
  //planeradius = NIBLSCALE(6);
  //tailsize = planesize/4+NIBLSCALE(2);
  innerradius = radius - NIBLSCALE(8);

  double alpha =0;
  int alphastep = 360/GC_NO_CIRCLE_PTS;
  for(int i =0; i < GC_NO_CIRCLE_PTS; i++)
  {
	circle[i].x =  Start.x + (LONG)(fastsine(alpha) * (radius+1));
	circle[i].y =  Start.y - (LONG)(fastcosine(alpha) * (radius+1));
	alpha += alphastep;
  }
  // [a][b]  a=0 external circle a=1 inner circle  b=1-4

  d00[0][0].x= Start.x ;
  d00[0][0].y= Start.y - radius;
  d00[1][0].x= Start.x ;
  d00[1][0].y= Start.y- innerradius;
  d00[0][1].x= Start.x ;
  d00[0][1].y= Start.y+ radius;
  d00[1][1].x= Start.x ;
  d00[1][1].y= Start.y+ innerradius;

  d15[0][0].y= Start.y - (long) (radius*fastcosine(15.0));
  d15[0][0].x= Start.x + (long) (radius*fastsine(15.0));
  d15[1][0].y= Start.y - (long) (innerradius*fastcosine(15.0));
  d15[1][0].x= Start.x + (long) (innerradius*fastsine(15.0));
  d15[0][1].y= Start.y - (long) (radius*fastcosine(15.0));
  d15[0][1].x= Start.x - (long) (radius*fastsine(15.0));
  d15[1][1].y= Start.y - (long) (innerradius*fastcosine(15.0));
  d15[1][1].x= Start.x - (long) (innerradius*fastsine(15.0));
  d15[0][2].y= Start.y + (long) (radius*fastcosine(15.0));
  d15[0][2].x= Start.x + (long) (radius*fastsine(15.0));
  d15[1][2].y= Start.y + (long) (innerradius*fastcosine(15.0));
  d15[1][2].x= Start.x + (long) (innerradius*fastsine(15.0));
  d15[0][3].y= Start.y + (long) (radius*fastcosine(15.0));
  d15[0][3].x= Start.x - (long) (radius*fastsine(15.0));
  d15[1][3].y= Start.y + (long) (innerradius*fastcosine(15.0));
  d15[1][3].x= Start.x - (long) (innerradius*fastsine(15.0));

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
  if(DrawInfo.Speed <5.5 && !DrawInfo.GyroscopeAvailable)
    disabled=true;

  if (disabled) {
	hpBlack = LKPen_Grey_N1;
	hbBlack = LKBrush_Grey;
  } else {
	hpBlack = LKPen_Black_N1;
	hbBlack = LKBrush_Black;
        beta = DrawInfo.GyroscopeAvailable ? DrawInfo.Roll : DerivedDrawInfo.BankAngle;
  }

  double gamma =  -DrawInfo.Pitch;

  beta  = -beta;
  double beta_sine = fastsine(beta);
  double beta_cosine = fastcosine(beta);

  hpWhite = LKPen_White_N1;
  hbWhite =LKBrush_Lake;// LKBrush_White;
  hpBorder = LKPen_Grey_N2;
  hbBorder = LKBrush_Grey;

  hpOld = (HPEN)SelectObject(hDC, hpWhite);
  hbOld = (HBRUSH)SelectObject(hDC, hbWhite);

  Circle(hDC, Start.x, Start.y, radius, rc, false, true );
/***************************************************************************************/

  POINT earth[GC_NO_CIRCLE_PTS+2];
  POINT left, right;
  int alphastep = 360/GC_NO_CIRCLE_PTS;
  right.x  = Start.x + (LONG) ( radius*fastcosine( beta + gamma));
  right.y  = Start.y + (LONG) ( radius*fastsine  ( beta + gamma));
  left.x   = Start.x - (LONG) ( radius*fastcosine( beta - gamma));
  left.y   = Start.y + (LONG) ( radius*fastsine  (-beta + gamma));

  int betacircle =  (int)beta%360;
  if (betacircle < 0)
	betacircle +=360;
  int angleleft   = (int)(betacircle - gamma) + 270;angleleft %= 360;
  int angleright  = (int)(betacircle + gamma) + 90;angleright %= 360;
  int iCnt =0;

  earth[iCnt++] = right;
  int steps = (angleleft - angleright )/alphastep;
  if(steps <0)
   steps += GC_NO_CIRCLE_PTS;
  int alpha = angleright/alphastep+1;
  for(int i=0;  i < steps; i++)
  {
	if(alpha < 0)
	  alpha = GC_NO_CIRCLE_PTS-1;
	if((alpha) > (GC_NO_CIRCLE_PTS-1))
	  alpha = 0;

	if(iCnt < (GC_NO_CIRCLE_PTS))
	  earth[iCnt++] =  circle[alpha];
    alpha++;
  }
  if(iCnt <= GC_NO_CIRCLE_PTS)
	earth[iCnt++] =  left;





  /*********************************************************************************************/
    HPEN   hpHorizonGround = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1),RGB_BLACK);
    HBRUSH hbHorizonGround = (HBRUSH)CreateSolidBrush(RGB( 	255,140,0));
    HPEN   oldpen = (HPEN)  SelectObject(hDC, hpHorizonGround);
    HBRUSH oldbrush = (HBRUSH) SelectObject(hDC, hbHorizonGround);

    SelectObject(hDC, hpHorizonGround);
    SelectObject(hDC, hbHorizonGround);
    Polygon(hDC, earth, iCnt);
    SelectObject(hDC, oldpen);
    SelectObject(hDC, oldbrush);
    DeleteObject(hpHorizonGround);
    DeleteObject(hbHorizonGround);

  /***************************************************************************************/


  if(DrawInfo.AccelerationAvailable)
    DrawAcceleration(hDC, rc);

  SelectObject(hDC, hpBorder);
  SelectObject(hDC, hbBorder);
  Circle(hDC, Start.x, Start.y, radius+NIBLSCALE(2), rc, false, false );


  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d00[0][0], d00[1][0], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d00[0][1], d00[1][1], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][0], d15[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][1], d15[1][1], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][2], d15[1][2], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d15[0][3], d15[1][3], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][0], d30[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][1], d30[1][1], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][2], d30[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d30[0][3], d30[1][3], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][0], d45[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][1], d45[1][1], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][2], d45[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d45[0][3], d45[1][3], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][0], d60[1][0], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][1], d60[1][1], RGB_BLUE,rc);
//  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][2], d60[1][2], RGB_BLUE,rc);
  _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), d60[0][3], d60[1][3], RGB_BLUE,rc);

  POINT a1, a2;



int lenght =0;

for (int i = -30; i <= 30 ; i+=5)
{
  if (abs(i%10) > 0)
    lenght = radius/10;
  else
    lenght = radius/5;
  int offaxis;
  POINT newcenter;
  offaxis = (int)(radius * fastsine(i));
  newcenter.x = (long)(offaxis * beta_sine);
  newcenter.y = (long)(-offaxis * beta_cosine);

  a1.x =  Start.x + (long)(lenght * beta_cosine ) + newcenter.x;
  a1.y =  Start.y + (long)(lenght * beta_sine   ) + newcenter.y;
  a2.x =  Start.x - (long)(lenght * beta_cosine ) + newcenter.x;
  a2.y =  Start.y - (long)(lenght * beta_sine   ) + newcenter.y ;
   _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), a1, a2, RGB_WHITE,rc);
}

a1.x =  Start.x + (long)(radius * beta_sine                 );
a1.y =  Start.y - (long)(radius * beta_cosine               );
a2.x =  Start.x + (long)((radius-NIBLSCALE(8)) * beta_sine  );
a2.y =  Start.y - (long)((radius-NIBLSCALE(8)) * beta_cosine);
 _DrawLine(hDC, PS_SOLID, NIBLSCALE(1), a1, a2, RGB_WHITE,rc);


    SelectObject(hDC, hpBlack);
    SelectObject(hDC, hbBlack);
//    Circle(hDC, Start.x, Start.y , planeradius, rc, false, true );
#define V_COLOR RGB_RED
double vscale = 0.25;
   a1.x = Start.x - (long)(planesize );
   a1.y = Start.y ;
   a2.x = Start.x - (long)(planesize*vscale );
   a2.y = Start.y ;
    if (disabled)
	  _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, RGB_GREY,rc);
    else
	  _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, V_COLOR,rc);

    a1.x = Start.x + (long)(planesize );
    a1.y = Start.y ;
    a2.x = Start.x + (long)(planesize*vscale );
    a2.y = Start.y ;
    if (disabled)
 	  _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, RGB_GREY,rc);
    else
 	  _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, V_COLOR,rc);

     a1.x = Start.x ;
     a1.y = Start.y + (long)(planesize*vscale );
     a2.x = Start.x + (long)(planesize*vscale );
     a2.y = Start.y ;
     if (disabled)
  	   _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, RGB_GREY,rc);
     else
  	   _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, V_COLOR,rc);

     a1.x = Start.x ;
     a1.y = Start.y + (long)(planesize*vscale );
     a2.x = Start.x - (long)(planesize*vscale );
     a2.y = Start.y ;
     if (disabled)
       _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, RGB_GREY,rc);
     else
       _DrawLine(hDC, PS_SOLID, NIBLSCALE(2), a1, a2, V_COLOR,rc);


     Circle(hDC, Start.x, Start.y, NIBLSCALE(2), rc, false, true );
/*
  a1.x = Start.x;
  a1.y = Start.y;
  a2.x = Start.x + (long)(tailsize * 0);
  a2.y = Start.y - (long)(tailsize * 1);
  if (disabled)
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
  else
	_DrawLine(hDC, PS_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc);
*/
  SelectObject(hDC, LK8TitleFont);
  int bankindy=Start.y-radius/2;
  if (beta > 1)
	_stprintf(Buffer, TEXT("%2.0f%s"), beta, gettext(_T("_@M2179_")));
  else if (beta < -1)
	_stprintf(Buffer, TEXT("%2.0f%s"), -beta, gettext(_T("_@M2179_")));
  else
	_tcscpy(Buffer, TEXT("--"));

  LKWriteText(hDC, Buffer, Start.x , bankindy, 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_BLUE, false);

//  MapDirty = true;
//  if (!disabled) MapWindow::RefreshMap();
  RECT rcCompass;
  rcCompass.left = (int)((double)Start.x*1.70);
  rcCompass.top  = (int)((double)Start.y*0.35);
  rcCompass.right = (int)((double)Start.x*1.95);
  rcCompass.bottom = rcCompass.top+  (rcCompass.right - rcCompass.left);

 // if(DrawInfo.MagneticHeadingAvailable)
    DrawCompassRose( hDC, rcCompass,DrawInfo.MagneticHeading);
//  DrawCompass(HDC hdc, const RECT rc);
  SelectObject(hDC, hbOld);
  SelectObject(hDC, hpOld);

}
#else // no AHRS
void MapWindow::DrawCompassRose(HDC hDC, const RECT rc, double direction) {
    return;
}
void MapWindow::DrawAHRS(HDC hDC, const RECT rc) {
    return;
}
#endif // AHRS

#else
	// LK COMPETITION VERSION HAS NO TRI 
void MapWindow::DrawTRI(HDC hDC, const RECT rc) {
	return;
}
#endif
