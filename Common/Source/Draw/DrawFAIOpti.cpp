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
#include "MapWindow.h"
#include "Draw/ScreenProjection.h"
#include "Math/Point2D.hpp"
#include "DrawFAIOpti.h"

//#define FAI_SECTOR_DEBUG
#ifdef HAVE_GLES
typedef FloatPoint ScreenPoint;
#else
typedef RasterPoint ScreenPoint;
#endif



#define MAX_FAI_SECTOR_PTS (8*FAI_SECTOR_STEPS)
extern LKColor taskcolor;
int RenderFAISector (LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, double lat1, double lon1, double lat2, double lon2, int iOpposite , const LKColor& fillcolor);
extern BOOL CheckFAILeg(double leg, double total);




// #define   FILL_FAI_SECTORS

int FAI_Sector::DrawFAISector (LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const LKColor& InFfillcolor)
{

int i;
LKColor fillcolor = InFfillcolor;
#ifdef KOBO
  fillcolor = RGB_SBLACK;
#endif

const GeoToScreen<ScreenPoint> ToScreen(_Proj);
const PixelRect ScreenRect(rc);

typedef std::vector<ScreenPoint> polyline_t;
polyline_t FAISector_polyline; // make it static for save memory Alloc/Free ( don't forget to clear in this case )


  /********************************************************************
   * draw polygon
   ********************************************************************/
LKPen   hpSectorPen(PEN_SOLID, IBLSCALE(2),  fillcolor );
const auto hpOldPen = Surface.SelectObject(hpSectorPen);
const auto hpOldBrush = Surface.SelectObject(LKBrush_Hollow);
Surface.SetBackgroundTransparent();

  bool bSectorvisible=false;
  FAISector_polyline.clear();
  if(!m_FAIShape.empty())
  {
    for (std::list<GeoPoint>::iterator it = m_FAIShape.begin(); it != m_FAIShape.end(); it++)
    {
      const ScreenPoint Pos = ToScreen(*it);

      if(ScreenRect.IsInside(Pos))
    	bSectorvisible = true;
      FAISector_polyline.push_back(Pos);
    }
  }
    if(bSectorvisible) {
		FAISector_polyline.push_back(FAISector_polyline.front());
	#ifdef  FAI_SECTOR_DEBUG
		StartupStore(_T("FAI Sector draw with lat:%8.4f  lon:%8.4f => screen x:%u y:%u  %s"),  m_FAIShape.begin()->latitude,  m_FAIShape.begin()->longitude , FAISector_polyline.begin()->x, FAISector_polyline.begin()->y , NEWLINE);
		StartupStore(_T("FAI Sector draw with %u points  %s"), iPointCnt, NEWLINE);
	#endif

	#ifdef FILL_FAI_SECTORS
		 Surface.Polygon(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#else
		 Surface.Polyline(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#endif
    }

	  FAISector_polyline.clear();
	  if(!m_FAIShape2.empty())
	  {
		for (std::list<GeoPoint>::iterator it = m_FAIShape2.begin(); it != m_FAIShape2.end(); it++)
		{
          const ScreenPoint Pos = ToScreen(*it);
		  if(ScreenRect.IsInside(Pos))
		 	bSectorvisible = true;
		  FAISector_polyline.push_back(Pos);
		}
	  }
	  if(bSectorvisible)
	  {
		FAISector_polyline.push_back(FAISector_polyline.front());
	#ifdef  FAI_SECTOR_DEBUG
		StartupStore(_T("FAI Sector draw 2nd section with lat:%8.4f  lon:%8.4f => screen x:%u y:%u  %s"),  m_FAIShape2.begin()->latitude,  m_FAIShape2.begin()->longitude , FAISector_polyline.begin()->x, FAISector_polyline.begin()->y , NEWLINE);
		StartupStore(_T("FAI Sector draw 2nd section with %u points  %s"), iPointCnt, NEWLINE);
	#endif
	#ifdef FILL_FAI_SECTORS
		 Surface.Polygon(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#else
		 Surface.Polyline(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#endif

      hpSectorPen.Create(PEN_DASH, ScreenThinSize, RGB_BLACK );
      Surface.SelectObject(hpSectorPen);
      Surface.SetBackgroundTransparent();
      const auto oldFont = Surface.SelectObject(LK8InfoSmallFont);
	  unsigned  int NumberGrids = m_FAIGridLines.size()-1;
	  unsigned int Grid_num = 0;
	  FAISector_polyline.clear();

	  if(!m_FAIGridLines.empty())
	  {
		for (std::list<GPS_Gridline_t>::iterator it = m_FAIGridLines.begin(); it != m_FAIGridLines.end(); it++)
		{
		  bool bGridVisible = false;
		  for(i=0; i < FAI_SECTOR_STEPS; i++)
		  {
            const ScreenPoint Pos = ToScreen(it->GridLine[i]);
		    if(ScreenRect.IsInside(Pos))
		      bGridVisible = true;
			FAISector_polyline.push_back(Pos);
		  }
		  if(bGridVisible)
		  {
		    i =0;
		    if( (Grid_num <  NumberGrids))
		  	  Surface.Polyline(FAISector_polyline.data(), FAI_SECTOR_STEPS, rc);

		    const ScreenPoint pt =  ToScreen(it->GridLine[i]);
		    MapWindow::LKWriteText(Surface, it->szLable, pt.x, pt.y, WTMODE_OUTLINED, WTALIGN_LEFT, fillcolor, true);

		    if( Grid_num > 0)
		    {
		      i =FAI_SECTOR_STEPS-1;
              const ScreenPoint pt =  ToScreen(it->GridLine[i]);
		      MapWindow::LKWriteText(Surface, it->szLable, pt.x, pt.y, WTMODE_OUTLINED, WTALIGN_LEFT, fillcolor, true);
		    }
		    if( Grid_num > 1)
		    {
		      i =FAI_SECTOR_STEPS/2;
              const ScreenPoint pt =  ToScreen(it->GridLine[i]);
		      MapWindow::LKWriteText(Surface, it->szLable, pt.x, pt.y, WTMODE_OUTLINED, WTALIGN_LEFT, fillcolor, true);
		    }
		  }
		  else
		  {
#ifdef  FAI_SECTOR_DEBUG
        	StartupStore(_T("FAI Grid %u skipped not in view window  %s"),Grid_num, NEWLINE);
#endif
		  }
          FAISector_polyline.clear();

		  Grid_num++;
		}
	  }
	  Surface.SelectObject(oldFont);
    } // if visible

  if(!FAISector_polyline.empty())  /* maybe not empty due visibility check */
    FAISector_polyline.clear();    /* clear if not empty                   */


  if(bSectorvisible)
  {
#ifdef  FAI_SECTOR_DEBUG
   	StartupStore(_T("FAI Sector skipped not in view window  %s"), NEWLINE);
#endif
  }


  Surface.SelectObject(hpOldPen);
  Surface.SelectObject(hpOldBrush);
  hpSectorPen.Release();
return 0;
}



extern BOOL CheckFAILeg(double leg, double total);

void FAI_Sector::FreeFAISectorMem(void)
{
  if(m_FAIGridLines.empty()) m_FAIGridLines.clear();
  if(m_FAIShape.empty()) m_FAIShape.clear();
  if(m_FAIShape2.empty()) m_FAIShape2.clear();
}

FAI_Sector::~FAI_Sector(void){
  FreeFAISectorMem();
#ifdef  FAI_SECTOR_DEBUG
  StartupStore(_T("FAI Sector ~FAI_Sector erased %s"),  NEWLINE);
#endif
};

FAI_Sector::FAI_Sector(void){
  m_lat1=0;
  m_lat2=0;
  m_lon1=0;
  m_lon2=0;
  m_fGrid =0;
  m_Side =0;
#ifdef  FAI_SECTOR_DEBUG
  StartupStore(_T("FAI_Sector constructor %s"),  NEWLINE);
#endif
};


bool FAI_Sector::CalcSectorCache(double lat1, double lon1, double lat2, double lon2, double fGrid, int iOpposite)
{

 if(fabs(m_lat1 - lat1) < 0.00001)
   if(fabs(m_lon1 - lon1) < 0.00001)
	 if(fabs(m_lat2 - lat2) < 0.00001)
	   if(fabs(m_lon2 - lon2) < 0.00001)
		 if(m_Side == iOpposite)
		   if(fabs(m_fGrid - fGrid) < 0.0001)
		   {
#ifdef  FAI_SECTOR_DEBUG
			 StartupStore(_T("FAI Sector use cached %s"),  NEWLINE);
#endif
			 return true;   /* already calculated with the same parameters */
		   }



double fDist_a, fDist_b, fDist_c, fAngle;
int i;
double lat_d,lon_d;
double alpha, fDistTri, cos_alpha=0;

DistanceBearing(lat1, lon1, lat2, lon2, &fDist_c, &fAngle);

if(fabs(fDist_c) < 1000.0)  /* distance too short for a FAI sector */
    return -1;


m_lat1  = lat1;   /* remember parameter */
m_lon1  = lon1;
m_lat2  = lat2;
m_lon2  = lon2;
m_Side  = iOpposite;
m_fGrid = fGrid;
#ifdef FAI_SECTOR_DEBUG
StartupStore(_T("FAI Sector recalc %s"),  NEWLINE);
#endif
FreeFAISectorMem();

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
  DrawLine(hdc, rc, x1, y1, x2, y2, style);
#endif

  /********************************************************************
   * right below threshold 1
   ********************************************************************/
  fA = 	fDistMin;
  if(fDistMax >= FAI28_45Threshold)
    fB = FAI28_45Threshold;
  else
    fB = fDistMax;

  m_FAIShape.clear();

  if(fA<fB)
  {
    fDelta_Dist =(fB-fA)/ (double)(FAI_SECTOR_STEPS-1);
    fDistTri = fA;
    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
      fDist_a = FAI_NORMAL_PERCENTAGE * fDistTri;
      fDist_b = fDistTri - fDist_a - fDist_c;
      if(bBigFAISector)
        if( fDist_b > fDistTri*FAI_BIG_MAX_PERCENTAGE)
              fDist_b = fDistTri*FAI_BIG_MAX_PERCENTAGE;
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
      alpha = acos(cos_alpha)*180/PI * dir;
      FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

       m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

      fDistTri += fDelta_Dist;
    }
  }

  /********************************************************************
   * right  threshold extender 2
   ********************************************************************/
if((fDist_c / FAI_NORMAL_PERCENTAGE) >= FAI28_45Threshold)
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

       m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

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

         m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

        fDistTri += fDelta_Dist;
      }
    }
  }
/********************************************************************
 * TOP limited round 4
 ********************************************************************/
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

     m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

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

         m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

        fDistTri -= fDelta_Dist;
     }
    }
  }

  /********************************************************************
   * LEFT threshold extender
   ********************************************************************/
