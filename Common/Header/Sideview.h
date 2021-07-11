/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Sideview.h,v 1.1 2012/01/17 10:35:29 root Exp root $
*/

#ifndef SIDEVIEW_H
#define SIDEVIEW_H

#define SV_BORDER_Y NIBLSCALE (10)

#define ENABLE_ALL_AS_FOR_SIDEVIEW
#define MIN_ALTITUDE 1200.0 /* maximum altitude (in m) */
#define MIN_OFFSET   500.0  /* maximum altitude offset (in m) */

//#define OFFSET_SETP 500.0

#define  MSL_SEA_DRAW
#define GC_SEA_LEVEL_TOLERANCE    100.0
#define FEET_FACTOR    3
//#define SHOW_YELLO_RED_WARNING
#define NEAR_AS_ZOOM_1000FT
#define NEAR_AS_ZOOM_1000M

// There are known as "shared multimaps" and they are sized inside Defines.h
// See NUMBER_OF_SHARED_MULTIMAPS.
#define IM_VISUALGLIDE 3
#define IM_NEAR_AS 2
#define IM_NEXT_WP 1
#define IM_HEADING 0

//
// TOPVIEW size
//
#define SIZE0 0
#define SIZE1 30
#define SIZE2 50
#define SIZE3 70
#define SIZE4 100


#define ZOOMFACTOR 1.3

#define GROUND_COLOUR	LKColor(184,152,137)
#define GROUND_TEXT_COLOUR RGB_WHITE
#define INV_GROUND_TEXT_COLOUR   LKColor(0,0,0)
#if (WINDOWSPC>0)
#define GC_NO_COLOR_STEPS  50
#else
#define GC_NO_COLOR_STEPS  25
#endif
#define SKY_SPACE_COL  LKColor(150,150,255)
#define SKY_HORIZON_COL  RGB_WHITE



#define ID_NO_LABLE    0
#define ID_SHORT_LABLE 1
#define ID_FULL_LABLE  2

#define ELV_FACT 2.2
#define MAXALTTODAY   2500.0
#define MAX_ALTITUDE   5000.0

#define NO_SIDEVIEW_PAGES 4

// VISUALGLIDE STUFF
// How many boxes on a row, max
#define MAXBSLOT 10
extern RECT Sideview_VGBox[];
extern int  Sideview_VGWpt[];
extern unsigned short Sideview_VGBox_Number;

// END OF VG STUFF



int SetSplitScreenSize(int);
int IncSideviewPage(void);
int DecSideviewPage(void);
int GetSideviewPage (void);
int SetSideviewPage (int i);

void DrawSelectionFrame(LKSurface& Surface, const RECT& rc);

void DrawWindRoseDirection(LKSurface& Surface, double fAngle, int x, int y);
void RenderSky(LKSurface& Surface, const RECT& rc, const LKColor& Col1, const LKColor& Col2 , int iSteps);
void RenderPlaneSideview(LKSurface& Surface, double fDist, double fAltitude, double brg, DiagrammStruct* psDia );

void RenderAirspaceTerrain(LKSurface& Surface, double PosLat, double PosLon,  double brg,  DiagrammStruct* psDiag );
int CalcHeightCoordinat(double fHeight,   DiagrammStruct* psDia);
int CalcHeightCoordinatOutbound(double fHeight,   DiagrammStruct* psDia);
int CalcDistanceCoordinat(double fDist,   DiagrammStruct* psDia);

void SetMMNorthUp( int iPage, int bVal);
int GetMMNorthUp( int iPage);
#endif
