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

#define BORDER_X 24
#define BORDER_Y 19

typedef struct
{
  double fXMin, fXMax;
  double fYMin, fYMax;
  RECT rc;
} DiagrammStruct;



void DrawTelescope (HDC hdc, double fAngle, int x, int y);
void DrawNorthArrow(HDC hdc, double fAngle, int x, int y);
void DrawWindRoseDirection(HDC hdc, double fAngle, int x, int y);
void RenderSky(HDC hdc, const RECT rc, COLORREF Col1, COLORREF Col2 , int iSteps);
void RenderPlaneSideview(HDC hdc, const RECT rc,double fDist, double fAltitude,double brg, DiagrammStruct* psDia );
void RenderBearingDiff(HDC hdc, const RECT rc,double brg, DiagrammStruct* psDia );
void RenderAirspaceTerrain(HDC hdc, const RECT rc,double PosLat, double PosLon,  double brg,  DiagrammStruct* psDiag );
int CalcHeightCoordinat(double fHeight, const RECT rc,  DiagrammStruct* psDia);
int CalcDistanceCoordinat(double fDist, const RECT rc,  DiagrammStruct* psDia);
COLORREF ChangeBrightness(long Color, double fBrightFact);
COLORREF MixColors(COLORREF Color1, COLORREF Color2, double fFact1);
bool PtInRect(int X,int Y, RECT rcd );

#endif
