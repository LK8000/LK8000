/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Topology.h,v 8.2 2010/12/15 12:35:45 root Exp root $
*/

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "StdAfx.h"
#include "mapshape.h"
#include "MapWindow.h"


class XShape {
 public:
  XShape();
  virtual ~XShape();

  virtual void load(shapefileObj* shpfile, int i);
  virtual void clear();
  virtual bool renderSpecial(HDC hdc, int x, int y, bool RetVal) { (void)x; (void)y; (void)hdc; return(RetVal);};

  bool hide;
  shapeObj shape;

};


class XShapeLabel: public XShape {
 public:
  XShapeLabel() {
    label= NULL;
  }
  virtual ~XShapeLabel();
  virtual void clear();
  char *label;
  void setlabel(const char* src);
  virtual bool renderSpecial(HDC hdc, int x, int y, bool RetVal);
};


class Topology {

 public:
  Topology(const char* shpname, COLORREF thecolor, bool doappend=false);
  Topology() {};
  
  virtual ~Topology(); //@ MATFIX1
  
  void Open();
  void Close();

  void updateCache(rectObj thebounds, bool purgeonly=false);
  void Paint(HDC hdc, RECT rc);

  double scaleThreshold;
  #if LKTOPO
  double scaleDefaultThreshold;
  int scaleCategory;
  #endif

  bool CheckScale();
  void TriggerIfScaleNowVisible();

  bool triggerUpdateCache;
  int shapes_visible_count;

  XShape** shpCache;

  bool checkVisible(shapeObj& shape, rectObj &screenRect);

  void loadBitmap(const int);

  char filename[MAX_PATH];

  virtual void removeShape(const int i);
  virtual XShape* addShape(const int i);
  
 protected:
  void flushCache();

  bool append;
  bool in_scale;
  HPEN hPen;
  HBRUSH hbBrush;
  HBITMAP hBitmap;
  shapefileObj shpfile;
  bool shapefileopen;
#ifdef TOPOFASTCACHE
  virtual bool isTopoLabelClass() { return false; }
  void initCache();
  int cache_mode;
  XShape **shps;
  rectObj* shpBounds;
  rectObj lastBounds;
#endif
 
};


class TopologyWriter: public Topology {
 public:
  TopologyWriter(const char *shpname, COLORREF thecolor);
  virtual ~TopologyWriter(); //@ MATFIX1 
  
  void addPoint(double x, double y);
  void Reset(void);
  void CreateFiles(void);
  void DeleteFiles(void);
};



class TopologyLabel: public Topology {
 public:
  TopologyLabel(const char* shpname, COLORREF thecolor, INT field1);
  virtual ~TopologyLabel(); //@ MATFIX1
  virtual XShape* addShape(const int i);
  void setField(int i);
  int field;
#ifdef TOPOFASTCACHE
  virtual bool isTopoLabelClass() { return true; }
#endif
};

void ClipPolygon(HDC hdc, POINT *ptin, unsigned int n, 
                 RECT rc, bool fill=true);


#endif
