/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "Asset.hpp"

//#define FAI_SECTOR_DEBUG
#ifdef HAVE_GLES
typedef FloatPoint ScreenPoint;
#else
typedef RasterPoint ScreenPoint;
#endif



#define MAX_FAI_SECTOR_PTS (8*FAI_SECTOR_STEPS)
extern LKColor taskcolor;

class AnalysisProjection {
public:
    AnalysisProjection(const GeoPoint& center, const RECT& rect) : _center(center), _Rect(rect) {}
    
    RasterPoint operator()(const GeoPoint& pt) const {
      double x1 = (pt.longitude - _center.longitude)*fastcosine(_center.latitude);
      double y1 = (pt.latitude - _center.latitude);

      return { 
          static_cast<RasterPoint::scalar_type>(Statistics::ScaleX(_Rect, x1)),
          static_cast<RasterPoint::scalar_type>(Statistics::ScaleY(_Rect, y1))
      };
    }
private:
    const GeoPoint& _center;
    const RECT& _Rect;
};

// #define   FILL_FAI_SECTORS

void FAI_Sector::AnalysisDrawFAISector (LKSurface& Surface, const RECT& rc, const GeoPoint& center, const LKColor& InFfillcolor) {
  
  AnalysisProjection ToScreen(center, rc);
  typedef std::vector<RasterPoint> polyline_t;
  polyline_t FAISector_polyline; // make it static for save memory Alloc/Free ( don't forget to clear in this case )

  FAISector_polyline.reserve( (7.5*FAI_SECTOR_STEPS) + 1); // avoid memory realloc.


  /********************************************************************
   * draw polygon
   ********************************************************************/
  LKPen hpSectorPen(PEN_SOLID, IBLSCALE(1), IsDithered()?RGB_RED:RGB_GREEN);
  LKBrush hbSectorFill(InFfillcolor);

  auto hpOldPen = Surface.SelectObject(hpSectorPen);
  auto hbOldBrush = Surface.SelectObject(hbSectorFill);
  auto oldFont = Surface.SelectObject(LK8PanelUnitFont);

  Surface.SetBackgroundTransparent();
  Surface.SetTextColor(IsDithered()? RGB_GREY : RGB_DARKGREY);

  if(!m_FAIShape.empty()) {
    FAISector_polyline.clear();
    
    for (GPS_Track::const_reference pt : m_FAIShape) {
      FAISector_polyline.push_back(ToScreen(pt));
    }
    FAISector_polyline.push_back(FAISector_polyline.front());
    Surface.Polygon(FAISector_polyline.data(), FAISector_polyline.size());
  }

  if(!m_FAIShape2.empty()) {
    FAISector_polyline.clear();
      
    for (GPS_Track::const_reference pt : m_FAIShape2) {
      FAISector_polyline.push_back(ToScreen(pt));
    }
	FAISector_polyline.push_back(FAISector_polyline.front());
    Surface.Polygon(FAISector_polyline.data(), FAISector_polyline.size());
  }
  
  Surface.SelectObject(hpOldPen);
  
  hpSectorPen.Create(PEN_DASH, ScreenThinSize, RGB_BLACK );
  hpOldPen = Surface.SelectObject(hpSectorPen);

  unsigned  int NumberGrids = m_FAIGridLines.size();
  unsigned int Grid_num = 0;
  
  for (GPS_Gridlines::const_reference line : m_FAIGridLines) {
    FAISector_polyline.clear();
    if (Grid_num <  NumberGrids) {
      for (const GeoPoint& pt : line.GridLine) {
        FAISector_polyline.push_back(ToScreen(pt));
      }
      if(Grid_num <  NumberGrids-1)
        Surface.Polyline(FAISector_polyline.data(), FAISector_polyline.size());
      
      {
        const RasterPoint& pt_start = FAISector_polyline.front();
        Surface.DrawText(pt_start.x, pt_start.y, line.szLable);      
      }

      if(Grid_num > 0) {
        const RasterPoint& pt_start = FAISector_polyline.back();
        Surface.DrawText(pt_start.x, pt_start.y, line.szLable);
      }

      if(Grid_num > 2) {
    	const RasterPoint& pt_mid = FAISector_polyline[FAISector_polyline.size()/2];
        Surface.DrawText(pt_mid.x, pt_mid.y, line.szLable);
      }
    }
    Grid_num++;
  } // if visible
  Surface.SelectObject(oldFont);
  Surface.SelectObject(hpOldPen);
  Surface.SelectObject(hbOldBrush);
}

