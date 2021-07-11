/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "Topology/ShapeSpecialRenderer.h"


Topology* TopoStore[MAXTOPOLOGY] = {};


void DrawTopology(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const bool wateronly)
{
  LockTerrainDataGraphics();
  static ShapeSpecialRenderer renderer;
  for(const Topology* topo: TopoStore) {
    if(!topo) {
      continue;
    }
    if(!wateronly || topo->scaleCategory == 5 || topo->scaleCategory == 10 || topo->scaleCategory == 20) {
      topo->Paint(renderer, Surface,rc, _Proj);
    }
  }
  renderer.Render(Surface);
  renderer.Clear();
  UnlockTerrainDataGraphics();
}
