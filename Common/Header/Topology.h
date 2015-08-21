/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Topology.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "mapshape.h"


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
  virtual bool HasLabel() const { return false; }

  virtual void load(shapefileObj* shpfile, int i);
  virtual void clear();

  virtual bool renderSpecial(LKSurface& Surface, int x, int y, const RECT& ClipRect) { (void)x; (void)y; (void)Surface; return false;};
  virtual bool nearestItem(int category, double lon, double lat) { (void)category; (void)lon; (void)lat; return(true);};

  bool hide;
  shapeObj shape;
};


class XShapeLabel: public XShape {
 public:
  XShapeLabel() : label() { }
  virtual ~XShapeLabel();
  
  virtual bool HasLabel() const { 
      return ( label && ( label[0] != TEXT('\0'))); 
  }

  virtual void clear();
  void setlabel(const char* src);
  
  virtual bool renderSpecial(LKSurface& Surface, int x, int y, const RECT& ClipRect);
  virtual bool nearestItem(int category, double lon, double lat);


protected:
  TCHAR *label;
};


class Topology {

 public:
#if USETOPOMARKS
  Topology(const TCHAR* shpname,  bool doappend=false);
#else
  Topology(const TCHAR* shpname);
#endif
  Topology() {};
  
  virtual ~Topology();
  
  void Open();
  void Close();

  void updateCache(rectObj thebounds, bool purgeonly=false);
  void Paint(LKSurface& Surface, const RECT& rc);
  void SearchNearest(RECT rc);

  double scaleThreshold;
  double scaleDefaultThreshold;
  int scaleCategory;

  bool CheckScale();
  void TriggerIfScaleNowVisible();

  bool triggerUpdateCache;
  int shapes_visible_count;

  XShape** shpCache;

  bool checkVisible(const shapeObj& shape, rectObj &screenRect);

  void loadBitmap(const int);
  void loadPenBrush(const LKColor thecolor);

  TCHAR filename[MAX_PATH];

  virtual void removeShape(const int i);
  virtual XShape* addShape(const int i);
  
 protected:

  void flushCache();
#if USETOPOMARKS
  bool append;
#endif
  bool in_scale;
  LKPen hPen;
  LKBrush hbBrush;
  LKBitmap hBitmap;
  shapefileObj shpfile;
  bool shapefileopen;
  void initCache();
  int cache_mode;
  XShape **shps;
  rectObj* shpBounds;
  rectObj lastBounds;
  bool in_scale_last;
 
};

#if USETOPOMARKS
class TopologyWriter: public Topology {
 public:
  TopologyWriter(const TCHAR *shpname);
  virtual ~TopologyWriter();
  
  void addPoint(double x, double y);
  void Reset(void);
  void CreateFiles(void);
  void DeleteFiles(void);
};
#endif


class TopologyLabel: public Topology {
 public:
  TopologyLabel(const TCHAR* shpname, INT field1);
  virtual ~TopologyLabel();
  virtual XShape* addShape(const int i);
  void setField(int i);
  int field;

};

#endif
