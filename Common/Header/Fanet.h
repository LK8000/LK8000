/*
 *  LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 *  Released under GNU/GPL License v.2
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

//
#define MAXFANETWEATHER 10 //max. FANET-Weatherstation to display
#define MAXFANETDEVICES 50 //max. FANET-DeviceNames

// These are always used +1 for safety
#define	MAXFANETNAME	30	// used by Local Ids
#define MAXFANETCN	3	// used by Local and Flarmnet, not changeble see Parser

typedef TCHAR Cn_t[MAXFANETCN];

inline
bool equals_Cn(const Cn_t& a, const Cn_t& b) {
  return std::equal(std::begin(a), std::end(a), std::begin(b));
}

struct FANET_DATA {
  double Time_Fix; //GPS-Time when we got the last msg
  Cn_t Cn; //ID of station (3 Bytes)
};

//
// FANET Weatherdata
//
struct FANET_WEATHER : public FANET_DATA {
  double Latitude; //latitude
  double Longitude; //longitude
  float windDir; //wind-direction 0-360 Deg
  float windSpeed; //wind-average [km/h]
  float windGust; //windgust (2sec) [km/h]
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

#endif
