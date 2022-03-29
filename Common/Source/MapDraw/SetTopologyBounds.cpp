/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "RGB.h"
#include "Multimap.h"


extern Topology* TopoStore[MAXTOPOLOGY];
#define MINRANGE 0.2

#define NUMRUNS	4 // how many times we loop scanning after a change detected.
//#define DEBUG_STB	1
//#define DEBUG_STB2	1

bool RectangleIsInside(rectObj r_exterior, rectObj r_interior) {
  if ((r_interior.minx >= r_exterior.minx)&&
      (r_interior.maxx <= r_exterior.maxx)&&
      (r_interior.miny >= r_exterior.miny)&&
      (r_interior.maxy <= r_exterior.maxy))
    return true;
  else
    return false;
}

//
// This is called FORCED when changing multimap
//
void SetTopologyBounds(const RECT& rcin, const ScreenProjection& _Proj, const bool force) {
  static rectObj bounds_active;
  static double range_active = 1.0;
  bool unchanged=false;
  static double oldmapscale=0;
  static short runnext=0;

  rectObj bounds_screen = MapWindow::CalculateScreenBounds(1.0, rcin, _Proj);

  if (!force) {
	if (oldmapscale!=MapWindow::zoom.Scale()) {
		#if DEBUG_STB2
		StartupStore(_T("... scale changed to %f\n"),MapWindow::zoom.Scale());
		#endif
		oldmapscale=MapWindow::zoom.Scale();
	} else
		unchanged=true;
  }

  bool recompute = false;

  // only recalculate which shapes when bounds change significantly
  // need to have some trigger for this..

  // trigger if the border goes outside the stored area
  if (!RectangleIsInside(bounds_active, bounds_screen)) {
    #if DEBUG_STB
    StartupStore(_T("(recompute for out of rectangle)\n"));
    #endif
    recompute = true;
  }

  // also trigger if the scale has changed heaps
  double range_real = max((bounds_screen.maxx-bounds_screen.minx),
			  (bounds_screen.maxy-bounds_screen.miny));
  double range = max(MINRANGE,range_real);

  double scale = range/range_active;

  if (max(scale, 1.0/scale)>2) {  // tighter threshold
	#if DEBUG_STB
	StartupStore(_T("(recompute for OUT OF SCALE)\n"),scale);
	#endif
	recompute = true;
  }

  if (recompute || force) {

    #if DEBUG_STB
    StartupStore(_T("..... Run Recompute, Force=%d,  unchanged=%d\n"),force,unchanged);
    #endif

    bounds_active = MapWindow::CalculateScreenBounds(scale, rcin, _Proj);

    range_active = max((bounds_active.maxx-bounds_active.minx),
		       (bounds_active.maxy-bounds_active.miny));

    for (int z=0; z<MAXTOPOLOGY; z++) {
      if (TopoStore[z]) {
	TopoStore[z]->triggerUpdateCache=true;
      }
    }

    // now update visibility of objects in the map window
    MapWindow::ScanVisibility(&bounds_active);

    runnext=NUMRUNS;
    unchanged=false;

  } else {
	if (unchanged) { // nothing has changed, can we skip?
		#if DEBUG_STB2
		StartupStore(_T("... Nothing has changed\n"));
		#endif
		if (runnext<=0) {
			#if DEBUG_STB2
			StartupStore(_T("..... no runnext, skip\n"));
			#endif
			return;
		} else {
			#if DEBUG_STB2
			StartupStore(_T("..... ONE MORE run\n"));
			#endif
			runnext--;
		}
	} else
		runnext=NUMRUNS;
  }


  #if DEBUG_STB
  StartupStore(_T("------ Run full update --- \n"));
  #endif

  // check if things have come into or out of scale limit
  for (int z=0; z<MAXTOPOLOGY; z++) {
    if (TopoStore[z]) {
      TopoStore[z]->TriggerIfScaleNowVisible();
    }
  }

    // check if any needs to have cache updates because wasnt
    // visible previously when bounds moved
    bool sneaked = false;
    bool rta;

    // we will make sure we update at least one cache per call
    // to make sure eventually everything gets refreshed

    for (int z = 0; z < MAXTOPOLOGY; z++) {
        if (TopoStore[z]) {
            rta = MapWindow::RenderTimeAvailable() || force || !sneaked;
            if (TopoStore[z]->triggerUpdateCache) {
                sneaked = true;
            }
            TopoStore[z]->updateCache(bounds_active, !rta);
        }
    }
}
