/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "MapWindow.h"
#include "BottomBar/BottomBar.h"
#include "DoInits.h"

namespace {

std::unique_ptr<bottom_bar> bottom_bar_ptr;

} // namespace

void MapWindow::DrawBottomBar(LKSurface& Surface, const RECT& rc) {

  if (DoInit[MDI_DRAWBOTTOMBAR]) {

    bottom_bar_ptr = nullptr;

    DoInit[MDI_DRAWBOTTOMBAR] = false;
  }  // end doinit

  if (!bottom_bar_ptr) {
    bottom_bar_ptr = std::make_unique<bottom_bar>();
  }

  bottom_bar_ptr->draw(Surface, PixelRect(rc));
}

void MapWindow::ResetBottomBarDrawer() {
  bottom_bar_ptr = nullptr;
}
