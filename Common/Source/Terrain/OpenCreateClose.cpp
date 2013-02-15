/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "LKProfiles.h"
#include "Dialogs.h"

#ifdef __MINGW32__
#define int_fast8_t jas_int_fast8_t
#endif

// Set the followings inside options:
// #define JP2000	1	// use old JPG2000 terrain, not in LKMAPS
// #define LKMTERRAIN	1	// load terrain DEM also from topology maps, NOT in LKMAPS



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

  TCHAR szOrigFile[MAX_PATH] = _T("\0");
  ExpandLocalPath(szFile);
  _tcscpy(szOrigFile, szFile);
  ContractLocalPath(szOrigFile);

  // If no terrain will be found, the registry will be invalid on next run
  _tcscpy(szTerrainFile,_T(""));


#ifdef LKMTERRAIN
  static TCHAR  szMFile[MAX_PATH] = TEXT("\0");
  if (_tcslen(szFile)==0) {
    StartupStore(_T(". NO TERRAIN file configured%s"),NEWLINE);
    _tcscpy(szMFile,szMapFile);
    ExpandLocalPath(szMFile);
    _tcscpy(szFile,szMFile);

    #ifdef JP2000
    _tcscat(szFile, _T("/terrain.jp2")); 
    StartupStore(_T(". Attempting to use JP2 <%s> inside mapfile%s"),szFile,NEWLINE);

	// support terrain.dat inside xcm files
	if (CreateTerrainMap(szFile)) {
		SetRegistryString(szRegistryTerrainFile, szOrigFile);
		terrain_initialised = true;
		return;
	} else {
   		 _tcscpy(szFile,szMFile);
		_tcscat(szFile, _T("/terrain.dem")); 
		StartupStore(_T(". Attempting to use DEM <%s> inside mapfile%s"),szFile,NEWLINE);
	}
     #else
     _tcscpy(szFile,szMFile);
     _tcscat(szFile, _T("/terrain.dem")); 
     StartupStore(_T(". Attempting to use DEM <%s> inside mapfile%s"),szFile,NEWLINE);
     #endif
  }

  if (CreateTerrainMap(szFile)) {
	_tcscpy(szTerrainFile,szOrigFile);
	terrain_initialised = true;
	return;
  } else {
	_tcscpy(szFile,szMFile);
	_tcscat(szFile, _T("/terrain.dat")); 
	StartupStore(_T(". Attempting to use DAT <%s> inside mapfile%s"),szFile,NEWLINE);

	if (CreateTerrainMap(szFile)) {
		_tcscpy(szTerrainFile,szOrigFile);
		terrain_initialised = true;
		return;
	}
   }
#else

  if ( (_tcslen(szFile)>0) && ( _tcsstr(szFile, _T(".DEM")) || _tcsstr(szFile, _T(".dem")) ) ) {
	if (CreateTerrainMap(szFile)) {
		_tcscpy(szterrainFile,szOrigFile);
		terrain_initialised = true;
		return;
	} else {
		StartupStore(_T("... INVALID TERRAIN file <%s>%s"),szFile,NEWLINE);
	}
  }

#endif

  if (TerrainMap) {
	TerrainMap->Close();
	delete TerrainMap;
	TerrainMap = NULL;
  }
  terrain_initialised = false;
  StartupStore(_T(". NO TERRAIN file available.%s"),NEWLINE);
}


bool RasterTerrain::CreateTerrainMap(const TCHAR *zfilename) {
#ifdef JP2000
  if (_tcsstr(zfilename, _T(".jp2"))) {
    TerrainMap = new RasterMapJPG2000();
    if (!TerrainMap) 
      return false;
    return TerrainMap->Open(zfilename);
  }
#endif

  TerrainMap = new RasterMapRaw();
  if (!TerrainMap) {
    return false;
  }
  if (TerrainMap->Open(zfilename)) {
    return true;
  } 

  #if RASTERCACHE
  TerrainMap->Close();
  delete TerrainMap;
  TerrainMap = new RasterMapCache();
  if (!TerrainMap) {
    return false;
  }
  if (TerrainMap->Open(zfilename)) {
    return true;
  } 
  #endif

  return false;
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

