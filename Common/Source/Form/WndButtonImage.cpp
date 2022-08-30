/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   WndButtonImage.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 10, 2024
 */

#include "WndButtonImage.h"
#include "Util/StringUtil.hpp"
#include "Bitmaps.h"
#include "ScreenGeometry.h"

void WndButtonImage::Paint(LKSurface& Surface) {

  if (!IsVisible()) return;

  WindowControl::Paint(Surface);

  DrawPushButton(Surface);

  const TCHAR * szImageName = GetWndText();
  if (!Icon && !StringIsEmpty(szImageName)) {
    Icon = LKLoadBitmap(szImageName);
  }

  if (Icon) {
    PixelRect rcIcon(GetClientRect());
    rcIcon.Grow(DLGSCALE(-3)); // todo border width
    Icon.Draw(Surface, rcIcon);
  }
}
