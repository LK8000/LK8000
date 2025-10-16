/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "Time/PeriodClock.hpp"


//
// 111109 This will force immediate repaint of the OLD screen.
// It will NOT update the map or calculations, only repaint
// an old bitmap image of the screen.
void MapWindow::RequestFastRefresh() {

#ifdef ENABLE_OPENGL
  assert(main_window);
  if (main_window) {
    main_window->Redraw(MapRect);
  }
#else
  drawTriggerEvent.set();
#endif
}


void MapWindow::RefreshMap() {
  MapDirty = true;
  RequestFastRefresh();
}


bool MapWindow::RenderTimeAvailable() {
    // it's been less than 700 ms since last data was posted
    return !MapDirty && !timestamp_newdata.Check(700);
}

void MapWindow::UpdateInfo(const NMEA_INFO& nmea_info, const DERIVED_INFO& derived_info) {
  ScopeLock lock(CritSec_FlightData);
  DrawInfo = nmea_info;
  DerivedDrawInfo = derived_info;
  zoom.UpdateMapScale(); // done here to avoid double latency due to locks
}



void MapWindow::UpdateCaches(const ScreenProjection& _Proj, bool force) {
  // map was dirtied while we were drawing, so skip slow process
  // (unless we haven't done it for 2000 ms)

  // have some time, do shape file cache update if necessary
  LockTerrainDataGraphics();
  SetTopologyBounds(DrawRect, _Proj, force||MapWindow::ForceVisibilityScan);
  UnlockTerrainDataGraphics();
  //
  // ForceVisibilityScan is checked and actively used only here, and in ScanVisibility since v6
  // ScanVisibility is called by SetTopologyBounds. We need to wait to clear ForceVisibility after
  // the SetTopologyBounds(), not before.
  //
  MapWindow::ForceVisibilityScan = false; // and no problem if it was already false
}
