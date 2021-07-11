/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef BARO_H
#define BARO_H

//
// Parser's IDs
//
#define BARO__RMZ               1
#define BARO__RMZ_FLARM         2       // RMZ coming from a Flarm device.
#define BARO__RMA               3
#define BARO__CUSTOMFROM        4
#define BARO__GM130             4
#define BARO__ROYALTEK3200      5
#define BARO__TASMAN            6
#define BARO__CPROBE            7
#define BARO__CUSTOMTO          7
#define BARO__END               8       // marking the limit

// Verbose debugging test
// #define DEBUGBARO    1

bool UpdateBaroSource( NMEA_INFO* pGPS, short parserid, const PDeviceDescriptor_t d, double fAlt);

#endif
