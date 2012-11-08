/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Defines.h"



int Statistics::RenderFAISector (HDC hdc, const RECT rc , double lat1, double lon1, double lat2, double lon2, double lat_c, double lon_c , int iOpposite , COLORREF fillcolor)
{

#define STEPS 10
#define N_PLOYGON (3*STEPS)

float fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
double fDist_a, fDist_b, fDist_c, fAngle;
int i;

int iPolyPtr=0;
double lat_d,lon_d;
double alpha, fDistTri, cos_alpha=0;
POINT apSectorPolygon[N_PLOYGON];
DistanceBearing(lat1, lon1, lat2, lon2, &fDist_c, &fAngle);

double x1=0,y1=0;
double fDistMax = fDist_c/FAI_BIG_THRESHOLD;
double fDistMin = fDist_c/(1.0-2.0*FAI_BIG_THRESHOLD);
double fDelta_Dist = 2.0* fDist_c*fFAI_Percentage / (double)(STEPS-1);

if(fDistMax < FAI_BIG_THRESHOLD)
{
  fDistMax = fDist_c/FAI_NORMAL_PERCENTAGE;
  fDistMin = fDist_c/(1.0-2.0*FAI_NORMAL_PERCENTAGE);
}


double dir = -1.0;

  if (iOpposite >0)
  {
	dir = 1.0;
  }


#ifdef HELP_LINES
  FindLatitudeLongitude(lat1, lon1, AngleLimit360 (fAngle), fDist_c/2, &lat_d, &lon_d);
  x1 = (lon_d - lon_c)*fastcosine(lat_d);
  y1 = (lat_d - lat_c);
  FindLatitudeLongitude(lat_d, lon_d, AngleLimit360 (fAngle-90.0), fDist_c, &lat_d, &lon_d);
  x2 = (lon_d - lon_c)*fastcosine(lat_d);
  y2 = (lat_d - lat_c);
  DrawLine(hdc, rc, x1, y1, x2, y2, style);
#endif

  /********************************************************************
   * calc right leg
   ********************************************************************/
  fDelta_Dist =(fDistMax-fDistMin)/ (double)(STEPS-1);
  fDistTri = fDistMin;
  if(fDistTri < FAI_BIG_THRESHOLD)
	fFAI_Percentage =  FAI_NORMAL_PERCENTAGE;
  else
	fFAI_Percentage =  FAI_BIG_PERCENTAGE;

  fDist_a = fDistMin * fFAI_Percentage;
  fDist_b = fDistMin * fFAI_Percentage;
  for(i =0 ;i < STEPS; i++)
  {
	LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
    x1 = (lon_d - lon_c)*fastcosine(lat_d);
    y1 = (lat_d - lat_c);

    apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
    apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

    fDistTri += fDelta_Dist;
    if(fDistTri < FAI_BIG_THRESHOLD)
  	  fFAI_Percentage =  FAI_NORMAL_PERCENTAGE;
    else
  	  fFAI_Percentage =  FAI_BIG_PERCENTAGE;

    fDist_a = fFAI_Percentage * fDistTri;
	fDist_b = fDistTri - fDist_a - fDist_c;
  }

  /********************************************************************
   * calc top leg
   ********************************************************************/
  if(fDistMax < FAI_BIG_THRESHOLD)
	  fFAI_Percentage =  FAI_NORMAL_PERCENTAGE;
  else
	  fFAI_Percentage =  FAI_BIG_PERCENTAGE;


  fDist_a = fDist_c;
  fDelta_Dist =  (fDistMax-2*fDist_a-fDist_c) / (double)(STEPS-1);
  fDist_b = fDistMax - fDist_a - fDist_c;
  for(i =0 ;i < STEPS; i++)
  {
	LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
    x1 = (lon_d - lon_c)*fastcosine(lat_d);
    y1 = (lat_d - lat_c);

    apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
    apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);
	fDist_a += fDelta_Dist;
	fDist_b = fDistMax - fDist_a - fDist_c;
  }

  /********************************************************************
   * calc left leg
   ********************************************************************/
  fDelta_Dist =(fDistMax-fDistMin)/ (double)(STEPS-1);
  fDistTri = fDistMax;
  if(fDistTri < FAI_BIG_THRESHOLD)
	  fFAI_Percentage =  FAI_NORMAL_PERCENTAGE;
  else
	  fFAI_Percentage =  FAI_BIG_PERCENTAGE;
  fDist_b = fDistMax * fFAI_Percentage;
  fDist_a = fDistTri - fDist_b - fDist_c;
  for(i =0 ;i < STEPS; i++)
  {
    LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
    x1 = (lon_d - lon_c)*fastcosine(lat_d);
    y1 = (lat_d - lat_c);

    apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
    apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

    fDistTri -= fDelta_Dist;
    if(fDistTri < FAI_BIG_THRESHOLD)
  	  fFAI_Percentage =  FAI_NORMAL_PERCENTAGE;
    else
  	  fFAI_Percentage =  FAI_BIG_PERCENTAGE;
    fDist_b = fFAI_Percentage * fDistTri;
	fDist_a = fDistTri - fDist_b - fDist_c;
  }

  /********************************************************************
   * draw polygon
   ********************************************************************/
  HPEN   hpSectorPen  = (HPEN)CreatePen(PS_SOLID, IBLSCALE(1), RGB_GREEN );
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


  /********************************************/
  Polygon(hdc, apSectorPolygon,iPolyPtr);
  /********************************************/

  SelectObject(hdc, (HPEN)hpOldPen);
  SelectObject(hdc, (HBRUSH)hpOldBrush);
  DeleteObject(hpSectorPen);
  if(hpSectorFill != NULL)
    DeleteObject(hpSectorFill);



  /********************************************************************
   * calc round leg grid
   ********************************************************************/
  hpSectorPen  = (HPEN)CreatePen(PS_SOLID, (1), RGB_GREY );
  SelectObject(hdc, hpSectorPen);

  double fTic= 1/DISTANCEMODIFY;
  if(fDist_c > 5/DISTANCEMODIFY)   fTic = 20/DISTANCEMODIFY;
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
	if(fDistTri < FAI_BIG_THRESHOLD)
	  fFAI_Percentage =  FAI_NORMAL_PERCENTAGE;
	else
	  fFAI_Percentage =  FAI_BIG_PERCENTAGE;

    fDist_a = fDistTri*fFAI_Percentage;
    fDelta_Dist =  (fDistTri-2*fDist_a-fDist_c)/ (double)(STEPS-1);
 //   fDist_c = fDist_a;
    fDist_b = fDistTri - fDist_a - fDist_c;
    for(i =0 ;i < STEPS; i++)
    {
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
      x1 = (lon_d - lon_c)*fastcosine(lat_d);
      y1 = (lat_d - lat_c);
      line[0].x = ScaleX(rc, x1);
	  line[0].y = ScaleY(rc, y1);

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
    SetTextColor(hdc, RGB_GREY);
    ExtTextOut(hdc, line[0].x, line[0].y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);

    fDistTri+=fTic;
  }

SelectObject(hdc, hfOld);
SelectObject(hdc, (HPEN)hpOldPen);
DeleteObject( hpSectorPen);
return 0;
}


