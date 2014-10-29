/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"
#include "LKObjects.h"

void RenderCircleSky(LKSurface& Surface, const RECT& rc, const LKColor& Col1, const LKColor& Col2 , int iSteps)
{
RECT rcd;
int i;


double fdy = (double)(rc.top - rc.bottom)/(double)iSteps;

LKColor Col;
double fTop;
LKASSERT(iSteps!=0);

/* just take something in order to store the old brush and pen for restoring them */
LKPen OldPen = Surface.SelectObject(LK_WHITE_PEN);
LKBrush OldBrush = Surface.SelectObject(LKBrush_Black);
	rcd = rc;

	fTop = (double)rcd.bottom;
	for(i=0 ; i < iSteps ; i++)
	{
	  rcd.bottom  = rcd.top ;
	  fTop += fdy;
	  rcd.top     = (long)fTop;

	  Col = Col2.MixColors(Col1,  (double) i / (double) iSteps);

//	  rcd = RectIntersect(rcd,rc);
	  LKPen hpHorizon(PEN_SOLID, IBLSCALE(1), Col);
	  LKBrush hbHorizon(Col);
	  Surface.SelectObject(hpHorizon);
	  Surface.SelectObject(hbHorizon);

	//  Rectangle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);
	//  Circle(hdc,rcd.left,rcd.top,rcd.right,rcd.bottom);
	//  int Circle(HDC hdc, long x, long y, int radius, RECT rc, bool clip, bool fill)
	  Surface.SelectObject(OldPen);
	  Surface.SelectObject(OldBrush);
   }
}



void RenderSky(LKSurface& Surface, const RECT& rci, const LKColor& Col1, const LKColor& Col2 , int iSteps)
{
	RECT rc = rci;

RECT rcd=rc;
int i;

#if BUGSTOP
LKASSERT(iSteps >=2)
#endif
if(iSteps == 1) iSteps++;
double fdy = (double)(rc.top - rc.bottom)/(double)(iSteps-1);

LKColor Col;
double fTop;
LKASSERT(iSteps!=0);

/* just take something in order to store the old brush and pen for restoring them */
LKPen OldPen = Surface.SelectObject(LK_WHITE_PEN);
LKBrush OldBrush = Surface.SelectObject(LKBrush_Black);
	rcd = rc;

	fTop = (double)rcd.bottom-fdy;
	for(i=0 ; i < iSteps ; i++)
	{
	  rcd.bottom  = rcd.top ;
	  fTop += fdy;
	  rcd.top     = (long)fTop;

	  Col = Col2.MixColors(Col1,  (double) i / (double) iSteps);

//	  rcd = RectIntersect(rcd,rc);
	  LKPen hpHorizon(PEN_SOLID, (1), Col);
	  LKBrush hbHorizon(Col);
	  Surface.SelectObject(hpHorizon);
	  Surface.SelectObject(hbHorizon);

	  Surface.Rectangle(rcd.left,rcd.top,rcd.right,rcd.bottom);

	  Surface.SelectObject(OldPen);
	  Surface.SelectObject(OldBrush);
   }
}


