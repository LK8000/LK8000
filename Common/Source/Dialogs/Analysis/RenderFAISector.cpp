/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Defines.h"
#include "LKObjects.h"

#ifdef PNA
#define FAI_SECTOR_STEPS 11
#else
#define FAI_SECTOR_STEPS 21

#endif
#define MAX_FAI_SECTOR_PTS (8*FAI_SECTOR_STEPS)


int Statistics::RenderFAISector (LKSurface& Surface, const RECT& rc , double lat1, double lon1, double lat2, double lon2, double lat_c, double lon_c , int iOpposite , const LKColor& fillcolor)
{
float fFAI_Percentage = FAI_NORMAL_PERCENTAGE;
double fDist_a, fDist_b, fDist_c, fAngle;
int i;

unsigned int iPolyPtr=0;
double lat_d,lon_d;
double alpha, fDistTri, cos_alpha=0;
POINT apSectorPolygon[MAX_FAI_SECTOR_PTS+1];
DistanceBearing(lat1, lon1, lat2, lon2, &fDist_c, &fAngle);

if(fabs(fDist_c) < 1000.0)  /* distance too short for a FAI sector */
	return -1;

double x1=0,y1=0;
double fDistMax = fDist_c/FAI_NORMAL_PERCENTAGE;
double fDistMin = fDist_c/(1.0-2.0*FAI28_45Threshold);
double fDelta_Dist = 2.0* fDist_c*fFAI_Percentage / (double)(FAI_SECTOR_STEPS-1);
double fA, fB;
double fMinLeg, fMaxLeg,fDiff=0;

double dir = -1.0;
BOOL bBigFAISector = false;

if(fDistMax > FAI28_45Threshold)
{
  bBigFAISector = true;
  fDistMax = fDist_c/FAI_BIG_PERCENTAGE;
}

if(fDistMin < FAI28_45Threshold)
{
  fDistMin = fDist_c/(1.0-2.0*FAI_NORMAL_PERCENTAGE);
}


  if (iOpposite >0)
  {
	dir = 1.0;
  }

//#define  HELP_LINES
#ifdef HELP_LINES
  int x2,y2, style;
  FindLatitudeLongitude(lat1, lon1, AngleLimit360 (fAngle), fDist_c/2, &lat_d, &lon_d);
  x1 = (lon_d - lon_c)*fastcosine(lat_d);
  y1 = (lat_d - lat_c);
  FindLatitudeLongitude(lat_d, lon_d, AngleLimit360 (fAngle-90.0), fDist_c, &lat_d, &lon_d);
  x2 = (lon_d - lon_c)*fastcosine(lat_d);
  y2 = (lat_d - lat_c);
  DrawLine(hdc, rc, x1, y1, x2, y2, style);
#endif

  /********************************************************************
   * right below threshold 1
   ********************************************************************/
  fA = 	fDistMin;
  if(fDistMax > FAI28_45Threshold)
    fB = FAI28_45Threshold;
  else
	fB = fDistMax ;


  if(fA<fB)
  {
	fDelta_Dist =(fB-fA)/ (double)(FAI_SECTOR_STEPS-1);
	fDistTri = fA;
    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
	  fDist_a = FAI_NORMAL_PERCENTAGE * fDistTri;
	  fDist_b = fDistTri - fDist_a - fDist_c;

	  LKASSERT(fDist_c*fDist_b!=0);
	  cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

	  x1 = (lon_d - lon_c)*fastcosine(lat_d);
	  y1 = (lat_d - lat_c);
	  LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
	  apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
	  apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

      fDistTri += fDelta_Dist;
    }
  }

  /********************************************************************
   * right  threshold extender 2
   ********************************************************************/
  if(bBigFAISector && (fDistMin < FAI28_45Threshold))
  {
	fMaxLeg = FAI28_45Threshold*FAI_BIG_MAX_PERCENTAGE;
	fMinLeg = FAI28_45Threshold*FAI_BIG_PERCENTAGE;
	fA = FAI28_45Threshold*FAI_NORMAL_PERCENTAGE;
	fB = FAI28_45Threshold-fMaxLeg-fDist_c;

	if(fB < fMinLeg)
		fB = fMinLeg;

    fDist_a = fA;
    fDelta_Dist =  (fB-fA) / (double)(FAI_SECTOR_STEPS/2-1);
    for(i =0 ;i < FAI_SECTOR_STEPS/2; i++)
    {
	  fDist_b = FAI28_45Threshold - fDist_a - fDist_c;
  	  LKASSERT(fDist_c*fDist_b!=0);
	  cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
      x1 = (lon_d - lon_c)*fastcosine(lat_d);
      y1 = (lat_d - lat_c);
      LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
      apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
      apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);
	  fDist_a += fDelta_Dist;
    }
  }
  /********************************************************************
   * right  above threshold  3
   ********************************************************************/
  if(bBigFAISector)
  {
    fA = 	FAI28_45Threshold;
    if(fDistMin > fA)
	  fA= fDistMin;
    fB =fDist_c/(1- FAI_BIG_PERCENTAGE-FAI_BIG_MAX_PERCENTAGE);

    if(fA < fB)
    {
      fDelta_Dist =(fB-fA)/ (double)(FAI_SECTOR_STEPS-1);
      fDistTri = fA;
      for(i =0 ;i < FAI_SECTOR_STEPS; i++)
      {
  	    fMaxLeg = fDistTri*FAI_BIG_MAX_PERCENTAGE;
	    fMinLeg = fDistTri*FAI_BIG_PERCENTAGE;
	    fDist_a = fDistTri-fMinLeg-fDist_c;;
	    fDist_b = fMinLeg;

	    if(fDist_a > fMaxLeg)
	    {
	      fDiff =  fDist_a - fMaxLeg;
	      fDist_b+=fDiff;
	      fDist_a-=fDiff;
	    }

	    LKASSERT(fDist_c*fDist_b!=0);
	    cos_alpha = ( fDist_a*fDist_a + fDist_c*fDist_c - fDist_b*fDist_b )/(2.0*fDist_c*fDist_a);
	    alpha = acos(cos_alpha)*180/PI * dir;
	    FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_a, &lat_d, &lon_d);
        x1 = (lon_d - lon_c)*fastcosine(lat_d);
        y1 = (lat_d - lat_c);
        LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
        apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
        apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

	    fDistTri += fDelta_Dist;
      }
    }
  }
