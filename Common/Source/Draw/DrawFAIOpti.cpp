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
#include "LKObjects.h"
#include "ScreenProjection.h"
#include "NavFunctions.h"

#ifdef PNA
  #define FAI_SECTOR_STEPS 11
#else
  #define FAI_SECTOR_STEPS 21
#endif
#define MAX_FAI_SECTOR_PTS (8*FAI_SECTOR_STEPS)
extern LKColor taskcolor;
int RenderFAISector (LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, double lat1, double lon1, double lat2, double lon2, int iOpposite , const LKColor& fillcolor);
extern BOOL CheckFAILeg(double leg, double total);


//#define   FILL_FAI_SECTORS

void MapWindow::DrawFAIOptimizer(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT &Orig_Aircraft)
{

  const auto whitecolor = RGB_WHITE;
  const auto origcolor = Surface.SetTextColor(whitecolor);
  const auto oldpen = Surface.SelectObject(hpStartFinishThin);
  const auto oldbrush = Surface.SelectObject(LKBrush_Hollow);

/********************************************************************/
  unsigned int ui;
  double lat1 = 0;
  double lon1 = 0;
  double lat2 = 0;
  double lon2 = 0;
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

  typedef struct
  {
    int    LegIdx;
    double LegDist;
    double LegAngle;
  } legtype;
  legtype  Legs[10];


  for(int i=0; i< 10;  i++)
  {
    Legs[i].LegIdx  =0;
    Legs[i].LegDist =0.0;
  }
int numlegs=0;
  if(((sType ==  CContestMgr::TYPE_FAI_TRIANGLE)
	 || (sType ==  CContestMgr::TYPE_FAI_TRIANGLE4)
#ifdef  FIVEPOINT_OPTIMIZER
	 || (sType ==  CContestMgr::TYPE_FAI_TRIANGLE5)
#endif
	 ) && (iSize>0))
  {
    LKASSERT(iSize<100);
    LockTaskData(); // protect from external task changes
    for(ui=0; ui< iSize-2; ui++)
    {
      lat1 = points[ui].Latitude();
      lon1 = points[ui].Longitude();
      lat2 = points[ui+1].Latitude();
      lon2 = points[ui+1].Longitude();
      DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);
      Legs[numlegs].LegIdx   = ui;
      Legs[numlegs].LegDist  = fDist;

      if(numlegs < 10)
        numlegs ++;
      LKASSERT (numlegs <10);
    }

    std::sort(std::begin(Legs), std::next(Legs, numlegs), [](const legtype& a, const legtype& b) {
        return (a.LegDist > b.LegDist);
    } );

    for(int i= 0 ; i < min(numlegs,2); i++)
    {
        #ifndef DITHER
            LKColor rgbCol = RGB_BLUE;
            switch(i)
            {
              case 0: rgbCol = RGB_YELLOW; break;
              case 1: rgbCol = RGB_CYAN  ; break;
              case 2: rgbCol = RGB_GREEN ; break;
              default:
              break;
            }
                #else
            LKColor rgbCol = RGB_DARKBLUE;
            switch(i)
            {
              case 0: rgbCol = RGB_LIGHTGREY; break;
              case 1: rgbCol = RGB_GREY  ; break;
              case 2: rgbCol = RGB_MIDDLEGREY ; break;
              default:
              break;
            }
#endif
      lat1 = points[Legs[i].LegIdx].Latitude();
      lon1 = points[Legs[i].LegIdx].Longitude();
      lat2 = points[Legs[i].LegIdx+1].Latitude();
      lon2 = points[Legs[i].LegIdx+1].Longitude();
      DistanceBearing(lat1, lon1, lat2, lon2, &fDist, &fAngle);
      if(fDist > FAI_MIN_DISTANCE_THRESHOLD)
      {
        RenderFAISector ( Surface, rc, _Proj, lat1, lon1, lat2, lon2, 1, rgbCol );
        RenderFAISector ( Surface, rc, _Proj, lat1, lon1, lat2, lon2, 0, rgbCol );
      }
    }

      UnlockTaskData();

/*********************************************************/


    if(ISPARAGLIDER && bFAI)
    {
      LKPen hpSectorPen(PEN_SOLID, IBLSCALE(2),  FAI_SECTOR_COLOR );
      const auto hOldPen = Surface.SelectObject(hpSectorPen);
      const POINT Pt1 = _Proj.ToRasterPoint(lon_CP, lat_CP);
      FindLatitudeLongitude(lat1, lon1, 0 , fFAIDistance* 0.20, &lat2, &lon2); /* 1000m destination circle */
      int iRadius = (int)((lat2-lat1)*zoom.DrawScale());
      Surface.DrawCircle(Pt1.x, Pt1.y, iRadius  , rc, false);
      FindLatitudeLongitude(lat1, lon1, 0 , 500, &lat2, &lon2); /* 1000m destination circle */
      iRadius = (int)((lat2-lat1)*zoom.DrawScale());
      Surface.DrawCircle(Pt1.x, Pt1.y, iRadius  , rc, false);
      Surface.SelectObject (hOldPen);
    }

/*********************************************************/

  }

