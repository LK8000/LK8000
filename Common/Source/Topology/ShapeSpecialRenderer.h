/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ShapeSpecialRenderer.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 30 juin 2016
 */

#ifndef SHAPESPECIALRENDERER_H
#define	SHAPESPECIALRENDERER_H

#include "tchar.h"
#include <list>
#include "Screen/Point.hpp"

class LKSurface;

class ShapeSpecialRenderer final {
public:
  ShapeSpecialRenderer() = default;

  void Add(RasterPoint&& pt, const TCHAR* szLabel) {
    lstLabel.emplace_back(std::forward<RasterPoint>(pt), szLabel);
  }

  void Render(LKSurface& Surface) const;
  
  void Clear() { 
    lstLabel.clear();
  }

private:
  struct Label_t {
    Label_t(RasterPoint&& p, const TCHAR* sz) 
        : point(std::forward<RasterPoint>(p)), szLabel(sz) { }

    const RasterPoint point;
    const TCHAR* szLabel;
  };

  using lstLabel_t = std::list<Label_t>;

  lstLabel_t lstLabel;
};

#endif	/* SHAPESPECIALRENDERER_H */
