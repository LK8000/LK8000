/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "Screen/PenReference.h"
#include "Screen/BrushReference.h"
#include "Asset.hpp"
#include "utils/printf.h"


#ifndef LKCOMPETITION
//
// Turn Rate Indicator
//
void MapWindow::DrawAcceleration(LKSurface& Surface, const RECT& rc) {
  if (!DrawInfo.Acceleration.available()) {
    return;
  }

  const double ScaleX = (rc.right - rc.left) / 10;
  const double ScaleY = (rc.top - rc.bottom) / 10;
  const double ScaleZ = (rc.top - rc.bottom) / 20;
  POINT Pos = {static_cast<decltype(POINT::x)>(
                   std::round((rc.right - rc.left) / 2 +
                              (DerivedDrawInfo.Acceleration.y * ScaleX))),
               static_cast<decltype(POINT::x)>(std::round(
                   (rc.bottom - rc.top) / 2 -
                   ((DerivedDrawInfo.Acceleration.z - 1) * ScaleY)))};
  const double radius =
      NIBLSCALE(15) + (DerivedDrawInfo.Acceleration.x * ScaleZ);

  const auto oldPen = Surface.SelectObject(LK_BLACK_PEN);
  const auto oldBrush = Surface.SelectObject(LKBrush_Red);

  Surface.DrawCircle(Pos.x, Pos.y - static_cast<PixelScalar>(radius/2), static_cast<PixelScalar>(radius), rc, true);

  Surface.SelectObject(oldBrush);
  Surface.SelectObject(oldPen);
}

