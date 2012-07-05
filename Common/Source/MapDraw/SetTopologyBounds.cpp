/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "STScreenBuffer.h"
#include "RGB.h"



extern Topology* TopoStore[MAXTOPOLOGY];
#define MINRANGE 0.2



bool RectangleIsInside(rectObj r_exterior, rectObj r_interior) {
  if ((r_interior.minx >= r_exterior.minx)&&
      (r_interior.maxx <= r_exterior.maxx)&&
      (r_interior.miny >= r_exterior.miny)&&
      (r_interior.maxy <= r_exterior.maxy))    
    return true;
  else 
    return false;
}


void SetTopologyBounds(const RECT rcin, const bool force) {
  static rectObj bounds_active;
  static double range_active = 1.0;
  rectObj bounds_screen;
  (void)rcin;
  bounds_screen = MapWindow::CalculateScreenBounds(1.0);

  bool recompute = false;

  // only recalculate which shapes when bounds change significantly
  // need to have some trigger for this..

  // trigger if the border goes outside the stored area
  if (!RectangleIsInside(bounds_active, bounds_screen)) {
    recompute = true;
  }
  
  // also trigger if the scale has changed heaps
  double range_real = max((bounds_screen.maxx-bounds_screen.minx), 
			  (bounds_screen.maxy-bounds_screen.miny));
  double range = max(MINRANGE,range_real);

  double scale = range/range_active;
  if (max(scale, 1.0/scale)>4) {
    recompute = true;
  }

  if (recompute || force || LKSW_ForceNearestTopologyCalculation) {

    // make bounds bigger than screen
    if (range_real<MINRANGE) {
      scale = BORDERFACTOR*MINRANGE/range_real;
    } else {
      scale = BORDERFACTOR;
    }
    bounds_active = MapWindow::CalculateScreenBounds(scale);

    range_active = max((bounds_active.maxx-bounds_active.minx), 
		       (bounds_active.maxy-bounds_active.miny));
    
    for (int z=0; z<MAXTOPOLOGY; z++) {
      if (TopoStore[z]) {
	TopoStore[z]->triggerUpdateCache=true;          
      }
    }
    #if USETOPOMARKS
    if (topo_marks) {
      topo_marks->triggerUpdateCache = true;
    }
    #endif

    // now update visibility of objects in the map window
    MapWindow::ScanVisibility(&bounds_active);

  }

  // check if things have come into or out of scale limit
  for (int z=0; z<MAXTOPOLOGY; z++) {
    if (TopoStore[z]) {
      TopoStore[z]->TriggerIfScaleNowVisible();          
    }
  }

  // ok, now update the caches
  #if USETOPOMARKS 
  if (topo_marks) {
    topo_marks->updateCache(bounds_active);
  }
  #endif
  
  if (EnableTopology) {
    // check if any needs to have cache updates because wasnt 
    // visible previously when bounds moved
    bool sneaked= false;
    bool rta;

    // we will make sure we update at least one cache per call
    // to make sure eventually everything gets refreshed

    int total_shapes_visible = 0;
    for (int z=0; z<MAXTOPOLOGY; z++) {
      if (TopoStore[z]) {
	rta = MapWindow::RenderTimeAvailable() || force || !sneaked;
	if (TopoStore[z]->triggerUpdateCache) {
	  sneaked = true;
	}
	TopoStore[z]->updateCache(bounds_active, !rta);
	total_shapes_visible += TopoStore[z]->shapes_visible_count;
      }
    }
#ifdef DEBUG_GRAPHICS
    DebugStore("%d # shapes\n", total_shapes_visible);
#endif

  }
}


