/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Multimap.h"
#include "ScreenProjection.h"
#include "Task/TaskRendererMgr.h"
#include <functional>
using std::placeholders::_1;

int MapWindow::iSnailNext=0;
int MapWindow::iLongSnailNext=0;

rectObj MapWindow::screenbounds_latlon;



rectObj MapWindow::CalculateScreenBounds(double scale, const RECT& rc, const ScreenProjection& _Proj) {
  // compute lat lon extents of visible screen
  rectObj sb;

  if (scale>= 1.0) {
    const POINT screen_center = _Proj.ToRasterPoint(PanLatitude, PanLongitude);

    sb.minx = sb.maxx = PanLongitude;
    sb.miny = sb.maxy = PanLatitude;
    
    unsigned int dx, dy;
    unsigned int maxsc=0;
    dx = screen_center.x-rc.right;
    dy = screen_center.y-rc.top;
    maxsc = max(maxsc, dx*dx+dy*dy);
    dx = screen_center.x-rc.left;
    dy = screen_center.y-rc.top;
    maxsc = max(maxsc, dx*dx+dy*dy);
    dx = screen_center.x-rc.left;
    dy = screen_center.y-rc.bottom;
    maxsc = max(maxsc, dx*dx+dy*dy);
    dx = screen_center.x-rc.right;
    dy = screen_center.y-rc.bottom;
    maxsc = max(maxsc, dx*dx+dy*dy);
    maxsc = isqrt4(maxsc);
    
    for (int i=0; i<10; i++) {
      double ang = i*360.0/10;
      double X, Y;
      const RasterPoint p {
        screen_center.x + iround(fastcosine(ang)*maxsc*scale),
        screen_center.y + iround(fastsine(ang)*maxsc*scale)
      };
      _Proj.Screen2LonLat(p, X, Y);
      sb.minx = min(X, sb.minx);
      sb.miny = min(Y, sb.miny);
      sb.maxx = max(X, sb.maxx);
      sb.maxy = max(Y, sb.maxy);
    }

  } else {

    const PixelRect ScreenRect(rc);
    double X, Y;
    
    _Proj.Screen2LonLat(ScreenRect.GetTopLeft(), X, Y);
    sb.minx = sb.maxx = X;
    sb.miny = sb.maxy = Y;

    _Proj.Screen2LonLat(ScreenRect.GetTopRight(), X, Y);
    sb.minx = min(sb.minx, X); sb.maxx = max(sb.maxx, X);
    sb.miny = min(sb.miny, Y); sb.maxy = max(sb.maxy, Y);
  
    _Proj.Screen2LonLat(ScreenRect.GetBottomRight(), X, Y);
    sb.minx = min(sb.minx, X); sb.maxx = max(sb.maxx, X);
    sb.miny = min(sb.miny, Y); sb.maxy = max(sb.maxy, Y);
  
    _Proj.Screen2LonLat(ScreenRect.GetBottomLeft(), X, Y);
    sb.minx = min(sb.minx, X); sb.maxx = max(sb.maxx, X);
    sb.miny = min(sb.miny, Y); sb.maxy = max(sb.maxy, Y);
  }

  return sb;
}



void MapWindow::CalculateScreenPositionsThermalSources(const ScreenProjection& _Proj) {
  for (int i=0; i<MAX_THERMAL_SOURCES; i++) {
    if (DerivedDrawInfo.ThermalSources[i].LiftRate>0) {
      double dh = DerivedDrawInfo.NavAltitude
        -DerivedDrawInfo.ThermalSources[i].GroundHeight;
      if (dh<0) {
        DerivedDrawInfo.ThermalSources[i].Visible = false;
        continue;
      }

      double t = dh/DerivedDrawInfo.ThermalSources[i].LiftRate;
      double lat, lon;
      FindLatitudeLongitude(DerivedDrawInfo.ThermalSources[i].Latitude, 
                            DerivedDrawInfo.ThermalSources[i].Longitude,
                            DerivedDrawInfo.WindBearing, 
                            -DerivedDrawInfo.WindSpeed*t,
                            &lat, &lon);
      if (PointVisible(lon,lat)) {
        DerivedDrawInfo.ThermalSources[i].Screen = _Proj.ToRasterPoint(lat, lon);
        DerivedDrawInfo.ThermalSources[i].Visible = 
          PointVisible(DerivedDrawInfo.ThermalSources[i].Screen);
      } else {
        DerivedDrawInfo.ThermalSources[i].Visible = false;
      }
    } else {
      DerivedDrawInfo.ThermalSources[i].Visible = false;
    }
  }
}



void MapWindow::CalculateScreenPositionsAirspace(const RECT& rcDraw, const ScreenProjection& _Proj)
{
#ifndef HAVE_HATCHED_BRUSH
  // iAirspaceBrush is not used and don't exist if we don't have Hatched Brush
  // this is workarround for compatibility with #CalculateScreenPositionsAirspace
  constexpr int iAirspaceBrush[AIRSPACECLASSCOUNT] = {}; 
#endif
  CAirspaceManager::Instance().CalculateScreenPositionsAirspace(screenbounds_latlon, iAirspaceMode, iAirspaceBrush, rcDraw, _Proj);
}




