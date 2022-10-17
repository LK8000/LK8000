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
#include <type_traits>
/**
 * used to manage Baro Altitude Source priority
 *   - Flarm device First ordered by port index
 *   - other external device after order by port index too.
 *   - some old WinCE device handel baro sensor withiut device config, 
 *     this have the lowest prriority any time (index greater than DeviceList size).
 */
struct BaroIndex {

    bool is_flarm;
    unsigned device_index;

    bool operator <= (BaroIndex& idx) const {
        if (is_flarm == idx.is_flarm) {
            return device_index <= idx.device_index;
        }
        return is_flarm;
    }
};

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
    double MacReady;
    bool ExternalWindAvailable;
    double ExternalWindSpeed;
    double ExternalWindDirection;
    bool NettoVarioAvailable;
    bool AirspeedAvailable;

    BaroIndex BaroSourceIdx;
    double BaroAltitude;

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

    double FLARM_SW_Version;
    double FLARM_HW_Version;
    FLARM_TRAFFIC FLARM_Traffic[FLARM_MAX_TRAFFIC];
    FLARM_TRACE	FLARM_RingBuf[MAX_FLARM_TRACES];
    bool FLARMTRACE_bBuffFull;
    int  FLARMTRACE_iLastPtr;
    FANET_WEATHER FANET_Weather[MAXFANETWEATHER];
    FANET_NAME FanetName[MAXFANETDEVICES];

    double SupplyBatteryVoltage;

    bool MagneticHeadingAvailable;
    double MagneticHeading;

    bool GyroscopeAvailable;
    double Pitch;
    double Roll;
};

static_assert(std::is_trivial_v<NMEA_INFO>, "mandatory while memset/memcpy is used to init/copy this struct");

#endif //_NMEA_INFO_H_
