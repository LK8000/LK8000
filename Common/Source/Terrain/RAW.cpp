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


#include "wcecompat/ts_string.h"



void RasterMapRaw::SetFieldRounding(double xr, double yr) {
  RasterMap::SetFieldRounding(xr, yr);
  if (!isMapLoaded()) {
    return;
  }
  if ((Xrounding==1)&&(Yrounding==1)) {
    DirectFine = true;
    xlleft = (int)(TerrainInfo.Left*fXroundingFine)+128;
    xlltop  = (int)(TerrainInfo.Top*fYroundingFine)-128;
  } else {
    DirectFine = false;
  }
}


////// Field access ////////////////////////////////////////////////////


short RasterMapRaw::_GetFieldAtXY(unsigned int lx,
                                  unsigned int ly) {

  unsigned int ix = CombinedDivAndMod(lx);
  unsigned int iy = CombinedDivAndMod(ly);
  
  if ((ly>=(unsigned int)TerrainInfo.Rows)
      ||(lx>=(unsigned int)TerrainInfo.Columns)) {
    return TERRAIN_INVALID;
  } 
  
  short *tm = TerrainMem+ly*TerrainInfo.Columns+lx;
  // perform piecewise linear interpolation
  int h1 = *tm; // (x,y)
  
  if (!ix && !iy) {
    return h1;
  }
  if (lx+1 >= (unsigned int)TerrainInfo.Columns) {
    return h1;
  }
  if (ly+1 >= (unsigned int)TerrainInfo.Rows) {
    return h1;
  }
  int h3 = tm[TerrainInfo.Columns+1]; // (x+1, y+1)
  if (ix>iy) {
    // lower triangle 
    int h2 = tm[1]; // (x+1,y)
    return (short)(h1+((ix*(h2-h1)-iy*(h2-h3))>>8));
  } else {
    // upper triangle
    int h4 = tm[TerrainInfo.Columns]; // (x,y+1)
    return (short)(h1+((iy*(h4-h1)-ix*(h4-h3))>>8));
  }
}



void RasterMapRaw::Lock(void) {
  EnterCriticalSection(&CritSec_TerrainFile);
}

void RasterMapRaw::Unlock(void) {
  LeaveCriticalSection(&CritSec_TerrainFile);
}



bool RasterMapRaw::Open(const TCHAR* zfilename) {
  ZZIP_FILE *fpTerrain;

  max_field_value = 0;
  terrain_valid = false;

  if (_tcslen(zfilename)<=0) 
    return false;

  #if TESTBENCH
  StartupStore(_T(". Terrain Open RasterMapRaw <%s>%s"),zfilename,NEWLINE);
  #endif

  fpTerrain = zzip_fopen(zfilename, "rb");
  if (fpTerrain == NULL) {
	StartupStore(_T(". Terrain RasterMapRaw Open failed%s"),NEWLINE);
	return false;
  }

  DWORD dwBytesRead;
  dwBytesRead = zzip_fread(&TerrainInfo, 1, sizeof(TERRAIN_INFO), fpTerrain);
    
  if (dwBytesRead != sizeof(TERRAIN_INFO)) {
	StartupStore(_T("------ Terrain read first failed, invalid header%s"),NEWLINE);
	zzip_fclose(fpTerrain);
	return false;
  }
    
  long nsize = TerrainInfo.Rows*TerrainInfo.Columns;
  #ifdef HC_DMALLOC
  StartupStore(_T(". Terrain size is %ld, max hblock %lu %s"),
    (long)nsize*sizeof(short), CheckMaxHeapBlock(), NEWLINE);
  #else
  StartupStore(_T(". Terrain size is %ld%s"),
    (long)nsize*sizeof(short), NEWLINE);
  #endif

  if (CheckFreeRam()>(unsigned long)(nsize*sizeof(short)+5000000)) {
    // make sure there is 5 meg of ram left after allocating space
    TerrainMem = (short*)malloc(sizeof(short)*nsize);
  } else {
    zzip_fclose(fpTerrain);
    StartupStore(_T(".... Load Terrain FAILED: Not enough memory (free=%ld need=%ld+5M)!\n"),
    CheckFreeRam(), (unsigned long)(nsize*sizeof(short)));
    TerrainMem = NULL;
    return false;
  }
    
  if (!TerrainMem) {
    OutOfMemory(__FILE__,__LINE__);
    zzip_fclose(fpTerrain);
    terrain_valid = false;
  } else {
    dwBytesRead = zzip_fread(TerrainMem, 1, nsize*sizeof(short), fpTerrain);
    
    for (int i=0; i< nsize; i++) {
      max_field_value = max(TerrainMem[i], max_field_value);
    }
    zzip_fclose(fpTerrain);
    terrain_valid = true;
  }
      
  if (!TerrainInfo.StepSize) {
    StartupStore(_T("... ERROR Terrain StepSize failure\n"));
    terrain_valid = false;
    zzip_fclose(fpTerrain);
    Close();
  }
  return terrain_valid;
}


void RasterMapRaw::Close(void) {
  terrain_valid = false;
  if (TerrainMem) {
    free(TerrainMem); TerrainMem = NULL;
  }
}



