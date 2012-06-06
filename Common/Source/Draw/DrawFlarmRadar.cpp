/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "RasterTerrain.h"
#include "LKAirspace.h"
#include "RGB.h"
#include "Sideview.h"
#include "NavFunctions.h"
#include "LKObjects.h"
#include "DoInits.h"
#include "MapWindow.h"
#include "LKMapWindow.h"
#include "FlarmIdFile.h"
#include "FlarmRadar.h"



int RADAR_TURN = 90 ;            /* radar plane orientation             */
#define HEIGHT_RANGE (800.0  )    /* max hight ifference above and below */
double ASYMETRIC_FACTOR = 0.7 ;     /* X center displacement               */
double SPLITSCREEN_FACTOR = 0.7 ;   /* % of top view window                */
#define MIN_DIST_SCALE  0.1       /* minimum radar distance              */
#define MAX_DIST_SCALE 25.0       /* maximum radar distance              */


int DrawFlarmObjectTrace(HDC hDC, DiagrammStruct* Dia, int iFlarmIdx);
//#define COLORLIST

using std::min;
using std::max;

static int nEntrys=0;
typedef struct
{
	double fLat;
	double fLon;
	double fAlt;
//	double fIntegrator;
	int iColorIdx;
} LastPositions;


#define NO_TRACE_PTS 150
typedef struct
{
	double fx;
	double fy;
	double fAlt;
	double fFlarmBearing;
	int iColorIdx;
	bool bBuffFull;
	int iLastPtr;
	TCHAR szGliderType[20];
	LastPositions asRingBuf[NO_TRACE_PTS];
} sFlarmPositions;
static sFlarmPositions asFLRAMPos[FLARM_MAX_TRAFFIC];


HBRUSH * variobrush[NO_VARIO_COLORS] = {
		  &LKBrush_Vario_neg4,
		  &LKBrush_Vario_neg3,
		  &LKBrush_Vario_neg2,
		  &LKBrush_Vario_neg1,
		  &LKBrush_Vario_0   ,
		  &LKBrush_Vario_pos1,
		  &LKBrush_Vario_pos2,
		  &LKBrush_Vario_pos3,
		  &LKBrush_Vario_pos4
};

COLORREF  variocol[NO_VARIO_COLORS] = {
		 ChangeBrightness(RGB_BLUE, 0.4),
		 ChangeBrightness(RGB_BLUE, 0.6),
		 ChangeBrightness(RGB_BLUE, 0.8),
		 ChangeBrightness(RGB_BLUE, 1.0),
		 ChangeBrightness(RGB_YELLOW, 0.8),
		 ChangeBrightness(RGB_GREEN, 0.6),
		 ChangeBrightness(RGB_GREEN, 0.7),
		 ChangeBrightness(RGB_GREEN, 0.8),
		 ChangeBrightness(RGB_GREEN, 1.0)
};
void FormatTicText(TCHAR *text, const double val, const double step) {
  if (step<1.0) {
    _stprintf(text, TEXT("%.1f"), val);
  } else {
    _stprintf(text, TEXT("%.0f"), val);
  }
}

typedef enum{
	DrawNoText,
	DrawInside,
	DrawOutside,
	DrawCenter
} TextAlign;

void MapWindow::DrawXGrid(HDC hdc, RECT rc, double ticstep,double unit_step, double zero, int iTextAling,
                           COLORREF color, DiagrammStruct *psDia) {

  POINT line[2];
  SIZE tsize;

  int xoff, yoff;
  double xval;
  if (psDia->fXMax == psDia->fXMin)
    psDia->fXMax++;
  double xscale = (rc.right-rc.left) / (psDia->fXMax-psDia->fXMin);
  int xmin, ymin, xmax, ymax;
  double x_max = psDia->fXMax;
  double x_min = psDia->fXMin;

  for (xval=zero; xval<= x_max; xval+= ticstep) {

    xmin = (int)((xval-x_min)*xscale)+rc.left;
    ymin = rc.top;
    xmax = xmin;
    ymax = rc.bottom;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    DrawDashLine(hdc,1, line[0], line[1], color, rc);


	if (iTextAling>TEXT_NO_TEXT)
	{
	   TCHAR unit_text[MAX_PATH];
	   FormatTicText(unit_text, xval*unit_step/ticstep, unit_step);
	   GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	   switch(iTextAling)
	   {
	     case TEXT_ABOVE_LEFT    : xoff = -tsize.cx  ; yoff= -tsize.cy  ; break;
	     case TEXT_ABOVE_RIGHT   : xoff = 1          ; yoff= -tsize.cy  ; break;
	     case TEXT_ABOVE_CENTER  : xoff = -tsize.cx/2; yoff= -tsize.cy  ; break;
	     case TEXT_UNDER_LEFT    : xoff = -tsize.cx  ; yoff= 0          ; break;
	     case TEXT_UNDER_RIGHT   : xoff = 1          ; yoff= 0          ; break;
	     default:
	     case TEXT_UNDER_CENTER  : xoff = -tsize.cx/2; yoff= 0          ; break;
	     case TEXT_MIDDLE_LEFT   : xoff = -tsize.cx  ; yoff= -tsize.cy/2; break;
	     case TEXT_MIDDLE_RIGHT  : xoff = 1          ; yoff= -tsize.cy/2; break;
	     case TEXT_MIDDLE_CENTER : xoff = -tsize.cx/2; yoff= -tsize.cy/2; break;
	   }

	   ExtTextOut(hdc, xmin+xoff, ymax+yoff,
	   ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);

	}

  }

  for (xval=zero; xval>= x_min; xval-= ticstep) {

    xmin = (int)((xval-x_min)*xscale)+rc.left;
    ymin = rc.top;
    xmax = xmin;
    ymax = rc.bottom;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;


    DrawDashLine(hdc,1, line[0], line[1],  color, rc);


    if (iTextAling>TEXT_NO_TEXT)
    {
       TCHAR unit_text[MAX_PATH];
       FormatTicText(unit_text, xval*unit_step/ticstep, unit_step);
       GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	   switch(iTextAling)
	   {
	     case TEXT_ABOVE_LEFT    : xoff = -tsize.cx   ; yoff= -tsize.cy  ; break;
	     case TEXT_ABOVE_RIGHT   : xoff = 1           ; yoff= -tsize.cy  ; break;
	     case TEXT_ABOVE_CENTER  : xoff = -tsize.cx/2 ; yoff= -tsize.cy  ; break;
	     case TEXT_UNDER_LEFT    : xoff = -tsize.cx-1 ; yoff= 0          ; break;
	     case TEXT_UNDER_RIGHT   : xoff = 1           ; yoff= 0          ; break;
	     default:
	     case TEXT_UNDER_CENTER  : xoff = -tsize.cx/2 ; yoff= 0          ; break;
	     case TEXT_MIDDLE_LEFT   : xoff = -tsize.cx-1 ; yoff= -tsize.cy/2; break;
	     case TEXT_MIDDLE_RIGHT  : xoff = 1           ; yoff= -tsize.cy/2; break;
	     case TEXT_MIDDLE_CENTER : xoff = -tsize.cx/2 ; yoff= -tsize.cy/2; break;
	   }

       ExtTextOut(hdc, xmin+xoff, ymax+yoff,
  	   ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);
    }



}
}

