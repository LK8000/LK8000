
#ifndef _FANET_H
#define _FANET_H

//
#define MAXFANETWEATHER 10 //max. FANET-Weatherstation to display
#define MAXFANETDEVICES 50 //max. FANET-DeviceNames

// These are always used +1 for safety
#define	MAXFANETNAME	30	// used by Local Ids
#define MAXFANETCN	3	// used by Local and Flarmnet, not changeble see Parser

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
