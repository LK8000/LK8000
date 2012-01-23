/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"
#include "Sideview.h"


using std::min;
using std::max;


void DrawTelescope(HDC hdc, double fAngle, int x, int y)
{
	POINT Telescope[17] = {
			{  6 ,  7  },    // 1
			{  6 ,  2  },    // 2
			{  8 , -2  },    // 3
			{  8 , -7  },    // 4
			{  1 , -7  },    // 5
			{  1 , -2  },    // 6
			{ -1 , -2  },    // 7
			{ -1 , -7  },    // 8
			{ -8 , -7  },    // 9
			{ -8 , -2  },    // 10
			{ -6 ,  2  },    // 11
			{ -6 ,  7  },    // 12
			{ -1 ,  7  },    // 13
			{ -1 ,  3  },    // 14
			{  1 ,  3  },    // 15
			{  1 ,  7  },    // 16
			{  4 ,  7  }     // 17
	};


// Direction arrow
bool bBlack = true;
DrawWindRoseDirection( hdc, AngleLimit360( fAngle ),  x,  y + NIBLSCALE(18));
PolygonRotateShift(Telescope, 17, x, y, AngleLimit360( fAngle  ));
if (!bBlack)
{
  SelectObject(hdc, GetStockObject(WHITE_PEN));
  SelectObject(hdc, GetStockObject(WHITE_BRUSH));
}
else
{
  SelectObject(hdc, GetStockObject(BLACK_PEN));
  SelectObject(hdc, GetStockObject(BLACK_BRUSH));
}
Polygon(hdc,Telescope,17);

if (!bBlack)
  SelectObject(hdc, GetStockObject(BLACK_PEN));
else
  SelectObject(hdc, GetStockObject(WHITE_PEN));

Polygon(hdc,Telescope,17);
/*

	POINT Reflect1[5] = {
			{  -4 ,  -3 },    // 1
			{  -4 ,  -2 },    // 2
			{  -3 ,  -2 },    // 3
			{  -4 ,  -3 },    // 4
			{  -4 ,  -3 },    // 5
	};

	POINT Reflect2[5] = {
			{  3 ,  -3  },    // 1
			{  3 ,  -2  },    // 2
			{  3 ,  -2  },    // 3
			{  3 ,  -3  },    // 4
			{  3 ,  -3  },    // 5

	};
HPEN Newpen = (HPEN)CreatePen(PS_SOLID, 1, RGB(198,198,198));
HPEN oldPen = (HPEN) SelectObject(hdc,(HPEN) Newpen);
SelectObject(hdc, Newpen);
PolygonRotateShift(Reflect1, 5, Start.x, Start.y, AngleLimit360( fAngle  ));
Polygon(hdc,Reflect1,5);
PolygonRotateShift(Reflect2, 5, Start.x, Start.y, AngleLimit360( fAngle  ));
Polygon(hdc,Reflect2,5);
SelectObject(hdc, oldPen);
*/
}




void DrawNorthArrow(HDC hdc, double fAngle, int x, int y)
{
  // Draw north arrow
  POINT Arrow[5] = { {0,-11}, {-5,9}, {0,3}, {5,9}, {0,-11}};
  DrawWindRoseDirection( hdc, AngleLimit360( fAngle ),  x,  y + NIBLSCALE(18));
  PolygonRotateShift(Arrow, 5, x, y, AngleLimit360( -fAngle));
  SelectObject(hdc, GetStockObject(WHITE_PEN));
  if(INVERTCOLORS)
   SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  else
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
  Polygon(hdc,Arrow,5);

  if(INVERTCOLORS)
	SelectObject(hdc, GetStockObject(WHITE_PEN));
  else
	SelectObject(hdc, GetStockObject(BLACK_PEN));


  Polygon(hdc,Arrow,5);

}

