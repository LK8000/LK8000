/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: XShape.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#ifndef _Terrain_XShape_h_
#define _Terrain_XShape_h_

#include "Topology/shapelib/mapserver.h"
#include "types.h"

class ShapeSpecialRenderer;
class LKSurface;

class XShape {
 public:
  XShape();
  virtual ~XShape();

  // no copy
  XShape(const XShape&) = delete;
  XShape& operator=(const XShape&) = delete;
  // no move
  XShape(XShape&&) = delete;
  XShape& operator=(XShape&&) = delete;

  /**
   * return true if shape have label ( XShapeLabel object without empty label )
   */
  virtual bool HasLabel() const {
    return false;
  }

  virtual void load(shapefileObj* shpfile, int i);
  virtual void clear();

  virtual bool renderSpecial(ShapeSpecialRenderer& renderer, LKSurface& Surface,
                             int x, int y, const RECT& ClipRect) const {
    return false;
  }

  virtual bool nearestItem(int category, double lon, double lat) const {
    return true;
  }

  bool hide = false;
  shapeObj shape;
};

#endif  // _Terrain_XShape_h_
