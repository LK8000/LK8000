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

Mutex bottom_bar_mutex;
std::unique_ptr<bottom_bar> bottom_bar_ptr;
unsigned last_bottom_bar_size = 0;

} // namespace

void MapWindow::InitBottomBar(LKSurface& Surface, const RECT& rc) {
  const std::lock_guard<Mutex> lock(bottom_bar_mutex);
  if (!bottom_bar_ptr) {
    bottom_bar_ptr = std::make_unique<bottom_bar>();
    bottom_bar_ptr->refresh_layout(Surface, PixelRect(rc));
    UpdateActiveScreenZone(rc);
  }
}

void MapWindow::DrawBottomBar(LKSurface& Surface, const RECT& rc) {

  if (DoInit[MDI_DRAWBOTTOMBAR]) {

    ResetBottomBarDrawer();

    DoInit[MDI_DRAWBOTTOMBAR] = false;
  }  // end doinit

  ScopeLock lock(bottom_bar_mutex);
  if (!bottom_bar_ptr) {
    bottom_bar_ptr = std::make_unique<bottom_bar>();
  }

  if (bottom_bar_ptr->draw(Surface, PixelRect(rc))) {
    UpdateActiveScreenZone(rc);
  }
}

void MapWindow::ResetBottomBarDrawer() {
  ScopeLock lock(bottom_bar_mutex);
  bottom_bar_ptr = nullptr;
  last_bottom_bar_size = 0;
}

unsigned MapWindow::GetBottomBarSize() {
  ScopeLock lock(bottom_bar_mutex);
  if (!bottom_bar_ptr) {
    return 0;
  }
  unsigned new_size = bottom_bar_ptr->get_size();
  if (last_bottom_bar_size != new_size) {
    // Screen size has changed
    DoInit[MDI_DRAWHSI] = true; 
    DoInit[MDI_DRAWINFOPAGE] = true;
    DoInit[MDI_DRAWLOOK8000] = true;
    DoInit[MDI_DRAWNEAREST] = true;
    DoInit[MDI_DRAWVARIO] = true;
    DoInit[MDI_DRAWFLIGHTMODE] = true;

    last_bottom_bar_size = new_size;
  }
  return new_size;
}