void MapWindow::DrawYGrid(HDC hdc, RECT rc, double ticstep,double unit_step, double zero, int iTextAling,
		 COLORREF color, DiagrammStruct *psDia) {

  POINT line[2];
  SIZE tsize;
 int  xoff =0  , yoff=0;

  double yval;
  double y_max = psDia->fYMax;
  double y_min = psDia->fYMin;

  int xmin, ymin, xmax, ymax;
  if (psDia->fYMax == psDia->fYMin)
    psDia->fYMax++;
  double yscale = (rc.bottom - rc.top) / (psDia->fYMax-psDia->fYMin);

  for (yval=zero; yval<= y_max; yval+= ticstep) {

    xmin = rc.left;
    ymin = (int)((y_max-yval)*yscale)+rc.top;
    xmax = rc.right;
    ymax = ymin;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;


    DrawDashLine(hdc,1, line[0], line[1], color, rc);

    if (iTextAling != TEXT_NO_TEXT)
    {
	  TCHAR unit_text[MAX_PATH];
	  FormatTicText(unit_text, yval*unit_step/ticstep, unit_step);
	  GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	  switch(iTextAling)
	  {
	    case TEXT_ABOVE_LEFT    : xoff = -tsize.cx  ; yoff= -tsize.cy-2  ; break;
	    case TEXT_ABOVE_RIGHT   : xoff = 1          ; yoff= -tsize.cy-2  ; break;
	    case TEXT_ABOVE_CENTER  : xoff = -tsize.cx/2; yoff= -tsize.cy-2  ; break;
	    case TEXT_UNDER_LEFT    : xoff = -tsize.cx  ; yoff= 0          ; break;
	    case TEXT_UNDER_RIGHT   : xoff = 1          ; yoff= 0          ; break;
	    default:
	    case TEXT_UNDER_CENTER  : xoff = -tsize.cx/2; yoff= 0          ; break;
	    case TEXT_MIDDLE_LEFT   : xoff = -tsize.cx  ; yoff= -tsize.cy/2-1; break;
	    case TEXT_MIDDLE_RIGHT  : xoff = 1          ; yoff= -tsize.cy/2-1; break;
	    case TEXT_MIDDLE_CENTER : xoff = -tsize.cx/2; yoff= -tsize.cy/2-1; break;
	  }
	  ExtTextOut(hdc, xmin+xoff, ymin+yoff,
	  ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);
    }
  }

  for (yval=zero; yval>= y_min; yval-= ticstep)
  {
    xmin = rc.left;
    ymin = (int)((y_max-yval)*yscale)+rc.top;
    xmax = rc.right;
    ymax = ymin;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    DrawDashLine((HDC)hdc,(int)1,(POINT) line[0],(POINT) line[1], color, rc);

    if (iTextAling != TEXT_NO_TEXT)
    {
	  TCHAR unit_text[MAX_PATH];
	  FormatTicText(unit_text, yval*unit_step/ticstep, unit_step);
	  GetTextExtentPoint(hdc, unit_text, _tcslen(unit_text), &tsize);
	  switch(iTextAling)
	  {
	    case TEXT_ABOVE_LEFT    : xoff = -tsize.cx  ; yoff= -tsize.cy  ; break;
	    case TEXT_ABOVE_RIGHT   : xoff = 0          ; yoff= -tsize.cy  ; break;
	    case TEXT_ABOVE_CENTER  : xoff = -tsize.cx/2; yoff= -tsize.cy  ; break;
	    case TEXT_UNDER_LEFT    : xoff = -tsize.cx  ; yoff= 0          ; break;
	    case TEXT_UNDER_RIGHT   : xoff = 0          ; yoff= 0          ; break;
	    default:
	    case TEXT_UNDER_CENTER  : xoff = -tsize.cx/2; yoff= 0          ; break;
	    case TEXT_MIDDLE_LEFT   : xoff = -tsize.cx  ; yoff= -tsize.cy/2; break;
	    case TEXT_MIDDLE_RIGHT  : xoff = 0          ; yoff= -tsize.cy/2; break;
	    case TEXT_MIDDLE_CENTER : xoff = -tsize.cx/2; yoff= -tsize.cy/2; break;
	  }
	  ExtTextOut(hdc, xmin+xoff, ymin+yoff,
		         ETO_OPAQUE, NULL, unit_text, _tcslen(unit_text), NULL);
    }
  }
}


int MapWindow::HeightToY(double fHeight, const RECT rc, DiagrammStruct* psDia)
{
  int y0 = rc.bottom;

//  fMaxAltToday = 3300;
//  double hmin = max(0.0, alt-2300);
//  double hmax = max(fMaxAltToday, alt+1000);
  double hmin = psDia->fYMin;
  double hmax = psDia->fYMax;
  if (hmax==hmin) hmax++; // RECOVER DIVISION BY ZERO!
  double gfh = (fHeight-hmin)/(hmax-hmin);
  int yPos = (int)(gfh*(rc.top-rc.bottom)+y0)-1;
  if(yPos < rc.top) yPos = rc.top-1;
  if(yPos > rc.bottom) yPos = rc.bottom+1;
  return yPos;
//  fHeigh
}