/********************************************************************/
    // restore original color
    Surface.SetTextColor(origcolor);
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);

}



int RenderFAISector (LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, double lat1, double lon1, double lat2, double lon2, int iOpposite , const LKColor& fillcolor)
{
double fDist_a, fDist_b, fDist_c, fAngle;
int i;

int iPolyPtr=0;
double lat_d,lon_d;
double alpha, fDistTri, cos_alpha=0;
POINT apSectorPolygon[MAX_FAI_SECTOR_PTS+1];
DistanceBearing(lat1, lon1, lat2, lon2, &fDist_c, &fAngle);

if(fabs(fDist_c) < 1000.0)  /* distance too short for a FAI sector */
    return -1;


double fDistMax = fDist_c/FAI_NORMAL_PERCENTAGE;
double fDistMin = fDist_c/(1.0-2.0*FAI_NORMAL_PERCENTAGE );
double fDelta_Dist = 2.0* fDist_c*FAI_NORMAL_PERCENTAGE / (double)(FAI_SECTOR_STEPS-1);
double fA, fB;
double fMinLeg, fMaxLeg,fDiff=0;
double dir = -1.0;
BOOL bBigFAISector = false;

if(fDistMax >= FAI28_45Threshold)
{
  bBigFAISector = true;
  fDistMax = fDist_c/FAI_BIG_PERCENTAGE;
}

if(fDistMin >= FAI28_45Threshold)
{
  fDistMin = fDist_c/FAI_BIG_MAX_PERCENTAGE;
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
  Surface.DrawLine(rc, x1, y1, x2, y2, style);
#endif

  /********************************************************************
   * right below threshold 1
   ********************************************************************/
  fA = 	fDistMin;
  if(fDistMax >= FAI28_45Threshold)
    fB = FAI28_45Threshold;
  else
    fB = fDistMax;


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

      LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
      apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);


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
    fDelta_Dist =  (fB-fA) / (double)(FAI_SECTOR_STEPS-1);
    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
      fDist_b = FAI28_45Threshold - fDist_a - fDist_c;
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
      alpha = acos(cos_alpha)*180/PI * dir;
      FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

      LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
      apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);

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
        fDist_a = fDistTri-fMinLeg-fDist_c;
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
        LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
        apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);;


        fDistTri += fDelta_Dist;
      }
    }
  }
/********************************************************************
 * TOP limited round 4
 ********************************************************************/
  if(fDistMax <= FAI28_45Threshold)
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
    LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
    apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);;


    fDist_a -= fDelta_Dist;
    fDist_b += fDelta_Dist;
  }

/********************************************************************
 * calc left leg
 ********************************************************************/

