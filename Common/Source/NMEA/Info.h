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
#include "from_device_data.h"


/**
 * used to manage Baro Altitude Source priority
 *   - Flarm device First ordered by port index
 *   - other external device after order by port index too.
 *   - some old WinCE device handel baro sensor withiut device config, 
 *     this have the lowest prriority any time (index greater than DeviceList size).
 */
struct BaroIndex {
    BaroIndex() = default;

    explicit BaroIndex(unsigned index) : device_index(index) {}

    BaroIndex& operator=(const DeviceDescriptor_t& d) {
        device_index = d.PortNumber;
        is_flarm = d.nmeaParser.isFlarm;
        return *this;
    }

    bool operator>=(const DeviceDescriptor_t& d) const {
        if (is_flarm == d.nmeaParser.isFlarm) {
            return device_index >= d.PortNumber;
        }
        return is_flarm;
    }

    bool is_flarm = false;
    unsigned device_index = NUMDEV;

    bool operator <= (BaroIndex& idx) const {
        if (is_flarm == idx.is_flarm) {
            return device_index <= idx.device_index;
        }
        return is_flarm;
    }

    bool operator != (BaroIndex& idx) const {
        return (is_flarm != idx.is_flarm) && (device_index != idx.device_index);
    }

    bool valid() const {
        return device_index < NUMDEV;
    }

    void reset() {
        device_index = NUMDEV;
        is_flarm = false;
    }

    operator unsigned () const {
        return device_index;
    }
};

struct WindData {
    double Speed = {};
    double Direction = {};
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

    bool AirspeedAvailable = {};

    int SatellitesUsed = {};

    from_device_data<double, BaroIndex> BaroAltitude = {};
    from_device_data<double> Vario = {};
    from_device_data<double> NettoVario = {};
    from_device_data<double> OutsideAirTemperature = {};
    from_device_data<double> RelativeHumidity = {};
    from_device_data<double> Gload = {};
    from_device_data<unsigned> HeartRate = {};
    from_device_data<double> MagneticHeading = {};
    from_device_data<WindData> ExternalWind = {};
    from_device_data<Point3D> Acceleration = {}; // in G

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

    bool GyroscopeAvailable = {};
    double Pitch = {};
    double Roll = {};

    void reset_availability(std::optional<unsigned> idx = {});
};

static_assert(std::is_copy_constructible_v<NMEA_INFO>, "mandatory...");

inline
AGeoPoint GetCurrentPosition(const NMEA_INFO& Info) {
  return {{ Info.Latitude, Info.Longitude }, Info.Altitude };
}


#endif //_NMEA_INFO_H_
