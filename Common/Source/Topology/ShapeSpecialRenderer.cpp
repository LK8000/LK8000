/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   ShapeSpecialRenderer.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 30 juin 2016
 */

#include "externs.h"
#include "ShapeSpecialRenderer.h"
#include "Asset.hpp"

void ShapeSpecialRenderer::Render(LKSurface& Surface) const {
  const auto hfOld = Surface.SelectObject(MapTopologyFont);
  Surface.SetBackgroundTransparent();
  Surface.SetTextColor(IsDithered() ? RGB_BLACK : LKColor(0, 50, 50));
  for (const auto& label : lstLabel) {
    Surface.DrawText(label.point, label.szLabel);
  }
  Surface.SelectObject(hfOld);
}
