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

ShapeSpecialRenderer::ShapeSpecialRenderer() {
}

ShapeSpecialRenderer::~ShapeSpecialRenderer() {
}

void ShapeSpecialRenderer::Render(LKSurface& Surface) const {
  const auto hfOld = Surface.SelectObject(MapTopologyFont);
  Surface.SetBackgroundTransparent();
  Surface.SetTextColor(IsDithered() ? LKColor(0, 0, 0) : LKColor(0, 50, 50));
  for (const auto& label : lstLabel) {
    Surface.DrawText(label.pt.x, label.pt.y, label.szLabel);
  }
  Surface.SelectObject(hfOld);
}

