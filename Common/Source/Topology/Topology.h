/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: Topology.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

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

  double scaleThreshold = 0;
  double scaleDefaultThreshold = 0;
  int scaleCategory = 0;

  bool CheckScale();
  void TriggerIfScaleNowVisible();

  bool triggerUpdateCache = false;

  using XShapePtr = std::shared_ptr<XShape>;
  std::vector<XShapePtr> shpCache;

  void loadBitmap(const int);
  void loadPenBrush(const LKColor thecolor);

  std::unique_ptr<XShape> loadShape(int i);

 protected:
  void flushCache();

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
  std::vector<XShapePtr> shps;
  std::vector<rectObj> shpBounds;
  rectObj lastBounds = {};
  bool in_scale_last = false;

  // utf8 filename, converted from platform encoding in ctor
  char filename[MAX_PATH];
  int field;
};

#endif
