/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: XShapeLabel.h,v 1.1 2011/12/21 10:35:29 root Exp root $
 */

#ifndef _Terrain_XShapeLabel_h_
#define _Terrain_XShapeLabel_h_

#include "XShape.h"
#include "tchar.h"

class XShapeLabel : public XShape {
 public:
  XShapeLabel() = default;
  ~XShapeLabel();

  // Prevent copying/moving due to raw pointer ownership
  XShapeLabel(const XShapeLabel&) = delete;
  XShapeLabel& operator=(const XShapeLabel&) = delete;
  XShapeLabel(XShapeLabel&&) = delete;
  XShapeLabel& operator=(XShapeLabel&&) = delete;

  bool HasLabel() const override {
    return (label && (label[0] != TEXT('\0')));
  }

  void clear() override;

  void clearLabel();

  void setLabel(const char* src);

  bool renderSpecial(ShapeSpecialRenderer& renderer, LKSurface& Surface, int x,
                     int y, const RECT& ClipRect) const override;
  bool nearestItem(int category, double lon, double lat) const override;

 protected:
  TCHAR* label = nullptr;
};

#endif  // _Terrain_XShapeLabel_h_
