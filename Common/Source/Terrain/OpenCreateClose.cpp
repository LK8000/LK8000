/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "LKProfiles.h"
#include "Dialogs/dlgProgress.h"


bool RasterTerrain::terrain_initialised = false;

void RasterTerrain::OpenTerrain(void)
{
  terrain_initialised = false;
  #if TESTBENCH
  StartupStore(TEXT(". Loading Terrain... %s"),NEWLINE);
  #endif
  CreateProgressDialog(gettext(TEXT("_@M900_"))); // Loading Terrain File...

  TCHAR szFile[MAX_PATH] = _T("\0");
  _tcscpy(szFile,szTerrainFile);

  ExpandLocalPath(szFile);

  if ( (_tcslen(szFile)>0) && ( _tcsstr(szFile, _T(".DEM")) || _tcsstr(szFile, _T(".dem")) ) ) {
	if (CreateTerrainMap(szFile)) {
      terrain_initialised = true;
    } else {
      // If no terrain will be found, the registry will be invalid on next run
      _tcscpy(szTerrainFile,_T(""));
      StartupStore(_T("... INVALID TERRAIN file <%s>%s"),szFile,NEWLINE);
	}
  } else {
    StartupStore(_T(". NO TERRAIN file available.%s"),NEWLINE);
  }
}


bool RasterTerrain::CreateTerrainMap(const TCHAR *zfilename) {
  LKASSERT(!TerrainMap); // memory leak;
  
  TerrainMap = new RasterMap();
  if (!TerrainMap) {
    return false;
  }
  if (!TerrainMap->Open(zfilename)) {
    delete TerrainMap;
    TerrainMap = nullptr;  
    return false;
  } 
  return true;
}

///////// Specialised open/close routines /////////////////// 


void RasterTerrain::CloseTerrain(void)
{
  #if TESTBENCH
  StartupStore(TEXT(". CloseTerrain%s"),NEWLINE);
  #endif

  // TODO code: lock it first?

  if (terrain_initialised) {

    if (TerrainMap) {
      TerrainMap->Close();
      delete TerrainMap; 
      TerrainMap = NULL;
    }
    terrain_initialised = false;
  }
}

