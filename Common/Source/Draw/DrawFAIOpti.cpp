/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "RGB.h"
#include "ContestMgr.h"
#include "Defines.h"
#include "Topology.h"

#ifdef PNA
  #define FAI_SECTOR_STEPS 7
#else
  #define FAI_SECTOR_STEPS 15
#endif
extern COLORREF taskcolor;
int RenderFAISector (HDC hdc, const RECT rc , double lat1, double lon1, double lat2, double lon2, int iOpposite , COLORREF fillcolor);


void MapWindow::DrawFAIOptimizer(HDC hdc, RECT rc, const POINT &Orig_Aircraft)
{

  COLORREF whitecolor = RGB_WHITE;
  COLORREF origcolor = SetTextColor(hDCTemp, whitecolor);
  HPEN oldpen = 0;
  HBRUSH oldbrush = 0;
  oldpen = (HPEN) SelectObject(hdc, hpStartFinishThick);
  oldbrush = (HBRUSH) SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));


/********************************************************************/
  unsigned int ui;
  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
  BOOL bFlat = false;
  BOOL bFAI = false;
  double fDist, fAngle;
  LockTaskData(); // protect from external task changes
    bFAI =  CContestMgr::Instance().FAI();
    CContestMgr::CResult result = CContestMgr::Instance().Result( CContestMgr::TYPE_FAI_TRIANGLE, true);
    const CPointGPSArray &points = result.PointArray();
    unsigned int iSize = points.size();
    CContestMgr::TType sType = result.Type();

    double lat_CP = CContestMgr::Instance().GetClosingPoint().Latitude();
    double lon_CP = CContestMgr::Instance().GetClosingPoint().Longitude();
    double fFAIDistance = result.Distance();
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


      if((fDist > FAI_MIN_DISTANCE_THRESHOLD) && (ui < 3) && !bFlat && (fDist/ fFAIDistance  > 0.05))
  	  {
  		COLORREF rgbCol = RGB_BLUE;
  		switch(ui)
  		{
  		  case 0: rgbCol = RGB_YELLOW; break;
  		  case 1: rgbCol = RGB_CYAN  ; break;
  		  case 2: rgbCol = RGB_GREEN ; break;
  		  default:
  		  break;
  		}
  		RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, 1, rgbCol );
  		RenderFAISector ( hdc, rc, lat1, lon1, lat2, lon2, 0, rgbCol );
  	  }
      if (fFAIDistance > 0)  /* check if triangle is too flat for second sector */
        if(fDist/ fFAIDistance  > 0.45)
    	  bFlat = true;
    }
/*********************************************************/


    if(ISPARAGLIDER && bFAI)
    {
      HPEN   hpSectorPen  = (HPEN)CreatePen(PS_SOLID, IBLSCALE(2),  FAI_SECTOR_COLOR );
      HPEN hOldPen = (HPEN) SelectObject (hdc, hpSectorPen);
      POINT Pt1;
	  MapWindow::LatLon2Screen(lon_CP, lat_CP,  Pt1);
      FindLatitudeLongitude(lat1, lon1, 0 , fFAIDistance* 0.20, &lat2, &lon2); /* 1000m destination circle */
      int iRadius = (int)((lat2-lat1)*zoom.DrawScale());
      Circle(hdc, Pt1.x, Pt1.y, iRadius  , rc, true ,  false);
      FindLatitudeLongitude(lat1, lon1, 0 , 500, &lat2, &lon2); /* 1000m destination circle */
      iRadius = (int)((lat2-lat1)*zoom.DrawScale());
      Circle(hdc, Pt1.x, Pt1.y, iRadius  , rc, true ,  false);
      SelectObject (hdc, hOldPen);
      DeleteObject(hpSectorPen);
    }

/*********************************************************/

  }



    
/********************************************************************/
    // restore original color
    SetTextColor(hDCTemp, origcolor);
    SelectObject(hdc, oldpen);
    SelectObject(hdc, oldbrush);

}



