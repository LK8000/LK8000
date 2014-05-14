/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"


extern double fSplitFact;
extern COLORREF  Sideview_TextColor;

COLORREF  Sideview_TextColor = RGB_WHITE;
using std::min;
using std::max;

int Sideview_asp_heading_task=0;
int Sideview_iNoHandeldSpaces=0;
AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS] = {0};



void SetMMNorthUp( int iPage, int  iVal)
{
  #if BUGSTOP
  LKASSERT(iPage<NUMBER_OF_SHARED_MULTIMAPS);
  #endif
  if(	iPage < 0 ) iPage=0;
  if(	iPage >= NUMBER_OF_SHARED_MULTIMAPS ) iPage=NUMBER_OF_SHARED_MULTIMAPS-1;
  MMNorthUp_Runtime[iPage]	= iVal;
}


int GetMMNorthUp( int iPage)
{
	if(	iPage < 0 ) iPage=0;
	if(	iPage >= NUMBER_OF_SHARED_MULTIMAPS ) iPage=NUMBER_OF_SHARED_MULTIMAPS-1;
	return MMNorthUp_Runtime[iPage];
}


int SetSplitScreenSize(int iPercent)
{
 LKASSERT(iPercent>=0 && iPercent<=100);
 int iOld = 	(int)(fSplitFact *100.0);
 if (iPercent==100)
	fSplitFact=1;
 else
	fSplitFact = (double)(iPercent%100)/ 100.0f;

 return iOld;
}


int IncSideviewPage(void)
{
	Sideview_asp_heading_task++;
	if (Sideview_asp_heading_task >= NO_SIDEVIEW_PAGES)
	  Sideview_asp_heading_task =0;
	return Sideview_asp_heading_task;
}

int DecSideviewPage(void)
{
	Sideview_asp_heading_task--;
	if (Sideview_asp_heading_task < 0)
	  Sideview_asp_heading_task = NO_SIDEVIEW_PAGES-1;
	return Sideview_asp_heading_task;
}

int GetSideviewPage (void)
{
return Sideview_asp_heading_task;
}
int SetSideviewPage (int i)
{
int oldPage = Sideview_asp_heading_task;
	Sideview_asp_heading_task = i;

return oldPage;
}




