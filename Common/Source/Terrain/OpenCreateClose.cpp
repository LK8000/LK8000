/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "LKProfiles.h"
#include "Dialogs/dlgProgress.h"
#include "Message.h"

RasterMap* RasterTerrain::TerrainMap = nullptr;
Mutex  RasterTerrain::mutex;

void RasterTerrain::OpenTerrain(void)
{
  #if TESTBENCH
  StartupStore(TEXT(". Loading Terrain... %s"),NEWLINE);
  #endif
  CreateProgressDialog(MsgToken(900)); // Loading Terrain File...

  TCHAR szFile[MAX_PATH] = _T("\0");
  LocalPath(szFile,_T(LKD_MAPS), szTerrainFile);

  if ( (_tcslen(szFile)>0) && ( _tcsstr(szFile, _T(".DEM")) || _tcsstr(szFile, _T(".dem")) ) ) {
	if (!CreateTerrainMap(szFile)) {
      // If no terrain will be found, the registry will be invalid on next run
      _tcscpy(szTerrainFile,_T(""));
      StartupStore(_T("... INVALID TERRAIN file <%s>%s"),szFile,NEWLINE);
      Message::AddMessage(12000, MSG_UNKNOWN, _T("Failed to load terrain, invalid dem file."));
	}
  } else {
    StartupStore(_T(". NO TERRAIN file available.%s"),NEWLINE);
  }
}


bool RasterTerrain::CreateTerrainMap(const TCHAR *zfilename) {
  ScopeLock lock(mutex);

  LKASSERT(!TerrainMap); // memory leak;

  TerrainMap = new(std::nothrow) RasterMap();
  if (!TerrainMap) {
    return false;
  }
  if (!TerrainMap->Open(zfilename)) {
    delete TerrainMap;
    TerrainMap = nullptr;
  }
  return isTerrainLoaded();
}

///////// Specialised open/close routines ///////////////////


void RasterTerrain::CloseTerrain(void)
{
  #if TESTBENCH
  StartupStore(TEXT(". CloseTerrain%s"),NEWLINE);
  #endif

  ScopeLock lock(mutex);

  delete TerrainMap;
  TerrainMap = nullptr;
}
