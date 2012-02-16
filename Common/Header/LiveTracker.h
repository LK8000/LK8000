/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights
*/

#ifndef LIVETRACKER_H
#define LIVETRACKER_H

#include "externs.h"

// Init Live Tracker services
void LiveTrackerInit();

// Shutdown Live Tracker
void LiveTrackerShutdown();

// Update live tracker data, non blocking
void LiveTrackerUpdate(NMEA_INFO *Basic, DERIVED_INFO *Calculated);

#endif // LIVETRACKER_H
