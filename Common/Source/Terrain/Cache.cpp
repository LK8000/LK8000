/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: RasterTerrain.cpp,v 8.4 2010/12/12 16:31:39 root Exp root $
*/

#include "externs.h"
#include "RasterTerrain.h"

#ifdef __MINGW32__
#define int_fast8_t jas_int_fast8_t
#endif


using std::min;
using std::max;


short RasterMapCache::_GetFieldAtXY(unsigned int lx,
                                    unsigned int ly) {

  //  unsigned int ix = CombinedDivAndMod(lx);
  //  unsigned int iy = CombinedDivAndMod(ly);

  if ((ly>=(unsigned int)TerrainInfo.Rows)
      ||(lx>=(unsigned int)TerrainInfo.Columns)) {
    return TERRAIN_INVALID;
  } 
  return LookupTerrainCache((ly*TerrainInfo.Columns+lx)*2
                            +sizeof(TERRAIN_INFO));
}


//////////// Cached load on demand ////////////////////////////////

//////////// Cache map ////////////////////////////////////////////////
CRITICAL_SECTION RasterMapCache::CritSec_TerrainFile;

int RasterMapCache::ref_count = 0;

ZZIP_FILE *RasterMapCache::fpTerrain;

void RasterMapCache::ServiceCache(void) {
  Lock();
  
  if (terraincachemisses > 0){
    OptimizeCash();
  }
  SetCacheTime();

  Unlock();
}


void RasterMapCache::Lock(void) {
  EnterCriticalSection(&CritSec_TerrainFile);
}


void RasterMapCache::Unlock(void) {
  LeaveCriticalSection(&CritSec_TerrainFile);
}


void RasterMapCache::SetCacheTime() {
  terraincachehits = 1;
  terraincachemisses = 0;
  cachetime++;
}


void RasterMapCache::ClearTerrainCache() {
  int i;
  for (i=0; i<MAXTERRAINCACHE; i++) {
    TerrainCache[i].index= -1;
    TerrainCache[i].recency= 0;
    TerrainCache[i].h= 0;
  }
  SortThresold = MAXTERRAINCACHE-1; 
}

static int _cdecl TerrainCacheCompare(const void *elem1, const void *elem2 ){
#ifdef PARANOID
  if (!elem1 && !elem2) {
    return(0);
  }
  if (elem1 && !elem2) {
    return(-1);
  }
  if (!elem1 && elem2) {
    return(1);
  }
#endif
  if (((TERRAIN_CACHE *)elem1)->recency > ((TERRAIN_CACHE *)elem2)->recency)
    return (-1);
  if (((TERRAIN_CACHE *)elem1)->recency < ((TERRAIN_CACHE *)elem2)->recency)
    return (+1);
  if (((TERRAIN_CACHE *)elem1)->index > ((TERRAIN_CACHE *)elem2)->index)
    return (-1);
  if (((TERRAIN_CACHE *)elem1)->index < ((TERRAIN_CACHE *)elem2)->index)
    return (+1);
  return (0);
}

void RasterMapCache::OptimizeCash(void){
  qsort(&TerrainCache, MAXTERRAINCACHE, 
        sizeof(_TERRAIN_CACHE), TerrainCacheCompare);
  SortThresold = MAXTERRAINCACHE-1; 
}


short RasterMapCache::LookupTerrainCacheFile(const long &SeekPos) {
  // put new value in slot tcpmin

  __int16 NewAlt = 0;
  long SeekRes;
  short Alt;

  if(!isMapLoaded())
    return TERRAIN_INVALID;

  Lock();

  SeekRes = zzip_seek(fpTerrain, SeekPos, SEEK_SET);
  if(SeekRes != SeekPos) {
    // error, not found!
    Alt = TERRAIN_INVALID;
  } else {
    if (zzip_fread(&NewAlt, 1, sizeof(__int16), fpTerrain) != sizeof(__int16))
      Alt = TERRAIN_INVALID;
    else {
	// FIX HERE NETHERLAND
      Alt = max((short int)0,NewAlt);
    }
  }
  Unlock();

  return Alt;
}


int TerrainCacheSearch(const void *key, const void *elem2 ){
#ifdef PARANOID
  if (!elem2) return (0);
#endif
  if ((long)key > ((TERRAIN_CACHE *)elem2)->index)
    return (-1);
  if ((long)key < ((TERRAIN_CACHE *)elem2)->index)
    return (+1);
  return (0);
}

short RasterMapCache::LookupTerrainCache(const long &SeekPos) {
  _TERRAIN_CACHE* tcp, *tcpmin, *tcplim;

  if(fpTerrain == NULL || TerrainInfo.StepSize == 0)
    return TERRAIN_INVALID;

  // search to see if it is found in the cache
  tcp = (_TERRAIN_CACHE *)bsearch((void *)SeekPos, &TerrainCache, 
                                  SortThresold, sizeof(_TERRAIN_CACHE), 
                                  TerrainCacheSearch); 

  if (tcp != NULL){
    tcp->recency = cachetime;
    terraincachehits++;
    return(tcp->h);
  }

  // bsearch failed, so try exhaustive search

  tcp = &TerrainCache[SortThresold];
  tcplim = tcp+MAXTERRAINCACHE-SortThresold;
  while (tcp< tcplim) {
    if (tcp->index == SeekPos) {
      tcp->recency = cachetime;
      terraincachehits++;
      return (tcp->h);
    }
    tcp++;
  }

  // if not found..
  terraincachemisses++;

  if (SortThresold>= MAXTERRAINCACHE) {
    SortThresold= MAXTERRAINCACHE-1;
  } else if (SortThresold<0) {
    SortThresold = 0;
  }

  tcpmin = &TerrainCache[SortThresold];
  
  short Alt = LookupTerrainCacheFile(SeekPos);
  
  tcpmin->recency = cachetime;
  tcpmin->h = Alt;
  tcpmin->index = SeekPos;
  
  SortThresold--;
  if (SortThresold<0) {
    SortThresold = 0;
  }

  return (Alt);
}


bool RasterMapCache::Open(const TCHAR* zfilename) {

  terrain_valid = false;
  if (_tcslen(zfilename)<=0) {
    return false;
  }
  if (!fpTerrain) {
    fpTerrain = zzip_fopen(zfilename, "rb");
    if (!fpTerrain) {
      return false;
    }
    if (!zzip_file_real(fpTerrain)) {
      // don't allow cache mode on files in zip, because way too slow
      zzip_fclose(fpTerrain);
      fpTerrain = NULL;   // was false
      return false;
    };
  }

  DWORD dwBytesRead;
  dwBytesRead = zzip_fread(&TerrainInfo, 1, sizeof(TERRAIN_INFO), 
                           fpTerrain);
    
  if (dwBytesRead != sizeof(TERRAIN_INFO)) {
    Close();
    return false;
  }

  if (!TerrainInfo.StepSize) {
    Close();
    return false;
  }
  terrain_valid = true;
  ClearTerrainCache();
  return terrain_valid;
}


void RasterMapCache::Close(void) {
  terrain_valid = false;
  if(fpTerrain) {
    if (ref_count==1) {
      zzip_fclose(fpTerrain);
      fpTerrain = NULL;
    }
  }
}



