/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Info.h
 */

#ifndef _NMEA_INFO_H_
#define _NMEA_INFO_H_

#include "tchar.h"
#include "Flarm.h"
#include "Fanet.h"

struct NMEA_INFO {
    double Latitude;
    double Longitude;
    double TrackBearing;
    double Speed;
    double Altitude; // GPS Altitude

    double Time;
    int Hour;
    int Minute;
    int Second;
    int Month;
    int Day;
    int Year;
    bool NAVWarning;
    double IndicatedAirspeed;
    double TrueAirspeed;
    double BaroAltitude;
    double MacReady;
    bool BaroAltitudeAvailable;
    bool ExternalWindAvailable;
    double ExternalWindSpeed;
    double ExternalWindDirection;
    bool NettoVarioAvailable;
    bool AirspeedAvailable;

    unsigned VarioSourceIdx;
    double Vario;

    double NettoVario;
    double Ballast;
    double Bugs;
    bool AccelerationAvailable;
    double AccelX;
    double AccelY;
    double AccelZ;
    int SatellitesUsed;
    bool TemperatureAvailable;
    double OutsideAirTemperature;
    bool HumidityAvailable;
    double RelativeHumidity;

    int	ExtBatt_Bank;
    double ExtBatt1_Voltage;
    double ExtBatt2_Voltage;

    unsigned short FLARM_RX;
    unsigned short FLARM_TX;
    unsigned short FLARM_GPS;
    unsigned short FLARM_AlarmLevel;
    bool FLARM_Available;
    bool haveRMZfromFlarm;
    double FLARM_SW_Version;
    double FLARM_HW_Version;
    FLARM_TRAFFIC FLARM_Traffic[FLARM_MAX_TRAFFIC];
    FLARM_TRACE	FLARM_RingBuf[MAX_FLARM_TRACES];
    bool FLARMTRACE_bBuffFull;
    int  FLARMTRACE_iLastPtr;
    FANET_WEATHER FANET_Weather[MAXFANETWEATHER];
    FANET_NAME FanetName[MAXFANETDEVICES];

    double SupplyBatteryVoltage;

#if USESWITCHES
    SWITCH_INFO SwitchState;
#endif

    bool MagneticHeadingAvailable;
    double MagneticHeading;

    bool GyroscopeAvailable;
    double Pitch;
    double Roll;
};

#endif //_NMEA_INFO_H_