int MapWindow::DistanceToX(double fDist, const RECT rc,  DiagrammStruct* psDia)
{

if ( psDia->fXMax == psDia->fXMin) psDia->fXMax++; // RECOVER DIVISION BY ZERO!
double xscale =   (double) (rc.right - rc.left)/(psDia->fXMax - psDia->fXMin);
int	xPos = (int)((fDist- psDia->fXMin)*xscale)+rc.left ;
  return xPos;

}


extern COLORREF Sideview_TextColor;



// draw Flarm aircraft
void RenderFlarmPlaneSideview(HDC hdc, const RECT rc,double fDist, double fAltitude,double brg, DiagrammStruct* psDia ,double fScale )
{


  #define NO_AP_PTS 17
  int deg = DEG_TO_INT(AngleLimit360(brg));
  double fCos = COSTABLE[deg];
  double fSin = SINETABLE[deg];

  int TAIL   = (int)(6.0  * fScale) ;
  int PROFIL = 1;
  int FINB   = (int)(3.0  * fScale);
  int BODY   = (int)(2.0  * fScale);
  int NOSE   = (int)(7.0  * fScale);
  int WING   = (int)(22.0 * fScale );
  int TUBE   = (int)(14.0 * fScale ) ;
  int FINH   = (int)((6+BODY) * fScale);

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
      {(int)(fSin * (HEAD+0)       ), -BODY-1}   // 8
  };

  #define  FACT 2

  BODY = (int)((double)(BODY+1) * fCos * fCos);

  int DIA = (BODY + PROFIL);

  /* both wings */
  POINT AircraftWing
  [13] = {
      {(int)(fCos * BODY              ) ,  -DIA},     // 1
      {(int)(fCos * (int)( FACT*BODY) ), -PROFIL},    // 2
      {(int)(fCos * WING              ) ,  -PROFIL},  // 3
      {(int)(fCos * WING              ), 0* PROFIL},  // 4
      {(int)(fCos * (int)( FACT*BODY) ) , PROFIL},    // 5
      {(int)(fCos *  BODY             ), DIA},        // 6
      {(int)(fCos * -BODY             ) , DIA},       // 7
      {(int)(fCos * (int)( -FACT*BODY)), PROFIL},     // 8
      {(int)(fCos * -WING             ), 0* PROFIL  },// 9
      {(int)(fCos * -WING             ) , -PROFIL}  , // 10
      {(int)(fCos * (int)( -FACT*BODY)), -PROFIL},    // 11
      {(int)(fCos * -BODY             ) , -DIA},      // 12
      {(int)(fCos *  BODY             ), -DIA}        // 13
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
 // int MapWindow::DistanceToX(double fDist, const RECT rc,  DiagrammStruct* psDia)
  Start.x =  MapWindow::DistanceToX(fDist,  rc, psDia);
  Start.y =  MapWindow::HeightToY(fAltitude,  rc, psDia);



  //SelectObject(hdc, GetStockObject(BLACK_PEN));
  PolygonRotateShift(AircraftWing, 13,  Start.x, Start.y,  0);
  PolygonRotateShift(AircraftSide, 8,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftTail, 5,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingL, 7,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingR, 7,   Start.x, Start.y,  0);



  if((brg < 180))
  {
    Polygon(hdc,AircraftWingL ,7 );
    Polygon(hdc,AircraftSide  ,8 );
    Polygon(hdc,AircraftWingR ,7 );
  }
  else
  {
    Polygon(hdc,AircraftWingR ,7 );
    Polygon(hdc,AircraftSide  ,8 );
    Polygon(hdc,AircraftWingL ,7 );
    Polygon(hdc,AircraftWing  ,13);
  }
  if((brg < 90)|| (brg > 270)) {
    Polygon(hdc,AircraftTail  ,5 );
  }





} //else !asp_heading_task

void ResetTraces(void)
{
int i;
  for(i=0; i < FLARM_MAX_TRAFFIC; i++)
  {
	asFLRAMPos[i].iLastPtr = 0;
	asFLRAMPos[i].bBuffFull = false;
  }
}

