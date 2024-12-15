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
#include "Devices/Fanet/Fanet.h"
#include "Geographic/GeoPoint.h"
#include "Math/Point3D.h"

struct DeviceDescriptor_t;

#define	NUMDEV		 6

/**
 * used to manage Baro Altitude Source priority
 *   - Flarm device First ordered by port index
 *   - other external device after order by port index too.
 *   - some old WinCE device handel baro sensor withiut device config, 
 *     this have the lowest prriority any time (index greater than DeviceList size).
 */
struct BaroIndex {

    bool is_flarm = false;
    unsigned device_index = NUMDEV;

    bool operator <= (BaroIndex& idx) const {
        if (is_flarm == idx.is_flarm) {
            return device_index <= idx.device_index;
        }
        return is_flarm;
    }
};

struct NMEA_INFO final {
    NMEA_INFO() = default;

    double Latitude = {};
    double Longitude = {};
    double TrackBearing = {};
    double Speed = {};
    double Altitude = {}; // GPS Altitude

    double Time = {};
    int Hour = {};
    int Minute = {};
    int Second = {};
    int Month = {};
    int Day = {};
    int Year = {};
    bool NAVWarning = {};
    double IndicatedAirspeed = {};
    double TrueAirspeed = {};

    unsigned ExternalWindIdx = {};
    double ExternalWindSpeed = {};
    double ExternalWindDirection = {};

    bool NettoVarioAvailable = {};
    bool AirspeedAvailable = {};

    BaroIndex BaroSourceIdx = {};
    double BaroAltitude = {};

    unsigned VarioSourceIdx = NUMDEV;
    double Vario = {};

    double NettoVario = {};

    unsigned AccelerationIdx = NUMDEV;
    std::vector<Point3D> Acceleration = {}; // in G

    int SatellitesUsed = {};
    bool TemperatureAvailable = {};
    double OutsideAirTemperature = {};
    bool HumidityAvailable = {};
    double RelativeHumidity = {};

    int	ExtBatt_Bank = {};
    double ExtBatt1_Voltage = {};
    double ExtBatt2_Voltage = {};

    unsigned short FLARM_RX = {};
    unsigned short FLARM_TX = {};
    unsigned short FLARM_GPS = {};
    unsigned short FLARM_AlarmLevel = {};
    bool FLARM_Available = {};

    double FLARM_SW_Version = {};
    double FLARM_HW_Version = {};
    FLARM_TRAFFIC FLARM_Traffic[FLARM_MAX_TRAFFIC] = {};
    FLARM_TRACE	FLARM_RingBuf[MAX_FLARM_TRACES] = {};
    bool FLARMTRACE_bBuffFull = {};
    int  FLARMTRACE_iLastPtr = {};
    FANET_WEATHER FANET_Weather[MAXFANETWEATHER] = {};
    FANET_NAME FanetName[MAXFANETDEVICES] = {};

    double SupplyBatteryVoltage = {};

    bool MagneticHeadingAvailable = {};
    double MagneticHeading = {};

    bool GyroscopeAvailable = {};
    double Pitch = {};
    double Roll = {};

    unsigned HeartRateIdx = NUMDEV;
    unsigned HeartRate = {};
};

static_assert(std::is_copy_constructible_v<NMEA_INFO>, "mandatory...");

inline
AGeoPoint GetCurrentPosition(const NMEA_INFO& Info) {
  return {{ Info.Latitude, Info.Longitude }, Info.Altitude };
}

void ResetHeartRateAvailable(NMEA_INFO& info);
bool HeartRateAvailable(const NMEA_INFO& info);
void UpdateHeartRate(NMEA_INFO& info, const DeviceDescriptor_t& d, unsigned bpm);

void ResetAccelerationAvailable(NMEA_INFO& info);
bool AccelerationAvailable(const NMEA_INFO& info);


#endif //_NMEA_INFO_H_
