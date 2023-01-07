/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef BARO_H
#define BARO_H

struct DeviceDescriptor_t;
struct NMEA_INFO;

bool BaroAltitudeAvailable(const NMEA_INFO& Info);

void ResetBaroAvailable(NMEA_INFO& Info);

void CheckBaroAltitudeValidity(NMEA_INFO& Info);

void UpdateBaroSource(NMEA_INFO* pGPS, DeviceDescriptor_t* d, double fAlt);

#endif
