/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "RGB.h"
#include "Sideview.h"
#include "NavFunctions.h"
#include "LKObjects.h"
#include "DoInits.h"
#include "LKMapWindow.h"
#include "FlarmIdFile.h"
#include "FlarmRadar.h"
#include "Globals.h"
#include "Multimap.h"
#include "Bitmaps.h"
#include "Dialogs.h"
#include "Screen/PenReference.h"
#include "Screen/BrushReference.h"
#include "InputEvents.h"
#include "ScreenGeometry.h"
#include "Asset.hpp"

extern POINT startScreen;

#define NO_DASH_LINES
/*
#undef LKASSERT
#define LKASSERT(arg)
*/


#define F_SIZE0 0.0
#define F_SIZE1 0.5
#define F_SIZE2 0.75
#define F_SIZE3 1.0

#define IM_NO_TRACE       0
#define ALL_TRACE         1
#define IM_POS_TRACE_ONLY 2

int  iTraceDotSize = 5;
int RADAR_TURN = 90 ;            /* radar plane orientation             */
#define HEIGHT_RANGE (300.0  )    /* max hight ifference above and below in meters */
double ASYMETRIC_FACTOR = 0.7 ;     /* X center displacement               */
double SPLITSCREEN_FACTOR = F_SIZE2 ;   /* % of top view window                */
int bTrace = 1;
#define MIN_DIST_SCALE  0.1       /* minimum radar distance              */
#define MAX_DIST_SCALE 25.0       /* maximum radar distance              */


using std::min;
using std::max;

static int nEntrys=0;


typedef struct
{
	double fx;
	double fy;
	double fAlt;
	double fFlarmBearing;
	int iColorIdx;
	TCHAR szGliderType[FLARMID_SIZE_NAME+1];
} sFlarmPositions;
static sFlarmPositions asFLARMPos[FLARM_MAX_TRAFFIC+1];


