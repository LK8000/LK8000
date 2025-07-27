/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"

void TerrainHeight(NMEA_INFO* Basic, DERIVED_INFO* Calculated) {
  short Alt = RasterTerrain::GetHeightAccurate({Basic->Latitude, Basic->Longitude});
  if (Alt != TERRAIN_INVALID) {  // terrain invalid is now positive  ex. 32767
    Calculated->TerrainValid = true;
    Calculated->TerrainAlt = std::max<short>(0, Alt);
  }
  else {
    Calculated->TerrainValid = false;
    Calculated->TerrainAlt = 0;
  }

  Calculated->AltitudeAGL = Calculated->NavAltitude - Calculated->TerrainAlt;
  if (!FinalGlideTerrain) {
    Calculated->TerrainBase = Calculated->TerrainAlt;
  }
}