int RenderFAISector (HDC hdc, const RECT rc , double lat1, double lon1, double lat2, double lon2, int iOpposite , COLORREF fillcolor)
{
float fFAI_Percentage = FAI_NORMAL_PERCENTAGE;

#define N_PLOYGON (3*FAI_SECTOR_STEPS)
double fDist_a, fDist_b, fDist_c, fAngle;
int i;

int iPolyPtr=0;
double lat_d,lon_d;
double alpha, fDistTri, cos_alpha=0;
POINT apSectorPolygon[N_PLOYGON];
POINT Pt1;
DistanceBearing(lat1, lon1, lat2, lon2, &fDist_c, &fAngle);

if(fabs(fDist_c) < 1000.0)  /* distance too short for a FAI sector */
	return -1;

LKASSERT(fFAI_Percentage>0);
double fDistMax = fDist_c/fFAI_Percentage;
LKASSERT(fFAI_Percentage!=0.5);
double fDistMin = fDist_c/(1.0-2.0*fFAI_Percentage);
double fDelta_Dist = 2.0* fDist_c*fFAI_Percentage / (double)(FAI_SECTOR_STEPS);

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
  fDelta_Dist =(fDistMax-fDistMin)/ (double)(FAI_SECTOR_STEPS);
  fDistTri = fDistMin;
  if(fDistTri < FAI_BIG_THRESHOLD)
  	fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
  else
	fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
  fDist_a = fDistMin * fFAI_Percentage;
  fDist_b = fDistMin * fFAI_Percentage;
  for(i =0 ;i < FAI_SECTOR_STEPS; i++)
  {
  	LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);


	MapWindow::LatLon2Screen(lon_d, lat_d,  Pt1);
    apSectorPolygon[iPolyPtr++] = Pt1;

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

  fDelta_Dist =  (fDistMax*(1.0-3.0*fFAI_Percentage)) / (double)(FAI_SECTOR_STEPS);
  fDist_a = fDist_c;
  fDist_b = fDistMax - fDist_a - fDist_c;
  for(i =0 ;i < FAI_SECTOR_STEPS; i++)
  {
	LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

	MapWindow::LatLon2Screen(lon_d, lat_d,  Pt1);
    apSectorPolygon[iPolyPtr++] = Pt1;

	fDist_a += fDelta_Dist;
	fDist_b = fDistMax - fDist_a - fDist_c;
  }

  /********************************************************************
   * calc left leg
   ********************************************************************/
  fDelta_Dist =(fDistMax-fDistMin)/ (double)(FAI_SECTOR_STEPS);
  fDistTri = fDistMax;

  fDist_b = fDistMax * fFAI_Percentage;
  fDist_a = fDistTri - fDist_b - fDist_c;
  for(i =0 ;i < FAI_SECTOR_STEPS; i++)
  {
    LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

	MapWindow::LatLon2Screen(lon_d, lat_d,  Pt1);
    apSectorPolygon[iPolyPtr++] = Pt1;

    fDistTri -= fDelta_Dist;
    if(fDistTri < FAI_BIG_THRESHOLD)
      fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
    else
      fFAI_Percentage = FAI_NORMAL_PERCENTAGE;

    fDist_b = fFAI_Percentage * fDistTri;
	fDist_a = fDistTri - fDist_b - fDist_c;
  }

  /********************************************************************
   * draw polygon
   ********************************************************************/
  HPEN   hpSectorPen  = (HPEN)CreatePen(PS_SOLID, IBLSCALE(2),  fillcolor );
  HBRUSH hpSectorFill = NULL;

  HPEN hpOldPen     = (HPEN)  SelectObject(hdc, hpSectorPen);
  HBRUSH hpOldBrush;
/*  if (fillcolor != 0)
  {
	hpSectorFill = (HBRUSH)CreateSolidBrush(fillcolor);
    hpOldBrush = (HBRUSH)SelectObject(hdc, hpSectorFill);
  }
  else */
    hpOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));


  /********************************************/
 // Polygon(hdc, apSectorPolygon,iPolyPtr);
  if (ForcedClipping || DeviceNeedClipping)
    ClipPolygon(hdc,apSectorPolygon,iPolyPtr,rc, true);
  else
    Polygon(hdc,apSectorPolygon,iPolyPtr);
  /********************************************/

  SelectObject(hdc, (HPEN)hpOldPen);
  SelectObject(hdc, (HBRUSH)hpOldBrush);
  DeleteObject(hpSectorPen);
  if(hpSectorFill != NULL)
    DeleteObject(hpSectorFill);



  /********************************************************************
   * calc round leg grid
   ********************************************************************/
  hpSectorPen  = (HPEN)CreatePen(PS_SOLID, (2), RGB_DARKGREY );
  SelectObject(hdc, hpSectorPen);

  double fTic= 1/DISTANCEMODIFY;
  if(fDist_c > 5/DISTANCEMODIFY)   fTic = 10/DISTANCEMODIFY;
  if(fDist_c > 50/DISTANCEMODIFY)  fTic = 25/DISTANCEMODIFY;
  if(fDist_c > 100/DISTANCEMODIFY) fTic = 50/DISTANCEMODIFY;
  if(fDist_c > 200/DISTANCEMODIFY) fTic = 100/DISTANCEMODIFY;
  if(fDist_c > 500/DISTANCEMODIFY) fTic = 250/DISTANCEMODIFY;
  POINT line[2];
  fDistTri = ((int)(fDistMin/fTic)+1) * fTic ;
  HFONT hfOld = (HFONT)SelectObject(hdc, LK8PanelUnitFont);
  int iCnt =0;
  SetTextColor(hdc, RGB_DARKGREY);
  while(fDistTri < fDistMax)
  {
    fDelta_Dist =  (fDistTri-fDistMin)*(1.0-2.0*fFAI_Percentage) / (double)(FAI_SECTOR_STEPS-1);
    fDist_a = fDistTri*fFAI_Percentage;
    fDist_b = fDistTri - fDist_a - fDist_c;
    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

	  MapWindow::LatLon2Screen(lon_d, lat_d,  line[0]);

      if(i> 0)
      {
  		ForcedClipping=true;
  		MapWindow::_DrawLine(hdc, PS_DASH, NIBLSCALE(1), line[0] , line[1] , RGB_BLACK, rc);
  		ForcedClipping=false;
      }
	  //  Polyline(hdc, line, 2);

      line[1] =  line[0];
	  fDist_a += fDelta_Dist;
	  fDist_b = fDistTri - fDist_a - fDist_c;

      TCHAR text[180]; SIZE tsize;
      if(iCnt==0)
        _stprintf(text, TEXT("%i%s"), (int)(fDistTri*DISTANCEMODIFY), Units::GetUnitName(Units::GetUserDistanceUnit()));
      else
        _stprintf(text, TEXT("%i"), (int)(fDistTri*DISTANCEMODIFY));
      GetTextExtentPoint(hdc, text, _tcslen(text), &tsize);
      if(i == 0)
        ExtTextOut(hdc, line[0].x, line[0].y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

      if(iCnt > 1)
  	    if(i == FAI_SECTOR_STEPS-1)
  	      ExtTextOut(hdc, line[0].x, line[0].y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

      if(iCnt > 2)
  	    if((i== (FAI_SECTOR_STEPS/2)))
  	      ExtTextOut(hdc, line[0].x, line[0].y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
    }
    iCnt++;
    fDistTri+=fTic;
  }

SelectObject(hdc, hfOld);
SelectObject(hdc, (HPEN)hpOldPen);
DeleteObject( hpSectorPen);
return 0;
}