/********************************************************************
 * LEFT above threshold 5
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
        LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
        apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);;


        fDistTri -= fDelta_Dist;
     }
    }
  }

  /********************************************************************
   * LEFT threshold extender 6
   ********************************************************************/
  if((fDistMin < FAI28_45Threshold) && (FAI28_45Threshold <= fDistMax))
  {
    fMaxLeg = FAI28_45Threshold*FAI_BIG_MAX_PERCENTAGE;
    fMinLeg = FAI28_45Threshold*FAI_BIG_PERCENTAGE;
    fA = FAI28_45Threshold*FAI_NORMAL_PERCENTAGE;
    fB = FAI28_45Threshold-fMaxLeg-fDist_c;

    if(fB < fMinLeg)
      fB = fMinLeg;

    fDist_b = fB;
    fDelta_Dist =  (fA-fB) / (double)(FAI_SECTOR_STEPS-1);

    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
      fDist_a = FAI28_45Threshold - fDist_b - fDist_c;
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
      alpha = acos(cos_alpha)*180/PI * dir;
      FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
      LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
      apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);;

      fDist_b += fDelta_Dist;
    }
  }


  /********************************************************************
   * LEFT below threshold 7
   ********************************************************************/
   fA = 	fDistMin;
   if(fDistMax >= FAI28_45Threshold)
    fB = FAI28_45Threshold;
   else
	fB = fDistMax;


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
      LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
      apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);;
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
        LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
        apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);;

        fDist_a += fDelta_Dist;
        fDist_b -= fDelta_Dist;
    }
  }

  /********************************************************************
   * draw polygon
   ********************************************************************/
  LKPen   hpSectorPen(PEN_SOLID, IBLSCALE(2),  fillcolor );

  const auto hpOldPen = Surface.SelectObject(hpSectorPen);
  const auto hpOldBrush = Surface.SelectObject(LKBrush_Hollow);

#ifdef FILL_FAI_SECTORS
  Surface.Polygon(apSectorPolygon,iPolyPtr,rc);
#else
  Surface.Polyline(apSectorPolygon,iPolyPtr,rc);
#endif



  /********************************************************************
   * calc 2nd sectors if needed
   ********************************************************************/
  if((fDistMax < FAI28_45Threshold) && (fDist_c/FAI_BIG_PERCENTAGE > FAI28_45Threshold))
  {
      iPolyPtr = 0;
      /********************************************************************
       * TOP limited round 9
       ********************************************************************/
        fDistMax = fDist_c/FAI_BIG_PERCENTAGE;
            fDist_b = fDistMax*FAI_BIG_MAX_PERCENTAGE;
        fDist_a = fDistMax-fDist_b-fDist_c;
        fDelta_Dist =  (fDist_a-fDist_b) / (double)(FAI_SECTOR_STEPS-1);
        for(i =0 ;i < FAI_SECTOR_STEPS; i++)
        {
            LKASSERT(fDist_c*fDist_b!=0);
            cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
            alpha = acos(cos_alpha)*180/PI * dir;
            FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
            LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
            apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);

            fDist_a -= fDelta_Dist;
            fDist_b += fDelta_Dist;
        }
          /********************************************************************
           * LOW limited round 10
           ********************************************************************/

        fDist_a = FAI28_45Threshold*FAI_BIG_MAX_PERCENTAGE;
        fDist_b = FAI28_45Threshold-fDist_a-fDist_c;
        fDelta_Dist =  -(fDist_a-fDist_b) / (double)(FAI_SECTOR_STEPS-1);

        for(i =0 ;i < FAI_SECTOR_STEPS; i++)
        {
              LKASSERT(fDist_c*fDist_b!=0);
              cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
              alpha = acos(cos_alpha)*180/PI * dir;
              FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
              LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
              apSectorPolygon[iPolyPtr++] = _Proj.ToRasterPoint(lon_d, lat_d);
          fDist_a += fDelta_Dist;
          fDist_b -= fDelta_Dist;
        }
        LKASSERT(iPolyPtr < MAX_FAI_SECTOR_PTS);
        apSectorPolygon[iPolyPtr++] = apSectorPolygon[0] ; // close the polygon
        /********************************************/
        #ifdef FILL_FAI_SECTORS
        Surface.Polygon(apSectorPolygon,iPolyPtr,rc);
       #else
       Surface.Polyline(apSectorPolygon,iPolyPtr,rc);
       #endif
        /********************************************/
  }

  Surface.SelectObject(hpOldPen);
  Surface.SelectObject(hpOldBrush);
  hpSectorPen.Release();

  /********************************************************************
   * calc round leg grid
   ********************************************************************/
  hpSectorPen.Create(PEN_SOLID, ScreenThinSize, RGB_BLACK );
  Surface.SelectObject(hpSectorPen);
  Surface.SetTextColor(RGB_BLACK);
  float fZoom = MapWindow::zoom.RealScale() ;
  double         fTic = 5;
  if(fZoom > 3)  fTic = 10;
  if(fZoom > 5)  fTic = 25;
  if(fZoom > 25) fTic = 50;
  if(fZoom > 60) fTic = 100;
  if( DISTANCEMODIFY > 0.0)
    fTic /= DISTANCEMODIFY;

  POINT line[2];
  BOOL bFirstUnit = true;
  LKASSERT(fTic!=0);
  fDistTri = fDistMin;
  const auto hfOld = Surface.SelectObject(LK8PanelUnitFont);