if((fDist_c / FAI_NORMAL_PERCENTAGE) >= FAI28_45Threshold)
  if((fDistMin < FAI28_45Threshold) && (FAI28_45Threshold <= fDistMax) && (bBigFAISector))
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

       m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

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

       m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

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

         m_FAIShape.push_back(GeoPoint(lat_d,lon_d));

        fDist_a += fDelta_Dist;
        fDist_b -= fDelta_Dist;
    }
  }

  /********************************************************************
   * calc 2nd sectors if needed
   ********************************************************************/
  m_FAIShape2.clear();
  if((fDistMax < FAI28_45Threshold) && (fDist_c/FAI_BIG_PERCENTAGE > FAI28_45Threshold))
  {
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

       m_FAIShape2.push_back(GeoPoint(lat_d,lon_d));

      fDist_a -= fDelta_Dist;
      fDist_b += fDelta_Dist;
    }
  /********************************************************************
   * LOW limited round 10
   ********************************************************************/

    fDist_a = FAI28_45Threshold*FAI_BIG_MAX_PERCENTAGE;
    fDist_b = FAI28_45Threshold-fDist_a-fDist_c;
    fDelta_Dist =  (fDist_a-fDist_b) / (double)(FAI_SECTOR_STEPS-1);
    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
      alpha = acos(cos_alpha)*180/PI * dir;
      FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);

       m_FAIShape2.push_back(GeoPoint(lat_d,lon_d));

      fDist_a -= fDelta_Dist;
      fDist_b += fDelta_Dist;
    }
  }

  /********************************************************************
   * calc round leg grid
   ********************************************************************/

  double  fTic = fGrid;
  BOOL bLast = false;
  BOOL bFirstUnit = true;
  LKASSERT(fTic!=0);
  fDistTri = fDistMin ; //((int)(fDistMin/fTic)+1) * fTic ;

  m_FAIGridLines.clear();
