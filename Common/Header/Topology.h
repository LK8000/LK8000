/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Topology.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "Thread/Thread.hpp"
#include "Topology/shapelib/mapserver.h"

class ShapeSpecialRenderer;

class XShape {
 public:
  XShape();
  virtual ~XShape();

  // no copy
  XShape(const XShape&) = delete;
  XShape& operator=(const XShape&) = delete;
  // no move
  XShape(XShape&&) = delete;
  XShape& operator=( XShape&& ) = delete;

  /**
   * return true if shape have label ( XShapeLabel object without empty label )
   */
  virtual bool HasLabel() const {
    return false;
  }

  virtual void load(shapefileObj* shpfile, int i);
  virtual void clear();

  virtual bool renderSpecial(ShapeSpecialRenderer& renderer, LKSurface& Surface, int x, int y, const RECT& ClipRect) const {
    return false;
  }

  virtual bool nearestItem(int category, double lon, double lat) const {
    return true;
  }

  bool hide = false;
  shapeObj shape;
};


class XShapeLabel: public XShape {
 public:
  XShapeLabel() = default;
  ~XShapeLabel();

  bool HasLabel() const override {
      return ( label && ( label[0] != TEXT('\0')));
  }

  void clear() override;

  void clearLabel();

  void setLabel(const char* src);

  bool renderSpecial(ShapeSpecialRenderer& renderer, LKSurface& Surface, int x, int y, const RECT& ClipRect) const override;
  bool nearestItem(int category, double lon, double lat) const override;

protected:
  TCHAR *label = nullptr;
};


class Topology final {
  Topology() = delete;

 public:
  Topology(const TCHAR* shpname, int field1);
  ~Topology();

  void Open();
  void Close();

  void updateCache(rectObj thebounds, bool purgeonly=false);
  void Paint(ShapeSpecialRenderer& renderer, LKSurface& Surface, const RECT& rc, const ScreenProjection& _Proj) const;

  void SearchNearest(const rectObj& bounds);

  double scaleThreshold;
  double scaleDefaultThreshold;
  int scaleCategory;

  bool CheckScale();
  void TriggerIfScaleNowVisible();

  bool triggerUpdateCache;
  int shapes_visible_count;

  XShape** shpCache;

  static bool checkVisible(const shapeObj& shape, const rectObj &screenRect);

  void loadBitmap(const int);
  void loadPenBrush(const LKColor thecolor);

  void removeShape(const int i);
  XShape* addShape(const int i);

 protected:

  void flushCache();

  bool in_scale;
  LKPen hPen;
  LKBrush hbBrush;
  LKIcon hBitmap;
  shapefileObj shpfile;
  bool shapefileopen;

  bool initCache_0();
  bool initCache_1();
#ifdef USE_TOPOLOGY_CACHE_LEVEL2
  bool initCache_2();
#endif  
  void initCache();

  int cache_mode;
  XShape **shps;
  rectObj* shpBounds;
  rectObj lastBounds;
  bool in_scale_last;

  char filename[MAX_PATH];
  int field;
};


/**
 * Thread class used by "Oracle" for find Topology Item nearest to current position.
 */
class WhereAmI : public Thread {
public:
    WhereAmI() : Thread("WhereAmI") {
        toracle[0] = _T('\0');
    }

    ~WhereAmI() { }


    const TCHAR* getText() const {
        return toracle;
    }

    bool Start() override {
        toracle[0] = _T('\0');
        return Thread::Start();
    }

    bool IsDone() {
        return !IsDefined();
    }

protected:
    void Run() override;

    TCHAR toracle[1000];
};


#endif
