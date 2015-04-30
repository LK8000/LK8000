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
    DirectAccess = true;
    Paged = false;
    
    TerrainMem = NULL;
  }
  ~RasterMap() {};

  inline bool isMapLoaded() const {
    return terrain_valid;
  }

  short max_field_value;
  TERRAIN_INFO TerrainInfo;

  bool GetMapCenter(double *lon, double *lat) const;

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
  
  void Lock() { CritSec_TerrainFile.lock(); }
  void Unlock() { CritSec_TerrainFile.unlock(); }
  
  bool IsDirectAccess(void) const { return DirectAccess; };
  bool IsPaged(void) const { return Paged; };

 protected:
  int xlleft;
  int xlltop;
  bool terrain_valid;
  bool DirectFine;
  bool Paged;
  bool DirectAccess;
  double fXrounding, fYrounding;
  double fXroundingFine, fYroundingFine;
  int Xrounding, Yrounding;
  
  short *TerrainMem;

  Poco::Mutex  CritSec_TerrainFile;

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
#if (WINDOWSPC>0)
        const unsigned int ix = Real2Int((Longitude - TerrainInfo.Left) * fXrounding) * Xrounding;
        const unsigned int iy = Real2Int((TerrainInfo.Top - Latitude) * fYrounding) * Yrounding;
#else
        const unsigned int ix = ((int) ((Longitude - TerrainInfo.Left) * fXrounding)) * Xrounding;
        const unsigned int iy = ((int) ((TerrainInfo.Top - Latitude) * fYrounding)) * Yrounding;
#endif

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

  static void SetViewCenter(const double &Latitude, 
                            const double &Longitude);
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
  static bool IsDirectAccess(void);
  static bool IsPaged(void);
  static void SetTerrainRounding(double x, double y);
  static void ServiceCache();
  static void ServiceTerrainCenter(double latitude, double longitude);
  static int GetEffectivePixelSize(double *pixel_D, 
                                   double latitude, double longitude);
  static bool WaypointIsInTerrainRange(double latitude, double longitude);
  static bool GetTerrainCenter(double *latitude,
                               double *longitude);
};


#endif