void FAI_Sector::DrawFAISector (LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj ,const LKColor& InFfillcolor) {

LKColor fillcolor = InFfillcolor;
if (IsDithered()) {
  fillcolor = RGB_SBLACK;
}
const PixelRect ScreenRect(rc);
const GeoToScreen<ScreenPoint> ToScreen(_Proj);

typedef std::vector<ScreenPoint> polyline_t;
polyline_t FAISector_polyline; // make it static for save memory Alloc/Free ( don't forget to clear in this case )

  FAISector_polyline.reserve( (7.5*FAI_SECTOR_STEPS) + 1); // avoid memory realloc.
// FAISector_polyline.clear();


  /********************************************************************
   * draw polygon
   ********************************************************************/
LKPen   hpSectorPen(PEN_SOLID, IBLSCALE(2),  fillcolor );
const auto hpOldPen = Surface.SelectObject(hpSectorPen);
const auto hpOldBrush = Surface.SelectObject(LKBrush_Hollow);
Surface.SetBackgroundTransparent();

  bool bSectorvisible=false;
  if(!m_FAIShape.empty())
  {
    for (GPS_Track::const_reference pt : m_FAIShape) {
      const ScreenPoint Pos = ToScreen(pt);

      if(ScreenRect.IsInside(Pos))
    	bSectorvisible = true;
      FAISector_polyline.push_back(Pos);
    }
  }
    if(bSectorvisible)
    {
		FAISector_polyline.push_back(FAISector_polyline.front());
	#ifdef  FAI_SECTOR_DEBUG
		StartupStore(_T("FAI Sector draw with lat:%8.4f  lon:%8.4f => screen x:%u y:%u  %s"),  m_FAIShape.begin()->latitude,  m_FAIShape.begin()->longitude , FAISector_polyline.begin()->x, FAISector_polyline.begin()->y , NEWLINE);
		StartupStore(_T("FAI Sector draw with %u points  %s"), FAISector_polyline.size(), NEWLINE);
	#endif

	#ifdef FILL_FAI_SECTORS
		 Surface.Polygon(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#else
		 Surface.Polyline(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#endif
    }

    bSectorvisible = false;
	  FAISector_polyline.clear();
	  if(!m_FAIShape2.empty())
	  {
        for (GPS_Track::const_reference pt : m_FAIShape2) {
          const ScreenPoint Pos = ToScreen(pt);
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
		StartupStore(_T("FAI Sector draw 2nd section with %u points  %s"), FAISector_polyline.size(), NEWLINE);
	#endif
	#ifdef FILL_FAI_SECTORS
		 Surface.Polygon(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#else
		 Surface.Polyline(FAISector_polyline.data(), FAISector_polyline.size(), rc);
	#endif
	  }
      hpSectorPen.Create(PEN_DASH, ScreenThinSize, RGB_BLACK );
      Surface.SelectObject(hpSectorPen);
      Surface.SetBackgroundTransparent();
      const auto oldFont = Surface.SelectObject(LK8InfoSmallFont);
	  unsigned  int NumberGrids = m_FAIGridLines.size()-1;
	  unsigned int Grid_num = 0;
	  FAISector_polyline.clear();

	  if(!m_FAIGridLines.empty())
	  {
        for (GPS_Gridlines::const_reference line : m_FAIGridLines) {
		  int iFirstVisible= -1;
		  int iLastVisible = -1;
		  int i=0;
          for (const GeoPoint& pt : line.GridLine) {
            const ScreenPoint Pos = ToScreen(pt);
		    if(ScreenRect.IsInside(Pos))
		    {
		      if(iFirstVisible < 0) iFirstVisible = i;
		      iLastVisible = i;
		    }
			FAISector_polyline.push_back(Pos);
			i++;
		  }
		  if(iFirstVisible >= 0)
		  {
		    if( (Grid_num <  NumberGrids))
		  	  Surface.Polyline(FAISector_polyline.data(), FAI_SECTOR_STEPS, rc);
/*
            const ScreenPoint& pt_start = FAISector_polyline.front();
            if(ScreenRect.IsInside(pt_start)) {
              MapWindow::LKWriteText(Surface, line.szLable, pt_start.x, pt_start.y, WTMODE_OUTLINED,
                                     WTALIGN_LEFT, fillcolor, true);
            }

		    if( Grid_num > 0)
		    {
              const ScreenPoint& pt_end = FAISector_polyline.back();
              if(ScreenRect.IsInside(pt_end)) {
                MapWindow::LKWriteText(Surface, line.szLable, pt_end.x, pt_end.y, WTMODE_OUTLINED,
                                       WTALIGN_LEFT, fillcolor, true);
              }
		    }
		    if( Grid_num > 1)/*/
		    {
              const ScreenPoint& pt_mid = FAISector_polyline[iFirstVisible + (iLastVisible-iFirstVisible)/2];
              if(ScreenRect.IsInside(pt_mid)) {
                MapWindow::LKWriteText(Surface, line.szLable, pt_mid.x, pt_mid.y, WTMODE_OUTLINED,
                                       WTALIGN_LEFT, fillcolor, true);
              }
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

  if(bSectorvisible)
  {
#ifdef  FAI_SECTOR_DEBUG
   	StartupStore(_T("FAI Sector skipped not in view window  %s"), NEWLINE);
#endif
  }


  Surface.SelectObject(hpOldPen);
  Surface.SelectObject(hpOldBrush);
  hpSectorPen.Release();
}

void FAI_Sector::FreeFAISectorMem(void)
{
  m_FAIGridLines.clear();
  m_FAIShape.clear();
  m_FAIShape2.clear();
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

if(fabs(fDist_c) < 10000.0) {  /* distance too short for a FAI sector */
    return false;
}


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
if((fDist_c / FAI_NORMAL_PERCENTAGE) >= FAI28_45Threshold) {
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
if((fDist_c / FAI_NORMAL_PERCENTAGE) >= FAI28_45Threshold) {
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
    fDistMin *= FAI_BIG_PERCENTAGE;
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
      _stprintf(text, TEXT("%i%s"), (int)Units::ToUserDistance(fDistTri+0.5), Units::GetDistanceName());
    else
      _stprintf(text, TEXT("%i"), (int)Units::ToUserDistance(fDistTri+0.5));
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
    NewGrid.fValue = (int)Units::ToUserDistance(fDistTri+0.5);
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

return true;
}


// Draw the current FAI ( or FREE under XC rules ) Cross Country triangle
void MapWindow::DrawXC(LKSurface &Surface, const RECT &rc, const ScreenProjection &_Proj, const POINT &Orig_Aircraft) {

  const GeoToScreen<ScreenPoint> ToScreen(_Proj);

  CContestMgr::XCFlightType fType = CContestMgr::Instance().GetBestXCTriangleType();
  CContestMgr::XCTriangleStatus fStatus ;
  if (fType == CContestMgr::XCFlightType::XC_INVALID )
    return;


  CContestMgr::TType type;
  if (fType == CContestMgr::XCFlightType::XC_FAI_TRIANGLE) {
    type = CContestMgr::TType::TYPE_XC_FAI_TRIANGLE;
    fStatus = CContestMgr::Instance().GetFAITriangleStatus();
  } else {
    type = CContestMgr::TType::TYPE_XC_FREE_TRIANGLE;
    fStatus = CContestMgr::Instance().GetFTTriangleStatus();
  }
  CContestMgr::CResult result = CContestMgr::Instance().Result( type, true);
  const CPointGPSArray &points = result.PointArray();
  unsigned int iSize = points.size();

  if (iSize != 5 )    // Should never be unless not yet calculated
    return;

  std::vector<ScreenPoint> triangle_polyline;

  for ( unsigned int i = 1 ; i< iSize-1 ; i++ ) {
    const ScreenPoint Pos = ToScreen(points[i]);
    triangle_polyline.push_back(Pos);
  }
  const ScreenPoint Pos = ToScreen(points[1]);
  triangle_polyline.push_back(Pos);

  double nextXCRadius = 0;   // the next circle radius to get a better XC scoring coefficient. 0 if we can not increase the scoring coefficient.
  LKColor nextXCRadiusColor = RGB_BLACK;
  if ( fStatus == CContestMgr::XCTriangleStatus::INVALID ) {
    nextXCRadius = CContestMgr::Instance().GetXCValidRadius();
    nextXCRadiusColor = IsDithered()?RGB_BLACK:RGB_GREEN;
    LKPen hpPen_invalid(PEN_SOLID, IBLSCALE(2),  IsDithered()?RGB_BLACK:RGB_ORANGE);
    const auto hpOldPen = Surface.SelectObject(hpPen_invalid);
    Surface.Polyline(triangle_polyline.data(), triangle_polyline.size(), rc);
    Surface.SelectObject(hpOldPen);
    hpPen_invalid.Release();
  }
  else if ( fStatus == CContestMgr::XCTriangleStatus::VALID   ) {
    nextXCRadius = CContestMgr::Instance().GetXCClosedRadius();
    nextXCRadiusColor = IsDithered()?RGB_BLACK:RGB_RED;
    LKPen hpPen_valid(PEN_SOLID, IBLSCALE(2),  IsDithered()?RGB_BLACK:RGB_GREEN );
    const auto hpOldPen = Surface.SelectObject(hpPen_valid);
    Surface.Polyline(triangle_polyline.data(), triangle_polyline.size(), rc);
    Surface.SelectObject(hpOldPen);
    hpPen_valid.Release();
  }
  else {      // fStatus == CContestMgr::XCTriangleStatus::CLOSED
    LKPen hpPen_closed(PEN_SOLID, IBLSCALE(2),  IsDithered()?RGB_BLACK:RGB_RED );
    const auto hpOldPen = Surface.SelectObject(hpPen_closed);
    Surface.Polyline(triangle_polyline.data(), triangle_polyline.size(), rc);
    Surface.SelectObject(hpOldPen);
    hpPen_closed.Release();
  }

  if (  nextXCRadius > 0 ) {
    const double lat_CP = CContestMgr::Instance().GetXCTriangleClosingPoint().Latitude();
    const double lon_CP = CContestMgr::Instance().GetXCTriangleClosingPoint().Longitude();
    if ( lat_CP !=0 && lon_CP!= 0) {
      const ScreenPoint Pos = ToScreen(lat_CP, lon_CP);
      int iRadius = (int) (nextXCRadius * zoom.ResScaleOverDistanceModify());

      double fDist,fAngle;
      DistanceBearing(lat_CP, lon_CP, GPS_INFO.Latitude, GPS_INFO.Longitude, &fDist, &fAngle);
      if ( OvertargetMode ==  OVT_XC ) {
        LKPen hpSectorPen(PEN_SOLID, IBLSCALE(2), nextXCRadiusColor);
        const auto hOldPen = Surface.SelectObject(hpSectorPen);
        Surface.DrawArc(Pos.x, Pos.y, iRadius, rc, fAngle - 20 - DisplayAngle, fAngle + 20 - DisplayAngle);
        Surface.SelectObject(hOldPen);
      }
      else {
        LKPen hpSectorPen(PEN_SOLID, IBLSCALE(1), nextXCRadiusColor);
        const auto hOldPen = Surface.SelectObject(hpSectorPen);
        Surface.DrawCircle(Pos.x, Pos.y, iRadius, rc, false);
        Surface.SelectObject(hOldPen);
      }
    }
  }
}

void MapWindow::DrawFAIOptimizer(LKSurface &Surface, const RECT &rc, const ScreenProjection &_Proj, const POINT &Orig_Aircraft) {

  static FAI_Sector FAI_SectorCache[5];
  const GeoToScreen<ScreenPoint> ToScreen(_Proj);
  CContestMgr::CResult result = CContestMgr::Instance().Result(CContestMgr::TYPE_XC_FREE_TRIANGLE, true);

  const CPointGPSArray &points = result.PointArray();
  unsigned int iSize = points.size();
  if (iSize != 5) {// Something went wrong here
    return;
  }

  const CContestMgr::TriangleLeg *max_leg = CContestMgr::Instance().GetFAIAssistantMaxLeg();
  const CContestMgr::TriangleLeg *leg0 = CContestMgr::Instance().GetFAIAssistantLeg(0);
  const CContestMgr::TriangleLeg *leg1 = CContestMgr::Instance().GetFAIAssistantLeg(1);
  const CContestMgr::TriangleLeg *leg2 = CContestMgr::Instance().GetFAIAssistantLeg(2);
  const double distance = leg0->LegDist + leg1->LegDist + leg2->LegDist;

  if (max_leg == nullptr || max_leg->LegDist < FAI_MIN_DISTANCE_THRESHOLD) {
    return;
  }

  float fZoom = MapWindow::zoom.RealScale();
  double fTic = 100;
  if (fZoom > 50) fTic = 100;
  else if (fZoom > 20) fTic = 50;
  else if (fZoom > 10) fTic = 25; else fTic = 25;

  fTic = Units::ToSysDistance(fTic);

  const auto whitecolor = RGB_WHITE;
  const auto origcolor = Surface.SetTextColor(whitecolor);
  const auto oldpen = Surface.SelectObject(hpStartFinishThin);
  const auto oldbrush = Surface.SelectObject(LKBrush_Hollow);

  if (!CContestMgr::Instance().LooksLikeAFAITriangleAttempt()) {
    // Does not look like a FAI attempt. Just draw both FAI sectors on longest leg.
    FAI_SectorCache[0].CalcSectorCache(max_leg->Lat1, max_leg->Lon1, max_leg->Lat2, max_leg->Lon2, fTic, 0);
    FAI_SectorCache[0].DrawFAISector(Surface, rc, _Proj, IsDithered()?RGB_BLACK:RGB_YELLOW);
    FAI_SectorCache[1].CalcSectorCache(max_leg->Lat1, max_leg->Lon1, max_leg->Lat2, max_leg->Lon2, fTic, 1);
    FAI_SectorCache[1].DrawFAISector(Surface, rc, _Proj, IsDithered()?RGB_BLACK:RGB_YELLOW);
  } else {
    if (leg0->LegDist > FAI_MIN_DISTANCE_THRESHOLD) {
      // Draw the yellow sector on the best current direction.
      FAI_SectorCache[2].CalcSectorCache(leg0->Lat1, leg0->Lon1, leg0->Lat2, leg0->Lon2, fTic, CContestMgr::Instance().isFAITriangleClockwise());
      FAI_SectorCache[2].DrawFAISector(Surface, rc, _Proj, IsDithered()?RGB_BLACK:RGB_YELLOW);
    }
    // Draw leg1 a bit before becoming a FAI one in the correct direction . We start drawing a bit before 28%
    if (leg1->LegDist > distance * 0.25) {
      FAI_SectorCache[3].CalcSectorCache(leg1->Lat1, leg1->Lon1, leg1->Lat2, leg1->Lon2, fTic, CContestMgr::Instance().isFAITriangleClockwise());
      FAI_SectorCache[3].DrawFAISector(Surface, rc, _Proj, IsDithered()?RGB_BLACK:RGB_CYAN);
    }
  }

  Surface.SetTextColor(origcolor);
  Surface.SelectObject(oldpen);
  Surface.SelectObject(oldbrush);

}

