/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   SetHome.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 19 September 2022
 */

#ifndef _Waypoints_SetHome_h_
#define _Waypoints_SetHome_h_

#include <stddef.h>

/**
 * @idx : Waypoint Index
 * @return : true if idx is valid waypoint
 */
bool SetNewHome(size_t idx);

/**
 * called asynchronously after Waypoint Loading to set "HomeWaypoint"
 * using  Airfield file or Waypoint Flags if Home is not set by Task.
 */
void SetHome(bool reset);

#endif // _Waypoints_SetHome_h_
