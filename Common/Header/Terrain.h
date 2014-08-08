#ifndef TERRAIN_H
#define TERRAIN_H
#include "externs.h"
#include "Topology.h"


typedef struct _COLORRAMP
{
  short h;
  unsigned char r;
  unsigned char g;
  unsigned char b;
} COLORRAMP;

#if USETOPOMARKS
extern bool reset_marks;
#endif
void ColorRampLookup(short h, BYTE &r, BYTE &g, BYTE &b,
		     const COLORRAMP* ramp_colors, 
                     const int numramp,
                     const unsigned char interp_bits=6);

void SetTopologyBounds(const RECT rcin, const bool force=false);
#if USETOPOMARKS
void TopologyInitialiseMarks();
void TopologyCloseMarks();
#endif
void OpenTopology();
void CloseTopology();
void ChangeZoomTopology(int iCategory, double newScale, short cztmode);
double ReadZoomTopology(int iCategory);
bool HaveZoomTopology(int iCategory);
void DrawTopology(const HDC hdc, const RECT rc, const bool wateronly=false);
void DrawTerrain(const HDC hdc, const RECT rc, const double sunazimuth, const double sunelevation);
void DrawSpotHeights(const HDC hdc);
#if USETOPOMARKS
void DrawMarks(const HDC hdc, const RECT rc);
void MarkLocation(const double lon, const double lat);
#else
void MarkLocation(const double lon, const double lat, const double altitude);
#endif
void CloseTerrainRenderer();
#endif
