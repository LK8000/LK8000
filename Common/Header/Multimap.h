/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef MULTIMAP_H
#define MULTIMAP_H

#define NUMBER_OF_MULTIMAPS	MP_TOP

extern int Current_Multimap_SizeY;

bool IsMultiMap(void);
void MultiMapSound(void);

void ToggleMultimapTerrain(void);
void ToggleMultimapAirspace(void);
void ToggleMultimapTopology(void);
void ToggleMultimapWaypoints(void);

bool IsMultimapTerrain(void);
bool IsMultimapAirspace(void);
bool IsMultimapTopology(void);
bool IsMultimapWaypoints(void);

short Get_Current_Multimap_Type();
void Reset_Multimap_Flags(void);


#endif
