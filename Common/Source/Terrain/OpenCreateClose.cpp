/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id$
 */

#include "externs.h"
#include "RasterTerrain.h"
#include "Dialogs/dlgProgress.h"
#include "Message.h"
#include "LocalPath.h"

std::unique_ptr<RasterMap> RasterTerrain::TerrainMap;
Mutex RasterTerrain::mutex;

void RasterTerrain::OpenTerrain() {
  TestLog(_T(". Loading Terrain..."));
  CreateProgressDialog(MsgToken<900>());  // Loading Terrain File...

  TCHAR szFile[MAX_PATH] = _T("\0");
  LocalPath(szFile, _T(LKD_MAPS), szTerrainFile);

  if ((_tcslen(szFile) > 0) && (_tcsstr(szFile, _T(".DEM")) || _tcsstr(szFile, _T(".dem")))) {
    if (!CreateTerrainMap(szFile)) {
      // If no terrain will be found, the registry will be invalid on next run
      _tcscpy(szTerrainFile, _T(""));
      StartupStore(_T("... INVALID TERRAIN file <%s>"), szFile);
      Message::AddMessage(12000, MSG_UNKNOWN, _T("Failed to load terrain, invalid dem file."));
    }
  } else {
    StartupStore(_T(". NO TERRAIN file available."));
  }
}

bool RasterTerrain::CreateTerrainMap(const TCHAR* zfilename) {
  ScopeLock lock(mutex);
  try {
    TerrainMap = std::make_unique<RasterMap>();
    if (!TerrainMap->Open(zfilename)) {
      TerrainMap = nullptr;
    }
  } catch (std::exception&) {
    TerrainMap = nullptr;
  }
  return static_cast<bool>(TerrainMap);
}

///////// Specialised open/close routines ///////////////////

void RasterTerrain::CloseTerrain() {
  TestLog(_T(". CloseTerrain"));

  ScopeLock lock(mutex);
  TerrainMap = nullptr;
}
