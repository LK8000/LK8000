/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   devXCTracer.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on June 1, 2016, 9:44 PM
 */

/*
 * Based on XCSoar Driver
 */

#include <externs.h>
#include "Baro.h"
#include "Calc/Vario.h"
#include "nmeaistream.h"
#include "Comm/ExternalWind.h"
#include "devLK8EX1.h"
#include "devGeneric.h"

namespace {
/**
 * Helper functions to parse and check an input field
 * Should these be added as methods to Class CSVLine ?
 * e.g. bool CSVLine::ReadCheckedRange(unsigned &value_r, unsigned min, unsigned max)
 *
 * @param line Input line
 * @param value_r Return parsed and checked value
 * @param min Minimum value to be accepted
 * @param max Maximum value to be accepted
 * @return true if parsed OK and within range
 */
bool ReadCheckedRange(const char *String, unsigned &value_r, unsigned min, unsigned max) {
    if (!String || strlen(String) == 0) {
      return false; // empty or empty string
    }

    const double value = StrToDouble(String, nullptr);

    /* check min/max as floating point to catch input values out of unsigned range */
    if (value < min || value > max) {
        return false;
    }

    /* finally we can cast/convert w/o being out of range */
    value_r = (unsigned) value;
    return true;
}

/* same helper as above with params of type double */
bool ReadCheckedRange(const char *String, double &value_r, double min, double max) {
    if (!String || strlen(String) == 0) {
      return false; // empty or empty string
    }

    const double value = StrToDouble(String, nullptr);

    /* check range */
    if (value < min || value > max)
        return false;

    value_r = value;
    return true;
}

bool ReadChecked(const char *String, double &value_r) {
    if (!String || strlen(String) == 0) {
        return false; // empty or empty string
    }
    value_r = StrToDouble(String, nullptr);

    return true;
}

/*
 * Native XTRC sentences
 * $XCTRC,2015,1,5,16,34,33,36,46.947508,7.453117,540.32,12.35,270.4,2.78,,,,964.93,98*67
 *
 * $XCTRC,year,month,day,hour,minute,second,centisecond,latitude,longitude,altitude,speedoverground,
 *      course,climbrate,res,res,res,rawpressure,batteryindication*checksum
 */
BOOL XTRC(DeviceDescriptor_t* d, char **params, size_t nparams, NMEA_INFO *pGPS) {

    if (nparams < 19) {
        return FALSE;
    }

    /* count the number of valid fields. If not as expected incr nmea error counter */
    int valid_fields = 0;

    unsigned year=0, month=0, day=0;
    valid_fields += ReadCheckedRange(params[1], year, 1800, 2500);
    valid_fields += ReadCheckedRange(params[2], month, 1, 12);
    valid_fields += ReadCheckedRange(params[3], day, 1, 31);

    unsigned hour=0, minute=0, second=0, centisecond=0;
    valid_fields += ReadCheckedRange(params[4], hour, 0, 23);
    valid_fields += ReadCheckedRange(params[5], minute, 0, 59);
    valid_fields += ReadCheckedRange(params[6], second, 0, 59);
    valid_fields += ReadCheckedRange(params[7], centisecond, 0, 99);

    double latitude=0, longitude=0;
    valid_fields += ReadCheckedRange(params[8], latitude, -90.0, 90.0);
    valid_fields += ReadCheckedRange(params[9], longitude, -180.0, 180.0);

    if (valid_fields == 3 + 4 + 2) {

        d->nmeaParser.gpsValid = ((latitude != 0.0) || (longitude != 0.0));

        if (d->nmeaParser.activeGPS) {
            pGPS->Year = year;
            pGPS->Month = month;
            pGPS->Day = day;

            pGPS->Hour = hour;
            pGPS->Minute = minute;
            pGPS->Second = second;

            static int startday = -1;
            pGPS->Time = TimeModify(pGPS, startday) + centisecond / 100.0;
            // TODO : check if TimeHasAdvanced check is needed (cf. Parser.cpp)

            pGPS->Latitude = latitude;
            pGPS->Longitude = longitude;

            pGPS->NAVWarning = !d->nmeaParser.gpsValid;
            pGPS->SatellitesUsed = -1;

            pGPS->Altitude = StrToDouble(params[10], nullptr);  // altitude

            pGPS->Speed = StrToDouble(params[11], nullptr);  // speedoverground
            if (pGPS->Speed > 1.0) {
                pGPS->TrackBearing = AngleLimit360(StrToDouble(params[12], nullptr));  // course
            }
        }

        double Vario = StrToDouble(params[13], nullptr); // climbrate
        UpdateVarioSource(*pGPS, *d, Vario);

        const double abs_press = StrToDouble(params[17], nullptr); // rawpressure
        UpdateBaroSource(pGPS, d, StaticPressureToQNHAltitude(abs_press * 100));

        // battery indication
        if (d->OnBatteryLevel) {
          d->OnBatteryLevel(*d, *pGPS, strtol(params[18], nullptr, 10));
        }
    }

    d->nmeaParser.connected = true;
    if (d->nmeaParser.gpsValid) {
        d->nmeaParser.lastGpsValid.Update();
        if (d->nmeaParser.activeGPS) {
            TriggerGPSUpdate();
        }
    }

    return TRUE;
}

bool LXWP0(DeviceDescriptor_t* d, char **params, size_t nparams, NMEA_INFO *pGPS) {
    // $LXWP0,logger_stored, airspeed, airaltitude,
    //   v1[0],v1[1],v1[2],v1[3],v1[4],v1[5], hdg, windspeed*CS<CR><LF>
    //
    // 1 logger_stored : [Y|N] (not used XCTracer)
    // 2 TAS [km/h]
    // 3 baroaltitude [m]
    // 4-9 vario values [m/s] (only first are available in XCTracer)
    // 10 heading of plane
    // 11 windcourse [deg]
    // 12 windspeed [km/h]
    //
    // e.g.:
    // $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

    if (nparams < 12) {
        return FALSE;
    }

    double QneAltitude = 0;
    if (ReadChecked(params[3], QneAltitude)) {
      UpdateBaroSource(pGPS, d, QNEAltitudeToQNHAltitude(QneAltitude));
    }
    else {
      QneAltitude = QNHAltitudeToQNEAltitude(pGPS->Altitude);
    }

    double TrueAirSpeed = 0;
    if (ReadChecked(params[2], TrueAirSpeed)) {
        TrueAirSpeed = Units::From(unKiloMeterPerHour, TrueAirSpeed);
        pGPS->TrueAirspeed = TrueAirSpeed;
        pGPS->IndicatedAirspeed = IndicatedAirSpeed(TrueAirSpeed, QneAltitude);
        pGPS->AirspeedAvailable = TRUE;
    }

    for(int i = 4; i <= 9; ++i) {
        double Vario = 0;
        if (ReadChecked(params[i], Vario)) { /* take the last value to be more recent */
            UpdateVarioSource(*pGPS, *d, Vario);
        }
    }

    if (ReadChecked(params[10], pGPS->MagneticHeading))
        pGPS->MagneticHeadingAvailable=TRUE;

    double wind_speed, wind_dir;
    if (ReadChecked(params[11], wind_dir) && ReadChecked(params[12], wind_speed)) {
        UpdateExternalWind(*pGPS, *d, Units::From(unKiloMeterPerHour, wind_speed), wind_dir + 180);
    }

    return true;
} // LXWP0()

BOOL XCTracerParseNMEA(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS) {
    using namespace std::string_view_literals;

    char ctemp[MAX_NMEA_LEN];
    char * params[MAX_NMEA_PARAMS];

    size_t n_params = NMEAParser::ValidateAndExtract(String, ctemp, params);
    if (n_params > 0) {
        if(params[0] == "$XCTRC"sv) {
            return XTRC(d, params, n_params, pGPS);
        }
        if(params[0] == "$LXWP0"sv) {
            return LXWP0(d, params, n_params, pGPS);
        }
        if(LK8EX1ParseNMEA(d, String, pGPS)) {
          return TRUE;
        }
    }
    return FALSE;
}

}  // namespace

void XCTracerInstall(DeviceDescriptor_t* d) {
    genInstall(d);
    d->ParseNMEA = XCTracerParseNMEA;
}
