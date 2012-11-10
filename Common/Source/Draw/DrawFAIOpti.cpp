/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "AATDistance.h"
#include "RGB.h"
#include "ContestMgr.h"
#include "Defines.h"

#define STEPS 6
#define N_PLOYGON (3*STEPS)
#define NO_CACHE_SECTORS 6
#define MAX_GRID_TEXT_LEN 20

typedef struct
{
	double fLon;
	double fLat;
}fPOINT ;

fPOINT apFAISectors[NO_CACHE_SECTORS][N_PLOYGON];
/*
fPOINT apFAISectorGrids[NO_CACHE_SECTORS][STEPS];
TCHAR  apFAISectorGridText[NO_CACHE_SECTORS][MAX_GRID_TEXT_LEN];
*/
int DrawFAISector(HDC hdc, fPOINT apSectorPolygon[],	COLORREF fillcolor);
int RenderFAISector (HDC hdc, const RECT rc , double lat1, double lon1, double lat2, double lon2,
		int iOpposite ,
		COLORREF fillcolor,
		BOOL bGrid,
		fPOINT apSectorPolygon[]);

extern COLORREF taskcolor;


void MapWindow::DrawFAIOptimizer(HDC hdc, RECT rc, const POINT &Orig_Aircraft)
{
  COLORREF whitecolor = RGB_WHITE;
  COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);
  HPEN oldpen = 0;
  HBRUSH oldbrush = 0;
  oldpen = (HPEN) SelectObject(hdc, hpStartFinishThick);
  oldbrush = (HBRUSH) SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
BOOL  bRecalc = false;
static int iCallCnt = 0;
if(iCallCnt++ > 10)
{
  bRecalc = true;
  iCallCnt =0;
}
/********************************************************************/
  unsigned int ui;
  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
  POINT Pt1,Pt2;
  BOOL bFAI = false;
  double fDist, fAngle;
  LockTaskData(); // protect from external task changes

    CContestMgr::CResult result = CContestMgr::Instance().Result( CContestMgr::TYPE_FAI_TRIANGLE, true);
    const CPointGPSArray &points = result.PointArray();
    unsigned int iSize = points.size();
    if(iSize > 4)
      bFAI = true;
    CContestMgr::TType sType = result.Type();

  UnlockTaskData(); // protect from external task changes
  if((sType ==  CContestMgr::TYPE_FAI_TRIANGLE) && iSize>0)
  {
    LKASSERT(iSize<100);
    for(ui=0; ui< iSize-1; ui++)
    {
      LockTaskData(); // protect from external task changes
      lat1 = points[ui].Latitude();
      lon1 = points[ui].Longitude();
      lat2 = points[ui+1].Latitude();
      lon2 = points[ui+1].Longitude();
      UnlockTaskData();

      DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);

      if((fDist > FAI_MIN_DISTANCE_THRESHOLD) && (ui < 2))
  	  {
  		COLORREF rgbCol = RGB_BLUE;
  		switch(ui)
  		{
  		  case 0: rgbCol = RGB_LIGHTYELLOW; break;
  		  case 1: rgbCol = RGB_LIGHTCYAN  ; break;
  		  case 2: rgbCol = RGB_LIGHTGREEN ; break;
  		  default:
  		  break;
  		}

  		if((ui < NO_CACHE_SECTORS) && (bRecalc))
  		{
  	 	  RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, 1, 0, 0, apFAISectors[ui] );
  		  RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, 0, 0, 0, apFAISectors[ui+1] );
  		}
  		DrawFAISector( hdc,  apFAISectors[ui],	 0);
  		DrawFAISector( hdc,  apFAISectors[ui+1], 0);
  	  }
    }


    for(ui=0; ui< iSize-1; ui++)
    {
      LockTaskData();
      lat1 = points[ui].Latitude();
      lon1 = points[ui].Longitude();
      lat2 = points[ui+1].Latitude();
      lon2 = points[ui+1].Longitude();
      UnlockTaskData();
      DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);
  	  MapWindow::LatLon2Screen(lon1, lat1,  Pt1);
  	  MapWindow::LatLon2Screen(lon2, lat2,  Pt2);

