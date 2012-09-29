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

using std::min;
using std::max;

#define RGB_ROYAL_BLUE  RGB(18,32,139)
#define RGB_STEEL_BLUE  RGB(70,130,180)

extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];
extern COLORREF Sideview_TextColor;
extern int Sideview_iNoHandeldSpaces;



void RenderAirspaceTerrain(HDC hdc, double PosLat, double PosLon,  double brg,  DiagrammStruct* psDiag )
{
RECT rc	= psDiag->rc;
rc.bottom +=BORDER_Y;
double range =psDiag->fXMax - psDiag->fXMin; // km
double hmax = psDiag->fYMax;
double lat, lon;
int i,j;
/*
#if (WINDOWSPC>0)
  if(INVERTCOLORS)
#else
  if(ISCAR && INVERTCOLORS)
#endif
*/
if (EnableTerrain )
    RenderSky( hdc, rc, SKY_HORIZON_COL , SKY_SPACE_COL , GC_NO_COLOR_STEPS);

  FindLatitudeLongitude(PosLat, PosLon, brg  , psDiag->fXMin , &lat, &lon);
  POINT apTerrainPolygon[AIRSPACE_SCANSIZE_X+4];
  double d_lat[AIRSPACE_SCANSIZE_X];
  double d_lon[AIRSPACE_SCANSIZE_X];
  double d_h[AIRSPACE_SCANSIZE_X];
//  double dfi = 1.0/(AIRSPACE_SCANSIZE_H-1);
  double dfj = 1.0/(AIRSPACE_SCANSIZE_X-1);




#define   FRAMEWIDTH 2
  RasterTerrain::Lock(); // want most accurate rounding here
  RasterTerrain::SetTerrainRounding(0,0);
  double fj;
  for (j=0; j< AIRSPACE_SCANSIZE_X; j++)
  { // scan range
    fj = j*1.0/(AIRSPACE_SCANSIZE_X-1);
    FindLatitudeLongitude(lat, lon, brg, range*fj, &d_lat[j], &d_lon[j]);
    d_h[j] = RasterTerrain::GetTerrainHeight(d_lat[j], d_lon[j]);
    if (d_h[j] == TERRAIN_INVALID) d_h[j]=0; //@ 101027 BUGFIX
    hmax = max(hmax, d_h[j]);
  }

  RasterTerrain::Unlock();


  /********************************************************************************
   * scan line
   ********************************************************************************/
  Sideview_iNoHandeldSpaces =  CAirspaceManager::Instance().ScanAirspaceLineList(d_lat, d_lon, d_h, Sideview_pHandeled,MAX_NO_SIDE_AS); //  Sideview_pHandeled[GC_MAX_NO];
  /********************************************************************************
   * bubble sort to start with biggest airspaces
   ********************************************************************************/
  int iSizeLookupTable[MAX_NO_SIDE_AS];
  for( i = 0 ; i < Sideview_iNoHandeldSpaces ;i++)
	iSizeLookupTable[i] = i;

  for( i = 0 ; i < Sideview_iNoHandeldSpaces ;i++)
	for( j = i ; j < Sideview_iNoHandeldSpaces ;j++)
      if(Sideview_pHandeled[iSizeLookupTable[i]].iAreaSize < Sideview_pHandeled[iSizeLookupTable[j]].iAreaSize )
      {
    	int iTmp = iSizeLookupTable[i];
    	iSizeLookupTable[i] = iSizeLookupTable[j];
    	iSizeLookupTable[j] = iTmp;
      }

  /**********************************************************************************
   * transform into diagram coordinates
   **********************************************************************************/
  double dx = dfj*(rc.right-rc.left);
  int x0 = rc.left; //+BORDER_X;


  for( i = 0 ; i < Sideview_iNoHandeldSpaces ;i++)
  {
	Sideview_pHandeled[i].rc.left   = iround((Sideview_pHandeled[i].rc.left  )*dx)+x0 -FRAMEWIDTH/2;
	Sideview_pHandeled[i].rc.right  = iround((Sideview_pHandeled[i].rc.right+1)*dx)+x0+ FRAMEWIDTH/2;

	Sideview_pHandeled[i].rc.bottom  = CalcHeightCoordinat((double)  Sideview_pHandeled[i].rc.bottom,  psDiag);//+FRAMEWIDTH/2;
	Sideview_pHandeled[i].rc.top     = CalcHeightCoordinat((double)  Sideview_pHandeled[i].rc.top,     psDiag)-FRAMEWIDTH/2;

	Sideview_pHandeled[i].iMaxBase  = Sideview_pHandeled[i].rc.bottom ;
	Sideview_pHandeled[i].iMinTop   = Sideview_pHandeled[i].rc.top ;

	int iN = Sideview_pHandeled[i].iNoPolyPts;
	if(Sideview_pHandeled[i].bRectAllowed == false)
      for(j =0 ; j < iN  ; j++)
      {
        Sideview_pHandeled[i].apPolygon[j].x = iround(Sideview_pHandeled[i].apPolygon[j].x * dx)+x0;
        Sideview_pHandeled[i].apPolygon[j].y = CalcHeightCoordinat((double)   Sideview_pHandeled[i].apPolygon[j].y, psDiag);
        if(j != iN-1)
        {
          if(( j < iN /2) )
            Sideview_pHandeled[i].iMaxBase = min ((long)Sideview_pHandeled[i].iMaxBase ,(long)Sideview_pHandeled[i].apPolygon[j].y);
          else
            Sideview_pHandeled[i].iMinTop  = max ((long)Sideview_pHandeled[i].iMinTop , (long)Sideview_pHandeled[i].apPolygon[j].y);
        }
      }
  }
  /**********************************************************************************
   * draw airspaces
   **********************************************************************************/
  HPEN mpen = (HPEN)CreatePen(PS_NULL, 0, RGB(0xf0,0xf0,0xb0));
  HPEN oldpen = (HPEN)SelectObject(hdc, (HPEN)NULL);
  _TCHAR text [80];
  SIZE tsize;

  for (int m=0 ; m < Sideview_iNoHandeldSpaces; m++)
  {
	int iSizeIdx =  iSizeLookupTable[m];

	  int  type = Sideview_pHandeled[iSizeIdx].iType;
	  RECT rcd  = Sideview_pHandeled[iSizeIdx].rc;
	  double fFrameColFact;
	  if(Sideview_pHandeled[iSizeIdx].bEnabled)
	  {
		SelectObject(hdc, MapWindow::GetAirspaceBrushByClass(type));
		SetTextColor(hdc, MapWindow::GetAirspaceColourByClass(type));

		fFrameColFact = 0.8;
	  }
	  else
	  {
		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		SetTextColor(hdc, RGB_GGREY);
		fFrameColFact = 1.2;
	  }
	  if(INVERTCOLORS)
		fFrameColFact *= 0.8;
	  else
		fFrameColFact *= 1.2;
	  long lColor = ChangeBrightness( MapWindow::GetAirspaceColourByClass(type), fFrameColFact);
	  HPEN mpen =(HPEN)CreatePen(PS_SOLID,FRAMEWIDTH,lColor);
	  HPEN oldpen = (HPEN)SelectObject(hdc, (HPEN)mpen);

	  if(Sideview_pHandeled[iSizeIdx].bRectAllowed == true)
		Rectangle(hdc,rcd.left+1,rcd.top,rcd.right,rcd.bottom);
	  else
	    Polygon(hdc,Sideview_pHandeled[iSizeIdx].apPolygon ,Sideview_pHandeled[iSizeIdx].iNoPolyPts );

	  SelectObject(hdc, (HPEN)oldpen);
	  DeleteObject (mpen);

#define LINE_DIFF 2

  /************************************/
//  SetBkMode(hdc, OPAQUE);  /* OPAQUE may be better readable but verry ugly
  /************************************/
  if(Sideview_pHandeled[iSizeIdx].bEnabled)
	SetTextColor(hdc, Sideview_TextColor); // RGB_MENUTITLEFG
  else
	SetTextColor(hdc, RGB_GGREY);

  /***********************************************
   * build view overlap for centering text
   ***********************************************/
  rcd.bottom = min(  rcd.bottom, (long)Sideview_pHandeled[iSizeIdx].iMaxBase  );
  rcd.top    = max(  rcd.top   , (long)Sideview_pHandeled[iSizeIdx].iMinTop );

  rcd.left   = max(rcd.left   ,rc.left);
  rcd.right  = min(rcd.right  ,rc.right);
  rcd.bottom = min(rcd.bottom ,rc.bottom);
  rcd.top    = max(rcd.top    ,rc.top);


/*
SelectObject(hdc, GetStockObject(WHITE_PEN));
Rectangle(hdc,rcd.left+1,rcd.top,rcd.right,rcd.bottom);
*/

  LK_tcsncpy(text, Sideview_pHandeled[iSizeIdx].szAS_Name,NAME_SIZE-1/* sizeof(text)/sizeof(text[0])*/);
  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);

  int x ;
  int y = rcd.bottom  + (rcd.top - rcd.bottom)/2 -tsize.cy;
  int iTextheight =  tsize.cy;

  if ( (tsize.cx < (rcd.right-rcd.left)) &&  (iTextheight < (rcd.bottom-rcd.top) ) )
  {
	x = rcd.left + (rcd.right - rcd.left - tsize.cx)/2;
	ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
	y = rcd.bottom  + (rcd.top   - rcd.bottom )/2;
	iTextheight = 2*tsize.cy;
  }

  LK_tcsncpy(text, CAirspaceManager::Instance().GetAirspaceTypeShortText( Sideview_pHandeled[iSizeIdx].iType), NAME_SIZE);
  GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
  x = rcd.left + (rcd.right - rcd.left)/2;
  if ( (tsize.cx < (rcd.right-rcd.left)) &&  (iTextheight < (rcd.bottom-rcd.top)) )
  {
	x = rcd.left + (rcd.right - rcd.left - tsize.cx)/2; // - NIBLSCALE(5);
	ExtTextOut(hdc, x, y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
  }
}

if(1==0)
  for (int m=0 ; m < Sideview_iNoHandeldSpaces; m++)
  {

	int iSizeIdx =  iSizeLookupTable[m];
	int  type = Sideview_pHandeled[iSizeIdx].iType;
	RECT rcd  = Sideview_pHandeled[iSizeIdx].rc;
	//	NULL_BRUSH
	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	double fFrameColFact = 1.0;
	if(INVERTCOLORS)
	  fFrameColFact = 0.8;
	else
	  fFrameColFact = 1.2;
	long lDisabledColor = ChangeBrightness( MapWindow::GetAirspaceColourByClass(type), fFrameColFact);

	HPEN Newpen = (HPEN)CreatePen(PS_SOLID, 3, lDisabledColor);
	HPEN Oldpen = (HPEN)SelectObject(hdc, Newpen);

	if(Sideview_pHandeled[iSizeIdx].bRectAllowed == true)
	  Rectangle(hdc,rcd.left+LINE_DIFF ,rcd.top - LINE_DIFF,rcd.right-LINE_DIFF,rcd.bottom+LINE_DIFF-1);
	else
	{
	  Polygon(hdc,Sideview_pHandeled[iSizeIdx].apPolygon ,Sideview_pHandeled[iSizeIdx].iNoPolyPts );
	}

#if TESTBENCH > 0
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	Rectangle(hdc,rcd.left+1,rcd.top,rcd.right,rcd.bottom);
#endif
	SelectObject(hdc, Oldpen);
	DeleteObject (Newpen);

  }


  /*************************************************************
   * draw ground
   *************************************************************/
  int iBottom = rc.bottom;
  // draw ground

  HPEN   hpHorizonGround;
  HBRUSH hbHorizonGround;

  hpHorizonGround = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1), RGB_BLACK /*GROUND_COLOUR*/);
  hbHorizonGround = (HBRUSH)CreateSolidBrush(GROUND_COLOUR);
  SelectObject(hdc, hpHorizonGround);
  SelectObject(hdc, hbHorizonGround);


