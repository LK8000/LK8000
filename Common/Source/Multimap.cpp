/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Sideview.h"

//
// It is IMPERATIVE that multimaps using a split screen keep these variables updated!
// Normally all "shared" multimaps should do it!
//

// SizeY is the percentage of screen size for topview.
int	Current_Multimap_SizeY=SIZE1;
// TopRect is the geometry of topview. Can be 0,0,0,0  if it is off.
RECT	Current_Multimap_TopRect={0,0,1,1};
// The view angle  in topview
double	Current_Multimap_TopAngle=0;
double	Current_Multimap_TopZoom=1;
POINT	Current_Multimap_TopOrig={0,0};




//
// All multimaps including MSM_MAP main
//
bool IsMultiMap() {

#if NEWMULTIMAPS
  if ( (MapSpaceMode >= MSM_MAPRADAR) && (MapSpaceMode <= MSM_MAPTEST)||(MapSpaceMode==MSM_MAP))
#else
  if ( (MapSpaceMode >= MSM_MAPRADAR) && (MapSpaceMode <= MSM_MAPTEST) )
#endif
	return true;
  else
	return false;
}


//
// All multimaps excluded MSM_MAP
//
bool IsMultiMapNoMain() {

  if ( (MapSpaceMode >= MSM_MAPRADAR) && (MapSpaceMode <= MSM_MAPTEST))
	return true;
  else
	return false;
}
	

//
// All multimapped page, not sharing map customkeys and events
//
bool IsMultiMapCustom() {
  //if ( (MapSpaceMode >= MSM_MAPRADAR) && (MapSpaceMode <= MSM_MAPTEST))
  if ( (MapSpaceMode == MSM_MAPRADAR) || (MapSpaceMode == MSM_MAPTEST))
	return true;
  else
	return false;
}


//
// Multimaps sharing events and customkeys with main map, INCLUDING MSM_MAP
//
bool IsMultiMapShared() {
  // TODO!
  // static bool shared[]={ false, false, false..} 
  // return shared[MapSpaceMode];  simply...
  //
  if ( (MapSpaceMode==MSM_MAP) || (MapSpaceMode == MSM_MAPTRK) || (MapSpaceMode == MSM_MAPWPT) || (MapSpaceMode==MSM_MAPASP) )
	return true;
  else
	return false;
}

bool IsMultiMapSharedNoMain() {
  if ( (MapSpaceMode == MSM_MAPTRK) || (MapSpaceMode == MSM_MAPWPT) || (MapSpaceMode==MSM_MAPASP) )
	return true;
  else
	return false;
}



//
// Back conversion from mapspace to maptype
//
short Get_Current_Multimap_Type() {

  short ret=MP_MOVING;
  if (!IsMultiMap()) return MP_MOVING;
  switch(MapSpaceMode) {
	case MSM_MAPRADAR:
		ret=MP_RADAR;
		break;
	case MSM_MAPTRK:
		ret=MP_MAPTRK;
		break;
	case MSM_MAPWPT:
		ret=MP_MAPWPT;
		break;
	case MSM_MAPASP:
		ret=MP_MAPASP;
		break;
	case MSM_MAPTEST:
		ret=MP_TEST;
		break;
	default:
		ret=MP_MOVING;
		break;
  }
  LKASSERT(ret<=(MP_TOP+1));
  return ret;
}

unsigned short GetMultimap_Labels(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  return Multimap_Labels[i];
}

void SetMultimap_Labels(const unsigned short val) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Labels[i]=val;
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

bool IsMultimapOverlays(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  return Multimap_Flags_Overlays[i];
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

void ToggleMultimapOverlays(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Overlays[i]=!Multimap_Flags_Overlays[i];
}



void EnableMultimapTerrain(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Terrain[i]=true;
}

void EnableMultimapTopology(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Topology[i]=true;
}

void EnableMultimapAirspace(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Airspace[i]=true;
}

void EnableMultimapWaypoints(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Waypoints[i]=true;
}

void EnableMultimapOverlays(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Overlays[i]=true;
}


void DisableMultimapTerrain(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Terrain[i]=false;
}

void DisableMultimapTopology(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Topology[i]=false;
}

void DisableMultimapAirspace(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Airspace[i]=false;
}

void DisableMultimapWaypoints(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Waypoints[i]=false;
}

void DisableMultimapOverlays(void) {
  short i=Get_Current_Multimap_Type();
  LKASSERT( (i>=0) && (i<(MP_TOP+1)));
  Multimap_Flags_Overlays[i]=false;
}


//
// Default flags for multimaps, used at init and at reset config
// They are eventually overloaded by a profile.
// See Defines.h for MP_ definitions.
// Of course MapWindowBg may decide not to use overlays, for example, by itself for a 
// particular MapSpaceMode.. nevertheless, here are the defaults.
//
void Reset_Multimap_Flags(void) {
  short i;
  for (i=0; i<=MP_TOP; i++) {
	Multimap_Flags_Terrain[i]=true;
	Multimap_Flags_Topology[i]=true;
	Multimap_Flags_Airspace[i]=true;
	Multimap_Flags_Waypoints[i]=true;
	Multimap_Flags_Overlays[i]=true;
	Multimap_Labels[i]=MAPLABELS_ALLON;
	Multimap_SizeY[i]=SIZE1;	// default
  }

  // We should always get these flags set correctly, because the LKInterface is working
  // in a separated thread by Draw
  Multimap_Flags_Overlays[MP_WELCOME]=false;
  Multimap_Flags_Overlays[MP_RADAR]=false;
  Multimap_Flags_Overlays[MP_TEST]=false;
}



void MultiMapSound() {
#ifndef DISABLEAUDIO
  if (EnableSoundModes) {
	switch(CURTYPE) {
		case 0: // MP_WELCOME
			break;
		case 1: // MP_MOVING
			PlayResource(TEXT("IDR_WAV_MM0"));
			break;
		case 2: 
			PlayResource(TEXT("IDR_WAV_MM1"));
			break;
		case 3: 
			PlayResource(TEXT("IDR_WAV_MM2"));
			break;
		case 4:
			PlayResource(TEXT("IDR_WAV_MM3"));
			break;
		case 5:
			PlayResource(TEXT("IDR_WAV_MM4"));
			break;
		case 6:
			PlayResource(TEXT("IDR_WAV_MM5"));
			break;
#if 0
		case 7:
			PlayResource(TEXT("IDR_WAV_MM6"));
			break;
#endif
		default:
			break;
	}
  }
#endif
}

