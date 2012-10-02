/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


bool IsMultiMap() {
  if ( (MapSpaceMode >= MSM_MAPRADAR) && (MapSpaceMode <= MSM_MAPTEST) )
	return true;
  else
	return false;
}


//
// Back conversion from mapspace to maptype
//
short Get_Current_Multimap_Type() {

  if (!IsMultiMap()) return MP_MOVING;
  switch(MapSpaceMode) {
	case MSM_MAPRADAR:
		return MP_RADAR;
		break;
	case MSM_MAPTRK:
		return MP_MAPTRK;
		break;
	case MSM_MAPWPT:
		return MP_MAPWPT;
		break;
	case MSM_MAPASP:
		return MP_MAPASP;
		break;
	case MSM_MAPTEST:
		return MP_TEST;
		break;
	default:
		return MP_MOVING;
		break;
  }
  return MP_MOVING;
}

bool IsMultimapTerrain(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  return Multimap_Flags_Terrain[i];
}

bool IsMultimapTopology(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  return Multimap_Flags_Topology[i];
}

bool IsMultimapAirspace(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  return Multimap_Flags_Airspace[i];
}

bool IsMultimapWaypoints(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  return Multimap_Flags_Waypoints[i];
}



void ToggleMultimapTerrain(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Terrain[i]=!Multimap_Flags_Terrain[i];
}

void ToggleMultimapTopology(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Topology[i]=!Multimap_Flags_Topology[i];
}

void ToggleMultimapAirspace(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Airspace[i]=!Multimap_Flags_Airspace[i];
}

void ToggleMultimapWaypoints(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Waypoints[i]=!Multimap_Flags_Waypoints[i];
}

void Reset_Multimap_Flags(void) {
  short i;
  for (i=0; i<=MP_TOP; i++) {
	Multimap_Flags_Terrain[i]=true;
	Multimap_Flags_Topology[i]=true;
	Multimap_Flags_Airspace[i]=true;
	Multimap_Flags_Waypoints[i]=true;
  }
}



void MultiMapSound() {
#ifndef DISABLEAUDIO
  if (EnableSoundModes) {
	switch(CURTYPE) {
		case 1: // MP_MOVING
			PlayResource(TEXT("IDR_WAV_MM0"));
			break;
		case 2: // MAPASP
			PlayResource(TEXT("IDR_WAV_MM1"));
			break;
		case 3: // MP_RADAR
			PlayResource(TEXT("IDR_WAV_MM2"));
			break;
		case 4:
			PlayResource(TEXT("IDR_WAV_MM3"));
			break;
		case 5:
			PlayResource(TEXT("IDR_WAV_MM4"));
			break;
		default:
			break;
	}
  }
#endif
}