/********************************************************************
 * TOP limited round 4
 ********************************************************************/
//  if(fDistMax <= FAI28_45Threshold)
 if(!bBigFAISector)
	fDist_b = fDistMax*(1.0-2*FAI_NORMAL_PERCENTAGE);
  else
	fDist_b = fDistMax*FAI_BIG_MAX_PERCENTAGE;

  fDist_a = fDistMax-fDist_b-fDist_c;
  fDelta_Dist =  (fDist_a-fDist_b) / (double)(FAI_SECTOR_STEPS-1);
  for(i =0 ;i < FAI_SECTOR_STEPS; i++)
  {
	LKASSERT(fDist_c*fDist_b!=0);
	cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	alpha = acos(cos_alpha)*180/PI * dir;
	FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
    x1 = (lon_d - lon_c)*fastcosine(lat_d);
    y1 = (lat_d - lat_c);
    LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
    apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
    apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

    fDist_a -= fDelta_Dist;
    fDist_b += fDelta_Dist;
  }

/********************************************************************
 * calc left leg
 ********************************************************************/

/********************************************************************
 * LEFT above threshold  5
 ********************************************************************/
  if(bBigFAISector)
  {
    fB = FAI28_45Threshold;
    if( fB < fDistMin)
	  fB = fDistMin;

    fA =fDist_c/(1- FAI_BIG_PERCENTAGE-FAI_BIG_MAX_PERCENTAGE);

    if(fA >= fB)
    {
      fDelta_Dist =(fA-fB)/ (double)(FAI_SECTOR_STEPS-1);
      fDistTri = fA;
      for(i =0 ;i < FAI_SECTOR_STEPS; i++)
      {
	    fMaxLeg = fDistTri*FAI_BIG_MAX_PERCENTAGE;
	    fMinLeg = fDistTri*FAI_BIG_PERCENTAGE;
	    fDist_a = fDistTri-fMinLeg-fDist_c;
	    fDist_b = fMinLeg;

	    if(fDist_a > fMaxLeg)
	    {
	      fDiff =  fDist_a - fMaxLeg;
	      fDist_b+=fDiff;
	      fDist_a-=fDiff;
  	    }

	    LKASSERT(fDist_c*fDist_b!=0);
	    cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	    alpha = acos(cos_alpha)*180/PI * dir;
	    FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
        x1 = (lon_d - lon_c)*fastcosine(lat_d);
        y1 = (lat_d - lat_c);
        LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
        apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
        apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

	    fDistTri -= fDelta_Dist;
     }
    }
  }

  /********************************************************************
   * LEFT threshold extender
   ********************************************************************/
  if((fDistMin < FAI28_45Threshold) && (FAI28_45Threshold < fDistMax) && (bBigFAISector))
  {
	fMaxLeg = FAI28_45Threshold*FAI_BIG_MAX_PERCENTAGE;
	fMinLeg = FAI28_45Threshold*FAI_BIG_PERCENTAGE;
	fA = FAI28_45Threshold*FAI_NORMAL_PERCENTAGE;
	fB = FAI28_45Threshold-fMaxLeg-fDist_c;

	if(fB < fMinLeg)
	  fB = fMinLeg;

	fDist_b = fB;
    fDelta_Dist =  (fA-fB) / (double)(FAI_SECTOR_STEPS/2-1);
    for(i =0 ;i < FAI_SECTOR_STEPS/2; i++)
    {
	  fDist_a = FAI28_45Threshold - fDist_b - fDist_c;
	  LKASSERT(fDist_c*fDist_b!=0);
	  cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
      x1 = (lon_d - lon_c)*fastcosine(lat_d);
      y1 = (lat_d - lat_c);
      LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
      apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
      apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);
	  fDist_b += fDelta_Dist;
    }
  }


  /********************************************************************
   * LEFT below threshold 7
   ********************************************************************/
  fA = 	fDistMin;
  if(fDistMax > FAI28_45Threshold)
    fB = FAI28_45Threshold;
  else
	fB = fDistMax ;

  if(fA<fB)
  {
	fDelta_Dist =(fB-fA)/ (double)(FAI_SECTOR_STEPS-1);
	fDistTri = fB;
    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
	  fDist_b = FAI_NORMAL_PERCENTAGE * fDistTri;
	  fDist_a = fDistTri - fDist_b - fDist_c;

	  LKASSERT(fDist_c*fDist_b!=0);
	  cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

	  x1 = (lon_d - lon_c)*fastcosine(lat_d);
	  y1 = (lat_d - lat_c);
	  LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
	  apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
	  apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

      fDistTri -= fDelta_Dist;
    }
  }

  /********************************************************************
   * low open PEAK round 8
   ********************************************************************/
  if(fDistMin >FAI28_45Threshold)
  {
	fDist_b = fDistMin*FAI_BIG_PERCENTAGE;
	fDist_a = fDistMin-fDist_b-fDist_c;
	fDelta_Dist =  (fDist_b-fDist_a) / (double)(FAI_SECTOR_STEPS-1);

	for(i =0 ;i < FAI_SECTOR_STEPS; i++)
	{
		LKASSERT(fDist_c*fDist_b!=0);
		cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
		alpha = acos(cos_alpha)*180/PI * dir;
		FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
	    x1 = (lon_d - lon_c)*fastcosine(lat_d);
	    y1 = (lat_d - lat_c);
	    LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
	    apSectorPolygon[iPolyPtr].x   = ScaleX(rc, x1);
	    apSectorPolygon[iPolyPtr++].y = ScaleY(rc, y1);

	  fDist_a += fDelta_Dist;
	  fDist_b -= fDelta_Dist;
	}
  }

  /********************************************************************
   * draw polygon
   ********************************************************************/
  #ifndef UNDITHER
  LKPen hpSectorPen(PEN_SOLID, IBLSCALE(1), RGB_GREEN);
  #else
  LKPen hpSectorPen(PEN_SOLID, IBLSCALE(1), RGB_RED);
  #endif
  const auto hpOldPen = Surface.SelectObject(hpSectorPen);
  LKBrush hbSectorFill;
  hbSectorFill.Create(fillcolor);
  const auto hbOldBrush = Surface.SelectObject(hbSectorFill);


  /********************************************/
  Surface.Polygon(apSectorPolygon,iPolyPtr);
  /********************************************/

  Surface.SelectObject(hpOldPen);
  Surface.SelectObject(hbOldBrush);
  hpSectorPen.Release();



  /********************************************************************
   * calc round leg grid
   ********************************************************************/
  #ifndef UNDITHER
  hpSectorPen.Create(PEN_SOLID, (1), RGB_GREY );
  #else
  hpSectorPen.Create(PEN_SOLID, (1), RGB_DARKGREY );
  #endif
  Surface.SelectObject(hpSectorPen);

  double fTic= 1/DISTANCEMODIFY;
  if(fDist_c > 5/DISTANCEMODIFY)   fTic = 10/DISTANCEMODIFY;
  if(fDist_c > 50/DISTANCEMODIFY)  fTic = 25/DISTANCEMODIFY;
  if(fDist_c > 100/DISTANCEMODIFY) fTic = 50/DISTANCEMODIFY;
