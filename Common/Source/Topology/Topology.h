/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: Topology.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "options.h"
#include "shapelib/mapserver.h"
#include "Screen/LKBrush.h"
#include "Screen/LKPen.h"
#include "Screen/LKIcon.h"
#include "types.h"
#include <memory>
#include <vector>

class ShapeSpecialRenderer;
class ScreenProjection;
class LKSurface;
class XShape;

#define TopoLog(...) StartupStore(_T("[Topology]") __VA_ARGS__)
#define TopoLogT(...) TestLog(_T("[Topology]") __VA_ARGS__)
#ifdef DEBUG_TOPOLOGY
  #define TopoLogD(...) DebugLog(_T("[Topology]") __VA_ARGS__)
#else
  #define TopoLogD(...)  // Does nothing
#endif

enum class topology_category : int {
  reserved = 0, 
  marked_locations = 1,
  coast_areas = 5,
  water_area = 10,
  water_line = 20,
  big_road = 30,
  medium_road = 40,
  small_road = 50,
  railroad = 60,
  big_city = 70,
  medium_city = 80,
  small_city = 90,
  very_small_city = 100,
  city_area = 110 // city area polyline with no name
};

inline bool IsWaterCategory(topology_category category) {
  return category == topology_category::coast_areas ||
         category == topology_category::water_area ||
         category == topology_category::water_line;
}

inline bool IsWaterArea(topology_category category) {
  return category == topology_category::coast_areas ||
         category == topology_category::water_area;
}

inline bool IsCity(topology_category category) {
  return category == topology_category::big_city ||
         category == topology_category::medium_city ||
         category == topology_category::small_city ||
         category == topology_category::very_small_city ||
         category == topology_category::city_area;
}

class Topology final {
  Topology() = delete;

  Topology(const Topology&) = delete;
  Topology& operator=(const Topology&) = delete;
  Topology(Topology&&) = delete;
  Topology& operator=(Topology&&) = delete;

 public:
  Topology(const TCHAR* shpname, int field1);
  ~Topology();

  void Open();
  void Close();

  void updateCache(rectObj thebounds, bool purgeonly = false);
  void Paint(ShapeSpecialRenderer& renderer, LKSurface& Surface, const RECT& rc,
             const ScreenProjection& _Proj) const;

  void SearchNearest(const rectObj& bounds);

  double scaleThreshold; // only used for unknown category
  topology_category Category = topology_category::reserved;

  double GetScaleThreshold() const;

  bool CheckScale();
  void TriggerIfScaleNowVisible();

  bool triggerUpdateCache = false;

  using XShapePtr = std::shared_ptr<XShape>;
  std::vector<XShapePtr> visibleShapes;

  void loadBitmap(const int);
  void loadPenBrush(const LKColor thecolor);

  std::unique_ptr<XShape> loadShape(int i);

 protected:
  void flushCache();
  void flushOutOfBounds(const rectObj& thebounds);

  bool in_scale = false;
  LKPen hPen;
  LKBrush hbBrush;
  LKIcon hBitmap;
  shapefileObj shpfile = {};
  bool shapefileopen = false;

  bool initCache_0();
  bool initCache_1();
#ifdef USE_TOPOLOGY_CACHE_LEVEL2
  bool initCache_2();
#endif
  void initCache();

  int cache_mode = 0;
  std::vector<XShapePtr> allShapes; // only used in cache mode 2
  std::vector<rectObj> shpBounds; // only used in cache mode 1, to avoid calling msSHPReadBounds multiple times
  rectObj lastBounds = {};
  bool in_scale_last = false;

  // utf8 filename, converted from platform encoding in ctor
  std::string filename;
  int field;
};

#endif
