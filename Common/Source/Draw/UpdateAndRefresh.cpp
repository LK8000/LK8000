/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Terrain.h"
#include "Poco/Timespan.h"


//
// 111109 This will force immediate repaint of the OLD screen.
// It will NOT update the map or calculations, only repaint
// an old bitmap image of the screen.
void MapWindow::RequestFastRefresh() {
  MapDirty = false;
  drawTriggerEvent.set();
}


void MapWindow::RefreshMap() {
  MapDirty = true;
  drawTriggerEvent.set();
}


bool MapWindow::RenderTimeAvailable() {
    const Poco::Timespan RenderInterval(0, 700*1000); // 700ms 

    // it's been less than 700 ms since last data was posted
    return !MapDirty && !timestamp_newdata.isElapsed(RenderInterval.totalMicroseconds());
}

void MapWindow::UpdateInfo(NMEA_INFO *nmea_info,
                           DERIVED_INFO *derived_info) {
  LockFlightData();
  memcpy(&DrawInfo,nmea_info,sizeof(NMEA_INFO));
  memcpy(&DerivedDrawInfo,derived_info,sizeof(DERIVED_INFO));
  zoom.UpdateMapScale(); // done here to avoid double latency due to locks 
  UnlockFlightData();
}



void MapWindow::UpdateCaches(bool force) {
  // map was dirtied while we were drawing, so skip slow process
  // (unless we haven't done it for 2000 ms)

  if (MapWindow::ForceVisibilityScan) {
    force = true;
    MapWindow::ForceVisibilityScan = false;
  }

  // have some time, do shape file cache update if necessary
  LockTerrainDataGraphics();
  SetTopologyBounds(DrawRect, force);
  UnlockTerrainDataGraphics();
}