//  if(fDist_c > 200/DISTANCEMODIFY) fTic = 100/DISTANCEMODIFY;
  if(fDist_c > 500/DISTANCEMODIFY) fTic = 250/DISTANCEMODIFY;
  POINT line[2];
  BOOL bFirstUnit = true;
  LKASSERT(fTic!=0);
  fDistTri = ((int)(fDistMin/fTic)+1) * fTic ;
  const auto hfOld = Surface.SelectObject(LK8PanelUnitFont);

int iCnt = 0;

  while(fDistTri <= fDistMax)
  {
    TCHAR text[180]; SIZE tsize;
	if(bFirstUnit)
	  _stprintf(text, TEXT("%i%s"), (int)(fDistTri*DISTANCEMODIFY), Units::GetUnitName(Units::GetUserDistanceUnit()));
	else
	  _stprintf(text, TEXT("%i"), (int)(fDistTri*DISTANCEMODIFY));
	bFirstUnit = false;
	Surface.GetTextSize(text, _tcslen(text), &tsize);
        #ifndef UNDITHER
	Surface.SetTextColor(RGB_GREY);
        #else
	Surface.SetTextColor(RGB_DARKGREY);
        #endif
    int j=0;

	if(fDistTri < FAI28_45Threshold)
	{
	  fDist_b = fDistTri*FAI_NORMAL_PERCENTAGE;
	  fDist_a = fDistTri-fDist_b-fDist_c;
	  fDelta_Dist =  (fDist_a-fDist_b) / (double)(FAI_SECTOR_STEPS-1);
	}
	else
	{
      fMaxLeg = fDistTri*FAI_BIG_MAX_PERCENTAGE;
      fMinLeg = fDistTri*FAI_BIG_PERCENTAGE;
	  fA = fMaxLeg;
	  fB = fDistTri-fA-fDist_c;
	  fDist_a = fA;
	  fDist_b = fB;
	  if(fB < fMinLeg)
	  {
		  fDiff = fMinLeg-fB;
		  fB+=2*fDiff;
		  fDist_b += fDiff;
		  fDist_a -= fDiff;
	  }
	  if(fB > fMaxLeg)
	  {
		  fDiff =  fB - fMaxLeg;
		  fB+=2*fDiff;
		  fDist_b-=fDiff;
		  fDist_a+=fDiff;
	  }
	  fFAI_Percentage =  FAI_BIG_PERCENTAGE;
	  fDelta_Dist =  (fA-fB) / (double)(FAI_SECTOR_STEPS-1);
	}


    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
	  LKASSERT(fDist_c*fDist_b!=0);
	  cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
	  alpha = acos(cos_alpha)*180/PI * dir;
	  FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
      x1 = (lon_d - lon_c)*fastcosine(lat_d);
      y1 = (lat_d - lat_c);

      line[0].x = ScaleX(rc, x1);
      line[0].y = ScaleY(rc, y1);

      if(j>0)
      {
	    Surface.Polyline(line, 2);
      }

      if(j==0)
      {
    	Surface.DrawText(line[0].x, line[0].y, text, _tcslen(text));
    	j=1;

      }
      line[1] =  line[0];



	  fDist_a -= fDelta_Dist;
	  fDist_b += fDelta_Dist;
    }
    fDistTri+=fTic;iCnt++;
  }

Surface.SelectObject(hfOld);
Surface.SelectObject(hpOldPen);
return 0;
}


