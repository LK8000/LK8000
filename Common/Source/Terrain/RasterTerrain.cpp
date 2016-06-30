/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: RasterTerrain.cpp,v 8.4 2010/12/12 16:31:39 root Exp root $
*/

#include "externs.h"
#include "RasterTerrain.h"
#include "Dialogs.h"

RasterMap* RasterTerrain::TerrainMap = NULL;


bool RasterMap::GetMapCenter(double *lon, double *lat) const {
  if(!isMapLoaded())
    return false;

  *lon = (TerrainInfo.Left + TerrainInfo.Right)/2;
  *lat = (TerrainInfo.Top + TerrainInfo.Bottom)/2;
  return true;
}


float RasterMap::GetFieldStepSize() const {
  if (!isMapLoaded()) {
    return 0;
  }
  // this is approximate of course..
  float fstepsize = (float)(250.0/0.0025*TerrainInfo.StepSize);
  return fstepsize;
}


// accurate method
int RasterMap::GetEffectivePixelSize(double *pixel_D,
                                     double latitude, double longitude) const
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


int RasterMap::GetEffectivePixelSize(double dist) const {
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

  if ((Xrounding==1)&&(Yrounding==1)) {
    DirectFine = true;
    xlleft = (int)(TerrainInfo.Left*fXroundingFine)+128;
    xlltop  = (int)(TerrainInfo.Top*fYroundingFine)-128;
  } else {
    DirectFine = false;
  }
}



////////// Map general /////////////////////////////////////////////


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
  if (TerrainMap && TerrainMap->isMapLoaded()) {
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

void RasterTerrain::SetTerrainRounding(double x, double y) {
  if (TerrainMap) {
    TerrainMap->SetFieldRounding(x, y);
  }
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


