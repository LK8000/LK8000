/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "OS/Memory.h"
#include "utils/make_unique.h"


////// Field access ////////////////////////////////////////////////////


bool RasterMap::Open(const TCHAR* zfilename) {

  if (_tcslen(zfilename)<=0) {
    return false;
  }

  StartupStore(_T(". Terrain Open RasterMapRaw <%s>\n"),zfilename);

  TerrainFile.open(zfilename, true);
  if(TerrainFile.is_open() && TerrainFile.data()) {
    
    TerrainInfo = reinterpret_cast<const TERRAIN_INFO*>(TerrainFile.data());
    TerrainMem = reinterpret_cast<const short*>(TerrainFile.data() + sizeof(TERRAIN_INFO));

    const size_t nsize = TerrainInfo->Rows*TerrainInfo->Columns;
    if(sizeof(TERRAIN_INFO) + (nsize*sizeof(short)) != TerrainFile.mapped_size()) {
      StartupStore(_T("... ERROR Terrain : Invalide file size\n"));
      Close();
    } else if (!TerrainInfo->StepSize) {
      StartupStore(_T("... ERROR Terrain StepSize failure\n"));
      Close();
    } else {
      StartupStore(_T(". Terrain size is %ld\n"), (long)nsize*sizeof(short));
      
#ifdef UNDER_CE
      // head memory are faster than memory mapped file,
      // if filesize are not to huge, store DEM data into heap memory.
      
      if (CheckFreeRam()>(nsize*sizeof(short)+5000000U)) {
        // make sure there is 5 meg of ram left after allocating space
        pTerrainMem.reset(new(std::nothrow) short[nsize]);
        if(pTerrainMem) {
            std::copy_n(TerrainMem, nsize, pTerrainMem.get());
            TerrainMem = pTerrainMem.get();

            TerrainFile.map(0, sizeof(TERRAIN_INFO));
            TerrainInfo = reinterpret_cast<const TERRAIN_INFO*>(TerrainFile.data());
        }
      }
#endif
      
    }
  } else {
    StartupStore(_T(". Terrain RasterMapRaw Open failed%s"),NEWLINE);
  }
  
  return isMapLoaded();
}


void RasterMap::Close(void) {
  TerrainMem = nullptr;
  TerrainInfo = nullptr;
  
#ifdef UNDER_CE
  pTerrainMem = nullptr;
#endif
  
  if(TerrainFile.is_open()) {
    TerrainFile.close();
  }
}



