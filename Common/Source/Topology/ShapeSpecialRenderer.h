/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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
  ShapeSpecialRenderer();
  ~ShapeSpecialRenderer();
  
  void Add(const RasterPoint& pt, const TCHAR* szLabel) {
    lstLabel.push_back({pt, szLabel});
  }
  
  void Render(LKSurface& Surface) const;
  
  inline void Clear() { 
      lstLabel.clear();
  }
  
private:
  typedef struct _Label_t {
    RasterPoint pt;
    const TCHAR* szLabel;
  } Label_t;
  
  typedef std::list<Label_t> lstLabel_t;

  lstLabel_t lstLabel;
};

#endif	/* SHAPESPECIALRENDERER_H */