#ifdef MSL_SEA_DRAW
  // draw sea
  if(psDiag->fYMin < GC_SEA_LEVEL_TOLERANCE)
  {
	RECT sea= {rc.left,rc.bottom - BORDER_Y,rc.right,rc.bottom-2*BORDER_Y};
	RenderSky( hdc,   sea, RGB_STEEL_BLUE, RGB_ROYAL_BLUE  , 7);
	iBottom-=BORDER_Y;

  }
#else
  if(psDiag->fYMin < GC_SEA_LEVEL_TOLERANCE)
	Rectangle(hdc,rc.left,rc.bottom,rc.right,rc.bottom-BORDER_Y);
#endif

  /*********************************************************************
   * draw terrain
   *********************************************************************/
  SelectObject(hdc, hpHorizonGround);
  for (j=0; j< AIRSPACE_SCANSIZE_X; j++) { // scan range
	apTerrainPolygon[j].x = iround(j*dx)+x0;
	apTerrainPolygon[j].y = CalcHeightCoordinat(d_h[j], psDiag)+2;

  }
  apTerrainPolygon[AIRSPACE_SCANSIZE_X].x = iround(AIRSPACE_SCANSIZE_X*dx)+x0;; // x0;
  apTerrainPolygon[AIRSPACE_SCANSIZE_X].y = CalcHeightCoordinat(0, psDiag)+2 ;//iBottom;

  apTerrainPolygon[AIRSPACE_SCANSIZE_X+1].x = iround(0*dx)+x0;  //iround(j*dx)+x0;
  apTerrainPolygon[AIRSPACE_SCANSIZE_X+1].y =  CalcHeightCoordinat(0, psDiag)+2 ;//iBottom;
  Polygon(hdc, apTerrainPolygon, AIRSPACE_SCANSIZE_X+2);



  SetTextColor(hdc, Sideview_TextColor); // RGB_MENUTITLEFG
  SelectObject(hdc, (HPEN)oldpen);
  DeleteObject(mpen);
  DeleteObject(hpHorizonGround);
  DeleteObject(hbHorizonGround);


}


