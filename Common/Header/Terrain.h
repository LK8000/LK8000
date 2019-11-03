#ifndef TERRAIN_H
#define TERRAIN_H
#include "externs.h"
#include "Topology.h"


void SetTopologyBounds(const RECT& rcin, const ScreenProjection& _Proj,  const bool force=false);

void OpenTopology();
void CloseTopology();
void ChangeZoomTopology(int iCategory, double newScale, short cztmode);
double ReadZoomTopology(int iCategory);
bool HaveZoomTopology(int iCategory);
void DrawTopology(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const bool wateronly=false);
bool DrawTerrain(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const double sunazimuth, const double sunelevation);

void MarkLocation(const double lon, const double lat, const double altitude);

void CloseTerrainRenderer();
#endif
