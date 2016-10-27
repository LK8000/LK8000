#ifndef TERRAIN_H
#define TERRAIN_H
#include "externs.h"
#include "Topology.h"

#define NUM_COLOR_RAMP_LEVELS 13
#define NUMRAMPS        16

typedef struct _COLORRAMP
{
  unsigned short h;
  unsigned char r;
  unsigned char g;
  unsigned char b;
} COLORRAMP;

#if USETOPOMARKS
extern bool reset_marks;
#endif

void SetTopologyBounds(const RECT& rcin, const ScreenProjection& _Proj,  const bool force=false);
#if USETOPOMARKS
void TopologyInitialiseMarks();
void TopologyCloseMarks();
#endif
void OpenTopology();
void CloseTopology();
void ChangeZoomTopology(int iCategory, double newScale, short cztmode);
double ReadZoomTopology(int iCategory);
bool HaveZoomTopology(int iCategory);
void DrawTopology(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const bool wateronly=false);
bool DrawTerrain(LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj, const double sunazimuth, const double sunelevation);

#if USETOPOMARKS
void DrawMarks(const HDC hdc, const RECT rc);
void MarkLocation(const double lon, const double lat);
#else
void MarkLocation(const double lon, const double lat, const double altitude);
#endif
void CloseTerrainRenderer();
#endif
