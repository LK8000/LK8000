/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"

#ifdef __MINGW32__
#define int_fast8_t jas_int_fast8_t
#endif

// Set the followings inside options:
// #define JP2000	1	// use old JPG2000 terrain, not in LKMAPS
// #define LKMTERRAIN	1	// load terrain DEM also from topology maps, NOT in LKMAPS


using std::min;
using std::max;


bool RasterTerrain::terrain_initialised = false;

void RasterTerrain::OpenTerrain(void)
{
  terrain_initialised = false;

  StartupStore(TEXT(". Loading Terrain... %s"),NEWLINE);
  CreateProgressDialog(gettext(TEXT("_@M900_"))); // Loading Terrain File...

  TCHAR szFile[MAX_PATH] = _T("\0");

  GetRegistryString(szRegistryTerrainFile, szFile, MAX_PATH);

  TCHAR szOrigFile[MAX_PATH] = _T("\0");
  ExpandLocalPath(szFile);
  _tcscpy(szOrigFile, szFile);
  ContractLocalPath(szOrigFile);

  // If no terrain will be found, the registry will be invalid on next run
  // This can be removed when we remove registry functions
  SetRegistryString(szRegistryTerrainFile, TEXT("\0"));


#ifdef LKMTERRAIN
  static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
  if (_tcslen(szFile)==0) {
    StartupStore(_T(". NO TERRAIN file configured%s"),NEWLINE);
    GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
    ExpandLocalPath(szMapFile);
    _tcscpy(szFile,szMapFile);

    #ifdef JP2000
    _tcscat(szFile, _T("/terrain.jp2")); 
    StartupStore(_T(". Attempting to use JP2 <%s> inside mapfile%s"),szFile,NEWLINE);

	// support terrain.dat inside xcm files
	if (CreateTerrainMap(szFile)) {
		SetRegistryString(szRegistryTerrainFile, szOrigFile);
		terrain_initialised = true;
		return;
	} else {
   		 _tcscpy(szFile,szMapFile);
		_tcscat(szFile, _T("/terrain.dem")); 
		StartupStore(_T(". Attempting to use DEM <%s> inside mapfile%s"),szFile,NEWLINE);
	}
     #else
     _tcscat(szFile, _T("/terrain.dem")); 
     StartupStore(_T(". Attempting to use DEM <%s> inside mapfile%s"),szFile,NEWLINE);
     #endif
  }

  if (CreateTerrainMap(szFile)) {
	SetRegistryString(szRegistryTerrainFile, szOrigFile);
	terrain_initialised = true;
	return;
  } else {
	_tcscpy(szFile,szMapFile);
	_tcscat(szFile, _T("/terrain.dat")); 
	StartupStore(_T(". Attempting to use DAT <%s> inside mapfile%s"),szFile,NEWLINE);

	if (CreateTerrainMap(szFile)) {
		SetRegistryString(szRegistryTerrainFile, szOrigFile);
		terrain_initialised = true;
		return;
	}
   }
#else

  if ( (_tcslen(szFile)>0) && ( _tcsstr(szFile, _T(".DEM")) || _tcsstr(szFile, _T(".dem")) ) ) {
	if (CreateTerrainMap(szFile)) {
		SetRegistryString(szRegistryTerrainFile, szOrigFile);
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
  StartupStore(_T(". No terrain file available.%s"),NEWLINE);
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
  TerrainMap->Close();
  delete TerrainMap;

  #if RASTERCACHE
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
  StartupStore(TEXT(". CloseTerrain%s"),NEWLINE);

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