int iCnt = 0;

  while(fDistTri <= fDistMax)
  {
    if(CheckFAILeg(fDist_c,fDistTri))
    {
    TCHAR text[180];
    if(bFirstUnit)
      _stprintf(text, TEXT("%i%s"), (int)(fDistTri*DISTANCEMODIFY+0.5), Units::GetUnitName(Units::GetUserDistanceUnit()));
    else
      _stprintf(text, TEXT("%i"), (int)(fDistTri*DISTANCEMODIFY+0.5));
    bFirstUnit = false;



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
    GPS_Gridline_t NewGrid;
    NewGrid.fValue = (int)(fDistTri*DISTANCEMODIFY+0.5);
    _tcscpy(NewGrid.szLable, text);

    for(i =0 ;i < FAI_SECTOR_STEPS; i++)
    {
      LKASSERT(fDist_c*fDist_b!=0);
      cos_alpha = ( fDist_b*fDist_b + fDist_c*fDist_c - fDist_a*fDist_a )/(2.0*fDist_c*fDist_b);
      alpha = acos(cos_alpha)*180/PI * dir;
      FindLatitudeLongitude(lat1, lon1, AngleLimit360( fAngle + alpha ) , fDist_b, &lat_d, &lon_d);
      NewGrid.GridLine[i] = GeoPoint(lat_d,lon_d);

      fDist_a -= fDelta_Dist;
      fDist_b += fDelta_Dist;
    }

     m_FAIGridLines.push_back(NewGrid);
  }

  if(iCnt == 0)
  {
    fDistTri = ((int)(fDistMin/fTic)+1.0) * fTic ;
    if(((int)(fDistTri - fDistMin  )) > (fTic*0.65))
      fDistTri -= fTic;
  }


  fDistTri += fTic;
  if( ((fDistTri+0.65*fTic) > fDistMax ) && !bLast)
  {
    fDistTri = fDistMax-0.5;
    bLast = true;
  }

  iCnt++;
}

