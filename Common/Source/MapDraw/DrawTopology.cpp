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


void DrawTopology(LKSurface& Surface, const RECT& rc, const bool wateronly)
{
  static double lastForceNearest=0;

  LockTerrainDataGraphics();

  // Ok I explain what is the logic here. We want the nearest, which is triggered by dlgOracle.
  // The dlgOracle function is zoomin in to 2km scale and this is causing reload of topology
  // up to very small cities details without including far away points.
  // At the same time, it is setting ForceNearestTopologyCalculation to true.
  // The oracle page is over the map now, and we dont paint topology: we calculate nearest instead.
  // Then we clear the flag, so we do the calculation only once.
  // The Oracle checks that we have cleared ForceNearestTopologyCalculation here, and that means
  // that data is available from SearchNearest.
  if (LKSW_ForceNearestTopologyCalculation) {
	if (lastForceNearest==0) {
		lastForceNearest=GPS_INFO.Time;
		goto _exit;
	}
	// We must wait for a few seconds to be sure cache was updated
	if ( (GPS_INFO.Time - lastForceNearest)<2) goto _exit;
	for (int z=0; z<MAXTOPOLOGY; z++) {
		if (TopoStore[z]) {
			// See also CheckScale for category checks! We should use a function in fact.
			if ( TopoStore[z]->scaleCategory == 10 ||
			     (TopoStore[z]->scaleCategory >= 70 && TopoStore[z]->scaleCategory <=100))
				TopoStore[z]->SearchNearest(rc); 
		}
	}
	LKSW_ForceNearestTopologyCalculation=false; // Done, the Oracle can compute now.
  } else {
	lastForceNearest=0;
	if (wateronly) {
		for (int z=0; z<MAXTOPOLOGY; z++) {
			if (TopoStore[z]) {
				if (	TopoStore[z]->scaleCategory == 5 ||
			     		TopoStore[z]->scaleCategory == 10 ||
			     		TopoStore[z]->scaleCategory == 20 
				) {
					TopoStore[z]->Paint(Surface,rc);
				}
			}
		}
	} else {
		for (int z=0; z<MAXTOPOLOGY; z++) {
			if (TopoStore[z]) {
				TopoStore[z]->Paint(Surface,rc);
			}
		}
	}
  }
_exit:
  UnlockTerrainDataGraphics();

}

