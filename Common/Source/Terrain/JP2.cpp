/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"


#ifdef __MINGW32__
#define int_fast8_t jas_int_fast8_t
#endif

#ifdef JP2000
#include "jasper/jasper.h"
#include "jasper/jpc_rtc.h"

#include "wcecompat/ts_string.h"

#include "utils/heapcheck.h"

using std::min;
using std::max;


void RasterMapJPG2000::SetFieldRounding(double xr, double yr) {
  RasterMap::SetFieldRounding(xr, yr);
  if (!isMapLoaded()) {
    return;
  }
  if ((Xrounding==1)&&(Yrounding==1)) {
    DirectFine = true;
    xlleft = (int)(TerrainInfo.Left*fXroundingFine)+128;
    xlltop  = (int)(TerrainInfo.Top*fYroundingFine)-128;
  } else {
    DirectFine = false;
  }
}


short RasterMapJPG2000::_GetFieldAtXY(unsigned int lx,
                                          unsigned int ly) {

  return raster_tile_cache.GetField(lx,ly);
}


//////////// JPG2000 //////////////////////////////////////////////////

CRITICAL_SECTION RasterMapJPG2000::CritSec_TerrainFile;

void RasterMapJPG2000::ServiceFullReload(double lat, double lon) {
  ReloadJPG2000Full(lat, lon);
}


RasterMapJPG2000::RasterMapJPG2000() {
  TriggerJPGReload = false;
  jp2_filename[0] = '\0';
  DirectAccess = true;
  Paged = true;
  if (ref_count==0) {
    jas_init();
    InitializeCriticalSection(&CritSec_TerrainFile);
  }
  ref_count++;
}

RasterMapJPG2000::~RasterMapJPG2000() {
  ref_count--;
  if (ref_count==0) {
    jas_cleanup();
    DeleteCriticalSection(&CritSec_TerrainFile);
  }
}


int RasterMapJPG2000::ref_count = 0;

void RasterMapJPG2000::Lock(void) {
  EnterCriticalSection(&CritSec_TerrainFile);
}

void RasterMapJPG2000::Unlock(void) {
  LeaveCriticalSection(&CritSec_TerrainFile);
}


void RasterMapJPG2000::ReloadJPG2000Full(double latitude, 
                                         double longitude) {
  // load all 16 tiles...
  for (int i=0; i<MAX_ACTIVE_TILES; i++) {
    TriggerJPGReload = true;
    SetViewCenter(latitude, longitude);
  }
}


void RasterMapJPG2000::ReloadJPG2000(void) {
  if (TriggerJPGReload) {

    Lock();
    TriggerJPGReload = false;

    raster_tile_cache.LoadJPG2000(jp2_filename);
    if (raster_tile_cache.GetInitialised()) {
      TerrainInfo.Left = raster_tile_cache.lon_min;
      TerrainInfo.Right = raster_tile_cache.lon_max;
      TerrainInfo.Top = raster_tile_cache.lat_max;
      TerrainInfo.Bottom = raster_tile_cache.lat_min;
      TerrainInfo.Columns = raster_tile_cache.GetWidth();
      TerrainInfo.Rows = raster_tile_cache.GetHeight();
      TerrainInfo.StepSize = (raster_tile_cache.lon_max - 
                              raster_tile_cache.lon_min)
        /raster_tile_cache.GetWidth();
    } 
    Unlock();
  }
}


void RasterMapJPG2000::SetViewCenter(const double &Latitude, 
                                     const double &Longitude) {
  Lock();
  if (raster_tile_cache.GetInitialised()) {
    int x = lround((Longitude-TerrainInfo.Left)*TerrainInfo.Columns
                   /(TerrainInfo.Right-TerrainInfo.Left));
    int y = lround((TerrainInfo.Top-Latitude)*TerrainInfo.Rows
                   /(TerrainInfo.Top-TerrainInfo.Bottom));
    TriggerJPGReload |= raster_tile_cache.PollTiles(x, y);
    ReloadJPG2000();
  }
  Unlock();
}

bool RasterMapJPG2000::Open(const TCHAR* zfilename) {
  _tcscpy(jp2_filename,zfilename);

  // force first-time load
  TriggerJPGReload = true;
  ReloadJPG2000();

  terrain_valid = raster_tile_cache.GetInitialised();
  if (!terrain_valid) {
    raster_tile_cache.Reset();
    max_field_value = 0;
  } else {
    max_field_value = raster_tile_cache.GetMaxElevation();
  }
  return terrain_valid;
}


///////////////// Close routines /////////////////////////////////////

void RasterMapJPG2000::Close(void) {
  if (terrain_valid) {
    Lock();
    raster_tile_cache.Reset();
    terrain_valid = false;
    Unlock();
  }
}


#endif // JP2000