int iCnt = 0;

  while(fDistTri <= fDistMax)
  {
    if(CheckFAILeg(fDist_c,fDistTri))
    {
        TCHAR text[180]; SIZE tsize;
        if(bFirstUnit)
          _stprintf(text, TEXT("%i%s"), (int)(fDistTri*DISTANCEMODIFY), Units::GetUnitName(Units::GetUserDistanceUnit()));
        else
          _stprintf(text, TEXT("%i"), (int)(fDistTri*DISTANCEMODIFY));
        bFirstUnit = false;
        Surface.GetTextSize(text, &tsize);

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

          fDelta_Dist =  (fA-fB) / (double)(FAI_SECTOR_STEPS-1);
        }


    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
      alpha = acos(cos_alpha)*180/PI * dir;
      FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
      line[0] = _Proj.ToRasterPoint(lon_d, lat_d);

      if(j> 0)
      {
        #ifdef NO_DASH_LINES
        Surface.DrawLine(PEN_SOLID, ScreenThinSize, line[0] , line[1] , RGB_BLACK, rc);
        #else
        Surface.DrawLine(PEN_DASH, NIBLSCALE(1), line[0] , line[1] , RGB_BLACK, rc);
        #endif
      }


      if(j==0)
      {
        Surface.DrawText(line[0].x, line[0].y, text);
        j=1;

      }

//      TCHAR text[180]; SIZE tsize;
      if(iCnt==0)
        _stprintf(text, TEXT("%i%s"), (int)(fDistTri*DISTANCEMODIFY), Units::GetUnitName(Units::GetUserDistanceUnit()));
      else
        _stprintf(text, TEXT("%i"), (int)(fDistTri*DISTANCEMODIFY));
      Surface.GetTextSize(text, &tsize);
      if(i == 0)
        Surface.DrawText(line[0].x, line[0].y, text);

      if(iCnt > 1)
        if(i == FAI_SECTOR_STEPS-1)
          Surface.DrawText(line[0].x, line[0].y, text);

      if(iCnt > 2)
        if(i== (FAI_SECTOR_STEPS/2))
          Surface.DrawText(line[0].x, line[0].y, text);

      line[1] =  line[0];



      fDist_a -= fDelta_Dist;
      fDist_b += fDelta_Dist;
    }
    }

    fDistTri = ((int)(fDistMin/fTic)+1) * fTic + (iCnt) * fTic;
    iCnt++;
 //   if((iCnt %2) ==0)
  //    DrawText(hdc, line[0].x, line[0].y, ETO_OPAQUE, NULL, text, _tcslen(text), NULL);
  }

Surface.SelectObject(hfOld);
Surface.SelectObject(hpOldPen);

return 0;
}
