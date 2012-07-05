/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"


// #define SKIPPOINTS 1		// skip closer points for drawing, causing flashing

#define TRAIL_DRIFT_FIX 1
//      Attempts to fix bug that caused trail points to disappear
//      (more so in stronger winds and when more zoomed in) while
//      in circling zoom with “trail drift” on.
//      by Eric Carden, March 4, 2012

// try not to use colors when over a useless mapscale
double MapWindow::LKDrawTrail( HDC hdc, const POINT Orig, const RECT rc)
{
  if(!TrailActive) return -1;

  int i, snail_index;
  SNAIL_POINT P1;
  int  nearby;

  bool usecolors=false;
  bool trail_is_drifted=false;

  double traildrift_lat = 0.0;
  double traildrift_lon = 0.0;

  #ifdef TRAIL_DRIFT_FIX
  double this_lat, this_lon;  // lat & lon of point as it is to be drawn on
                              // screen (accounts for “trail drift” if
                              // appropriate)
  #endif

  double trailFirstTime = -1;

  if (MapWindow::zoom.RealScale() <2) {  // 1.5 is also quite good
	usecolors=true;
  }

  if ( EnableTrailDrift && MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING) && DerivedDrawInfo.WindSpeed >=1 ) {

	trail_is_drifted=true;

	double tlat1, tlon1;
	FindLatitudeLongitude(DrawInfo.Latitude, DrawInfo.Longitude, 
		DerivedDrawInfo.WindBearing, DerivedDrawInfo.WindSpeed,
		&tlat1, &tlon1);

	traildrift_lat = (DrawInfo.Latitude-tlat1);
	traildrift_lon = (DrawInfo.Longitude-tlon1);
  }

  //  Trail size
  int num_trail_max;
  if (TrailActive!=2) {
	// scan entire trail for sink magnitude
	num_trail_max = TRAILSIZE;
  } else {
	// scan only recently for lift magnitude
	num_trail_max = TRAILSIZE/TRAILSHRINK;
  }
  if (MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING)) {
	num_trail_max /= TRAILSHRINK;
  }

  // Snail skipping 

  const int skip_divisor = num_trail_max/5;
  int skip_border = skip_divisor;
  int skip_level= 3; // TODO code: try lower level?

  int snail_offset = TRAILSIZE+iSnailNext-num_trail_max;
  while (snail_offset>= TRAILSIZE) {
    snail_offset -= TRAILSIZE;
  }
  while (snail_offset< 0) {
    snail_offset += TRAILSIZE;
  }
  const int zero_offset = (TRAILSIZE-snail_offset);
  skip_border += zero_offset % skip_level;

  int index_skip = ((int)DrawInfo.Time)%skip_level;

  // TODO code: Divide by time step cruise/circling for zero_offset

  // Keep track of what's drawn

  bool this_visible = true;
  bool last_visible = false;
  POINT point_lastdrawn;
  point_lastdrawn.x = 0;
  point_lastdrawn.y = 0;

  #if SKIPPOINTS
  // Average colour display for skipped points
  float vario_av = 0;
  int vario_av_num = 0;
  #endif

  // Constants for speedups

  const bool display_circling = MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING);
  const double display_time = DrawInfo.Time;

  // expand bounds so in strong winds the appropriate snail points are
  // still visible (since they are being tested before drift is applied)
  // this expands them by one minute

  // NOT a good idea, other functions will assume to be within screen boundaries..
  rectObj bounds_thermal = screenbounds_latlon;
  if (trail_is_drifted) {
	screenbounds_latlon.minx -= fabs(60.0*traildrift_lon);
	screenbounds_latlon.maxx += fabs(60.0*traildrift_lon);
	screenbounds_latlon.miny -= fabs(60.0*traildrift_lat);
	screenbounds_latlon.maxy += fabs(60.0*traildrift_lat);
  }
  const rectObj bounds = bounds_thermal;

  const int deg = DEG_TO_INT(AngleLimit360(DisplayAngle));
  const int cost = ICOSTABLE[deg];
  const int sint = ISINETABLE[deg];
  const int xxs = Orig_Screen.x*1024-512;
  const int yys = Orig_Screen.y*1024+512;
  const double mDrawScale = zoom.DrawScale();
  const double mPanLongitude = PanLongitude;
  const double mPanLatitude = PanLatitude;

  if (usecolors)
	nearby=NIBLSCALE(2);
  else
	nearby=NIBLSCALE(4);

  // int skipped=0, painted=0; // test mode

  // 
  // Main loop
  //
  for(i=1;i< num_trail_max; ++i) 
  {
    ///// Handle skipping

    if (i>=skip_border) {
      skip_level= max(1,skip_level-1);
      skip_border= i+2*(zero_offset % skip_level)+skip_divisor;
      index_skip = skip_level;
    }

    index_skip++;
    if ((i<num_trail_max-10) && (index_skip < skip_level)) {
      continue;
    } else {
      index_skip=0;
    }

    ////// Find the snail point

    snail_index = snail_offset+i;
    while (snail_index>=TRAILSIZE) {
      snail_index-= TRAILSIZE;
    }

    P1 = SnailTrail[snail_index];

    /////// Mark first time of display point

    if (((trailFirstTime<0) || (P1.Time<trailFirstTime)) && (P1.Time>=0)) {
      trailFirstTime = P1.Time;
    }

    //////// Ignoring display elements for modes

    if (display_circling) {
      if ((!P1.Circling)&&( i<num_trail_max-60 )) {
        // ignore cruise mode lines unless very recent
	last_visible = false;
        continue;
      }
    }
    #if 0
    else {
      //  if ((P1.Circling)&&( snail_index % 5 != 0 )) {
        // JMW TODO code: This won't work properly!
        // draw only every 5 points from circling when in cruise mode
	//        continue;
      //      }
    }
    #endif

    ///////// Filter if far visible

    if (!P1.FarVisible) {
      last_visible = false;
      continue;
    }

    #ifdef TRAIL_DRIFT_FIX  // calc drifted lat/lon BEFORE determining “visibility”
    if (trail_is_drifted) {
      double dt = max(0.0, (display_time - P1.Time) * P1.DriftFactor);
      this_lat = P1.Latitude + traildrift_lat * dt;
      this_lon = P1.Longitude + traildrift_lon * dt;
    } else {                    // lat & lon NOT drifted
      this_lat = P1.Latitude;
      this_lon = P1.Longitude;
    }

    this_visible = ((this_lon > bounds.minx) &&
                    (this_lon < bounds.maxx) &&
                    (this_lat > bounds.miny) &&
                    (this_lat < bounds.maxy));
    #else

    ///////// Determine if this is visible

    this_visible =   ((P1.Longitude> bounds.minx) &&
		     (P1.Longitude< bounds.maxx) &&
		     (P1.Latitude> bounds.miny) &&
		     (P1.Latitude< bounds.maxy)) ;
    #endif

    if (!this_visible && !last_visible) {
      last_visible = false;
      continue;
    }

    ////////// Find coordinates on screen after applying trail drift

    // now we know either point is visible, better get screen coords
    // if we don't already.

    if (trail_is_drifted) {

        #ifndef TRAIL_DRIFT_FIX

	double dt = max(0.0,(display_time-P1.Time)*P1.DriftFactor);
	double this_lon = P1.Longitude+traildrift_lon*dt;
	double this_lat = P1.Latitude+traildrift_lat*dt;

        #endif

	#if 1
	// this is faster since many parameters are const
	int Y = Real2Int((mPanLatitude-this_lat)*mDrawScale);
	int X = Real2Int((mPanLongitude-this_lon)*fastcosine(this_lat)*mDrawScale);
	P1.Screen.x = (xxs-X*cost + Y*sint)/1024;
	P1.Screen.y = (Y*cost + X*sint + yys)/1024;
	#else
	LatLon2Screen(this_lon, this_lat, P1.Screen);
	#endif
    } else {
	int Y = Real2Int((mPanLatitude-P1.Latitude)*mDrawScale);
	int X = Real2Int((mPanLongitude-P1.Longitude)*fastcosine(P1.Latitude)*mDrawScale);
	P1.Screen.x = (xxs-X*cost + Y*sint)/1024;
	P1.Screen.y = (Y*cost + X*sint + yys)/1024;
    }


    // skip closer points
    if (last_visible && this_visible ) {
	// only average what's visible
	if ( (abs(P1.Screen.y-point_lastdrawn.y) + abs(P1.Screen.x-point_lastdrawn.x))<=nearby ) {
		#if SKIPPOINTS
		if (usecolors) {
			vario_av += P1.Vario;
			vario_av_num ++;
		}
		#endif
		// skipped++;
		continue;
		// don't draw if very short line
	}
    }
    // painted++;

    if (usecolors) {
	float offval=1.0;
	int usecol;

	#if SKIPPOINTS
	float useval;
	if ( vario_av_num ) useval=vario_av/(vario_av_num+1); else useval=P1.Vario; // 091202 avnum
	if (useval<0) offval=-1;
	useval=fabs(useval);
	#else
	if (P1.Vario<0) offval=-1;
	const float useval=fabs(P1.Vario);
	#endif

	
	if ( useval <0.1 ) {
		P1.Colour=7;
		goto go_selcolor;
	}
	if (useval <=0.5 ) {; usecol=1; goto go_setcolor; }
	if (useval <=1.0 ) {; usecol=2; goto go_setcolor; }
	if (useval <=1.5 ) {; usecol=3; goto go_setcolor; }
	if (useval <=2.0 ) {; usecol=4; goto go_setcolor; }
	if (useval <=3.0 ) {; usecol=5; goto go_setcolor; }
	if (useval <=4.0 ) {; usecol=6; goto go_setcolor; }
	usecol=7; // 7th : 4ms and up

go_setcolor:
	P1.Colour = 7+(short int)(usecol*offval);
    } else {
	P1.Colour = 3; // blue
    }

go_selcolor:
    SelectObject(hdc, hSnailPens[P1.Colour]);

    if (!last_visible) { // draw set cursor at P1
	#ifndef NOLINETO
	MoveToEx(hdc, P1.Screen.x, P1.Screen.y, NULL);
	#endif
    } else {
	#ifndef NOLINETO
	LineTo(hdc, P1.Screen.x, P1.Screen.y);
	#else
	DrawSolidLine(hdc, P1.Screen, point_lastdrawn, rc);
	#endif
    }
    point_lastdrawn = P1.Screen;
    last_visible = this_visible;
  } // big loop 

  // draw final point to glider
  if (last_visible) {
#ifndef NOLINETO 
    LineTo(hdc, Orig.x, Orig.y);
#else
    DrawSolidLine(hdc, Orig, point_lastdrawn, rc);
#endif
  }

  // StartupStore(_T("....zoom=%.3f trail max=%d  painted=%d   skipped=%d\n"), MapWindow::zoom.Scale(), num_trail_max,painted,skipped);

  return trailFirstTime;
}