void MapWindow::LKDrawFlarmRadar(HDC hdc, const RECT rci)
{
RECT rc  = rci; /* rectangle for sideview */
RECT rct = rc;  /* rectangle for topview */
rct.bottom = (long)((rc.bottom-rc.top  )*SPLITSCREEN_FACTOR); /* 2/3 for topview */
rc.top     = rct.bottom;
int i,j;
static double fScaleFact = 5.0;
static int iCircleSize    = 9;
static int iRectangleSize = 5;
static short scaler[5];
static short tscaler=0;
static short column0;
static POINT Arrow[5];
HPEN   hOrangePen ;
HPEN   hGreenPen ;
HPEN   hWhitePen ;
HPEN   hOldPen;
HBRUSH hOldBrush;
HPEN   hDrawPen ;
HBRUSH hDrawBrush;
bool bSideview = true;

HFONT hfOldFont = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
COLORREF rgbGridColor = RGB_DARKGREEN;
COLORREF rgbDrawColor = RGB_GREEN;
COLORREF rgb_targetlinecol = RGB_RED;
double fPlaneSize = 1.0;
double fOwnTopPlaneSize = 1.2;
double fTopViewPlaneSize = 1.2;
/*********************************************************************************
 * change colors on inversion
 *********************************************************************************/


if(INVERTCOLORS)
{
  rgbDrawColor = RGB_GREY;
  rgbGridColor = RGB_GREY;
  rgb_targetlinecol = RGB_LIGHTBLUE;
  hDrawPen   = (HPEN)  GetStockObject( WHITE_PEN );
  hDrawBrush = (HBRUSH)GetStockObject( WHITE_BRUSH) ;
  hOrangePen = (HPEN)CreatePen(PS_SOLID, 2,RGB_ORANGE);
  hGreenPen  = (HPEN)CreatePen(PS_SOLID, 2,RGB_GREEN);
  hWhitePen  = (HPEN)CreatePen(PS_SOLID, 1,RGB_WHITE);

}
else
{
  rgbDrawColor = RGB_DARKGREY;
  rgbGridColor = RGB_DARKGREY;
  rgb_targetlinecol = RGB_BLUE;
  hDrawPen   = (HPEN)  GetStockObject( BLACK_PEN );
  hDrawBrush = (HBRUSH)GetStockObject( BLACK_BRUSH) ;
  hOrangePen = (HPEN)CreatePen(PS_SOLID, 2,RGB_LIGHTORANGE);
  hGreenPen  = (HPEN)CreatePen(PS_SOLID, 2,RGB_DARKGREEN);
  hWhitePen  = (HPEN)CreatePen(PS_SOLID, 1,RGB_BLACK);

}

SetTextColor(hdc, rgbDrawColor);
hOldPen   = (HPEN)SelectObject(hdc, hDrawPen);
hOldBrush = (HBRUSH)SelectObject(hdc, hDrawBrush);



#define NUMAIRCRAFTPTS 16
POINT AircraftTop[NUMAIRCRAFTPTS] = {
  { 1,-6},
  {2,-1},
  {15,0},
  {15,2},
  {2,2},
  {1,10},
  {4,11},
  {4,12},
  {-4,12},
  {-4,11},
  {-1,10},
  {-2,2},
  {-15,2},
  {-15,0},
  {-2,-1},
  {-1,-6}
};


for(i=0; i < NUMAIRCRAFTPTS; i++)
{
  AircraftTop[i].x =  (long)( AircraftTop[i].x * fOwnTopPlaneSize);
  AircraftTop[i].y =  (long)( AircraftTop[i].y * fOwnTopPlaneSize);
}


static short iTurn =0;

switch(LKevent)
{
  case LKEVENT_UP:
    fScaleFact /= 1.5;
  break;
  case LKEVENT_DOWN:
    fScaleFact *= 1.5;
  break;

  case LKEVENT_PAGEUP:
	fScaleFact = MIN_DIST_SCALE;
  break;
  case LKEVENT_PAGEDOWN:
	fScaleFact = MAX_DIST_SCALE;
  break;
  case LKEVENT_ENTER:
      iTurn = 	(iTurn+1)%4;
      switch(iTurn)
      {
        case 0: {RADAR_TURN = 90; ASYMETRIC_FACTOR = 0.7 ; SPLITSCREEN_FACTOR = 0.7;} break;
        case 1: {RADAR_TURN = 0 ; ASYMETRIC_FACTOR = 0.5 ; SPLITSCREEN_FACTOR = 0.7;} break;
        case 2: {RADAR_TURN = 0 ; ASYMETRIC_FACTOR = 0.5 ; SPLITSCREEN_FACTOR = 1.0;} break;
        case 3: {RADAR_TURN = 90; ASYMETRIC_FACTOR = 0.5 ; SPLITSCREEN_FACTOR = 0.7;} break;
      }
  break;
  default:
  break;
}
LKevent=LKEVENT_NONE; /* remove event from list */
fScaleFact = max (fScaleFact, MIN_DIST_SCALE); /* check ranges */
fScaleFact = min (fScaleFact, MAX_DIST_SCALE);/* check ranges */

if(SPLITSCREEN_FACTOR >0.95)
	bSideview = false;



double range = 1000; // km
double GPSlat, GPSlon, GPSalt, GPSbrg  ;
bool GPSValid;
double fMaxHeight  ;
double fMinHeight  ;
double fx,fy;
DiagrammStruct sDia;


bool bTrace = !DerivedDrawInfo.Circling;
bTrace =TRUE;
  GPSlat = DrawInfo.Latitude;
  GPSlon = DrawInfo.Longitude;
  GPSalt = DrawInfo.Altitude;
  GPSbrg = DrawInfo.TrackBearing;
  GPSValid = !DrawInfo.NAVWarning;
    
  if (DrawInfo.BaroAltitudeAvailable && EnableNavBaroAltitude) {
   	DerivedDrawInfo.NavAltitude = DrawInfo.BaroAltitude;
  } else {
  	DerivedDrawInfo.NavAltitude = DrawInfo.Altitude;
  }
  GPSalt =  DerivedDrawInfo.NavAltitude;

  fMaxHeight = GPSalt;
  fMinHeight = GPSalt;

static bool bFirstCall = false;
	if (bFirstCall == false)
	{
		ResetTraces();
		bFirstCall = true;
	}


  if (DoInit[MDI_FLARMRADAR]) {
	  fScaleFact =5.0;
	  ResetTraces();
	  switch (ScreenSize) {
		case ss480x640:
		case ss480x800:
		case ss896x672:
		case ss800x480:
		case ss640x480:
			iCircleSize = 9;
			iRectangleSize = 5;
			scaler[0]=(short)(-1*(NIBLSCALE(4)-2) * fTopViewPlaneSize);
			scaler[1]=(short)((NIBLSCALE(5)-2)    * fTopViewPlaneSize);
			scaler[2]=(short)(-1*(NIBLSCALE(6)-2) * fTopViewPlaneSize);
			scaler[3]=(short)((NIBLSCALE(4)-2)    * fTopViewPlaneSize);
			scaler[4]=(short)((NIBLSCALE(2)-2)    * fTopViewPlaneSize);
			tscaler=(NIBLSCALE(7)-2)    ;
			break;
		case ss240x320:
		case ss272x480:
		case ss320x240:
		case ss480x272:
		case ss720x408:
		case ss480x234:
		case ss400x240:
			iCircleSize = 7;
			iRectangleSize = 4;
			scaler[0]=(short)(-1*(NIBLSCALE(8)-2)  * fTopViewPlaneSize);
			scaler[1]=(short)((NIBLSCALE(10)-2)    * fTopViewPlaneSize);
			scaler[2]=(short)(-1*(NIBLSCALE(12)-2) * fTopViewPlaneSize);
			scaler[3]=(short)((NIBLSCALE(8)-2)     * fTopViewPlaneSize);
			scaler[4]=(short)((NIBLSCALE(4)-2)     * fTopViewPlaneSize);
			tscaler=(NIBLSCALE(13)-2)      ;
			break;
		default:
			iCircleSize = 7;
			iRectangleSize = 4;
			scaler[0]=(short)(-1*NIBLSCALE(4) * fTopViewPlaneSize);
			scaler[1]=(short)(NIBLSCALE(5)    * fTopViewPlaneSize);
			scaler[2]=(short)(-1*NIBLSCALE(6) * fTopViewPlaneSize);
			scaler[3]=(short)(NIBLSCALE(4)    * fTopViewPlaneSize);
			scaler[4]=(short)(NIBLSCALE(2)    * fTopViewPlaneSize);
			tscaler=NIBLSCALE(7)      ;
			break;
	}
	Arrow[0].x = scaler[0];
	Arrow[0].y = scaler[1];
	Arrow[1].x = 0;
	Arrow[1].y = scaler[2];
	Arrow[2].x = scaler[3];
	Arrow[2].y = scaler[1];
	Arrow[3].x = 0;
	Arrow[3].y = scaler[4];
	Arrow[4].x = scaler[0];
	Arrow[4].y = scaler[1];

	// Stuff for raw 0 mapspace
	#define HEADRAW       NIBLSCALE(6)
	SIZE MITextSize;
	SelectObject(hdc, LK8PanelMediumFont);
	GetTextExtentPoint(hdc, _T("4.4"), 3, &MITextSize);
	column0=MITextSize.cx+LEFTLIMITER+NIBLSCALE(5);
	SelectObject(hdc, LK8PanelUnitFont);

	DoInit[MDI_FLARMRADAR]=false;
  }


  /****************************************************************************************************
   * scale X axis
   ****************************************************************************************************/

  sDia.fXMin = -fScaleFact*(1.0-ASYMETRIC_FACTOR)*10000.0;
  sDia.fXMax =  fScaleFact*(    ASYMETRIC_FACTOR)*10000.0;
  range =sDia.fXMax - sDia.fXMin ;
  sDia.rc = rc;


  /*******************************************************
   * draw sky
   *******************************************************/
  #if (WINDOWSPC>0)
      if(!INVERTCOLORS)
        RenderSky( hdc, rc, SKY_HORIZON_COL , SKY_SPACE_COL , GC_NO_COLOR_STEPS);
  #endif

  double xtick = 0.001;
  if (range>0.05*1000.0) xtick = 0.01;
  if (range>0.1*1000.0) xtick = 0.1;
  if (range>1.0*1000.0) xtick = 1.0;
  if (range>10.0*1000.0) xtick = 5.0;
  if (range>50.0*1000.0) xtick = 10.0;
  if (range>100.0*1000.0) xtick = 20.0;
  if (range>200.0*1000.0) xtick = 25.0;
  if (range>250.0*1000.0) xtick = 50.0;
  if (range>500.0*1000.0) xtick = 100.0;


  if(xtick == 0.0) xtick = 1.0;
  LKASSERT(!xtick == 0.0)

  RECT rc34 = rc;
  rc34.top += (rct.top-rct.bottom)/2;
  DrawXGrid(hdc, rc34, xtick/DISTANCEMODIFY, xtick, 0,TEXT_ABOVE_RIGHT, rgbGridColor,  &sDia);

  /*********************************************************************************
   * Draw Y Grid
   *********************************************************************************/
  if(GPSalt < HEIGHT_RANGE)
    fMaxHeight = HEIGHT_RANGE;
  else
	fMaxHeight = GPSalt;
  fMinHeight = GPSalt;
  for (i=0; i<FLARM_MAX_TRAFFIC; i++)
    if(DrawInfo.FLARM_Traffic[i].Status != LKT_EMPTY)
    {
  	  fMaxHeight = max (fMaxHeight, DrawInfo.FLARM_Traffic[i].Altitude);
  	  fMinHeight = min (fMinHeight, DrawInfo.FLARM_Traffic[i].Altitude);
    }


  sDia.fYMin = max(-GPSalt, -HEIGHT_RANGE);
  sDia.fYMax =HEIGHT_RANGE;

  double fScale = 1000;
  if((sDia.fYMax-sDia.fYMin) < 800)
	fScale = 400.0f;
  else
	fScale = 600.0f;
  if (Units::GetUserInvAltitudeUnit() == unFeet)
	  fScale /= 2;

  if(bSideview)
    DrawYGrid(hdc, rc, fScale/ALTITUDEMODIFY,fScale, 0,TEXT_ABOVE_RIGHT ,rgbGridColor,  &sDia);


  /****************************************************************************************************
   * draw side elements
   ****************************************************************************************************/

double fLon;
double fLat;
double fDistBearing;
double fFlarmBearing=0.0;
double fFlarmAlt;
double fFlarmDist;
TCHAR lbuffer[50];

LKASSERT( rct.right !=rct.left )
double fRatio = (double)( rct.bottom-rct.top) / (double)( rct.right-rct.left);
DiagrammStruct sTopDia;
sTopDia.rc    = rct;
sTopDia.fXMin = sDia.fXMin;
sTopDia.fXMax = sDia.fXMax;
sTopDia.fYMin = -(sDia.fXMax-sDia.fXMin)/2 * fRatio;
sTopDia.fYMax =  (sDia.fXMax-sDia.fXMin)/2 * fRatio;

 int x_middle = DistanceToX  (0, rct, &sTopDia); // (rct.right-rct.left)/2;
 int y_middle = HeightToY    (0, rct, &sTopDia);//(rct.bottom-rct.top)/2;


/*******************************************************
 * draw radar circles
 *******************************************************/
double fRing = 0; //sTopDia.fXMax;
int iCircleRadius = 0;
RECT rcc = rct;
    rcc.bottom -=3;
	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	xtick *=1000.0;
	SelectObject(hdc, hGreenPen);
	if(sDia.fXMax ==sDia.fXMin)
	  sDia.fXMax= sDia.fXMin+1.0;
	LKASSERT( sDia.fXMax !=sDia.fXMin )
	double fCScale =(double)( rct.right-rct.left)/(sDia.fXMax-sDia.fXMin );
	for ( i = 0; i < (sDia.fXMax /xtick); i++)
	{
	  iCircleRadius =(int) (fRing* fCScale);
	  Circle(hdc, x_middle, y_middle, iCircleRadius, rcc, true, false );
	  fRing = fRing + xtick;
	}
	Rectangle(hdc,rct.left , rct.bottom ,rct.right, rct.top);

	SelectObject(hdc, hOrangePen);
	fRing = xtick/2;
	if((sDia.fXMax /xtick)  < 3)
	  for ( i = 0; i < (sDia.fXMax /xtick); i++)
	  {
	    iCircleRadius = (int) (fRing * fCScale);
	    Circle(hdc, x_middle, y_middle, iCircleRadius, rcc, true, false );
	    fRing = fRing + xtick;
	  }

	SelectObject(hdc, hDrawBrush);
	SelectObject(hdc, hDrawPen);


	/***********************************************
	 * sort indexes for altitude
	 ***********************************************/
	  nEntrys=0;

	 int aiSortArray[FLARM_MAX_TRAFFIC];

	int iTmp;
	for (i=0; i<FLARM_MAX_TRAFFIC; i++)
	{
	  if(DrawInfo.FLARM_Traffic[i].Status == LKT_EMPTY)
	  {
		asFLRAMPos[i].bBuffFull= false;
		asFLRAMPos[i].iLastPtr = 0;
	  }
	  else
	  {
		/*************************************************************************
		 * calculate positions
		 *************************************************************************/
		//DrawInfo.FLARM_Traffic[i].RelativeAltitude = DrawInfo.FLARM_Traffic[i].Altitude - GPSalt;
		fLon          =  DrawInfo.FLARM_Traffic[i].Longitude;
		fLat          =  DrawInfo.FLARM_Traffic[i].Latitude;
		fFlarmBearing =  DrawInfo.FLARM_Traffic[i].TrackBearing;
		fFlarmAlt     =  DrawInfo.FLARM_Traffic[i].RelativeAltitude;
		LL_to_BearRange( GPSlat, GPSlon, fLat,  fLon, &fDistBearing, &fFlarmDist);

		fDistBearing = ( fDistBearing - GPSbrg + RADAR_TURN);
		asFLRAMPos[i].fFlarmBearing= (fFlarmBearing - GPSbrg + RADAR_TURN);
		asFLRAMPos[i].fx = fFlarmDist * sin(fDistBearing*DEG_TO_RAD);
		asFLRAMPos[i].fy = fFlarmDist * cos(fDistBearing*DEG_TO_RAD);
		asFLRAMPos[i].fAlt = fFlarmAlt;

		asFLRAMPos[i].iColorIdx = (int)(2*DrawInfo.FLARM_Traffic[i].Average30s-0.5)+NO_VARIO_COLORS/2;
		asFLRAMPos[i].iColorIdx = max( asFLRAMPos[i].iColorIdx, 0);
		asFLRAMPos[i].iColorIdx = min( asFLRAMPos[i].iColorIdx, NO_VARIO_COLORS-1);

		extern FlarmIdFile file; // in Utils

		wsprintf(asFLRAMPos[i].szGliderType,_T(""));
		FlarmId* flarmId = file.GetFlarmIdItem(DrawInfo.FLARM_Traffic[i].ID);


		if(flarmId!= NULL)
		  _tcscat(asFLRAMPos[i].szGliderType,flarmId->type);

	  	int iCnt= 19;
	    while ((asFLRAMPos[i].szGliderType[iCnt] ==_T(' ')) && (iCnt > 0))
		  asFLRAMPos[i].szGliderType[iCnt--]= 0;

        /**************************************
         * Fill trace buffer
         **************************************/
#define FLARM_TRACES
#ifdef FLARM_TRACES
		asFLRAMPos[i].asRingBuf[asFLRAMPos[i].iLastPtr].fLat = fLat;
		asFLRAMPos[i].asRingBuf[asFLRAMPos[i].iLastPtr].fLon = fLon;
		asFLRAMPos[i].asRingBuf[asFLRAMPos[i].iLastPtr].fAlt = DrawInfo.FLARM_Traffic[i].Altitude;
		asFLRAMPos[i].asRingBuf[asFLRAMPos[i].iLastPtr].iColorIdx = asFLRAMPos[i].iColorIdx;
		asFLRAMPos[i].iLastPtr++;
		if(asFLRAMPos[i].iLastPtr > NO_TRACE_PTS)
		{
		  asFLRAMPos[i].iLastPtr=0;
		  asFLRAMPos[i].bBuffFull = true;
		}
#endif
		aiSortArray[nEntrys++] = i;
	  }

	}

    for (i=0; i < nEntrys; i++)
      for(j=i+1; j < nEntrys; j++)
		if(asFLRAMPos[aiSortArray[i]].fAlt  > asFLRAMPos[aiSortArray[j]].fAlt )
		{
		  iTmp = aiSortArray[i];
		  aiSortArray[i] = aiSortArray[j];
		  aiSortArray[j] = iTmp;
		}


/***********************************************
 * FLARM object loop
 ***********************************************/
bool bCenter = false;
for (j=0; j<nEntrys; j++)
{
  i = aiSortArray[j];
  {
	/*************************************************************************
	 * calculate positions
	 *************************************************************************/
	fx = asFLRAMPos[i].fx;
	fy = asFLRAMPos[i].fy;
	fFlarmAlt = asFLRAMPos[i].fAlt;
	int x  = DistanceToX(fx, rct, &sTopDia);
	int y  = HeightToY  (fy, rct, &sTopDia);
	TextInBoxMode_t displaymode = {1};
	displaymode.NoSetFont = 1;
	displaymode.Border=1;

    if(bTrace)
	    DrawFlarmObjectTrace(hdc, &sTopDia, i);

    if(fx > sTopDia.fXMin )  /* sing sight ? */
    if(fx < sTopDia.fXMax )
    if(fy < sTopDia.fYMax )
	if(fy > sTopDia.fYMin )
	if(fFlarmAlt < sDia.fYMax )
	if(fFlarmAlt > sDia.fYMin )
	{
	  /***********************************************
	   * draw center aircraft if first time above
	   ***********************************************/
	  if(bCenter == false)
	    if(fFlarmAlt > 0 )
	    {
	      bCenter = true;
		  SelectObject(hdc, hDrawBrush);
		  SelectObject(hdc, hDrawPen);
		  SelectObject(hdc,GetStockObject(BLACK_PEN));
		  PolygonRotateShift(AircraftTop, NUMAIRCRAFTPTS, x_middle, y_middle,RADAR_TURN);
		  Polygon(hdc,AircraftTop,NUMAIRCRAFTPTS);
		  SelectObject(hdc, hDrawPen);
	    }
	  /*************************************************************************
	   * calculate climb color
	   *************************************************************************/
	  double fInteg30 = DrawInfo.FLARM_Traffic[i].Average30s;
	  int iVarioIdx = (int)(2*fInteg30-0.5)+NO_VARIO_COLORS/2;
	  if(iVarioIdx < 0) iVarioIdx =0;
	  if(iVarioIdx >= NO_VARIO_COLORS) iVarioIdx =NO_VARIO_COLORS-1;
	  SelectObject(hdc, *variobrush[iVarioIdx]);


	  /*************************************************************************
	   * draw side view
	   *************************************************************************/
	  switch (DrawInfo.FLARM_Traffic[i].Status) { // 100321
		case LKT_GHOST:
			Rectangle(hdc,x-iRectangleSize, y-iRectangleSize,x+iRectangleSize, y+iRectangleSize);
		//	asFLRAMPos[i].bBuffFull= 0;
		//	asFLRAMPos[i].iLastPtr = 0;
			break;
		case LKT_ZOMBIE:
			Circle(hdc, x, y, iCircleSize, rct, true, true );
		//	asFLRAMPos[i].bBuffFull= 0;
		//	asFLRAMPos[i].iLastPtr = 0;
			break;
		default:
			POINT Triangle[5] = {Arrow[0],Arrow[1],Arrow[2],Arrow[3],Arrow[4]};
			PolygonRotateShift(Triangle, 5, x, y, AngleLimit360( asFLRAMPos[i].fFlarmBearing ));
			Polygon(hdc,Triangle,5);

		    /*************************************************************************
		     * draw label
		     *************************************************************************/
		    wsprintf(lbuffer,_T(""));
			_stprintf(lbuffer,_T("%3.1f"),LIFTMODIFY*DrawInfo.FLARM_Traffic[i].Average30s);

		    SIZE tsize;
		    SetBkMode(hdc, TRANSPARENT);
		    GetTextExtentPoint(hdc, lbuffer, _tcslen(lbuffer), &tsize);
		    if (_tcslen(lbuffer)>0)
			  TextInBox(hdc, lbuffer, x+tscaler,  y+tscaler, 0, &displaymode, false);

			break;
	  }
	  /*********************************************
	   * draw lines to target if target selected
	   */
	  if(DrawInfo.FLARM_Traffic[i].Locked)
	  {
		DrawDashLine(hdc, 4, (POINT){x_middle, y_middle},(POINT){ x, y} ,rgb_targetlinecol, rct );
	  }
	}
  }
}
/***********************************************
 * draw center aircraft if highest (was never drawn)
 ***********************************************/
if(bCenter == false)
{
  SelectObject(hdc, hDrawBrush);
  SelectObject(hdc, hDrawPen);
  SelectObject(hdc,GetStockObject(BLACK_PEN));
  PolygonRotateShift(AircraftTop, NUMAIRCRAFTPTS, x_middle, y_middle,RADAR_TURN);
  Polygon(hdc,AircraftTop,NUMAIRCRAFTPTS);
  SelectObject(hdc, hDrawPen);
}

/*************************************************************************
 * sideview
 *************************************************************************/
for (i=0; i < nEntrys; i++)
  for(j=i+1; j < nEntrys; j++)
	if(asFLRAMPos[aiSortArray[i]].fy  < asFLRAMPos[aiSortArray[j]].fy )
	{
	  iTmp = aiSortArray[i];
	  aiSortArray[i] = aiSortArray[j];
	  aiSortArray[j] = iTmp;
	}
/***********************************************
 * FLARM object loop
 ***********************************************/
if(bSideview)
{
  bCenter = false;
  for (j=0; j<nEntrys; j++)
  {
    i = aiSortArray[j];
	/*************************************************************************
	 * calculate positions
	 *************************************************************************/
	fx = asFLRAMPos[i].fx;
	fy = asFLRAMPos[i].fy;
	fFlarmAlt = asFLRAMPos[i].fAlt;
	int x  = DistanceToX(fx, rct, &sTopDia);
	int hy = HeightToY  (fFlarmAlt, rc, &sDia);
	TextInBoxMode_t displaymode = {1};
	displaymode.NoSetFont = 1;
	displaymode.Border=1;
    if(fx > sTopDia.fXMin )  /* sing sight ? */
    if(fx < sTopDia.fXMax )
	if(fFlarmAlt < sDia.fYMax )
	if(fFlarmAlt > sDia.fYMin )
	{
	  if(bCenter == false)
		if(fy < 0 )
		{
		  bCenter = true;
		  SelectObject(hdc, hDrawBrush);
		  SelectObject(hdc, hDrawPen);
		  RenderFlarmPlaneSideview( hdc, rc,0 , 0,RADAR_TURN, &sDia , fPlaneSize);
		}
	  /*************************************************************************
	   * get the climb color
	   *************************************************************************/
	  SelectObject(hdc, *variobrush[asFLRAMPos[i].iColorIdx]);

	  /*************************************************************************
	   * draw side view
	   *************************************************************************/
	  switch (DrawInfo.FLARM_Traffic[i].Status) { // 100321
		case LKT_GHOST:
			Rectangle(hdc,x-iRectangleSize,hy-iRectangleSize,x+iRectangleSize,hy+iRectangleSize);
			break;
		case LKT_ZOMBIE:
			Circle(hdc, x, hy, iCircleSize, rc, true, true );
			break;
		default:
			RenderFlarmPlaneSideview( hdc,   rc, fx,  fFlarmAlt, asFLRAMPos[i].fFlarmBearing , &sDia , fPlaneSize/*1.0 - cos(fDistBearing*DEG_TO_RAD)/4*/);
			break;
	  }
	  wsprintf(lbuffer,_T(""));
	  if (DrawInfo.FLARM_Traffic[i].Cn && DrawInfo.FLARM_Traffic[i].Cn[0]!=_T('?')) { // 100322
	  	_tcscat(lbuffer,  asFLRAMPos[i].szGliderType);
	  	_tcscat(lbuffer,_T(": "));
	  	_tcscat(lbuffer,DrawInfo.FLARM_Traffic[i].Cn);
	  }


	  SIZE tsize;
	  SetBkMode(hdc, TRANSPARENT);
	  GetTextExtentPoint(hdc, lbuffer,  _tcslen(lbuffer), &tsize);
	  if (_tcslen(lbuffer)>0)
		TextInBox(hdc, lbuffer, x+tscaler,  hy+tscaler, 0, &displaymode, false);

	  /*********************************************
	   * draw lines to target if target selected
	   */
	  if(DrawInfo.FLARM_Traffic[i].Locked)
	  {
		int  h0 = HeightToY(GPSlat,rc,&sDia);
		DrawDashLine(hdc, 4, (POINT){x_middle,       h0},(POINT){ x, hy} ,rgb_targetlinecol, rc );
	  }
	}
  }
  /*************************************************************************
   * draw own plane position
   *************************************************************************/
  SelectObject(hdc, hDrawBrush);

  if(!bCenter)
    RenderFlarmPlaneSideview( hdc, rc,0 , 0,RADAR_TURN, &sDia , fPlaneSize);
}

  //
  // Draw MapSpace index on top left screen
  //
  _stprintf(lbuffer,TEXT("%d.%d"),ModeIndex,CURTYPE+1);
  SelectObject(hdc, LK8PanelMediumFont);
  LKWriteText(hdc, lbuffer, LEFTLIMITER, rci.top+TOPLIMITER , 0,  WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

  SelectObject(hdc, LK8InfoNormalFont);
  _stprintf(lbuffer,TEXT("RDR.%d"),iTurn+1);
  LKWriteText(hdc, lbuffer, column0, HEADRAW-NIBLSCALE(1) , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_LIGHTGREEN, false);

SelectObject(hdc, hfOldFont);
SelectObject(hdc, hOldPen);
SelectObject(hdc, hOldBrush);
DeleteObject (hGreenPen);
DeleteObject (hOrangePen);
DeleteObject (hWhitePen);

}


bool PtInRect(POINT p0, RECT rcd )
{
  if( p0.x  > rcd.left   )
    if( p0.x  < rcd.right  )
      if( p0.y  < rcd.bottom )
        if( p0.y > rcd.top    )
          return true;
  return false;
}


int MapWindow::DrawFlarmObjectTrace(HDC hDC, DiagrammStruct* pDia, int iFlarmIdx)
{
double GPSlat = DrawInfo.Latitude;
double GPSlon = DrawInfo.Longitude;
//double GPSalt = DrawInfo.Altitude;
double GPSbrg = DrawInfo.TrackBearing;

double fDistBearing;
double fFlarmDist;
double fFlarmBearing;
double fx,fy;
//double fAlt;
COLORREF BgCol;
POINT line[2]= {{0,0},{0,0}};

int i, iCnt=0;
int iTo= asFLRAMPos[iFlarmIdx].iLastPtr;
int iIdx = 0;
#define FADE_CNT 20
double fBrithness = 1.0;
if(iTo > (NO_TRACE_PTS-FADE_CNT) )
  fBrithness =0.05;

if(INVERTCOLORS)
	BgCol = RGB_BLACK;
else
	BgCol = RGB_WHITE;


if( asFLRAMPos[iFlarmIdx].bBuffFull)
{
  fBrithness = 0.0;
  iTo  = NO_TRACE_PTS;
  iIdx = asFLRAMPos[iFlarmIdx].iLastPtr;
  if(iIdx++ >=NO_TRACE_PTS)
    iIdx = 0;
}

HPEN oldPen =	(HPEN)SelectObject(hDC, GetStockObject(NULL_PEN));
iCnt =0;
for(i= 0; i < iTo; i++)
{
  iCnt++;
  LL_to_BearRange( GPSlat, GPSlon, asFLRAMPos[iFlarmIdx].asRingBuf[iIdx].fLat ,asFLRAMPos[iFlarmIdx].asRingBuf[iIdx].fLon, &fDistBearing, &fFlarmDist);

  fDistBearing = ( fDistBearing - GPSbrg + RADAR_TURN);
  fFlarmBearing= (fFlarmBearing - GPSbrg + RADAR_TURN);
  fx = fFlarmDist * sin(fDistBearing*DEG_TO_RAD);
  fy = fFlarmDist * cos(fDistBearing*DEG_TO_RAD);

  line[1].x  = DistanceToX(fx, pDia->rc, pDia);
  line[1].y  = HeightToY  (fy, pDia->rc, pDia);
 // fAlt = asFLRAMPos[iFlarmIdx].asRingBuf[iIdx].fAlt;


  if(i > 0)
	if(PtInRect(line[0], pDia->rc ))
	  if(PtInRect(line[1], pDia->rc ))
	  {
#ifndef LINE_TRACE
		SelectObject(hDC, *variobrush[asFLRAMPos[iFlarmIdx].asRingBuf[iIdx].iColorIdx]);
		Circle(hDC, line[0].x, line[0].y, 5,  pDia->rc, true, true );


#else
		color = variocol[ asFLRAMPos[iFlarmIdx].asRingBuf[iIdx].iColorIdx];
#ifdef FADE_CNT
		if(fBrithness <1.0)
		{
		  fBrithness += 1.0/(double)FADE_CNT;;
		  color = MixColors( color, BgCol, fBrithness );
		}
#endif
	    DrawDashLine(hDC,5, line[0], line[1],color, pDia->rc);
#endif
	  }

  line[0]= line[1];
  if(iIdx++ >=NO_TRACE_PTS)
    iIdx = 0;

}
 SelectObject(hDC, (HPEN) oldPen);

return 0;
}
