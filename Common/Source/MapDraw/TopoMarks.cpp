/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#if USETOPOMARKS

/*
 * THIS IS THE OLD MARKERS SYSTEM, OBSOLETED BY THE NEW ONE IN LK8000
 *
 * It is based on creating a shapefile with new topology items.
 * Pretty complicated, but it may come useful for something else.
 */

#include "Terrain.h"
#include "STScreenBuffer.h"
#include "LKProcess.h"
#include "Waypointparser.h"
#include "RGB.h"



TopologyWriter *topo_marks = NULL;
bool reset_marks = false;

// inititalise shapes for markers, not the text file surviving restarts
void TopologyInitialiseMarks() {

  StartupStore(TEXT(". Initialise marks%s"),NEWLINE);

  LockTerrainDataGraphics();

  // TODO code: - This convert to non-unicode will not support all languages
  //		(some may use more complicated PATH names, containing Unicode)
  //  char buffer[MAX_PATH];
  //  ConvertTToC(buffer, LocalPath(TEXT("xcsoar-marks")));
  // DISABLED LocalPath
  // JMW localpath does NOT work for the shapefile renderer!
  if (topo_marks) {
    topo_marks->DeleteFiles();
    delete topo_marks;
  }

  TCHAR buf[MAX_PATH];
  LocalPath(buf, _T(LKD_CONF));
  _tcscat(buf, _T(DIRSEP)); _tcscat(buf, _T(LKF_SMARKS));
  topo_marks = new TopologyWriter(buf, LKColor(0xD0,0xD0,0xD0));
  if (topo_marks) {
    topo_marks->scaleThreshold = 30.0;
    //topo_marks->scaleDefaultThreshold = 30.0;	// 101212
    topo_marks->scaleCategory = 0;		// 101212 marked locations
    topo_marks->loadBitmap(IDB_MARK);
  }
  UnlockTerrainDataGraphics();
}


void TopologyCloseMarks() {
  StartupStore(TEXT(". CloseMarks%s"),NEWLINE);
  LockTerrainDataGraphics();
  if (topo_marks) {
    topo_marks->DeleteFiles();
    delete topo_marks;
    topo_marks = NULL;
  }
  UnlockTerrainDataGraphics();
}


void DrawMarks (const HDC hdc, const RECT rc)
{

  LockTerrainDataGraphics();
  if (topo_marks) {
	if (reset_marks) {
		topo_marks->Reset();
		reset_marks = false;
	}
	topo_marks->Paint(hdc, rc);
  }
  UnlockTerrainDataGraphics();

}

#endif // USETOPOMARKS