const LKBrush * variobrush[NO_VARIO_COLORS] = {
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

void MapWindow::DrawXGrid(LKSurface& Surface, const RECT& rc, double ticstep,double unit_step, double zero, int iTextAling,
                           const LKColor& color, DiagrammStruct *psDia, const TCHAR *pLable) {

  POINT line[2];
  SIZE tsize;

  int xoff=0, yoff=0;
  double xval;
  if (psDia->fXMax == psDia->fXMin)
    psDia->fXMax++;
  double xscale = (rc.right) / (psDia->fXMax-psDia->fXMin);
  int xmin=0, ymin=0, xmax=0, ymax=0;
  double x_max = psDia->fXMax;
  double x_min = psDia->fXMin;
  TCHAR unit_text[MAX_PATH];
  for (xval=zero; xval< x_max; xval+= ticstep) {

    xmin = (int)((xval-x_min)*xscale)+rc.left;
    ymin = rc.top;
    xmax = xmin;
    ymax = rc.bottom;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    // We always print the dashed vertical lines in the sideview.
    // But we cannot print them in the topview, if available, when topology is also painted!
    // Correction v6.2: screen is too cluttered with vertical lines on top view even without
    // topology. 
    // if ( IsMultiMapShared() && (Current_Multimap_SizeY>SIZE0) && IsMultimapTopology() ) {
    if (1) {
	ymin = Current_Multimap_TopRect.bottom;
	line[0].y = ymin;
    }

#ifdef NO_DASH_LINES
    Surface.DrawLine(PEN_SOLID,1, line[0], line[1], color, rc);
#else
    Surface.DrawDashLine(ScreenThinSize, line[0], line[1], color, rc);
#endif



	if (iTextAling>TEXT_NO_TEXT)
	{

	   FormatTicText(unit_text, xval*unit_step/ticstep, unit_step);


	   Surface.GetTextSize(unit_text, &tsize);
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

	   Surface.DrawText(xmin+xoff, ymax+yoff, unit_text);

	   if(pLable != NULL)
		if((xval+ticstep) > x_max)
		  Surface.DrawText(xmin, ymax+yoff, pLable);
	}

  }

  for (xval=zero; xval> x_min; xval-= ticstep) {

    xmin = (int)((xval-x_min)*xscale)+rc.left;
    ymin = rc.top;
    xmax = xmin;
    ymax = rc.bottom;
    line[0].x = xmin;
    line[0].y = ymin;
    line[1].x = xmax;
    line[1].y = ymax;

    // Dont print vertical lines on topview in v6.2. 
    //if ( IsMultiMapShared() && (Current_Multimap_SizeY>SIZE0) && IsMultimapTopology() ) {
    if (1) {
	ymin = Current_Multimap_TopRect.bottom;
	line[0].y = ymin;
    }

#ifdef NO_DASH_LINES
    Surface.DrawLine(PEN_SOLID,1, line[0], line[1], color, rc);
#else
    Surface.DrawDashLine(ScreenThinSize, line[0], line[1], color, rc);
#endif


    if (iTextAling>TEXT_NO_TEXT)
    {

       FormatTicText(unit_text, xval*unit_step/ticstep, unit_step);
       Surface.GetTextSize(unit_text, &tsize);
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

       Surface.DrawText(xmin+xoff, ymax+yoff, unit_text);
    }
  }
}

void MapWindow::DrawYGrid(LKSurface& Surface, const RECT& rc, double ticstep,double unit_step, double zero, int iTextAling,
		 const LKColor& color, DiagrammStruct *psDia,  const TCHAR *pUnit) {
  POINT line[2];
  SIZE tsize;

 int  xoff =0  , yoff=0;

  double yval;
  double y_max = psDia->fYMax;
  double y_min = psDia->fYMin;

  int xmin=0, ymin=0, xmax=0, ymax=0;
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

    // If sideview is at minimal size, print units but not the dashed lines to avoid cluttering.
    // These are the elevation horizontal lines in sideview.
    if (Current_Multimap_SizeY<SIZE3) {
#ifdef NO_DASH_LINES
        Surface.DrawLine(PEN_SOLID,1, line[0], line[1], color, rc);
#else
        Surface.DrawDashLine(ScreenThinSize, line[0], line[1], color, rc);
#endif
    }

    // Do not print 0 altitude in shared maps, it is useless and we have no spare space to do it.
    // Beside, we would print it black on blue.
    if (IsMultiMapShared() && yval==0) continue;

    if (iTextAling != TEXT_NO_TEXT)
    {
	  TCHAR unit_text[MAX_PATH];
	  LKASSERT(ticstep!=0);
	  FormatTicText(unit_text, yval*unit_step/ticstep, unit_step);
	  if(pUnit != NULL)
            if(yval+ticstep >y_max)
              _stprintf(unit_text + _tcslen(unit_text), TEXT("%s"), pUnit);
	  Surface.GetTextSize(unit_text, &tsize);
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
	  Surface.DrawText(xmin+xoff, ymin+yoff, unit_text);
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

    if (Current_Multimap_SizeY<SIZE3) {
       // Who knows where is this line painted? 
#ifdef NO_DASH_LINES
        Surface.DrawLine(PEN_SOLID,1, (POINT) line[0], (POINT) line[1], color, rc);
#else
        Surface.DrawDashLine(ScreenThinSize, (POINT) line[0], (POINT) line[1], color, rc);
#endif
    }
    // Do not print 0 altitude in shared maps, it is useless and we have no spare space to do it.
    // Beside, we would print it black on blue.
    if (IsMultiMapShared() && yval==0) continue;

    if (iTextAling != TEXT_NO_TEXT)
    {
	  TCHAR unit_text[MAX_PATH];
	  FormatTicText(unit_text, yval*unit_step/ticstep, unit_step);
	  Surface.GetTextSize(unit_text, &tsize);
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
	  Surface.DrawText(xmin+xoff, ymin+yoff, unit_text);
    }
  }

}


int MapWindow::HeightToY(double fHeight, DiagrammStruct* psDia)
{
  int y0 = psDia->rc.bottom;

//  fMaxAltToday = 3300;
//  double hmin = max(0.0, alt-2300);
//  double hmax = max(fMaxAltToday, alt+1000);
  double hmin = psDia->fYMin;
  double hmax = psDia->fYMax;
  if (hmax==hmin) hmax++; // RECOVER DIVISION BY ZERO!
  double gfh = (fHeight-hmin)/(hmax-hmin);
  int yPos = (int)(gfh*(psDia->rc.top-psDia->rc.bottom)+y0)-1;
  if(yPos < psDia->rc.top) yPos = psDia->rc.top-1;
  if(yPos > psDia->rc.bottom) yPos = psDia->rc.bottom+1;
  return yPos;
//  fHeigh
}

int MapWindow::DistanceToX(double fDist,   DiagrammStruct* psDia)
{

if ( psDia->fXMax == psDia->fXMin) psDia->fXMax++; // RECOVER DIVISION BY ZERO!
double xscale =   (double) (psDia->rc.right - psDia->rc.left)/(psDia->fXMax - psDia->fXMin);
int	xPos = (int)((fDist- psDia->fXMin)*xscale)+psDia->rc.left ;
  return xPos;

}



// draw Flarm aircraft
void RenderFlarmPlaneSideview(LKSurface& Surface, const RECT& rc,double fDist, double fAltitude,double brg, DiagrammStruct* psDia ,double fScale )
{


  #define NO_AP_PTS 17
  const double fCos = fastcosine(brg);
  const double fSin = fastsine(brg);

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
  Start.x =  MapWindow::DistanceToX(fDist,  psDia);
  Start.y =  MapWindow::HeightToY(fAltitude,  psDia);

  const auto oldPen = Surface.SelectObject(LKPen_Grey_N0);
  PolygonRotateShift(AircraftWing, 13,  Start.x, Start.y,  0);
  PolygonRotateShift(AircraftSide, 8,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftTail, 5,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingL, 7,   Start.x, Start.y,  0);
  PolygonRotateShift(AircraftWingR, 7,   Start.x, Start.y,  0);



  if((brg < 180))
  {
    Surface.Polygon(AircraftWingL ,7 );
    Surface.Polygon(AircraftSide  ,8 );
    Surface.Polygon(AircraftWingR ,7 );
  }
  else
  {
    Surface.Polygon(AircraftWingR ,7 );
    Surface.Polygon(AircraftSide  ,8 );
    Surface.Polygon(AircraftWingL ,7 );
    Surface.Polygon(AircraftWing  ,13);
  }
  if((brg < 90)|| (brg > 270)) {
    Surface.Polygon(AircraftTail  ,5 );
  }
  Surface.SelectObject(oldPen);
} //else !asp_heading_task



//#define OWNPOS  // optional

void MapWindow::LKDrawFlarmRadar(LKSurface& Surface, const RECT& rci)
{
RECT rc  = rci; /* rectangle for sideview */
RECT rct = rc;  /* rectangle for topview */
rct.bottom = (long)((rc.bottom-rc.top  )*SPLITSCREEN_FACTOR); /* 2/3 for topview */
rc.top     = rct.bottom;
static double fScaleFact = 5.0;
static int iCircleSize    = IBLSCALE(4);
static int iRectangleSize = IBLSCALE(4);

static short tscaler=0;
static POINT Arrow[5];
TCHAR text[80];
static RECT PositionTopView[FLARM_MAX_TRAFFIC];
static RECT PositionSideView[FLARM_MAX_TRAFFIC];
#ifdef OWNPOS
static RECT OwnPosTopView;
static RECT OwnPosSideView;
#endif
int iTouchAreaSize = 45;
LKPen   hOrangePen ;
LKPen   hGreenPen ;
PenReference hDrawPen;
BrushReference hDrawBrush;
bool bSideview = true;


const auto hfOldFont = Surface.SelectObject(LK8PanelUnitFont);
LKColor rgbGridColor = RGB_DARKGREEN;
LKColor rgbDrawColor = RGB_GREEN;
LKColor rgb_targetlinecol = RGB_RED;
double fPlaneSize = 1.0;
double fOwnTopPlaneSize = 1.0;

static int aiSortArray[FLARM_MAX_TRAFFIC];

static bool bHeightScale = false;
static double fHeigtScaleFact = 1.0f;

bool bInvCol =  INVERTCOLORS;
/*********************************************************************************
 * change colors on inversion
 *********************************************************************************/
if(bInvCol)
{
  rgbDrawColor = IsDithered() ? RGB_WHITENOREV : RGB_GREY;
  rgbGridColor = IsDithered() ? RGB_WHITENOREV : RGB_GREY;
  hGreenPen.Create(PEN_SOLID, ScreenThinSize, IsDithered() ? RGB_WHITENOREV : RGB_GREEN);
  rgb_targetlinecol = RGB_LIGHTBLUE;
  hDrawPen   = LKPen_Grey_N0;
  hDrawBrush = LKBrush_White;
  hOrangePen.Create(PEN_SOLID, NIBLSCALE (1),RGB_ORANGE);
}
else
{
  rgbDrawColor = IsDithered() ? RGB_BLACK : RGB_DARKGREY;
  rgbGridColor = IsDithered() ? RGB_BLACK : RGB_DARKGREY;
  hGreenPen.Create(PEN_SOLID, ScreenThinSize,IsDithered() ? RGB_SBLACK : RGB_DARKGREY);
  rgb_targetlinecol = RGB_BLUE;
  hDrawPen   = LKPen_Grey_N0; // GetStockObject( WHITE_PEN );
  hDrawBrush = LKBrush_Black;
  hOrangePen.Create(PEN_SOLID, NIBLSCALE (1),RGB_LIGHTORANGE);
}

Surface.SetTextColor(rgbDrawColor);
const auto hOldPen   = Surface.SelectObject(hDrawPen);
const auto hOldBrush = Surface.SelectObject(hDrawBrush);

/****************************************************************
 * clear background
 ****************************************************************/
BOOL bFound = false;
switch(LKevent)
{
  case LKEVENT_NEWRUN:
	// CALLED ON ENTRY: when we select this page coming from another mapspace
//	fHeigtScaleFact = 1.0;
		bHeightScale	= false;
  break;
  case LKEVENT_UP:
	if(bHeightScale)
	  fHeigtScaleFact /= ZOOMFACTOR;
	else
      fScaleFact /= ZOOMFACTOR;
  break;
  case LKEVENT_DOWN:
	if(bHeightScale)
	  fHeigtScaleFact *= ZOOMFACTOR;
	else
      fScaleFact *= ZOOMFACTOR;
  break;
  case LKEVENT_TOPLEFT:
  {
    bTrace++;
    bTrace %= 3;
  }
  break;
  case LKEVENT_TOPRIGHT:
	  iFlarmDirection = 	(iFlarmDirection+1)%3;

  break;
  case LKEVENT_LONGCLICK:
	if( PtInRect(&rct, startScreen))
		bHeightScale	= false;
	#ifdef OWNPOS
	if( PtInRect(&OwnPosSideView, startScreen)||
	    PtInRect(&OwnPosTopView, startScreen) )
	{
	  iFlarmDirection = 	(iFlarmDirection+1)%3;
	}
	else
	#endif // OWNPOS
	    for (int i=0; i < nEntrys; i++)
		{
		  LKASSERT(i<FLARM_MAX_TRAFFIC);
		  LKASSERT(aiSortArray[i]>=0 && aiSortArray[i]<FLARM_MAX_TRAFFIC);
		  if( PtInRect(&PositionTopView[aiSortArray[i]], startScreen) ||
		      PtInRect(&PositionSideView[aiSortArray[i]], startScreen) )
		  {
		    for (int j = 0; j < FLARM_MAX_TRAFFIC; j++ ) {
			  LKASSERT(aiSortArray[i]>=0 && aiSortArray[i]<FLARM_MAX_TRAFFIC);
			  if(LKTraffic[aiSortArray[i]].RadioId == LKTraffic[j].RadioId)
			  {
#ifdef FLARM_MS
			    dlgAddMultiSelectListItem( (long*) &LKTraffic[j], j, IM_FLARM, LKTraffic[j].Distance);
#else
			    InputEvents::processPopupDetails(InputEvents::PopupTraffic, j); // With no Multiselect
#endif
			    bFound = true;
			  }
		    }
		  }
	    }

#ifdef FLARM_MS
    if(bFound) {
        /*
         * we can't show dialog from Draw thread
         * instead, new event is queued, dialog will be popup by main thread
         */
        InputEvents::processGlideComputer(GCE_POPUP_MULTISELECT);
    }
#endif
	if(!bFound)
	  if( PtInRect(&rc, startScreen))
		bHeightScale	= !bHeightScale;

  break;

  case LKEVENT_PAGEUP:
	  if(SPLITSCREEN_FACTOR == F_SIZE1) SPLITSCREEN_FACTOR = F_SIZE0;
	  if(SPLITSCREEN_FACTOR == F_SIZE2) SPLITSCREEN_FACTOR = F_SIZE1;
	  if(SPLITSCREEN_FACTOR == F_SIZE3) SPLITSCREEN_FACTOR = F_SIZE2;
  break;
  case LKEVENT_PAGEDOWN:
	  if(SPLITSCREEN_FACTOR == F_SIZE2) SPLITSCREEN_FACTOR = F_SIZE3;
	  if(SPLITSCREEN_FACTOR == F_SIZE1) SPLITSCREEN_FACTOR = F_SIZE2;
	  if(SPLITSCREEN_FACTOR == F_SIZE0) SPLITSCREEN_FACTOR = F_SIZE1;
  break;
  case LKEVENT_ENTER:

  break;
  default:
  break;
}
Current_Multimap_SizeY = (int)(SPLITSCREEN_FACTOR*100);
LKevent=LKEVENT_NONE; /* remove event from list */
if(SPLITSCREEN_FACTOR >0.95)
	bSideview = false;

switch(iFlarmDirection)
{
	case 0: {RADAR_TURN = 90; ASYMETRIC_FACTOR = 0.7 ; } break;
	case 1: {RADAR_TURN = 0 ; ASYMETRIC_FACTOR = 0.5 ; } break;
	case 2: {RADAR_TURN = 0 ; ASYMETRIC_FACTOR = 0.5 ; } break;
}

static double oldSplit = 0;
  if(oldSplit != SPLITSCREEN_FACTOR)
  {
	oldSplit=SPLITSCREEN_FACTOR;
//	SetSplitScreenSize(SPLITSCREEN_FACTOR);
	rc.top     = (long)((double)(rci.bottom-rci.top  )*SPLITSCREEN_FACTOR);
	rct.bottom = rc.top ;
  }

/****************************************************************/



#define NUMAIRCRAFTPTS (std::size(AircraftTop))
POINT AircraftTop[] = {
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
  {-1,-6},
  { 1,-6}
};


for(unsigned i=0; i < NUMAIRCRAFTPTS; i++)
{
  AircraftTop[i].x =  ( AircraftTop[i].x * fOwnTopPlaneSize);
  AircraftTop[i].y =  ( AircraftTop[i].y * fOwnTopPlaneSize);
}




fScaleFact = max (fScaleFact, MIN_DIST_SCALE); /* check ranges */
fScaleFact = min (fScaleFact, MAX_DIST_SCALE);/* check ranges */



BOOL bLandscape = true;

double range = 1000; // km
double GPSlat, GPSlon, GPSalt, GPSbrg ;
double Planebrg =0.0 ;
double fMaxHeight  ;
double fMinHeight  ;
double fx,fy;
DiagrammStruct sDia;


  GPSlat = DrawInfo.Latitude;
  GPSlon = DrawInfo.Longitude;
  GPSalt = DrawInfo.Altitude;
  GPSbrg = DrawInfo.TrackBearing;


  LastDoTraffic=0;
  DoTraffic(&DrawInfo,&DerivedDrawInfo);
  if (DrawInfo.BaroAltitudeAvailable && EnableNavBaroAltitude) {
	DerivedDrawInfo.NavAltitude = DrawInfo.BaroAltitude;
  } else {
	DerivedDrawInfo.NavAltitude = DrawInfo.Altitude;
  }
  GPSalt =  DerivedDrawInfo.NavAltitude;

  fMaxHeight = GPSalt;
  fMinHeight = GPSalt;

	Arrow[0].x = -4;
	Arrow[0].y = 5;
	Arrow[1].x = 0;
	Arrow[1].y = -6;
	Arrow[2].x = 4;
	Arrow[2].y = 5;
	Arrow[3].x = 0;
	Arrow[3].y = 2;
	Arrow[4].x = -4;
	Arrow[4].y = 5;
		for (int q=0; q < 5; q++)
		{
			Arrow[q].x  = (long) ((double)Arrow[q].x * 1.7);
			Arrow[q].y  = (long) ((double)Arrow[q].y * 1.7);
		}
  if (DoInit[MDI_FLARMRADAR])
  {

	  fScaleFact =5.0;

	  switch (ScreenSize) {
		case ss480x640:
		case ss480x800:
		case ss800x480:
		case ss640x480:
			iCircleSize = 9;
			iTraceDotSize = 5;
			iRectangleSize = 7;
			tscaler=(NIBLSCALE(7)-2)    ;
			break;
		case ss240x320:
		case ss272x480:
		case ss320x240:
		case ss480x272:
		case ss480x234:
		case ss400x240:
			iCircleSize = 7;
			iTraceDotSize = 3;
			iRectangleSize = 5;
			tscaler=(NIBLSCALE(13)-2)      ;
			break;
		default:
			iCircleSize = 7;
			iTraceDotSize = 3;
			iRectangleSize = 5;
			tscaler=NIBLSCALE(7)      ;
			break;
	}



        bLandscape = ScreenLandscape;


	if(   bLandscape)
	  {RADAR_TURN = 90; ASYMETRIC_FACTOR = 0.7 ; }
	else
	  {RADAR_TURN = 0 ; ASYMETRIC_FACTOR = 0.5 ; };


	iCircleSize    = IBLSCALE(4);
	iTraceDotSize  = IBLSCALE(2);
	iRectangleSize = IBLSCALE(4);
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

  if (!IsDithered()) {
    if (!bInvCol)
      RenderSky(Surface, rc, SKY_HORIZON_COL, SKY_SPACE_COL, GC_NO_COLOR_STEPS);
  }


  double xtick = 0.001;
  if (range>0.05*1000.0) xtick = 0.1;
  if (range>0.1*1000.0) xtick = 0.2;
  if (range>1.0*1000.0) xtick = 0.5;
  if (range>2.0*1000.0) xtick = 1.0;
  if (range>10.0*1000.0) xtick = 5.0;
  if (range>50.0*1000.0) xtick = 10.0;
  if (range>100.0*1000.0) xtick = 20.0;
  if (range>200.0*1000.0) xtick = 25.0;
  if (range>250.0*1000.0) xtick = 50.0;
  if (range>500.0*1000.0) xtick = 100.0;

  LKASSERT(range!=0);

  if(xtick == 0.0) xtick = 1.0;

  RECT rc34 = rc;
  rc34.top += (rct.top-rct.bottom)/2;

  _stprintf(text, TEXT("%s"),Units::GetUnitName(Units::GetUserDistanceUnit()));
  DrawXGrid(Surface, rc34, xtick/DISTANCEMODIFY, xtick, 0,TEXT_ABOVE_LEFT, rgbGridColor,  &sDia, text);


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

 int x_middle = DistanceToX  (0, &sTopDia); // (rct.right-rct.left)/2;
 int y_middle = HeightToY    (0, &sTopDia);//(rct.bottom-rct.top)/2;
#ifdef OWNPOS
 OwnPosTopView.left   = x_middle-iTouchAreaSize;
 OwnPosTopView.right  = x_middle+iTouchAreaSize;
 OwnPosTopView.top    = y_middle-iTouchAreaSize;
 OwnPosTopView.bottom = y_middle+iTouchAreaSize;
#endif

/*******************************************************
 * draw radar circles
 *******************************************************/

double fRing = 0; //sTopDia.fXMax;
int iCircleRadius = 0;
RECT rcc = rct;
double scl = xtick;
    rcc.bottom -=3;
	Surface.SelectObject(LKBrush_Hollow);
	scl *=1000.0;
	Surface.SelectObject(hGreenPen);
	if(sDia.fXMax ==sDia.fXMin)
	  sDia.fXMax= sDia.fXMin+1.0;
	LKASSERT( sDia.fXMax !=sDia.fXMin )
	double fCScale =(double)( rct.right-rct.left)/((sDia.fXMax-sDia.fXMin ));
	for ( int i = 0; i < (sDia.fXMax /scl); i++)
	{
	  iCircleRadius =(int) (fRing* fCScale / (DISTANCEMODIFY*1000.0f));
	  Surface.DrawCircle(x_middle, y_middle, iCircleRadius, rcc, false );
	  fRing = fRing + scl;
	}


        // Borders in green not used in v6 for the flarm radar multimap page.
        // Surface.Rectangle(rct.left , rct.bottom ,rct.right, rct.top);

	Surface.SelectObject(hOrangePen);
	fRing = xtick/2;
	if((sDia.fXMax /xtick)  < 3)
	  for (int i = 0; i < (sDia.fXMax /xtick); i++)
	  {
	    iCircleRadius = (int) (fRing * fCScale / (DISTANCEMODIFY*1000.0f));
	    Surface.DrawCircle(x_middle, y_middle, iCircleRadius, rcc, false);
	    fRing = fRing + xtick;
	  }

	Surface.SelectObject(hDrawBrush);
	Surface.SelectObject(hDrawPen);


	/***********************************************
	 * sort indexes for altitude
	 ***********************************************/
	  nEntrys=0;



	int iTmp;


	/**********************************************
	 * loop over FLARM objects.
	 */
	Surface.SelectObject(hDrawPen);
	if(iFlarmDirection == 2)
	{
		Planebrg = GPSbrg;
		GPSbrg =0.0;
	}
	for (int i=0; i<FLARM_MAX_TRAFFIC; i++)
	{
	  if (LKTraffic[i].Status != LKT_EMPTY)
	  {
		/*************************************************************************
		 * calculate positions
		 *************************************************************************/
		fLon          =  LKTraffic[i].Longitude;
		fLat          =  LKTraffic[i].Latitude;
		fFlarmBearing =  LKTraffic[i].TrackBearing;
		fFlarmAlt     =  LKTraffic[i].Altitude -  DrawInfo.Altitude;
		DistanceBearing( GPSlat, GPSlon, fLat,  fLon, &fFlarmDist, &fDistBearing);

		fDistBearing = ( fDistBearing - GPSbrg + RADAR_TURN);
		asFLARMPos[i].fFlarmBearing= (fFlarmBearing - GPSbrg + RADAR_TURN);
		asFLARMPos[i].fx = fFlarmDist * sin(fDistBearing*DEG_TO_RAD);
		asFLARMPos[i].fy = fFlarmDist * cos(fDistBearing*DEG_TO_RAD);
		asFLARMPos[i].fAlt = fFlarmAlt;
		asFLARMPos[i].iColorIdx = (int)(2*LKTraffic[i].Average30s    -0.5)+NO_VARIO_COLORS/2;
		asFLARMPos[i].iColorIdx = max( asFLARMPos[i].iColorIdx, 0);
		asFLARMPos[i].iColorIdx = min( asFLARMPos[i].iColorIdx, NO_VARIO_COLORS-1);


		_tcscpy(asFLARMPos[i].szGliderType,_T(""));

		const FlarmId* flarmId = LookupFlarmId(LKTraffic[i].RadioId);
		if(flarmId!= NULL) {
		  LK_tcsncpy(asFLARMPos[i].szGliderType,flarmId->type,FLARMID_SIZE_NAME);
		}

		int iCnt= FLARMID_SIZE_NAME;

	        for ( (iCnt = FLARMID_SIZE_NAME);iCnt>0 ; iCnt--)
	        {
	            if(asFLARMPos[i].szGliderType[iCnt] ==_T(' '))
		          asFLARMPos[i].szGliderType[iCnt]= 0;
	        }

		LKASSERT(nEntrys>=0 && nEntrys<FLARM_MAX_TRAFFIC);
		aiSortArray[nEntrys++] = i;
	  }

	}

    for (int i=0; i < nEntrys; i++)
    {
      for(int j=i+1; j < nEntrys; j++)
      {
	    LKASSERT(i<FLARM_MAX_TRAFFIC);
		LKASSERT(aiSortArray[i]>=0 && aiSortArray[i]<FLARM_MAX_TRAFFIC);
		if(asFLARMPos[aiSortArray[i]].fAlt  > asFLARMPos[aiSortArray[j]].fAlt )
		{
		  LKASSERT(j<FLARM_MAX_TRAFFIC);
		  iTmp = aiSortArray[i];
		  aiSortArray[i] = aiSortArray[j];
		  aiSortArray[j] = iTmp;
		}
      }
    }

    /***********************************************
     * draw traces
     ***********************************************/
     if(bTrace)
       if(SPLITSCREEN_FACTOR >0)
         DrawFlarmObjectTrace(Surface, fScaleFact,&sTopDia);

    /***********************************************
     * FLARM object loop
     ***********************************************/
bool bCenter = false;
Surface.SelectObject(hDrawPen);
if(SPLITSCREEN_FACTOR >0)
{
  for (int j=0; j<nEntrys; j++)
  {
    int i = aiSortArray[j];
    LKASSERT(i>=0 && i<FLARM_MAX_TRAFFIC);
    {
	  /*************************************************************************
	   * calculate positions
	   *************************************************************************/
	  fx = asFLARMPos[i].fx;
	  fy = asFLARMPos[i].fy;
	  fFlarmAlt = asFLARMPos[i].fAlt;
	  int x  = DistanceToX(fx,  &sTopDia);
	  int y  = HeightToY  (fy,  &sTopDia);
	  PositionTopView[i].left   = x - iTouchAreaSize;
	  PositionTopView[i].right  = x + iTouchAreaSize;
	  PositionTopView[i].top    = y - iTouchAreaSize;
	  PositionTopView[i].bottom = y + iTouchAreaSize;
	  TextInBoxMode_t displaymode = {};
	  displaymode.NoSetFont = true;
	  displaymode.Border = true;


      if(fx > sTopDia.fXMin )  /* sing sight ? */
      if(fx < sTopDia.fXMax )
      if(fy < sTopDia.fYMax )
	  if(fy > sTopDia.fYMin )
	#if 0
	// These would cause topview not to show all objects outside vertical scale
	  if(fFlarmAlt < sDia.fYMax )
	  if(fFlarmAlt > sDia.fYMin )
	#endif
	  {
	    /***********************************************
	     * draw center aircraft if first time above
	     ***********************************************/
	    if(bCenter == false)
	    {
	      if(fFlarmAlt > 0 )
	      {
	        bCenter = true;
		    Surface.SelectObject(hDrawBrush);
		    Surface.SelectObject(hDrawPen);
			PolygonRotateShift(AircraftTop, NUMAIRCRAFTPTS, x_middle, y_middle,RADAR_TURN+Planebrg);
		    Surface.Polygon(AircraftTop,NUMAIRCRAFTPTS);
	      }
	    }
	    /*************************************************************************
	     * calculate climb color
	     *************************************************************************/
	    double fInteg30 = LKTraffic[i].Average30s;
	    int iVarioIdx = (int)(2*fInteg30-0.5)+NO_VARIO_COLORS/2;
	    if(iVarioIdx < 0) iVarioIdx =0;
	    if(iVarioIdx >= NO_VARIO_COLORS) iVarioIdx =NO_VARIO_COLORS-1;
	    Surface.SelectObject(*variobrush[iVarioIdx]);
	    Surface.SelectObject(hDrawPen);

	    /*************************************************************************
	     * draw side view
	     *************************************************************************/
	    switch (LKTraffic[i].Status) { // 100321
		  case LKT_GHOST:
			Surface.Rectangle(x-iRectangleSize, y-iRectangleSize,x+iRectangleSize, y+iRectangleSize);
			break;
		  case LKT_ZOMBIE:
			Surface.DrawCircle(x, y, iCircleSize, rct, true );
			break;
		  default:
			POINT Triangle[5] = {Arrow[0],Arrow[1],Arrow[2],Arrow[3],Arrow[4]};
			PolygonRotateShift(Triangle, 5, x, y, AngleLimit360( asFLARMPos[i].fFlarmBearing ));
			Surface.Polygon(Triangle,5);

		    /*************************************************************************
		     * draw label
		     *************************************************************************/
		   _stprintf(lbuffer,_T("%3.1f"),LIFTMODIFY*LKTraffic[i].Average30s);

		    Surface.SetBackgroundTransparent();
		    if (_tcslen(lbuffer)>0) {
		      SIZE tsize;
              Surface.GetTextSize(lbuffer, &tsize);
			  TextInBox(Surface, &rct, lbuffer,x+tscaler, y+tsize.cy/4, &displaymode, false);
            }
			break;
	      }
	    /*********************************************
	     * draw lines to target if target selected
	     */
	      if(LKTraffic[i].Locked)
	      {
                    #ifdef NO_DASH_LINES
		    Surface.DrawLine(PEN_SOLID,2, (POINT){x_middle, y_middle},(POINT){ x, y} ,rgb_targetlinecol, rct );
                    #else
		    Surface.DrawDashLine(4, (POINT){x_middle, y_middle},(POINT){ x, y} ,rgb_targetlinecol, rct );
                    #endif
	      }
	    }
      }
    }
  }
  /***********************************************
   * draw center aircraft if highest (was never drawn)
   ***********************************************/
  if(SPLITSCREEN_FACTOR >0)
  {
    if(bCenter == false)
    {
      Surface.SelectObject(hDrawBrush);
      Surface.SelectObject(hDrawPen);
      PolygonRotateShift(AircraftTop, NUMAIRCRAFTPTS, x_middle, y_middle,RADAR_TURN+Planebrg);
      Surface.Polygon(AircraftTop,NUMAIRCRAFTPTS);
    }
  }

  /****************************************************************
   * clear background
   ****************************************************************/
  if(!bInvCol)
    Surface.SelectObject(LKBrush_White);
  else
    Surface.SelectObject(LKBrush_Black);
  Surface.SetTextColor(rgbDrawColor);
  Surface.Rectangle(rc.left , rc.bottom+5 ,rc.right, rc.top);
  Surface.SelectObject(LKBrush_Hollow);
  DrawXGrid(Surface, rc34, xtick/DISTANCEMODIFY, xtick, 0,TEXT_ABOVE_LEFT, rgbGridColor,  &sDia, text);


  /*********************************************************************************
   * Draw Y Grid
   *********************************************************************************/
  if(GPSalt < HEIGHT_RANGE)
    fMaxHeight = HEIGHT_RANGE;
  else
	fMaxHeight = GPSalt;
  fMinHeight = GPSalt;
  for (int i=0; i<FLARM_MAX_TRAFFIC; i++)
    if(LKTraffic[i].Status != LKT_EMPTY)
    {
	  fMaxHeight = max (fMaxHeight, LKTraffic[i].Altitude);
	  fMinHeight = min (fMinHeight, LKTraffic[i].Altitude);
    }



  if(HEIGHT_RANGE* fHeigtScaleFact > 4000.0 )
	  fHeigtScaleFact /=ZOOMFACTOR;

  if(HEIGHT_RANGE* fHeigtScaleFact < 100.0 )
	  fHeigtScaleFact *=ZOOMFACTOR;


  sDia.fYMin *= fHeigtScaleFact;
  sDia.fYMin = max(-GPSalt, -HEIGHT_RANGE*fHeigtScaleFact);
  sDia.fYMax =HEIGHT_RANGE* fHeigtScaleFact;



  double  ytick = 10.0;
  double  fHeight = (sDia.fYMax-sDia.fYMin);
  if (fHeight >50.0) ytick = 50.0;
  if (fHeight >100.0) ytick = 100.0;
  if (fHeight >500.0) ytick = 200.0;
  if (fHeight >1000.0) ytick = 500.0;
  if (fHeight >2000.0) ytick = 1000.0;
  if (fHeight >4000.0) ytick = 2000.0;
  if (fHeight >8000.0) ytick = 4000.0;
  if(Units::GetUserAltitudeUnit() == unFeet)
	 ytick = ytick * 4.0;
 // sDia.rc = rc34;
  if(bSideview)
  {
    _stprintf(text, TEXT("%s"),Units::GetUnitName(Units::GetUserAltitudeUnit()));
    DrawYGrid(Surface, rc, ytick/ALTITUDEMODIFY,ytick, 0,TEXT_UNDER_RIGHT ,rgbGridColor,  &sDia, text);
  }

/*************************************************************************
 * sideview
 *************************************************************************/
  for (int i=0; i < nEntrys; i++){
    for(int j=i+1; j < nEntrys; j++) {
	  LKASSERT(i>=0 && i<FLARM_MAX_TRAFFIC);
	  LKASSERT(aiSortArray[i]>=0 && aiSortArray[i]<FLARM_MAX_TRAFFIC);
	  if(asFLARMPos[aiSortArray[i]].fy  < asFLARMPos[aiSortArray[j]].fy )
	  {
	    iTmp = aiSortArray[i];
	    LKASSERT(j<FLARM_MAX_TRAFFIC);
	    aiSortArray[i] = aiSortArray[j];
	    aiSortArray[j] = iTmp;
	  }
    }
  }
/***********************************************
 * FLARM object loop
 ***********************************************/
if(bSideview)
{
  bCenter = false;
  for (int j=0; j<nEntrys; j++)
  {
    LKASSERT(j<FLARM_MAX_TRAFFIC);
    int i = aiSortArray[j];
    LKASSERT(i>=0 && i<FLARM_MAX_TRAFFIC);

	/*************************************************************************
	 * calculate positions
	 *************************************************************************/
	fx = asFLARMPos[i].fx;
	fy = asFLARMPos[i].fy;
	fFlarmAlt = asFLARMPos[i].fAlt;
	int x  = DistanceToX(fx, &sTopDia);
	int hy = HeightToY  (fFlarmAlt, &sDia);
	PositionSideView[i].left   = x  - iTouchAreaSize;
	PositionSideView[i].right  = x  + iTouchAreaSize;
	PositionSideView[i].top    = hy - iTouchAreaSize;
	PositionSideView[i].bottom = hy + iTouchAreaSize;
	TextInBoxMode_t displaymode = {};
	displaymode.NoSetFont = true;
	displaymode.Border = true;
    if(fx > sTopDia.fXMin )  /* in sight ? */
    if(fx < sTopDia.fXMax )
	if(fFlarmAlt < sDia.fYMax )
	if(fFlarmAlt > sDia.fYMin )
	{
	  if(bCenter == false)
		if(fy < 0 )
		{
		  bCenter = true;
		  Surface.SelectObject(hDrawBrush);
		  Surface.SelectObject(hDrawPen);
		  RenderFlarmPlaneSideview(Surface, rc,0 , 0,RADAR_TURN+Planebrg, &sDia , fPlaneSize);
		}
	  /*************************************************************************
	   * get the climb color
	   *************************************************************************/
	  LKASSERT(asFLARMPos[i].iColorIdx>=0 && asFLARMPos[i].iColorIdx<NO_VARIO_COLORS);
	  Surface.SelectObject(*variobrush[asFLARMPos[i].iColorIdx]);
	  Surface.SelectObject(hDrawPen);
	  /*************************************************************************
	   * draw side view
	   *************************************************************************/
	  switch (LKTraffic[i].Status) { // 100321
		case LKT_GHOST:
			Surface.Rectangle(x-iRectangleSize,hy-iRectangleSize,x+iRectangleSize,hy+iRectangleSize);
			break;
		case LKT_ZOMBIE:
			Surface.DrawCircle(x, hy, iCircleSize, rc, true );
			break;
		default:
			RenderFlarmPlaneSideview(Surface,   rc, fx,  fFlarmAlt, asFLARMPos[i].fFlarmBearing , &sDia , fPlaneSize/*1.0 - cos(fDistBearing*DEG_TO_RAD)/4*/);
			break;
	  }
	  _tcscpy(lbuffer,_T(""));
	  if (LKTraffic[i].Cn[0]!=_T('?')) { // 100322
	    _stprintf(lbuffer,_T("%s: %s"),asFLARMPos[i].szGliderType,LKTraffic[i].Cn);
	  }


	  SIZE tsize;
	  Surface.SetBackgroundTransparent();
	  Surface.GetTextSize(lbuffer, &tsize);
	  if (_tcslen(lbuffer)>0)
		TextInBox(Surface, &rc, lbuffer, x+tscaler,  hy+tsize.cy/4, &displaymode, false);
	  /*********************************************
	   * draw lines to target if target selected
	   */
	  if(LKTraffic[i].Locked)
	  {
		int  h0 = HeightToY(0,&sDia);
                #ifdef NO_DASH_LINES
		Surface.DrawLine(PEN_SOLID, 2, (POINT){x_middle,       h0},(POINT){ x, hy} ,rgb_targetlinecol, rc );
                #else
		Surface.DrawDashLine(4, (POINT){x_middle,       h0},(POINT){ x, hy} ,rgb_targetlinecol, rc );
                #endif
	  }
	}
  }
  /*************************************************************************
   * draw own plane position
   *************************************************************************/
  Surface.SelectObject(hDrawBrush);
  #ifdef OWNPOS
  OwnPosSideView.left   = x_middle-iTouchAreaSize;
  OwnPosSideView.right  = x_middle+iTouchAreaSize;
  OwnPosSideView.top    = HeightToY(0,&sDia)-iTouchAreaSize;
  OwnPosSideView.bottom = HeightToY(0,&sDia)+iTouchAreaSize;
  #endif

  if(!bCenter)
    RenderFlarmPlaneSideview( Surface, rc,0 , 0,RADAR_TURN+Planebrg, &sDia , fPlaneSize);

  /*****************************************
   * draw sideview frame
   *****************************************/
  Surface.SelectObject(hGreenPen);
  Surface.SelectObject(LKBrush_Hollow);
  Surface.Rectangle(rc.left,rc.top,rc.right,rc.bottom+4);
}

  const LKIcon* pBmpTemp = NULL;

  /********************************************************
   * draw trace icon
   ********************************************************/
  switch(bTrace)
  {
    default:
    case 0:  pBmpTemp = &hNoTrace; break; //  no trace
    case 1:  pBmpTemp = &hFullTrace; break; //  climb/sink trace
    case 2:  pBmpTemp = &hClimbTrace; break; //  climb trace
  }
  if(pBmpTemp) {
    pBmpTemp->Draw(Surface, rci.left+NIBLSCALE(5),rci.top+TOPLIMITER, NIBLSCALE(22), NIBLSCALE(22));
  }


  /********************************************************
   * draw head up/right icon
   ********************************************************/
  switch(iFlarmDirection)
  {
    default:
    case 0:  pBmpTemp = &hHeadUp ; break; //     "Head Up"
    case 1:  pBmpTemp = &hNorthUp; break; //      Head Right"
    case 2:  pBmpTemp = &hHeadRight ; break; //     "Head Up"
  }
  if(pBmpTemp) {
     pBmpTemp->Draw(Surface, rci.right-NIBLSCALE(27),	rci.top+TOPLIMITER,	NIBLSCALE(22), NIBLSCALE(22));
  }

  if(bHeightScale)
    DrawSelectionFrame(Surface,  rc);
#ifdef TOP_SELECTION_FRAME
  else
	DrawSelectionFrame(hdc,  rci);
#endif


Surface.SelectObject(hfOldFont);
Surface.SelectObject(hOldPen);
Surface.SelectObject(hOldBrush);
}




int MapWindow::DrawFlarmObjectTrace(LKSurface& Surface, double fZoom,DiagrammStruct* pDia)
{
double GPSlat = DrawInfo.Latitude;
double GPSlon = DrawInfo.Longitude;
//double GPSalt = DrawInfo.Altitude;
double GPSbrg = DrawInfo.TrackBearing;
//double Planebrg = 0.0;
double fDistBearing;
double fFlarmDist;

//double fAlt;
POINT Pnt;
int i;
int iTo= DrawInfo.FLARMTRACE_iLastPtr;
int iIdx = DrawInfo.FLARMTRACE_iLastPtr;
int iCnt = 0;
if(fZoom  < 0.05)
 return 0;

if( DrawInfo.FLARMTRACE_bBuffFull)
{
  iTo  = MAX_FLARM_TRACES;
}
if(iFlarmDirection == 2)
{
	//Planebrg = GPSbrg;
	GPSbrg =0.0;
}

int iStep =(int)  (fZoom *3.0 / (double)GC_TRACE_TIME_SKIP);
if (iStep < 1)
  iStep = 1;
iStep = 1;

    PeriodClock StartTime;
    StartTime.Update();

	for(i= 0; i < iTo; i=i+iStep)
	{
	  LKASSERT(iIdx>=0 && iIdx<MAX_FLARM_TRACES);
      DistanceBearing( GPSlat, GPSlon, DrawInfo.FLARM_RingBuf[iIdx].fLat ,DrawInfo.FLARM_RingBuf[iIdx].fLon, &fFlarmDist, &fDistBearing);

	  fDistBearing = ( fDistBearing - GPSbrg + RADAR_TURN);

	  Pnt.x  = DistanceToX(fFlarmDist * sin(fDistBearing*DEG_TO_RAD), pDia);
	  Pnt.y  = HeightToY  (fFlarmDist * cos(fDistBearing*DEG_TO_RAD), pDia);

      if(PtInRect(&pDia->rc, Pnt)) {
        if((bTrace == IM_POS_TRACE_ONLY) && (DrawInfo.FLARM_RingBuf[iIdx].iColorIdx <(NO_VARIO_COLORS/2)))
          ; // do nothing (skip drawing if neg vario)!!
        else
        {
          LKASSERT(DrawInfo.FLARM_RingBuf[iIdx].iColorIdx>=0 && DrawInfo.FLARM_RingBuf[iIdx].iColorIdx<NO_VARIO_COLORS);

          const LKBrush* pBrush = variobrush[DrawInfo.FLARM_RingBuf[iIdx].iColorIdx];

          const PixelRect rcDot = {
              Pnt.x-iTraceDotSize,
              Pnt.y-iTraceDotSize,
              Pnt.x+iTraceDotSize,
              Pnt.y+iTraceDotSize
          };

          Surface.FillRect(&rcDot, *pBrush);
          iCnt++;
        }
	  }
	  iIdx-=iStep ;  /* draw backward to cut the oldest trace parts in case the drawing time exceeds */
	  if(iIdx < 0) {
		iIdx += MAX_FLARM_TRACES;
	  }
      /************************************************************************
       * check drawing timeout (350m)
       */
	  if(StartTime.Check(350)) {
        /* drawing still took less than 350ms */
        /* fast exit on timeout               */
        i = iTo;
      }
	}

return iCnt;
}


#ifndef FLARM_MS
void MapWindow::DrawFlarmPicto(LKSurface& Surface, const RECT& rc, FLARM_TRAFFIC* pTraf) {
}
#else
//
// THIS FUNCTION IS NOT THREADSAFE
//
void MapWindow::DrawFlarmPicto(LKSurface& Surface, const RECT& rc, FLARM_TRAFFIC* pTraf)
{
	static POINT Arrow[5];
int cx = rc.right-rc.left;
int cy = rc.bottom-rc.top;
int x = rc.left + cx/2;
int y = rc.top + cy/2;
double fInteg30 =  pTraf->Average30s;
int iRectangleSize = cy/5;
int iCircleSize    = cy/5;
static double zoomfact = (double)cy/NIBLSCALE(18);
//if (DoInit[MDI_DRAWFLARMTRAFFIC])
{
	Arrow[0].x = (long)(-4.0*zoomfact);
	Arrow[0].y = (long) (5.0*zoomfact);
	Arrow[1].x = (long) (0.0*zoomfact);
	Arrow[1].y = (long) (-6.0*zoomfact);
	Arrow[2].x = (long) (4.0*zoomfact);
	Arrow[2].y = (long) (5.0*zoomfact);
	Arrow[3].x = (long) (0.0*zoomfact);
	Arrow[3].y = (long) (2.0*zoomfact);
	Arrow[4].x = (long) (-4.0*zoomfact);
	Arrow[4].y = (long) (5.0*zoomfact);
}

    int iVarioIdx = (int)(2*fInteg30-0.5)+NO_VARIO_COLORS/2;
    if(iVarioIdx < 0) iVarioIdx =0;
    if(iVarioIdx >= NO_VARIO_COLORS) iVarioIdx =NO_VARIO_COLORS-1;
	const auto oldb = Surface.SelectObject(*variobrush[iVarioIdx]);

	    switch (pTraf->Status) { // 100321
		  case LKT_GHOST:
			Surface.Rectangle(x-iRectangleSize, y-iRectangleSize,x+iRectangleSize, y+iRectangleSize);
			break;
		  case LKT_ZOMBIE:
			Surface.DrawCircle(x, y, iCircleSize, rc, true );
			break;
		  default:
			POINT Triangle[5] = {Arrow[0],Arrow[1],Arrow[2],Arrow[3],Arrow[4]};
			PolygonRotateShift(Triangle, 5, x, y, AngleLimit360(  pTraf->TrackBearing ));
			Surface.Polygon(Triangle,5);
	    }
		Surface.SelectObject(oldb);
}
// This is painting traffic icons on the screen.
#endif
