/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#ifndef RASTERTERRAIN_H
#define RASTERTERRAIN_H

#include <memory>
#include "Library/cpp-mmf/memory_mapped_file.hpp"

struct TERRAIN_INFO {
  double Left;
  double Right;
  double Top;
  double Bottom;
  double StepSize;
  uint32_t Rows;
  uint32_t Columns;
};


class RasterMap final {
 public:
  RasterMap() {
    Interpolate = false;

    TerrainMem = nullptr;
  }
  ~RasterMap() { Close(); }

  inline bool isMapLoaded() const {
    return (TerrainMem);
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

protected:
  friend class TerrainRenderer;

  inline bool interpolate() const { return Interpolate; }
  inline short GetFieldInterpolate(const double &Latitude, const double &Longitude) const;
  inline short GetFieldFine(const double &Latitude, const double &Longitude) const;

 private:

  int xlleft;
  int xlltop;

  bool Interpolate;

  double fXrounding, fYrounding;
  double fXroundingFine, fYroundingFine;
  int Xrounding, Yrounding;

  TERRAIN_INFO TerrainInfo;
  const short* TerrainMem;

  std::unique_ptr<short[]> pTerrainMem;
#ifndef UNDER_CE
  memory_mapped_file::read_only_mmf TerrainFile;
#endif
};
/**
 * JMW rounding further reduces data as required to speed up terrain display on low zoom levels
 *
 * Attention ! allways check if Terrain IsValid before call this.
 */

/**
 * @brief return terrain elevation with piecewise linear interpolation
 * @optimization : return invalid terrain for right&bottom line.
 */
inline
short RasterMap::GetFieldInterpolate(const double &Latitude, const double &Longitude) const {
    assert(Interpolate);

    unsigned int lx = (int)(Longitude * fXroundingFine) - xlleft;
    unsigned int ly = xlltop - (int) (Latitude * fYroundingFine);

    const unsigned ix = CombinedDivAndMod(lx);
    const unsigned iy = CombinedDivAndMod(ly);

    if (gcc_unlikely((lx + 1) >= TerrainInfo.Columns || (ly + 1) >= TerrainInfo.Rows)) {
        return TERRAIN_INVALID;
    }
    assert(((ly+1) * TerrainInfo.Columns + (lx+1)) < (TerrainInfo.Columns*TerrainInfo.Rows));
    const short *tm = TerrainMem + ly * TerrainInfo.Columns + lx;

#ifdef _BILINEAR_INTERP
    // load the four neighboring pixels
    const short& h1 = tm[0];                        // (x  ,y)
    const short& h2 = tm[1];                        // (x+1,y)
    const short& h3 = tm[TerrainInfo.Columns];     // (x  ,y+1)
    const short& h4 = tm[1 + TerrainInfo.Columns]; // (x+1,y+1)

    // Calculate the weights for each pixel
    const unsigned ix1 = 0x0ff - ix;
    const unsigned iy1 = 0x0ff - iy;

    const unsigned w1 = ix1 * iy1;
    const unsigned w2 = ix  * iy1;
    const unsigned w3 = ix1 * iy;
    const unsigned w4 = ix  * iy;

    // Calculate the weighted sum of pixels (for each color channel)
    return (h1 * w1 + h2 * w2 + h3 * w3 + h4 * w4) >> 16;
#else
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
#endif
}

/**
 * @brief return terrain elevation without interpolation
 * @optimization : return invalid terrain for right&bottom line.
 */
inline
short RasterMap::GetFieldFine(const double &Latitude, const double &Longitude) const {
    if(gcc_unlikely(Longitude < TerrainInfo.Left || Latitude > TerrainInfo.Top)) {
        return TERRAIN_INVALID;
    }

    const unsigned int lx = uround((Longitude - TerrainInfo.Left) * fXrounding) * Xrounding;
    const unsigned int ly = uround((TerrainInfo.Top - Latitude) * fYrounding) * Yrounding;

    if (gcc_unlikely(lx >= (TerrainInfo.Columns) || ly >= (TerrainInfo.Rows))) {
        return TERRAIN_INVALID;
    }

    assert(((ly) * TerrainInfo.Columns + (lx)) < (TerrainInfo.Columns*TerrainInfo.Rows));

    return *(TerrainMem + ly * TerrainInfo.Columns + lx);
}

inline
short RasterMap::GetField(const double &Latitude, const double &Longitude) const {
    if (interpolate()) {
        return GetFieldInterpolate(Latitude, Longitude);
    } else {
        return GetFieldFine(Latitude, Longitude);
    }
}

class RasterTerrain {
public:

  static void OpenTerrain();
  static void CloseTerrain();
  static bool isTerrainLoaded() {
    ScopeLock lock(mutex);
    return static_cast<bool>(TerrainMap);
  }

  static std::unique_ptr<RasterMap> TerrainMap;
  static Mutex mutex;

public:

  // RasterTerrain::mutex Lock Required
  static short GetTerrainHeight(double Latitude, double Longitude);

  // RasterTerrain::mutex Lock Required
  static void SetTerrainRounding(double x, double y);

  static bool WaypointIsInTerrainRange(double latitude, double longitude);
  static bool GetTerrainCenter(double *latitude, double *longitude);

protected:
  static bool CreateTerrainMap(const TCHAR *zfilename);

};


#endif
