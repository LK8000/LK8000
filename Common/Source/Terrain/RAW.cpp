/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "OS/Memory.h"
#include "OS/ByteOrder.hpp"
#include <memory>

static_assert(IsLittleEndian(), "Big-Endian Arch is not supported");

////// Field access ////////////////////////////////////////////////////


bool RasterMap::Open(const TCHAR* zfilename) {

  if (_tcslen(zfilename)<=0) {
    return false;
  }
  
  StartupStore(_T(". Terrain Open RasterMapRaw <%s>"),zfilename);
  FILE* file = _tfopen(zfilename, _T("rb"));
  if(file) {
    size_t read_size = fread(&TerrainInfo,1 , sizeof(TERRAIN_INFO), file);
    fclose(file);
    if(read_size != sizeof(TERRAIN_INFO)) {
      StartupStore(_T("... ERROR Terrain : failed to read file header"));
      return false;
    }
  }
  if (!TerrainInfo.StepSize) {
      StartupStore(_T("... ERROR Terrain StepSize failure"));
      Close();
      return false;
  }
  
  const size_t nsize = TerrainInfo.Rows*TerrainInfo.Columns;
  StartupStore(_T("... Terrain size is %u"), static_cast<unsigned>(nsize*sizeof(short)));
  StartupStore(_T("... Available memory is %ukB"), static_cast<unsigned>(CheckFreeRam()/1024));

  /* 
   * for small terrain file heap memory are faster (and more secure) 
   * than memory mapped file,
   * if file size less than 10MB, store DEM data into heap memory. 
   */
  if((nsize*sizeof(short)) < (10*1024*1024)) {
    /*
     *  try to create buffer for store DEM data
     */
    pTerrainMem.reset(new(std::nothrow) short[nsize]);
    /*
     * if available memory is less than 5MB
     * after allocate buffer we don't load terrain.
     */
    if( CheckFreeRam() < (5*1024*1024) ) {
       pTerrainMem.reset();
    }
  }
  
  if(pTerrainMem) {
    StartupStore(_T("... Terrain : use heap memory"));

    FILE* file = _tfopen(zfilename, _T("rb"));
    if(file) {
      fseek(file, sizeof(TERRAIN_INFO), SEEK_SET);  
      size_t read_size = fread(pTerrainMem.get(), sizeof(short), nsize, file);
      fclose(file);
      if(read_size == nsize) {
          TerrainMem = pTerrainMem.get();
      } else {
          pTerrainMem.reset();
      }
    }
  } else {
    /* 
     * Note : memory mapped file require SEH exception handling 
     * Mingw32ce don't implement SEH, so this part of code is disabled
     */
#ifndef UNDER_CE
    StartupStore(_T("... Terrain : use memory mapped file"));

    TerrainFile.open(zfilename, false);
    if(TerrainFile.is_open()) {
      TerrainFile.map(sizeof(TERRAIN_INFO), nsize*sizeof(short));
      if(TerrainFile.data()) {
        TerrainMem = reinterpret_cast<const short*>(TerrainFile.data());
      } else {
        TerrainFile.close();
      }
    }
#else
    StartupStore(_T("... Terrain File to huge."));
#endif    
  }
  
  if(!isMapLoaded()) {
    Close();
    StartupStore(_T("... Terrain RasterMapRaw load failed"));
    return false;
  }
  return true;
}


void RasterMap::Close(void) {
  // this 2 line are needed for debug diagnostics
  TerrainInfo.Columns = 0;
  TerrainInfo.Rows = 0;

  TerrainMem = nullptr;

  pTerrainMem.reset();

#ifndef UNDER_CE  
  if(TerrainFile.is_open()) {
    TerrainFile.close();
  }
#endif
}