#ifdef DRAW_TRIANGLE
      DrawDashLine(hdc, NIBLSCALE(1), Pt1, Pt2, taskcolor, rc);
	  if(result.Distance()> DISTANCE_THRESHOLD)
	  {
		HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
		TCHAR text[180];
		SIZE tsize;
		_stprintf(text, TEXT("%3.1f%%"), (fDist/result.Distance()*100.0));
		GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
	    SetTextColor(hdc, RGB_DARKGREY);
		ExtTextOut(hdc, Pt1.x+(Pt2.x-Pt1.x)/2-tsize.cx/2, Pt1.y+(Pt2.y-Pt1.y)/2 , ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
		SetTextColor(hdc, RGB_BLUE);
		SelectObject(hdc,(HFONT)  hfOld);
	  }
#endif

   	  if ((ISPARAGLIDER) &&  bFAI && (ui == 0))
   	  {
        double lat, lon;
        HPEN hpSectorPen  = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_DARKGREY );
        HPEN hpOldPen     = (HPEN)SelectObject(hdc, hpSectorPen);
	    FindLatitudeLongitude(lat1, lon1, 0 , result.Distance()/5, &lat, &lon);/* 5%*/
	    int iRadius = (int)((lat-lat1)*zoom.DrawScale());

	    Circle(hdc, Pt1.x, Pt1.y, iRadius  , rc, true ,  false);/* 20% */
	    Circle(hdc, Pt1.x, Pt1.y, iRadius/4, rc, true ,  false);/* 5%  */
	    SelectObject(hdc, hpOldPen);
	    DeleteObject(hpSectorPen);
      }
    }
  }



    
/********************************************************************/
    // restore original color
    SetTextColor(hDCTemp, origcolor);
    SelectObject(hdc, oldpen);
    SelectObject(hdc, oldbrush);
}



int RenderFAISector (HDC hdc, const RECT rc , double lat1, double lon1, double lat2, double lon2,
		int iOpposite ,
		COLORREF fillcolor,
		BOOL bGrid,
		fPOINT apSectorPolygon[])
{
float fFAI_Percentage = FAI_NORMAL_PERCENTAGE;

double fDist_a, fDist_b, fDist_c, fAngle;
int i;

int iPolyPtr=0;
double lat_d,lon_d;
double alpha, fDistTri, cos_alpha=0;


DistanceBearing(lat1, lon1, lat2, lon2, &fDist_c, &fAngle);


double fDistMax = fDist_c/fFAI_Percentage;
double fDistMin = fDist_c/(1.0-2.0*fFAI_Percentage);
double fDelta_Dist = 2.0* fDist_c*fFAI_Percentage / (double)(STEPS);

double dir = -1.0;



  if(fDistMax < FAI_BIG_THRESHOLD)
  {
    fDistMax = fDist_c/FAI_NORMAL_PERCENTAGE;
    fDistMin = fDist_c/(1.0-2.0*FAI_NORMAL_PERCENTAGE);
  }

  if (iOpposite >0)
  {
	dir = 1.0;
  }


  /********************************************************************
   * calc right leg
   ********************************************************************/
  fDelta_Dist =(fDistMax-fDistMin)/ (double)(STEPS);
  fDistTri = fDistMin;
  if(fDistTri < FAI_BIG_THRESHOLD)
  	fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
  else
	fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
  fDist_a = fDistMin * fFAI_Percentage;
  fDist_b = fDistMin * fFAI_Percentage;
  for(i =0 ;i < STEPS; i++)
  {
	LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);


    apSectorPolygon[iPolyPtr].fLon   = lon_d;
    apSectorPolygon[iPolyPtr++].fLat = lat_d;

    fDistTri += fDelta_Dist;
    if(fDistTri < FAI_BIG_THRESHOLD)
      fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
    else
      fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
    fDist_a = fFAI_Percentage * fDistTri;
	fDist_b = fDistTri - fDist_a - fDist_c;
  }

  /********************************************************************
   * calc top leg
   ********************************************************************/
  if(fDistMax < FAI_BIG_THRESHOLD)
    fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
  else
    fFAI_Percentage = FAI_NORMAL_PERCENTAGE;

  fDelta_Dist =  (fDistMax*(1.0-3.0*fFAI_Percentage)) / (double)(STEPS);
  fDist_a = fDist_c;
  fDist_b = fDistMax - fDist_a - fDist_c;
  for(i =0 ;i < STEPS; i++)
  {
	LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

    apSectorPolygon[iPolyPtr].fLon   = lon_d;
    apSectorPolygon[iPolyPtr++].fLat = lat_d;

	fDist_a += fDelta_Dist;
	fDist_b = fDistMax - fDist_a - fDist_c;
  }

  /********************************************************************
   * calc left leg
   ********************************************************************/
  fDelta_Dist =(fDistMax-fDistMin)/ (double)(STEPS);
  fDistTri = fDistMax;

  fDist_b = fDistMax * fFAI_Percentage;
  fDist_a = fDistTri - fDist_b - fDist_c;
  for(i =0 ;i < STEPS; i++)
  {
    LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

    apSectorPolygon[iPolyPtr].fLon   = lon_d;
    apSectorPolygon[iPolyPtr++].fLat = lat_d;

    fDistTri -= fDelta_Dist;
    if(fDistTri < FAI_BIG_THRESHOLD)
      fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
    else
      fFAI_Percentage = FAI_NORMAL_PERCENTAGE;

    fDist_b = fFAI_Percentage * fDistTri;
	fDist_a = fDistTri - fDist_b - fDist_c;
  }




  /********************************************************************
   * calc round leg grid
   ********************************************************************/