void DrawWindRoseDirection(HDC hdc, double fAngle, int x, int y)
{
BOOL bInvCol = true ; //INVERTCOLORS
TCHAR text[80];
SIZE tsize;
#define DEG_RES 45
int iHead = (int)(AngleLimit360(fAngle+DEG_RES/2) /DEG_RES);
iHead *= DEG_RES;


switch (iHead)
{
  case 0   : _stprintf(text,TEXT("N"  )); break;
  case 22  : _stprintf(text,TEXT("NNE")); break;
  case 45  : _stprintf(text,TEXT("NE" )); break;
  case 67  : _stprintf(text,TEXT("ENE")); break;
  case 90  : _stprintf(text,TEXT("E"  )); break;
  case 112 : _stprintf(text,TEXT("ESE")); break;
  case 135 : _stprintf(text,TEXT("SE" )); break;
  case 157 : _stprintf(text,TEXT("SSE")); break;
  case 180 : _stprintf(text,TEXT("S"  )); break;
  case 179 : _stprintf(text,TEXT("SSW")); break;
  case 225 : _stprintf(text,TEXT("SW" )); break;
  case 247 : _stprintf(text,TEXT("WSW")); break;
  case 270 : _stprintf(text,TEXT("W"  )); break;
  case 202 : _stprintf(text,TEXT("WNW")); break;
  case 315 : _stprintf(text,TEXT("NW" )); break;
  case 337 : _stprintf(text,TEXT("NNW")); break;
 default   : _stprintf(text,TEXT("--" )); break;
};

SetBkMode(hdc, TRANSPARENT);
 if(bInvCol)
   SetTextColor(hdc, RGB_BLACK);
 else
   SetTextColor(hdc, RGB_WHITE);

GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
ExtTextOut(hdc,  x-tsize.cx/2,  y-tsize.cy/2 , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

return;
}



void DrawSelectionFrame(HDC hdc, RECT rc)
{
SetBkMode(hdc, TRANSPARENT);
RECT rci = rc;
#define SHRINK 1
	rci.left +=1;
	rci.top -=1;
	rci.right -=2;
	rci.bottom -=2;
	int iSize = NIBLSCALE(2);
	COLORREF col = RGB_BLACK;

  MapWindow::_DrawLine   (hdc, PS_SOLID, iSize, (POINT) {rci.left,rci.top}     ,(POINT) {rci.left,rci.bottom} , col, rci);
  MapWindow::_DrawLine   (hdc, PS_SOLID, iSize, (POINT) {rci.left,rci.bottom}  ,(POINT) {rci.right,rci.bottom}, col, rci);
  MapWindow::_DrawLine   (hdc, PS_SOLID, iSize, (POINT) {rci.right,rci.bottom} ,(POINT) {rci.right,rci.top}   , col, rci);
  MapWindow::_DrawLine   (hdc, PS_SOLID, iSize, (POINT) {rci.right,rci.top}    ,(POINT) {rci.left,rci.top}    , col, rci);

  col = RGB_YELLOW;
  MapWindow::DrawDashLine(hdc,iSize,(POINT) {rci.left,rci.top}    ,(POINT) {rci.left,rci.bottom} ,  col, rci);
  MapWindow::DrawDashLine(hdc,iSize,(POINT) {rci.left,rci.bottom} ,(POINT) {rci.right,rci.bottom},  col, rci);
  MapWindow::DrawDashLine(hdc,iSize,(POINT) {rci.right,rci.bottom},(POINT) {rci.right,rci.top}   ,  col, rci);
  MapWindow::DrawDashLine(hdc,iSize,(POINT) {rci.right,rci.top}   ,(POINT) {rci.left,rci.top}    ,  col, rci);


}




int CalcHeightCoordinatOutbound(double fHeight, DiagrammStruct* psDia)
{
	RECT rc =	psDia->rc;
	  int y0 = rc.bottom ; //-BORDER_Y;

	//  fMaxAltToday = 3300;
	//  double hmin = max(0.0, alt-2300);
	//  double hmax = max(fMaxAltToday, alt+1000);
	  double hmin = psDia->fYMin;
	  double hmax = psDia->fYMax;
	  if (hmax==hmin) hmax++; // RECOVER DIVISION BY ZERO!
	  double gfh = (fHeight-hmin)/(hmax-hmin);
	  int yPos = (int)(gfh*(rc.top-rc.bottom)+y0)-1;
	  return yPos;
}


int CalcHeightCoordinat(double fHeight, DiagrammStruct* psDia)
{
int yPos;
RECT rc =	psDia->rc;
yPos = CalcHeightCoordinatOutbound( fHeight,  psDia);
	  if(yPos < rc.top )
		  yPos  = rc.top;
	  if(yPos > rc.bottom )
		  yPos  = rc.bottom;
	  return yPos;

}


int CalcDistanceCoordinat(double fDist, DiagrammStruct* psDia)
{
RECT rc =	psDia->rc;
if ( psDia->fXMax == psDia->fXMin) psDia->fXMax++; // RECOVER DIVISION BY ZERO!
double xscale =   (double) (rc.right - rc.left)/(psDia->fXMax - psDia->fXMin);
int	xPos = (int)((fDist- psDia->fXMin)*xscale)+rc.left ;
  return xPos;

}


bool PtInRect(int X,int Y, RECT rcd )
{
  if( X  > rcd.left   )
    if( X  < rcd.right  )
      if( Y  < rcd.bottom )
        if( Y  > rcd.top    )
          return true;
  return false;
}


COLORREF ChangeBrightness(long Color, double fBrightFact)
{
int  red    = (int)GetRValue(Color );
int  green  = (int)GetGValue(Color );
int  blue   = (int)GetBValue(Color );
red   = (int)(fBrightFact * (double)red  ); if(red > 255)   red = 255;
blue  = (int)(fBrightFact * (double)blue ); if(blue > 255)   blue = 255;
green = (int)(fBrightFact * (double)green); if(green > 255)  green = 255;

COLORREF  result = RGB((BYTE)red,(BYTE)green,(BYTE) blue);
return(result);

}



COLORREF MixColors(COLORREF Color1, COLORREF Color2, double fFact1)
{

double fFact2 = 1.0f- fFact1;
BYTE  red1    = GetRValue(Color1 );
BYTE  green1  = GetGValue(Color1 );
BYTE  blue1   = GetBValue(Color1 );

BYTE  red2    = GetRValue(Color2 );
BYTE  green2  = GetGValue(Color2 );
BYTE  blue2   = GetBValue(Color2 );

int  red    = (int)(fFact1 * (double)red1   + fFact2 * (double)red2  ); if(red   > 255)  red   = 255;
int  green  = (int)(fFact1 * (double)green1 + fFact2 * (double)green2); if(green > 255)  green = 255;
int  blue   = (int)(fFact1 * (double)blue1  + fFact2 * (double)blue2 ); if(blue  > 255)  blue  = 255;
COLORREF  result = RGB((BYTE)red,(BYTE)green,(BYTE) blue);
return(result);

}




RECT RectIntersect(RECT A, RECT B)
{
  RECT Inter;
  Inter.left    = max( A.left  , B.left);
  Inter.right   = min( A.right , B.right);
  Inter.top     = min( A.top   , B.top);
  Inter.bottom  = max( A.bottom, B.bottom);

return Inter;

}

