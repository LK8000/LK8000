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




RasterMap* RasterTerrain::TerrainMap = NULL;


bool RasterMap::GetMapCenter(double *lon, double *lat) {
  if(!isMapLoaded())
    return false;

  *lon = (TerrainInfo.Left + TerrainInfo.Right)/2;
  *lat = (TerrainInfo.Top + TerrainInfo.Bottom)/2;
  return true;
}


float RasterMap::GetFieldStepSize() {
  if (!isMapLoaded()) {
    return 0;
  }
  // this is approximate of course..
  float fstepsize = (float)(250.0/0.0025*TerrainInfo.StepSize);
  return fstepsize;
}


// accurate method
int RasterMap::GetEffectivePixelSize(double *pixel_D,
                                     double latitude, double longitude)
{
  double terrain_step_x, terrain_step_y;
  double step_size = TerrainInfo.StepSize*sqrt(2.0);

  if ((*pixel_D<=0) || (step_size==0)) {
    *pixel_D = 1.0;
    return 1;
  }

  // how many steps are in the pixel size
  DistanceBearing(latitude, longitude, latitude+step_size, 
                  longitude, &terrain_step_x, NULL);
  terrain_step_x = fabs(terrain_step_x);

  DistanceBearing(latitude, longitude, latitude, 
                  longitude+step_size, &terrain_step_y, NULL);
  terrain_step_y = fabs(terrain_step_y);

  double rfact = max(terrain_step_x,terrain_step_y)/(*pixel_D);

  int epx = (int)(max(1.0,ceil(rfact)));
  //  *pixel_D = (*pixel_D)*rfact/epx;

  return epx;
}


int RasterMap::GetEffectivePixelSize(double dist) {
  int grounding;
  LKASSERT(dist!=0);
  grounding = iround(2.0*(GetFieldStepSize()/1000.0)/dist);
  if (grounding<1) {
    grounding = 1;
  }
  return grounding;
}

void RasterMap::SetFieldRounding(double xr, double yr) {
  if (!isMapLoaded()) {
    return;
  }

  Xrounding = iround(xr/TerrainInfo.StepSize);
  Yrounding = iround(yr/TerrainInfo.StepSize);

  if (Xrounding<1) {
    Xrounding = 1;
  } 
  fXrounding = 1.0/(Xrounding*TerrainInfo.StepSize);
  fXroundingFine = fXrounding*256.0;
  if (Yrounding<1) {
    Yrounding = 1;
  }
  fYrounding = 1.0/(Yrounding*TerrainInfo.StepSize);
  fYroundingFine = fYrounding*256.0;

  DirectFine = false;
}



////////// Map general /////////////////////////////////////////////


// JMW rounding further reduces data as required to speed up terrain
// display on low zoom levels
short RasterMap::GetField(const double &Latitude, 
                          const double &Longitude)
{
  if(isMapLoaded()) {
    if (DirectFine) {
      return _GetFieldAtXY((int)(Longitude*fXroundingFine)-xlleft,
                           xlltop- (int)(Latitude*fYroundingFine));
    } else {
	#if (WINDOWSPC>0)
      unsigned int ix = 
        Real2Int((Longitude-TerrainInfo.Left)*fXrounding)*Xrounding;
      unsigned int iy = 
        Real2Int((TerrainInfo.Top-Latitude)*fYrounding)*Yrounding;
	#else
      unsigned int ix = ((int)((Longitude-TerrainInfo.Left)*fXrounding)) *Xrounding;
      unsigned int iy = ((int)((TerrainInfo.Top-Latitude)*fYrounding))*Yrounding;
	#endif
      
      return _GetFieldAtXY(ix<<8, iy<<8);
    }
  } else {
    return TERRAIN_INVALID;
  }
}



void RasterTerrain::Lock(void) {
  if (TerrainMap) {
    TerrainMap->Lock();
  }
}

void RasterTerrain::Unlock(void) {
  if (TerrainMap) {
    TerrainMap->Unlock();
  }
}

short RasterTerrain::GetTerrainHeight(const double &Latitude,
                                      const double &Longitude) {
  if (TerrainMap) {
    return TerrainMap->GetField(Latitude, Longitude);
  } else {
    return TERRAIN_INVALID;
  }
}

bool RasterTerrain::IsDirectAccess(void) {
  if (TerrainMap) {
    return TerrainMap->IsDirectAccess();
  } else {
    return false;
  }
}


bool RasterTerrain::IsPaged(void) {
  if (TerrainMap) {
    return TerrainMap->IsPaged();
  } else {
    return false;
  }
}


void RasterTerrain::ServiceCache(void) {
  Lock();
  if (TerrainMap) {
    TerrainMap->ServiceCache();
  }
  Unlock();
}

void RasterTerrain::SetTerrainRounding(double x, double y) {
  if (TerrainMap) {
    TerrainMap->SetFieldRounding(x, y);
  }
}

void RasterTerrain::ServiceTerrainCenter(double lat, double lon) {
  Lock();

  if (TerrainMap) {
    TerrainMap->SetViewCenter(lat, lon);
  }
  Unlock();
}


void RasterTerrain::ServiceFullReload(double lat, double lon) {

  Lock();
  if (TerrainMap) {
    CreateProgressDialog(gettext(TEXT("_@M901_"))); // Loading terrain tiles...
	StartupStore(_T(". Loading terrain tiles...%s"),NEWLINE);
    TerrainMap->ServiceFullReload(lat, lon);
  }
  Unlock();
}


int RasterTerrain::GetEffectivePixelSize(double *pixel_D, 
                                         double latitude, double longitude) {
  if (TerrainMap) {
    return TerrainMap->GetEffectivePixelSize(pixel_D, latitude, longitude);
  } else {
    return 1;
  }
}


bool RasterTerrain::WaypointIsInTerrainRange(double latitude, 
                                             double longitude) {
  if (TerrainMap) {
    if ((latitude<= TerrainMap->TerrainInfo.Top)&&
        (latitude>= TerrainMap->TerrainInfo.Bottom)&&
        (longitude<= TerrainMap->TerrainInfo.Right)&&
        (longitude>= TerrainMap->TerrainInfo.Left)) {
      return true;
    } else {
      return false;
    }
  } else {
    return true;
  }
}


bool RasterTerrain::GetTerrainCenter(double *latitude,
                                     double *longitude) {
  if (TerrainMap) {
    *latitude = (TerrainMap->TerrainInfo.Top+
                 TerrainMap->TerrainInfo.Bottom)/2.0;
    *longitude = (TerrainMap->TerrainInfo.Left+
                  TerrainMap->TerrainInfo.Right)/2.0;
    return true;
  } else {
    return false;
  }
}


