#ifndef RASTERTERRAIN_H
#define RASTERTERRAIN_H

#include <zzip/lib.h>
#include "jasper/RasterTile.h"

typedef struct _TERRAIN_INFO
{
  double Left;
  double Right;
  double Top;
  double Bottom;
  double StepSize;
  uint32_t Rows;
  uint32_t Columns;
} TERRAIN_INFO;


class RasterMap final {
 public:
  RasterMap() {
    terrain_valid = false;
    max_field_value = 0;
    DirectFine = false;
   
    TerrainMem = NULL;
  }
  ~RasterMap() {};

  inline bool isMapLoaded() const {
    return terrain_valid;
  }

  bool GetMapCenter(double *lat, double *lon) const;
  bool IsInside(double lat, double lon) const;

  float GetFieldStepSize() const;

  // inaccurate method
  int GetEffectivePixelSize(double pixelsize) const;

  // accurate method
  int GetEffectivePixelSize(double *pixel_D, 
                            double latitude, double longitude) const;
  
  void SetFieldRounding(double xr, double yr);

  inline short GetField(const double &Latitude, const double &Longitude) const;

  bool Open(const TCHAR* filename);
  void Close();
  
  void Lock() { CritSec_TerrainFile.Lock(); }
  void Unlock() { CritSec_TerrainFile.Unlock(); }
  
 protected:
  short max_field_value;
  TERRAIN_INFO TerrainInfo;
   
  int xlleft;
  int xlltop;
  bool terrain_valid;
  bool DirectFine;
  double fXrounding, fYrounding;
  double fXroundingFine, fYroundingFine;
  int Xrounding, Yrounding;
  
  short *TerrainMem;

  Mutex  CritSec_TerrainFile;

  inline short _GetFieldAtXY(unsigned int lx, unsigned int ly) const;
};
/**
 * JMW rounding further reduces data as required to speed up terrain display on low zoom levels
 * 
 * Attention ! allways check if Terrain IsValid before call this.
 */
inline 
short RasterMap::GetField(const double &Latitude, const double &Longitude) const {
    if (DirectFine) {
        return _GetFieldAtXY((int) (Longitude * fXroundingFine) - xlleft,
                xlltop - (int) (Latitude * fYroundingFine));
    } else {
        const unsigned int ix = iround((Longitude - TerrainInfo.Left) * fXrounding) * Xrounding;
        const unsigned int iy = iround((TerrainInfo.Top - Latitude) * fYrounding) * Yrounding;

        return _GetFieldAtXY(ix << 8, iy << 8);
    }
}

/**
 * @brief return terrain elevation with piecewise linear interpolation
 * @optimization : return invalid terrain for right&bottom line.
 */
inline
short RasterMap::_GetFieldAtXY(unsigned int lx, unsigned int ly) const {

    const unsigned ix = CombinedDivAndMod(lx);
    if (lx + 1 >= TerrainInfo.Columns) {
        return TERRAIN_INVALID;
    }

    const unsigned iy = CombinedDivAndMod(ly);
    if (ly + 1 >= TerrainInfo.Rows) {
        return TERRAIN_INVALID;
    }

    const short *tm = TerrainMem + ly * TerrainInfo.Columns + lx;
    // perform piecewise linear interpolation
    const short &h1 = tm[0]; // (x,y)
    const short &h3 = tm[TerrainInfo.Columns+1]; // (x+1,y+1)
    if (ix > iy) {
        // lower triangle 
        const short &h2 = tm[1]; // (x+1,y)
        return (short) (h1 + ((ix * (h2 - h1) - iy * (h2 - h3)) >> 8));
    } else {
        // upper triangle
        const short &h4 = tm[TerrainInfo.Columns]; // (x,y+1)
        return (short) (h1 + ((iy * (h4 - h1) - ix * (h4 - h3)) >> 8));
    }
}

class RasterTerrain {
public:

  RasterTerrain() {
    terrain_initialised = false;
  }

  static void OpenTerrain();
  static void CloseTerrain();
  static bool terrain_initialised;
  static bool isTerrainLoaded() {
    return terrain_initialised;
  }
  static RasterMap* TerrainMap;
  static bool CreateTerrainMap(const TCHAR *zfilename);

 public:
  static void Lock(void);
  static void Unlock(void);
  static short GetTerrainHeight(const double &Latitude,
                                const double &Longitude);

  static void SetTerrainRounding(double x, double y);

  static int GetEffectivePixelSize(double *pixel_D, 
                                   double latitude, double longitude);
  static bool WaypointIsInTerrainRange(double latitude, double longitude);
  static bool GetTerrainCenter(double *latitude,
                               double *longitude);
};


#endif