if(bGrid)
{
HPEN  hpSectorPen  = (HPEN)CreatePen(PS_SOLID, (2), RGB_DARKGREY );
  SelectObject(hdc, hpSectorPen);

  double fTic= 1/DISTANCEMODIFY;
  if(fDist_c > 5/DISTANCEMODIFY)   fTic = 10/DISTANCEMODIFY;
  if(fDist_c > 50/DISTANCEMODIFY)  fTic = 25/DISTANCEMODIFY;
  if(fDist_c > 100/DISTANCEMODIFY) fTic = 50/DISTANCEMODIFY;
  if(fDist_c > 200/DISTANCEMODIFY) fTic = 100/DISTANCEMODIFY;
  if(fDist_c > 500/DISTANCEMODIFY) fTic = 250/DISTANCEMODIFY;
  POINT line[2];
  BOOL bFirstUnit = true;
  fDistTri = ((int)(fDistMin/fTic)+1) * fTic ;
  HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
  while(fDistTri < fDistMax)
  {
    fDelta_Dist =  (fDistTri-fDistMin)*(1.0-2.0*fFAI_Percentage) / (double)(STEPS-1);
    fDist_a = fDistTri*fFAI_Percentage;
    fDist_b = fDistTri - fDist_a - fDist_c;
    for(i =0 ;i < STEPS; i++)
    {
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

	  MapWindow::LatLon2Screen(lon_d, lat_d,  line[0]);


      if(i> 0)
	    Polyline(hdc, line, 2);
      line[1] =  line[0];
	  fDist_a += fDelta_Dist;
	  fDist_b = fDistTri - fDist_a - fDist_c;
    }
    TCHAR text[180]; SIZE tsize;
    if(bFirstUnit)
      _stprintf(text, TEXT("%i%s"), (int)(fDistTri*DISTANCEMODIFY), Units::GetUnitName(Units::GetUserDistanceUnit()));
    else
      _stprintf(text, TEXT("%i"), (int)(fDistTri*DISTANCEMODIFY));
    bFirstUnit = false;
    GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
    SetTextColor(hdc, RGB_DARKGREY);
    ExtTextOut(hdc, line[0].x, line[0].y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

    fDistTri+=fTic;
  }
  SelectObject(hdc, hfOld);
}


return 0;
}




int DrawFAISector(HDC hdc, fPOINT apSectPolygon[],	COLORREF fillcolor)
{
	/********************************************************************
	 * draw polygon
	 ********************************************************************/
	HPEN   hpSectorPen  = (HPEN)CreatePen(PS_SOLID, IBLSCALE(2),  FAI_SECTOR_COLOR );
	HBRUSH hpSectorFill = NULL;

	HPEN hpOldPen     = (HPEN)  SelectObject(hdc, hpSectorPen);
	HBRUSH hpOldBrush;
	if (fillcolor != 0)
	{
	  hpSectorFill = (HBRUSH)CreateSolidBrush(fillcolor);
	  hpOldBrush = (HBRUSH)SelectObject(hdc, hpSectorFill);
	}
	else
	  hpOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));


	POINT apDrawPoly[N_PLOYGON];

	for(int i=0 ; i <  N_PLOYGON; i++)
	  MapWindow::LatLon2Screen(apSectPolygon[i].fLon,  apSectPolygon[i].fLat, apDrawPoly [i]);
	/********************************************/
	Polygon(hdc, apDrawPoly,N_PLOYGON);
	/********************************************/

	SelectObject(hdc, (HPEN)hpOldPen);
	SelectObject(hdc, (HBRUSH)hpOldBrush);
	DeleteObject(hpSectorPen);
	if(hpSectorFill != NULL)
	  DeleteObject(hpSectorFill);
	return 0;
}
