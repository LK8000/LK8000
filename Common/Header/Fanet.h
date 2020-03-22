/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2 or later
 *  See CREDITS.TXT file for authors and copyrights
 *
 *  File:   Fanet.h
 *  Author: Gerald Eichler
 *
 *  Created on 13 march 2020, 14:45
 */

#ifndef _FANET_H
#define _FANET_H

#include <iterator>
struct NMEA_INFO;

//
#define MAXFANETWEATHER 10 //max. FANET-Weatherstation to display
#define MAXFANETDEVICES 50 //max. FANET-DeviceNames

// These are always used +1 for safety
#define	MAXFANETNAME	30	// used by Local Ids

struct FANET_DATA {
  double Time_Fix; //GPS-Time when we got the last msg
  uint32_t ID; //ID of station (3 Bytes)
};

//
// FANET Weatherdata
//
struct FANET_WEATHER : public FANET_DATA {
  double Latitude; //latitude
  double Longitude; //longitude
  float windDir; //wind-direction 0-360 Deg
  float windSpeed; //wind-average [m/s]
  float windGust; //windgust (2sec) [m/s]
  float temp; //temperature [°C]
  float hum; //humidity [%]
  float pressure; //pressure [hPa]
  float Battery; //charge state [%] 
  unsigned short Status; //status of station
};

//
// FANET Weatherdata
//
struct FANET_NAME : public FANET_DATA {
  TCHAR Name[MAXFANETNAME+1]; //name of station
};

/**
 * to get Name from ID, return false and empty szName if not found
 */
bool GetFanetName(uint32_t ID, const NMEA_INFO &info, TCHAR* szName, size_t size);

template<size_t size>
bool GetFanetName(uint32_t ID, const NMEA_INFO &info, TCHAR (&szName)[size]) {
  static_assert(size > (MAXFANETNAME+1), "out string too small");
  return GetFanetName(ID, info, szName, size);
}

//
// FANET Weatherdata
//
typedef struct _FANET_WEATHER
{
  double Latitude; //latitude
  double Longitude; //longitude
  float windDir; //wind-direction 0-360 Deg
  float windSpeed; //wind-average [km/h]
  float windGust; //windgust (2sec) [km/h]
  float temp; //temperature [°C]
  float hum; //humidity [%]
  float pressure; //pressure [hPa]
  float Battery; //charge state [%] 
  TCHAR Cn[MAXFANETCN]; //ID of station (3 Bytes)
  unsigned short Status; //status of station
  double Time_Fix; //GPS-Time when we got the last msg
} FANET_WEATHER;

//
// FANET Weatherdata
//
typedef struct _FANET_NAME
{
  TCHAR Name[MAXFANETNAME+1]; //name of station
  TCHAR Cn[MAXFANETCN]; //ID of station (3 Bytes)
  double Time_Fix; //GPS-Time when we got the last msg
} FANET_NAME;

#endif
