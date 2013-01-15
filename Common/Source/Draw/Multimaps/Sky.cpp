/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"


void RenderCircleSky(HDC hdc, const RECT rc, COLORREF Col1, COLORREF Col2 , int iSteps)
{
RECT rcd;
int i;


double fdy = (double)(rc.top - rc.bottom)/(double)iSteps;
HPEN   hpHorizon;
HBRUSH hbHorizon;
COLORREF Col;
double fTop;
LKASSERT(iSteps!=0);

/* just take something in order to store the old brush and pen for restoring them */
HPEN OldPen     = (HPEN)   SelectObject(hdc, GetStockObject(WHITE_PEN));
HBRUSH OldBrush = (HBRUSH) SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	rcd = rc;

	fTop = (double)rcd.bottom;
	for(i=0 ; i < iSteps ; i++)
	{
	  rcd.bottom  = rcd.top ;
	  fTop += fdy;
	  rcd.top     = (long)fTop;

	  Col = MixColors( Col2, Col1,  (double) i / (double) iSteps);

//	  rcd = RectIntersect(rcd,rc);
	  hpHorizon = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1), Col);
	  hbHorizon = (HBRUSH)CreateSolidBrush(Col);
	  SelectObject(hdc, hpHorizon);
	  SelectObject(hdc, hbHorizon);

	//  Rectangle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);
	//  Circle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);
	//  int Circle(HDC hdc, long x, long y, int radius, RECT rc, bool clip, bool fill)
	  SelectObject(hdc, OldPen);
	  SelectObject(hdc, OldBrush);

	  DeleteObject(hpHorizon);
	  DeleteObject(hbHorizon);
   }
}



void RenderSky(HDC hdc, const RECT rci, COLORREF Col1, COLORREF Col2 , int iSteps)
{
	RECT rc = rci;

RECT rcd=rc;
int i;

#if BUGSTOP
LKASSERT(iSteps >=2)
#endif
if(iSteps == 1) iSteps++;
double fdy = (double)(rc.top - rc.bottom)/(double)(iSteps-1);
HPEN   hpHorizon;
HBRUSH hbHorizon;
COLORREF Col;
double fTop;
LKASSERT(iSteps!=0);

/* just take something in order to store the old brush and pen for restoring them */
HPEN OldPen     = (HPEN)   SelectObject(hdc, GetStockObject(WHITE_PEN));
HBRUSH OldBrush = (HBRUSH) SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	rcd = rc;

	fTop = (double)rcd.bottom-fdy;
	for(i=0 ; i < iSteps ; i++)
	{
	  rcd.bottom  = rcd.top ;
	  fTop += fdy;
	  rcd.top     = (long)fTop;

	  Col = MixColors( Col2, Col1,  (double) i / (double) iSteps);

//	  rcd = RectIntersect(rcd,rc);
	  hpHorizon = (HPEN)CreatePen(PS_SOLID, (1), Col);
	  hbHorizon = (HBRUSH)CreateSolidBrush(Col);
	  SelectObject(hdc, hpHorizon);
	  SelectObject(hdc, hbHorizon);

	  Rectangle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);

	  SelectObject(hdc, OldPen);
	  SelectObject(hdc, OldBrush);

	  DeleteObject(hpHorizon);
	  DeleteObject(hbHorizon);
   }
}