void MapWindow::DrawTRI(LKSurface& Surface, const RECT& rc) {
#ifdef USE_AHRS
    if (DrawInfo.Gyroscope.available()) {
        DrawAHRS(Surface, rc);
        return;
    }
#endif

  POINT Start;

  static short top=(((rc.bottom-BottomSize-(rc.top+TOPLIMITER)-BOTTOMLIMITER)/PANELROWS)+rc.top+TOPLIMITER)- (rc.top+TOPLIMITER);

  Start.y = ((rc.bottom-BottomSize-top)/2)+top-NIBLSCALE(10);
  Start.x = (rc.right - rc.left)/2;

  PenReference hpBlack;
  BrushReference hbBlack;

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

   if (DoInit[MDI_DRAWTRI])
  {

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
  if (DrawInfo.Speed < 5.5 && !DrawInfo.Gyroscope.available()) {
    disabled = true;
  }

  if (disabled) {
    hpBlack = LKPen_Grey_N1;
    hbBlack = LKBrush_Grey;
  }
  else {
    hpBlack = LKPen_Black_N1;
    hbBlack = LKBrush_Black;
    beta = DrawInfo.Gyroscope.available() ? DrawInfo.Gyroscope.value().Roll
                                          : DerivedDrawInfo.BankAngle;
  }

  const BrushReference hbWhite = LKBrush_White;
  const BrushReference hbBorder = LKBrush_Grey;

  const auto hpOld = Surface.SelectObject(LKPen_White_N1);
  const auto hbOld = Surface.SelectObject(hbWhite);
  Surface.DrawCircle(Start.x, Start.y, radius, true );

  DrawAcceleration(Surface, rc);

  Surface.SelectObject(LKPen_Grey_N2);
  Surface.SelectObject(hbBorder);
  Surface.DrawCircle(Start.x, Start.y, radius+NIBLSCALE(2), false );

  Surface.SelectObject(hpBlack);
  Surface.SelectObject(hbBlack);
  Surface.DrawCircle(Start.x, Start.y, planeradius, true );

  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d00[0][0], d00[1][0], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d00[0][1], d00[1][1], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d15[0][0], d15[1][0], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d15[0][1], d15[1][1], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d15[0][2], d15[1][2], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d15[0][3], d15[1][3], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d30[0][0], d30[1][0], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d30[0][1], d30[1][1], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d30[0][2], d30[1][2], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d30[0][3], d30[1][3], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d45[0][0], d45[1][0], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d45[0][1], d45[1][1], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d45[0][2], d45[1][2], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d45[0][3], d45[1][3], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d60[0][0], d60[1][0], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d60[0][1], d60[1][1], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d60[0][2], d60[1][2], RGB_BLUE,rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d60[0][3], d60[1][3], RGB_BLUE,rc);

  POINT a1, a2;
  const double beta_sine = fastsine(beta);
  const double beta_cosine = fastcosine(beta);
  a1.x = Start.x - (long)(planesize * beta_cosine);
  a1.y = Start.y - (long)(planesize * beta_sine);
  a2.x = Start.x + (long)(planesize * beta_cosine);
  a2.y = Start.y + (long)(planesize * beta_sine);
    if (disabled)
	Surface.DrawLine(PEN_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
    else
	Surface.DrawLine(PEN_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc);

  a1.x = Start.x;
  a1.y = Start.y;
  a2.x = Start.x + (long)(tailsize * beta_sine);
  a2.y = Start.y - (long)(tailsize * beta_cosine);
  if (disabled)
	Surface.DrawLine(PEN_SOLID, NIBLSCALE(4), a1, a2, RGB_GREY,rc);
  else
	Surface.DrawLine(PEN_SOLID, NIBLSCALE(4), a1, a2, RGB_BLACK,rc);

  Surface.SelectObject(LK8TitleFont);
  int bankindy=Start.y-radius/2;

  if (beta > 1)
	_stprintf(Buffer, TEXT("%2.0f%s"), beta, MsgToken<2179>());
  else if (beta < -1)
	_stprintf(Buffer, TEXT("%2.0f%s"), -beta, MsgToken<2179>());
  else
	lk::strcpy(Buffer, TEXT("--"));

  LKWriteText(Surface, Buffer, Start.x , bankindy, WTMODE_NORMAL, WTALIGN_CENTER, RGB_BLUE, false);

//  MapDirty = true;
//  if (!disabled) MapWindow::RefreshMap();


  Surface.SelectObject(hbOld);
  Surface.SelectObject(hpOld);
}


#ifdef USE_AHRS
void MapWindow::DrawCompassRose(LKSurface& Surface, const RECT& rc, double direction)
{
	POINT Needle[4];
	SIZE sz = {rc.right-rc.left, rc.bottom-rc.top};
	POINT Center = {rc.left + sz.cx/2,rc.top + sz.cy/2};
    int cx = ((rc.right-rc.left))/5/10; ///10/NIBLSCALE(1);
    int cy = (int)((double)(rc.bottom-rc.top)*0.9/5.0);///NIBLSCALE(1);

	Needle[0].x = 0 ;
	Needle[0].y = cy;
	Needle[1].x = -cx;
	Needle[1].y = 0;
	Needle[2].x = cx;
	Needle[2].y = 0;
	Needle[3] = Needle[0];

	PolygonRotateShift(Needle, 4,  Center.x-1, Center.y, (direction));
	const auto oldPen = Surface.SelectObject(LKPen_Red_N1);
	const auto oldBrush = Surface.SelectObject(LKBrush_Red);
	Surface.Polygon(Needle  ,4 );


	Needle[0].x = 0 ;
	Needle[0].y = -cy;
	Needle[1].x = -cx;
	Needle[1].y = 0;
	Needle[2].x = cx;
	Needle[2].y = 0;
	Needle[3] = Needle[0];

	Surface.SelectObject(LKBrush_Blue);
	Surface.SelectObject(LKPen_Blue_N1);
	PolygonRotateShift(Needle, 4, Center.x, Center.y,  (direction));
	Surface.Polygon(Needle  ,4 );

    Surface.SelectObject(oldPen);
    Surface.SelectObject(oldBrush);
}

void MapWindow::DrawAHRS(LKSurface& Surface, const RECT& rc) {

  PixelScalar top =
      (((rc.bottom - BottomSize - (rc.top + TOPLIMITER) - BOTTOMLIMITER) /
        PANELROWS) +
       rc.top + TOPLIMITER) -
      (rc.top + TOPLIMITER);

  RasterPoint Start = {
      (rc.right - rc.left) / 2,
      ((rc.bottom - BottomSize - top) / 2) + top - NIBLSCALE(10)
  };

  // gauge size radius
  static int radius = NIBLSCALE(75);
  static int planesize = radius-NIBLSCALE(10);
  // planebody
  //static int planeradius = NIBLSCALE(6);
  //static int tailsize = planesize/4+NIBLSCALE(2);
  static int innerradius = radius - NIBLSCALE(8);
#define GC_NO_CIRCLE_PTS 90
  static RasterPoint d00[2][2], d15[2][4], d30[2][4], d45[2][4], d60[2][4];
  static RasterPoint circle[GC_NO_CIRCLE_PTS];

  TCHAR Buffer[LKSIZEBUFFERVALUE];

  if (DoInit[MDI_DRAWAHRS]) {
    innerradius = radius - NIBLSCALE(8);

    double alpha = 0;
    int alphastep = 360 / GC_NO_CIRCLE_PTS;
    for (int i = 0; i < GC_NO_CIRCLE_PTS; i++) {
      circle[i].x = Start.x + (fastsine(alpha) * (radius + 1));
      circle[i].y = Start.y - (fastcosine(alpha) * (radius + 1));
      alpha += alphastep;
    }
    // [a][b]  a=0 external circle a=1 inner circle  b=1-4

    d00[0][0].x = Start.x;
    d00[0][0].y = Start.y - radius;
    d00[1][0].x = Start.x;
    d00[1][0].y = Start.y - innerradius;
    d00[0][1].x = Start.x;
    d00[0][1].y = Start.y + radius;
    d00[1][1].x = Start.x;
    d00[1][1].y = Start.y + innerradius;

    d15[0][0].y = Start.y - radius * fastcosine(15.0);
    d15[0][0].x = Start.x + radius * fastsine(15.0);
    d15[1][0].y = Start.y - innerradius * fastcosine(15.0);
    d15[1][0].x = Start.x + innerradius * fastsine(15.0);
    d15[0][1].y = Start.y - radius * fastcosine(15.0);
    d15[0][1].x = Start.x - radius * fastsine(15.0);
    d15[1][1].y = Start.y - innerradius * fastcosine(15.0);
    d15[1][1].x = Start.x - innerradius * fastsine(15.0);
    d15[0][2].y = Start.y + radius * fastcosine(15.0);
    d15[0][2].x = Start.x + radius * fastsine(15.0);
    d15[1][2].y = Start.y + innerradius * fastcosine(15.0);
    d15[1][2].x = Start.x + innerradius * fastsine(15.0);
    d15[0][3].y = Start.y + radius * fastcosine(15.0);
    d15[0][3].x = Start.x - radius * fastsine(15.0);
    d15[1][3].y = Start.y + innerradius * fastcosine(15.0);
    d15[1][3].x = Start.x - innerradius * fastsine(15.0);

    d30[0][0].x = Start.x - radius * fastcosine(30.0);
    d30[0][0].y = Start.y + radius * fastsine(30.0);
    d30[1][0].x = Start.x - innerradius * fastcosine(30.0);
    d30[1][0].y = Start.y + innerradius * fastsine(30.0);
    d30[0][1].x = Start.x - radius * fastcosine(30.0);
    d30[0][1].y = Start.y - radius * fastsine(30.0);
    d30[1][1].x = Start.x - innerradius * fastcosine(30.0);
    d30[1][1].y = Start.y - innerradius * fastsine(30.0);
    d30[0][2].x = Start.x + radius * fastcosine(30.0);
    d30[0][2].y = Start.y + radius * fastsine(30.0);
    d30[1][2].x = Start.x + innerradius * fastcosine(30.0);
    d30[1][2].y = Start.y + innerradius * fastsine(30.0);
    d30[0][3].x = Start.x + radius * fastcosine(30.0);
    d30[0][3].y = Start.y - radius * fastsine(30.0);
    d30[1][3].x = Start.x + innerradius * fastcosine(30.0);
    d30[1][3].y = Start.y - innerradius * fastsine(30.0);

    d45[0][0].x = Start.x - radius * fastcosine(45.0);
    d45[0][0].y = Start.y + radius * fastsine(45.0);
    d45[1][0].x = Start.x - innerradius * fastcosine(45.0);
    d45[1][0].y = Start.y + innerradius * fastsine(45.0);
    d45[0][1].x = Start.x - radius * fastcosine(45.0);
    d45[0][1].y = Start.y - radius * fastsine(45.0);
    d45[1][1].x = Start.x - innerradius * fastcosine(45.0);
    d45[1][1].y = Start.y - innerradius * fastsine(45.0);
    d45[0][2].x = Start.x + radius * fastcosine(45.0);
    d45[0][2].y = Start.y + radius * fastsine(45.0);
    d45[1][2].x = Start.x + innerradius * fastcosine(45.0);
    d45[1][2].y = Start.y + innerradius * fastsine(45.0);
    d45[0][3].x = Start.x + radius * fastcosine(45.0);
    d45[0][3].y = Start.y - radius * fastsine(45.0);
    d45[1][3].x = Start.x + innerradius * fastcosine(45.0);
    d45[1][3].y = Start.y - innerradius * fastsine(45.0);

    d60[0][0].x = Start.x - radius * fastcosine(60.0);
    d60[0][0].y = Start.y + radius * fastsine(60.0);
    d60[1][0].x = Start.x - innerradius * fastcosine(60.0);
    d60[1][0].y = Start.y + innerradius * fastsine(60.0);
    d60[0][1].x = Start.x - radius * fastcosine(60.0);
    d60[0][1].y = Start.y - radius * fastsine(60.0);
    d60[1][1].x = Start.x - innerradius * fastcosine(60.0);
    d60[1][1].y = Start.y - innerradius * fastsine(60.0);
    d60[0][2].x = Start.x + radius * fastcosine(60.0);
    d60[0][2].y = Start.y + radius * fastsine(60.0);
    d60[1][2].x = Start.x + innerradius * fastcosine(60.0);
    d60[1][2].y = Start.y + innerradius * fastsine(60.0);
    d60[0][3].x = Start.x + radius * fastcosine(60.0);
    d60[0][3].y = Start.y - radius * fastsine(60.0);
    d60[1][3].x = Start.x + innerradius * fastcosine(60.0);
    d60[1][3].y = Start.y - innerradius * fastsine(60.0);

    DoInit[MDI_DRAWAHRS] = false;
  }  // end dirty hack doinit

  const PenReference hpBlack = LKPen_Black_N1;
  const BrushReference hbBlack = LKBrush_Black;
  const BrushReference hbWhite = LKBrush_Lake;  // LKBrush_White;
  const BrushReference hbBorder = LKBrush_Grey;

  double beta = -DrawInfo.Gyroscope.value().Roll;
  double gamma = DrawInfo.Gyroscope.value().Pitch;

  const auto hpOld = Surface.SelectObject(LKPen_White_N1);
  const auto hbOld = Surface.SelectObject(hbWhite);

  Surface.DrawCircle(Start.x, Start.y, radius, true);
  /***************************************************************************************/

  double beta_sine = fastsine(beta);
  double beta_cosine = fastcosine(beta);

  RasterPoint right = Start + RasterPoint{
    static_cast<PixelScalar>(radius * fastcosine(beta + gamma)),
    static_cast<PixelScalar>(radius * fastsine(beta + gamma))
  };
  RasterPoint left = Start - RasterPoint{
    static_cast<PixelScalar>(radius * fastcosine(beta - gamma)),
    static_cast<PixelScalar>(radius * fastsine(beta - gamma))
  };

  int alphastep = 360 / GC_NO_CIRCLE_PTS;

  int start_idx = (90 + beta + gamma) / alphastep;
  int end_idx = (270 + beta - gamma) / alphastep;
  while (start_idx < 0 ) {
      start_idx += GC_NO_CIRCLE_PTS;
      end_idx += GC_NO_CIRCLE_PTS;
  }

  std::vector<POINT> earth_poly;
  earth_poly.reserve(GC_NO_CIRCLE_PTS + 3);

  earth_poly.push_back(right);

  for (int idx = start_idx; idx < end_idx; ++idx) {
    earth_poly.push_back(circle[idx % GC_NO_CIRCLE_PTS]);
  }

  earth_poly.push_back(left);
  earth_poly.push_back(earth_poly.front());  // close polygon

  /*********************************************************************************************/
  LKPen hpHorizonGround(PEN_SOLID, IBLSCALE(1), RGB_BLACK);
  LKBrush hbHorizonGround = LKBrush(IsDithered() ? LKColor(125, 20, 0) : LKColor(255, 140, 0));

  const auto oldpen = Surface.SelectObject(hpHorizonGround);
  const auto oldbrush = Surface.SelectObject(hbHorizonGround);

  Surface.SelectObject(hpHorizonGround);
  Surface.SelectObject(hbHorizonGround);
  Surface.Polygon(earth_poly.data(), earth_poly.size());
  Surface.SelectObject(oldpen);
  Surface.SelectObject(oldbrush);

  /***************************************************************************************/
  DrawAcceleration(Surface, rc);

  Surface.SelectObject(LKPen_Grey_N2);
  Surface.SelectObject(hbBorder);
  Surface.DrawCircle(Start.x, Start.y, radius + NIBLSCALE(2), false);

  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d00[0][0], d00[1][0], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d15[0][0], d15[1][0], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d15[0][1], d15[1][1], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d30[0][1], d30[1][1], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d30[0][3], d30[1][3], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d45[0][1], d45[1][1], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d45[0][3], d45[1][3], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d60[0][1], d60[1][1], RGB_BLUE, rc);
  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), d60[0][3], d60[1][3], RGB_BLUE, rc);

  PixelScalar lenght = 0;

  for (int i = -30; i <= 30; i += 5) {
    if (abs(i % 10) > 0) {
      lenght = radius / 10;
    }
    else {
      lenght = radius / 5;
    }
    PixelScalar offaxis = radius * fastsine(i);
    RasterPoint newcenter = Start + RasterPoint{
        static_cast<PixelScalar>(offaxis * beta_sine),
        static_cast<PixelScalar>(-offaxis * beta_cosine)
    };
    RasterPoint a = {
        static_cast<PixelScalar>(lenght * beta_cosine),
        static_cast<PixelScalar>(lenght * beta_sine)
    };
    Surface.DrawLine<RasterPoint>(PEN_SOLID, NIBLSCALE(1), newcenter + a, newcenter - a, RGB_WHITE, rc);
  }

  RasterPoint a1 = Start + RasterPoint{
    static_cast<PixelScalar>(radius * beta_sine),
    static_cast<PixelScalar>(-radius * beta_cosine)
  };
  RasterPoint a2 = Start + RasterPoint{
    static_cast<PixelScalar>((radius - NIBLSCALE<PixelScalar>(8)) * beta_sine),
    static_cast<PixelScalar>((-radius - NIBLSCALE<PixelScalar>(8)) * beta_cosine)
  };

  Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), a1, a2, RGB_WHITE, rc);

  Surface.SelectObject(hpBlack);
  Surface.SelectObject(hbBlack);

  PixelScalar vsize = planesize * 0.25;

  const RasterPoint lines[][2] = {
    {{Start.x - planesize, Start.y}, {Start.x - vsize, Start.y}},
    {{Start.x + planesize, Start.y}, {Start.x + vsize, Start.y}},
    {{Start.x, Start.y + vsize}, {Start.x + vsize, Start.y}},
    {{Start.x, Start.y + vsize}, {Start.x - vsize, Start.y}}
  };

  for (auto& line : lines) {
    Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), line[0], line[1], RGB_RED, rc);
  }

  Surface.DrawCircle(Start.x, Start.y, NIBLSCALE(2), true);
  Surface.SelectObject(LK8TitleFont);

  int bankindy = Start.y - radius / 2;
  double abs_beta = std::abs(beta);
  if (abs_beta >= 1) {
    lk::snprintf(Buffer, TEXT("%2.0f%s"), beta, MsgToken<2179>());
  }
  else {
    lk::strcpy(Buffer, TEXT("--"));
  }

  LKWriteText(Surface, Buffer, Start.x, bankindy, WTMODE_NORMAL, WTALIGN_CENTER,
              RGB_BLUE, false);

  if (DrawInfo.MagneticHeading.available()) {
    RECT rcCompass;
    rcCompass.left = Start.x * 1.70;
    rcCompass.top = Start.y * 0.35;
    rcCompass.right = Start.x * 1.95;
    rcCompass.bottom = rcCompass.top + (rcCompass.right - rcCompass.left);

    DrawCompassRose(Surface, rcCompass, DrawInfo.MagneticHeading.value());
  }

  Surface.SelectObject(hbOld);
  Surface.SelectObject(hpOld);
}
#else // no AHRS
void MapWindow::DrawCompassRose(LKSurface& Surface, const RECT& rc, double direction) {

}
void MapWindow::DrawAHRS(LKSurface& Surface, const RECT& rc) {

}
#endif // AHRS

#else
	// LK COMPETITION VERSION HAS NO TRI
void MapWindow::DrawTRI(LKSurface& Surface, const RECT& rc) {

}
#endif
