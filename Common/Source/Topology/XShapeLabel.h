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
#include "Util/tstring.hpp"

class XShapeLabel : public XShape {
 public:
  XShapeLabel() = default;

  bool HasLabel() const override {
    return !label.empty();
  }

  void clear() override;

  void setLabel(const char* src);

  bool renderSpecial(ShapeSpecialRenderer& renderer, LKSurface& Surface, int x,
                     int y, const RECT& ClipRect) const override;
  bool nearestItem(int category, double lon, double lat) const override;

 protected:
  tstring label;
};

#endif  // _Terrain_XShapeLabel_h_
