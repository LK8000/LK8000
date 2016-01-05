/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "RasterTerrain.h"
#include "RGB.h"




extern Topology* TopoStore[MAXTOPOLOGY];


void DrawTopology(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const bool wateronly)
{
    LockTerrainDataGraphics();

	if (wateronly) {
		for (int z=0; z<MAXTOPOLOGY; z++) {
			if (TopoStore[z]) {
				if (	TopoStore[z]->scaleCategory == 5 ||
			     		TopoStore[z]->scaleCategory == 10 ||
			     		TopoStore[z]->scaleCategory == 20 
				) {
					TopoStore[z]->Paint(Surface,rc, _Proj);
				}
			}
		}
	} else {
		for (int z=0; z<MAXTOPOLOGY; z++) {
			if (TopoStore[z]) {
				TopoStore[z]->Paint(Surface,rc, _Proj);
			}
		}
	}
    UnlockTerrainDataGraphics();
}

