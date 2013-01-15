/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include "MapWindow.h"
#include "Sideview.h"
#include "LKInterface.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "Multimap.h"
#include "LKObjects.h"


extern double fSplitFact;
extern COLORREF  Sideview_TextColor;

COLORREF  Sideview_TextColor = RGB_WHITE;
using std::min;
using std::max;

int Sideview_asp_heading_task=0;
int Sideview_iNoHandeldSpaces=0;
AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];



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




// draw aircraft
void RenderPlaneSideview(HDC hdc, double fDist, double fAltitude,double brg, DiagrammStruct* psDia )
{
//BOOL bInvCol = true ; //INVERTCOLORS
  #define NO_AP_PTS 17
  int deg = DEG_TO_INT(AngleLimit360(brg));
  double fCos = COSTABLE[deg];
  double fSin = SINETABLE[deg];

  int TAIL   = 6;
  int PROFIL = 1;
  int FINB   = 3;
  int BODY   = 2;
  int NOSE   = 7;
  int WING   = (int) (22.0 );
  int TUBE   = (int) (14.0  ) ;
  int FINH   = 6+BODY;

  POINT Start;
  int HEAD = TUBE / 2;
  TUBE =  3 * TUBE/ 2;
  POINT AircraftSide
  [8] = {
      {(int)(fSin * (HEAD+0   )    ), -BODY-1},  // 1
      {(int)(fSin * (HEAD+NOSE)    ),  0},       // 2
      {(int)(fSin * (HEAD+0   )    ),  BODY+1},  // 3
      {(int)(fSin * (-TUBE)        ),  BODY},    // 4   -1
      {(int)(fSin * -TUBE          ), -FINH},    // 5
      {(int)(fSin * (-TUBE+FINB)   ), -FINH},    // 6
      {(int)(fSin * (-TUBE+FINB+3) ), -BODY+1},  // 7  +1
      {(int)(fSin * (HEAD+0)       ), -BODY-1}     // 8
  };

  #define  FACT 2

  BODY = (int)((double)(BODY+1) * fCos * fCos);

  int DIA = (BODY + PROFIL);

  /* both wings */
  POINT AircraftWing
  [13] = {
      {(int)(fCos * BODY              ) ,  -DIA},    // 1
      {(int)(fCos * (int)( FACT*BODY) ), -PROFIL},    // 2
      {(int)(fCos * WING              ) ,  -PROFIL},    // 3
      {(int)(fCos * WING              ), 0* PROFIL},    // 4
      {(int)(fCos * (int)( FACT*BODY) ) , PROFIL},    // 5
      {(int)(fCos *  BODY             ), DIA},    // 6
      {(int)(fCos * -BODY             ) , DIA},    // 7
      {(int)(fCos * (int)( -FACT*BODY)), PROFIL},    // 8
      {(int)(fCos * -WING             ), 0* PROFIL  },    // 9
      {(int)(fCos * -WING             ) , -PROFIL}  ,    // 10
      {(int)(fCos * (int)( -FACT*BODY)), -PROFIL},    // 11
      {(int)(fCos * -BODY             ) , -DIA},    // 12
      {(int)(fCos *  BODY             ), -DIA}    // 13
  };


  POINT AircraftWingL
  [7] = {

      {(int)(0 * -BODY                ),  DIA       },    // 1
      {(int)(fCos * (int)( -FACT*BODY)),  PROFIL    },    // 2
      {(int)(fCos * -WING             ),  0* PROFIL },    // 3
      {(int)(fCos * -WING             ), -PROFIL    },    // 4
      {(int)(fCos * (int)( -FACT*BODY)), -PROFIL    },    // 5
      {(int)(0 * -BODY                ), -DIA       },    // 6
      {(int)(0 * -BODY                ),  DIA       }     // 7
  };


  POINT AircraftWingR
  [7] = {
      {(int)(0 * BODY                 ) ,  -DIA    },   // 1
      {(int)(fCos * (int)( FACT*BODY) ) , -PROFIL  },   // 2
      {(int)(fCos * WING              ) ,  -PROFIL },   // 3
      {(int)(fCos * WING              ) , 0* PROFIL},   // 4
      {(int)(fCos * (int)( FACT*BODY) ) , PROFIL   },   // 5
      {(int)(0 *  BODY                ) , DIA      },   // 6
      {(int)(0 *  BODY                ) , -DIA     }    // 7
  };



  POINT AircraftTail
  [5] = {
      {(int)(fCos *  TAIL - fSin*TUBE), -FINH},            // 1
      {(int)(fCos *  TAIL - fSin*TUBE), -FINH +PROFIL},    // 2
      {(int)(fCos * -TAIL - fSin*TUBE), -FINH +PROFIL},    // 3
      {(int)(fCos * -TAIL - fSin*TUBE), -FINH },           // 4
      {(int)(fCos *  TAIL - fSin*TUBE), -FINH},            // 5

  };

  Start.x = CalcDistanceCoordinat(fDist,  psDia);
  Start.y = CalcHeightCoordinat(fAltitude, psDia);
  HBRUSH oldBrush;
  HPEN   oldPen;
/*
  if(bInvCol)
  {
    oldPen   = (HPEN)SelectObject(hdc, GetStockObject(WHITE_PEN));
    oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  }
  else
*/
  {
    oldPen   = (HPEN) SelectObject(hdc, GetStockObject(BLACK_PEN));
    oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(WHITE_BRUSH));
  }

  //SelectObject(hdc, GetStockObject(BLACK_PEN));
  PolygonRotateShift(AircraftWing, 13,  Start.x, Start.y,  0);
  PolygonRotateShift(AircraftSide, 8,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftTail, 5,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingL, 7,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingR, 7,   Start.x, Start.y,  0);

  HBRUSH GreenBrush = CreateSolidBrush(COLORREF RGB_GREEN);
  HBRUSH RedBrush = CreateSolidBrush(COLORREF RGB_RED);
  if((brg < 180))
  {
    SelectObject(hdc, RedBrush);
    Polygon(hdc,AircraftWingL ,7 );

    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
    Polygon(hdc,AircraftSide  ,8 );

    SelectObject(hdc, GreenBrush);
    Polygon(hdc,AircraftWingR ,7 );

    SelectObject(hdc, oldBrush);
  }
  else
  {
    SelectObject(hdc, GreenBrush);
    Polygon(hdc,AircraftWingR ,7 );

    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
    Polygon(hdc,AircraftSide  ,8 );

    SelectObject(hdc, RedBrush);
    Polygon(hdc,AircraftWingL ,7 );

    SelectObject(hdc, oldBrush);
   //Polygon(hdc,AircraftWing  ,13);
  }
  if((brg < 90)|| (brg > 270)) {
    Polygon(hdc,AircraftTail  ,5 );
  }

  SelectObject(hdc, oldPen);
  SelectObject(hdc, oldBrush);
  DeleteObject(RedBrush);
  DeleteObject(GreenBrush);

} //else !asp_heading_task


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


