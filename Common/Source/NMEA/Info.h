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
 * Priority index for the Baro Altitude source.
 *
 * Priority rules (highest to lowest):
 *   1. Flarm device, ordered by port index (lowest port number = highest priority).
 *   2. Any other external device, ordered by port index.
 *   3. Invalid/unset state: device_index == NUMDEV (no source active).
 *
 * A new source is accepted only if its priority is greater than or equal to
 * the current source (operator>=). Equal priority allows the same device to
 * keep refreshing its value.
 *
 * Note: some legacy WinCE devices report a baro sensor without a device
 * config entry; they are represented with device_index >= NUMDEV and therefore
 * always have the lowest possible priority.
 */
class BaroIndex {
public:
    BaroIndex() = default;

    BaroIndex& operator = (const DeviceDescriptor_t& d) {
        device_index = d.PortNumber;
        is_flarm = d.nmeaParser.isFlarm;
        return *this;
    }

    bool operator >= (const DeviceDescriptor_t& d) const {
        if (is_flarm == d.nmeaParser.isFlarm) {
            return device_index >= d.PortNumber;
        }
        return d.nmeaParser.isFlarm;
    }

    bool operator == (const BaroIndex& idx) const {
        return (is_flarm == idx.is_flarm) && (device_index == idx.device_index);
    }

    bool operator != (const BaroIndex& idx) const {
        return (is_flarm != idx.is_flarm) || (device_index != idx.device_index);
    }

    bool operator == (unsigned idx) const {
        return device_index == idx;
    }

    bool operator != (unsigned idx) const {
        return device_index != idx;
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
private:
    unsigned device_index = NUMDEV;
    bool is_flarm = false;    
};

struct WindData {
    double Speed = {};
    double Direction = {};
};

struct GyroscopeData {
    double Pitch = {};
    double Roll = {};
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
    from_device_data<double> IndicatedAirSpeed = {};
    from_device_data<double> TrueAirSpeed = {};
    from_device_data<GyroscopeData> Gyroscope = {};

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

    void reset_availability(std::optional<unsigned> idx = {});
};

static_assert(std::is_copy_constructible_v<NMEA_INFO>, "mandatory...");

inline
AGeoPoint GetCurrentPosition(const NMEA_INFO& Info) {
  return {{ Info.Latitude, Info.Longitude }, Info.Altitude };
}


#endif //_NMEA_INFO_H_