void DrawWindRoseDirection(HDC hdc, double fAngle, int x, int y)
{
TCHAR text[80];
SIZE tsize;
#define DEG_RES 45
int iHead = (int)(AngleLimit360(fAngle+DEG_RES/2) /DEG_RES);
iHead *= DEG_RES;
return ; // don't do it

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
GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
ExtTextOut(hdc,  x-tsize.cx/2,  y-tsize.cy/2 , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
/*
_stprintf(text,TEXT("%i"),iHead);
GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
ExtTextOut(hdc,  x-tsize.cx/2,  y+ NIBLSCALE(25) , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
*/
return;
}




void RenderBearingDiff(HDC hdc, const RECT rc,double brg, DiagrammStruct* psDia )
{
  // Print Bearing difference
  TCHAR BufferValue[LKSIZEBUFFERVALUE];
  TCHAR BufferUnit[LKSIZEBUFFERUNIT];
  TCHAR BufferTitle[LKSIZEBUFFERTITLE];

  bool ret = false;
  // Borrowed from LKDrawLook8000.cpp
  switch (OvertargetMode) {
    case OVT_TASK:
      // Do not use FormatBrgDiff for TASK, could be AAT!
      ret = MapWindow::LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
      break;
    case OVT_ALT1:
      MapWindow::LKFormatBrgDiff(Alternate1, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_ALT2:
      MapWindow::LKFormatBrgDiff(Alternate2, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_BALT:
      MapWindow::LKFormatBrgDiff(BestAlternate, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_THER:
      MapWindow::LKFormatBrgDiff(RESWP_LASTTHERMAL, true, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_HOME:
      MapWindow::LKFormatBrgDiff(HomeWaypoint, false, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_MATE:
      MapWindow::LKFormatBrgDiff(RESWP_TEAMMATE, true, BufferValue, BufferUnit);
      ret = true;
      break;
    case OVT_FLARM:
      MapWindow::LKFormatBrgDiff(RESWP_FLARMTARGET, true, BufferValue, BufferUnit);
      ret = true;
      break;
    default:
      ret = MapWindow::LKFormatValue(LK_BRGDIFF, false, BufferValue, BufferUnit, BufferTitle);
      break;
  }


  if (ret) {
    SIZE tsize;
    SelectObject(hdc, LK8MediumFont);
    GetTextExtentPoint(hdc, BufferValue, _tcslen(BufferValue), &tsize);
    SetBkMode(hdc, OPAQUE);
    SetBkMode(hdc, TRANSPARENT);
//    SetTextColor(hdc, RGB_TEXT_COLOR);
    ExtTextOut(hdc, (rc.left + rc.right - tsize.cx)/2, rc.top, ETO_OPAQUE, NULL, BufferValue, _tcslen(BufferValue), NULL);
    SetBkMode(hdc, TRANSPARENT);
  }
}


// draw aircraft
void RenderPlaneSideview(HDC hdc, const RECT rc,double fDist, double fAltitude,double brg, DiagrammStruct* psDia )
{


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

  Start.x = CalcDistanceCoordinat(fDist,  rc, psDia);
  Start.y = CalcHeightCoordinat(fAltitude,  rc, psDia);
/*
  if(INVERTCOLORS)
  {
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    SelectObject(hdc, GetStockObject(BLACK_BRUSH));
  }
  else
*/
  {
    SelectObject(hdc, GetStockObject(BLACK_PEN));
    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
  }

  //SelectObject(hdc, GetStockObject(BLACK_PEN));
  PolygonRotateShift(AircraftWing, 13,  Start.x, Start.y,  0);
  PolygonRotateShift(AircraftSide, 8,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftTail, 5,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingL, 7,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingR, 7,   Start.x, Start.y,  0);
  HBRUSH oldBrush;
  HBRUSH GreenBrush = CreateSolidBrush(COLORREF RGB_GREEN);
  HBRUSH RedBrush = CreateSolidBrush(COLORREF RGB_RED);
  if((brg < 180))
  {
    oldBrush= (HBRUSH)  SelectObject(hdc, RedBrush);
    Polygon(hdc,AircraftWingL ,7 );

    SelectObject(hdc, oldBrush);
    Polygon(hdc,AircraftSide  ,8 );

    oldBrush= (HBRUSH)  SelectObject(hdc, GreenBrush);
    Polygon(hdc,AircraftWingR ,7 );

    SelectObject(hdc, oldBrush);
  }
  else
  {
    oldBrush= (HBRUSH)  SelectObject(hdc, GreenBrush);
    Polygon(hdc,AircraftWingR ,7 );

    SelectObject(hdc, oldBrush);
    Polygon(hdc,AircraftSide  ,8 );

    oldBrush= (HBRUSH)  SelectObject(hdc, RedBrush);
    Polygon(hdc,AircraftWingL ,7 );

    SelectObject(hdc, oldBrush);
   //Polygon(hdc,AircraftWing  ,13);
  }
  if((brg < 90)|| (brg > 270)) {
    Polygon(hdc,AircraftTail  ,5 );
  }

  DeleteObject(RedBrush);
  DeleteObject(GreenBrush);

} //else !asp_heading_task




int CalcHeightCoordinat(double fHeight, const RECT rc, DiagrammStruct* psDia)
{
  int y0 = rc.bottom-BORDER_Y;

//  fMaxAltToday = 3300;
//  double hmin = max(0.0, alt-2300);
//  double hmax = max(fMaxAltToday, alt+1000);
  double hmin = psDia->fYMin;
  double hmax = psDia->fYMax;
  double gfh = (fHeight-hmin)/(hmax-hmin);
  int yPos = (int)(gfh*(rc.top-rc.bottom+BORDER_Y)+y0)-1;

  return yPos;
//  fHeigh
}

int CalcDistanceCoordinat(double fDist, const RECT rc,  DiagrammStruct* psDia)
{
double xscale =   (double) (rc.right - rc.left-BORDER_X)/(psDia->fXMax - psDia->fXMin);
int	xPos = (int)((fDist- psDia->fXMin)*xscale)+rc.left +BORDER_X;
  return xPos;

}


bool PtInRect(int X,int Y, RECT rcd )
{
  if( X  > rcd.left   )
    if( X  < rcd.right  )
      if( Y  > rcd.bottom )
        if( Y  < rcd.top    )
          return true;
  return false;
}

COLORREF ChangeBrightness(long Color, double fBrightFact)
{
BYTE  red    = GetRValue(Color );
BYTE  green  = GetGValue(Color );
BYTE  blue   = GetBValue(Color );
red   = (BYTE)(fBrightFact * (double)red  );
blue  = (BYTE)(fBrightFact * (double)blue );
green = (BYTE)(fBrightFact * (double)green);
COLORREF  result = RGB(red,green, blue);
return(result);

//return(RGB(red,green,blue));
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

BYTE  red    = (int)(fFact1 * (double)red1   + fFact2 * (double)red2  ) ;
BYTE  green  = (int)(fFact1 * (double)green1 + fFact2 * (double)green2) ;
BYTE  blue   = (int)(fFact1 * (double)blue1  + fFact2 * (double)blue2 ) ;
COLORREF  result = RGB(red,green, blue);
return(result);

//return(RGB(red,green,blue));
}




void RenderSky(HDC hdc, const RECT rc, COLORREF Col1, COLORREF Col2 , int iSteps)
{


RECT rcd;
int i;

double idy = (double)(rc.top - rc.bottom)/(double)iSteps+0.5f;
HPEN   hpHorizon;
HBRUSH hbHorizon;
COLORREF Col;

/* just take something in order to store the old brush and pen for restoring them */
HPEN OldPen     = (HPEN)   SelectObject(hdc, GetStockObject(WHITE_PEN));
HBRUSH OldBrush = (HBRUSH) SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	rcd = rc;
	rcd.top = rcd.bottom;
	for(i=0 ; i < iSteps ; i++)
	{
	  rcd.bottom  = rcd.top ;
	  rcd.top     = (long)((double)rcd.bottom + ( idy));

	  Col = MixColors( Col2, Col1,  (double) i / (double) iSteps);

	  hpHorizon = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1), Col);
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