ScreenProjection MapWindow::CalculateScreenPositions(const POINT& Orig, const RECT& rc, POINT *Orig_Aircraft )
{
  Orig_Screen = Orig;

  if (!mode.AnyPan()) {
  
    if (GliderCenter 
        && DerivedDrawInfo.Circling 
        && (EnableThermalLocator==2)) {
      
      if (DerivedDrawInfo.ThermalEstimate_R>0) {
        PanLongitude = DerivedDrawInfo.ThermalEstimate_Longitude; 
        PanLatitude = DerivedDrawInfo.ThermalEstimate_Latitude;
        // TODO enhancement: only pan if distance of center to
        // aircraft is smaller than one third screen width

        const ScreenProjection _Proj;
        *Orig_Aircraft = _Proj.ToRasterPoint(DrawInfo.Latitude, DrawInfo.Longitude);
        const POINT screen = _Proj.ToRasterPoint(PanLatitude, PanLongitude);
        
        if ((fabs((double)Orig_Aircraft->x-screen.x)<(rc.right-rc.left)/3)
            && (fabs((double)Orig_Aircraft->y-screen.y)<(rc.bottom-rc.top)/3)) {
          
        } else {
          // out of bounds, center on aircraft
          PanLongitude = DrawInfo.Longitude;
          PanLatitude = DrawInfo.Latitude;
        }
      } else {
        PanLongitude = DrawInfo.Longitude;
        PanLatitude = DrawInfo.Latitude;
      }
    } else {
      // Pan is off
      PanLongitude = DrawInfo.Longitude;
      PanLatitude = DrawInfo.Latitude;
    }
  }

  const ScreenProjection _Proj;
  *Orig_Aircraft = _Proj.ToRasterPoint(DrawInfo.Latitude, DrawInfo.Longitude);

  // very important
  screenbounds_latlon = CalculateScreenBounds(0.0, rc, _Proj);

  CalculateScreenPositionsThermalSources(_Proj);
  CalculateScreenPositionsGroundline(_Proj);
  
  // Old note obsoleted 121111: 
  // preserve this calculation for 0.0 until next round!
  // This is already done since screenbounds_latlon is global. Beware that DrawTrail will change it later on
  // to expand boundaries by 1 minute

  // get screen coordinates for all task waypoints

  LockTaskData();

  if (!WayPointList.empty()) {
    /* Is needed ? */
    for(auto& wpt : WayPointList) {
        wpt.Visible = PointVisible(wpt.Longitude, wpt.Latitude);
    }
  }

  if(TrailActive)
  {
    iSnailNext = SnailNext; 
    iLongSnailNext = LongSnailNext; 
    // set this so that new data doesn't arrive between calculating
    // this and the screen updates
  }

  gStartSectorRenderer.CalculateScreenPosition(screenbounds_latlon, _Proj);
  gTaskSectorRenderer.CalculateScreenPosition(screenbounds_latlon, _Proj);

  if (gTaskType == TSK_AAT) {
		if(ValidTaskPoint(ActiveTaskPoint)) {
			TASKSTATS_POINT& StatPt =  TaskStats[ActiveTaskPoint];
			for (int j=0; j<MAXISOLINES; j++) {
				if (StatPt.IsoLine_valid[j]) {
					StatPt.IsoLine_Screen[j] = _Proj.ToRasterPoint(StatPt.IsoLine_Latitude[j], StatPt.IsoLine_Longitude[j]);
				}
			}
		}

		if ((mode.Is(Mode::MODE_TARGET_PAN) && ValidTaskPoint(TargetPanIndex))) {
			TASKSTATS_POINT& StatPt =  TaskStats[TargetPanIndex];
			for (int j=0; j<MAXISOLINES; j++) {
				if (StatPt.IsoLine_valid[j]) {
					StatPt.IsoLine_Screen[j] = _Proj.ToRasterPoint(StatPt.IsoLine_Latitude[j], StatPt.IsoLine_Longitude[j]);
				}
			}
		}
	}
  UnlockTaskData();

  return _Proj;
}

void MapWindow::CalculateScreenPositionsGroundline(const ScreenProjection& _Proj) {
    static_assert(Groundline.size() == std::size(DerivedDrawInfo.GlideFootPrint), "wrong array size");

    typedef decltype(Groundline) array_t;
    typedef array_t::value_type point_t;

    typedef decltype(Groundline2) array2_t;
    typedef array2_t::value_type point2_t;


    static_assert(std::is_same<point2_t, point_t>::value, "Groundline & Groundline2 need to same value type");


    const GeoToScreen<point_t> ToScreen(_Proj);

    if (DerivedDrawInfo.GlideFootPrint_valid) {
        std::transform(
                std::begin(DerivedDrawInfo.GlideFootPrint),
                std::end(DerivedDrawInfo.GlideFootPrint),
                std::begin(Groundline), std::ref(ToScreen));

#ifdef ENABLE_OPENGL
        // first point is center of polygon (OpenGL GL_TRIANGLE_FAN), polyline start is second point
        assert((*std::next(Groundline.begin())) == Groundline.back());
#else
        assert(Groundline.front() == Groundline.back());
#endif
    }

    static_assert(Groundline2.size() == std::size(DerivedDrawInfo.GlideFootPrint2), "wrong array size");

    if (DerivedDrawInfo.GlideFootPrint2_valid) {
        // show next-WP line
        std::transform(
                std::begin(DerivedDrawInfo.GlideFootPrint2),
                std::end(DerivedDrawInfo.GlideFootPrint2),
                std::begin(Groundline2), std::ref(ToScreen));

        assert(Groundline2.front() == Groundline2.back());
    }
}


