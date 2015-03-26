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


#if RASTERCACHE
typedef struct _TERRAIN_CACHE
{
  short h;
  long index;
  unsigned int recency;
} TERRAIN_CACHE;
#endif

class RasterMap {
 public:
  RasterMap() {
    terrain_valid = false;
    max_field_value = 0;
    DirectFine = false;
    DirectAccess = false;
    Paged = false;
  }
  virtual ~RasterMap() {};

  inline bool isMapLoaded() const {
    return terrain_valid;
  }

  short max_field_value;
  TERRAIN_INFO TerrainInfo;

  virtual void SetViewCenter(const double &Latitude, 
                             const double &Longitude) {};

  bool GetMapCenter(double *lon, double *lat) const;

  float GetFieldStepSize() const;

  // inaccurate method
  int GetEffectivePixelSize(double pixelsize) const;

  // accurate method
  int GetEffectivePixelSize(double *pixel_D, 
                            double latitude, double longitude) const;
  
  virtual void SetFieldRounding(double xr, double yr);

  inline short GetField(const double &Latitude, const double &Longitude) const;

  virtual bool Open(const TCHAR* filename) = 0;
  virtual void Close() = 0;
  virtual void Lock() = 0;
  virtual void Unlock() = 0;
  virtual void ServiceCache() {};
  virtual void ServiceFullReload(double lat, double lon) {};
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

  virtual short _GetFieldAtXY(unsigned int lx, unsigned int ly) const = 0;
};
/**
 * JMW rounding further reduces data as required to speed up terrain display on low zoom levels
 * 
 * Attention ! allways check if Terrain IsValid before call this.
 */
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

#if RASTERCACHE
class RasterMapCache: public RasterMap {
 public:
  RasterMapCache() {
    terraincacheefficiency=0;
    terraincachehits = 1;
    terraincachemisses = 1;
    cachetime = 0;
    DirectAccess = false;
    if (ref_count==0) {
      fpTerrain = NULL;
      InitializeCriticalSection(&CritSec_TerrainFile);
    }
    ref_count++;
  }

  ~RasterMapCache() {
    ref_count--;
    if (ref_count==0) {
      DeleteCriticalSection(&CritSec_TerrainFile);
    }
  }

  // shared!
  static ZZIP_FILE *fpTerrain;
  static int ref_count;

  void ServiceCache();

  void SetCacheTime();
  void ClearTerrainCache();
  short LookupTerrainCache(const long &SeekPos);
  short LookupTerrainCacheFile(const long &SeekPos);
  void OptimizeCash(void);

  virtual bool Open(const TCHAR* filename);
  virtual void Close();
  void Lock();
  void Unlock();
 protected:
  static Poco::Mutex CritSec_TerrainFile;
  TERRAIN_CACHE TerrainCache[MAXTERRAINCACHE]; 

  int terraincacheefficiency;
  long terraincachehits;
  long terraincachemisses;
  unsigned int cachetime;
  int SortThresold;

  short _GetFieldAtXY(unsigned int lx,
                      unsigned int ly);
  //
};
#endif // RASTERCACHE

class RasterMapRaw: public RasterMap {
 public:
  RasterMapRaw() {
    TerrainMem = NULL;
    DirectAccess = true;
  }
  ~RasterMapRaw() {
  }
  short *TerrainMem;
  virtual void SetFieldRounding(double xr, double yr);
  virtual bool Open(const TCHAR* filename);
  virtual void Close();
  void Lock();
  void Unlock();
 protected:
  virtual short _GetFieldAtXY(unsigned int lx, unsigned int ly) const;
  Poco::Mutex  CritSec_TerrainFile;
};


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
  static void ServiceFullReload(double latitude, double longitude);
  static int GetEffectivePixelSize(double *pixel_D, 
                                   double latitude, double longitude);
  static bool WaypointIsInTerrainRange(double latitude, double longitude);
  static bool GetTerrainCenter(double *latitude,
                               double *longitude);
};


#endif
