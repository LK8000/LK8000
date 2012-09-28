/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Sideview.h,v 1.1 2012/01/17 10:35:29 root Exp root $
*/

#ifndef SIDEVIEW_H
#define SIDEVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

#define BORDER_X 0 //24
#define BORDER_Y 19


#define  MSL_SEA_DRAW
#define GC_SEA_LEVEL_TOLERANCE       100

//#define SHOW_YELLO_RED_WARNING
#define NEAR_AS_ZOOM_1000FT
#define NEAR_AS_ZOOM_1000M

#define MAXPAGE 8
#define ZOOMFACTOR 1.3

#define GROUND_COLOUR	RGB(157,101,60)
#define GROUND_TEXT_COLOUR RGB_WHITE
#define INV_GROUND_TEXT_COLOUR   RGB(25, 25, 64)
#if (WINDOWSPC>0)
#define GC_NO_COLOR_STEPS  50
#else
#define GC_NO_COLOR_STEPS  25
#endif
#define SKY_SPACE_COL  RGB(150,150,255)
#define SKY_HORIZON_COL  RGB_WHITE



#define ID_NO_LABLE    0
#define ID_SHORT_LABLE 1
#define ID_FULL_LABLE  2

#define ELV_FACT 2.2
#define MAXALTTODAY   2500.0

#define NO_SIDEVIEW_PAGES 3

  enum {
    STYLE_BLUETHIN,
    STYLE_REDTHICK,
    STYLE_ORANGETHICK,
    STYLE_GREENTHICK,
    STYLE_GREENMEDIUM,
    STYLE_DASHGREEN,
    STYLE_MEDIUMBLACK,
    STYLE_THINDASHPAPER,
    STYLE_WHITETHICK
  };


int SetSplitScreenSize(int);
int IncSideviewPage(void);
int DecSideviewPage(void);
int GetSideviewPage (void);
int SetSideviewPage (int i);

void DrawTelescope (HDC hdc, double fAngle, int x, int y);
void DrawNorthArrow(HDC hdc, double fAngle, int x, int y);
void DrawWindRoseDirection(HDC hdc, double fAngle, int x, int y);
void RenderSky(HDC hdc, const RECT rc, COLORREF Col1, COLORREF Col2 , int iSteps);
void RenderPlaneSideview(HDC hdc, double fDist, double fAltitude,double brg, DiagrammStruct* psDia );
void RenderBearingDiff(HDC hdc, double brg, DiagrammStruct* psDia );
void RenderAirspaceTerrain(HDC hdc, double PosLat, double PosLon,  double brg,  DiagrammStruct* psDiag );
int CalcHeightCoordinat(double fHeight,   DiagrammStruct* psDia);
int CalcHeightCoordinatOutbound(double fHeight,   DiagrammStruct* psDia);
int CalcDistanceCoordinat(double fDist,   DiagrammStruct* psDia);
COLORREF ChangeBrightness(long Color, double fBrightFact);
COLORREF MixColors(COLORREF Color1, COLORREF Color2, double fFact1);
bool PtInRect(int X,int Y, RECT rcd );

#endif