return 0;
}

void MapWindow::DrawFAIOptimizer(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const POINT &Orig_Aircraft)
{

static	FAI_Sector FAI_SectorCache[6];

  const auto whitecolor = RGB_WHITE;
  const auto origcolor = Surface.SetTextColor(whitecolor);
  const auto oldpen = Surface.SelectObject(hpStartFinishThin);
  const auto oldbrush = Surface.SelectObject(LKBrush_Hollow);

  const GeoToScreen<ScreenPoint> ToScreen(_Proj);
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

    float fZoom = MapWindow::zoom.RealScale() ;

    double         fTic = 100;
    if(fZoom > 50) fTic = 100; else
    if(fZoom > 20) fTic = 50;  else
    if(fZoom > 10) fTic = 25;  else fTic = 25;
   /* if(fZoom > 3)  fTic = 10;  else fTic = 10;  // FAI grid below 10km need to much CPU power in moving map and PAN mode!!!
    if(fZoom > 1)  fTic = 5;   else   // on slow devices like MIO
    if(fZoom > 0.5)fTic = 2;   else   // the user should use Analysis page for this
    if(fZoom > 0.2)fTic = 1;*/
    if( DISTANCEMODIFY > 0.0)
      fTic =  fTic/ DISTANCEMODIFY;

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
        FAI_SectorCache[2*i].CalcSectorCache( lat1,  lon1,  lat2,  lon2, fTic, 0);
        FAI_SectorCache[2*i].DrawFAISector ( Surface, rc, _Proj, rgbCol );
        FAI_SectorCache[2*i+1].CalcSectorCache( lat1,  lon1,  lat2,  lon2, fTic, 1);
        FAI_SectorCache[2*i+1].DrawFAISector ( Surface, rc, _Proj,  rgbCol );
      }
    }

      UnlockTaskData();

/*********************************************************/

	  // TODO : check if visible for avoid to far draw outside screen.
	  // for 1000m radius cylinder, it's better to build cylinder in screen coordinate
	  // and draw this like polygon ; cf. Task sector drawing



      if (ISPARAGLIDER && bFAI) {
          LKPen hpSectorPen(PEN_SOLID, IBLSCALE(2), FAI_SECTOR_COLOR);
          const auto hOldPen = Surface.SelectObject(hpSectorPen);
          const RasterPoint Pt1 = _Proj.ToRasterPoint(lon_CP, lat_CP);

          int iRadius = (int) ((fFAIDistance * 0.20) * zoom.ResScaleOverDistanceModify());
          Surface.DrawCircle(Pt1.x, Pt1.y, iRadius, rc, false);                    /* 20% destination circle */

          iRadius = (int) ((500) * zoom. ResScaleOverDistanceModify());             /* 1000m circle  */
          Surface.DrawCircle(Pt1.x, Pt1.y, iRadius, rc, false);

          Surface.SelectObject(hOldPen);

      }

/*********************************************************/

  }

/********************************************************************/
    // restore original color
    Surface.SetTextColor(origcolor);
    Surface.SelectObject(oldpen);
    Surface.SelectObject(oldbrush);

}

