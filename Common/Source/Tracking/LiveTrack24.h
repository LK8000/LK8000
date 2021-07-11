/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights
*/

#ifndef TRACKING_LIVETRACK24_H
#define TRACKING_LIVETRACK24_H


// Init Live Tracker services
void LiveTrackerInit();

// Shutdown Live Tracker
void LiveTrackerShutdown();

// Update live tracker data, non blocking
void LiveTrackerUpdate(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

#endif // TRACKING_LIVETRACK24_H
