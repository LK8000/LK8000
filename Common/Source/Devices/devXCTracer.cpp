/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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
#include <nmeaistream.h>

static const TCHAR DeviceName[] = TEXT("XCTracer");

static BOOL XCTracerTrue(PDeviceDescriptor_t) {
    return (true);
} // GetTrue()

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
static bool
ReadCheckedRange(TCHAR *String, unsigned &value_r, unsigned min, unsigned max) {
    if (_tcslen(String) == 0) {
        return false; // empty string
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
static bool
ReadCheckedRange(TCHAR *String, double &value_r, double min, double max) {
    if (_tcslen(String) == 0) {
        return false; // empty stringg
    }

    const double value = StrToDouble(String, nullptr);

    /* check range */
    if (value < min || value > max)
        return false;

    value_r = value;
    return true;
}

static bool
ReadChecked(TCHAR *String, double &value_r) {
    if (_tcslen(String) == 0) {
        return false; // empty stringg
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
static BOOL XTRC(PDeviceDescriptor_t d, TCHAR **params, size_t nparams, NMEA_INFO *pGPS) {

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

        pGPS->NAVWarning = ( (latitude == 0.0) && (longitude == 0.0) );
        pGPS->SatellitesUsed = -1;

        pGPS->Altitude = StrToDouble(params[10], nullptr); // altitude

        pGPS->Speed = StrToDouble(params[11], nullptr); // speedoverground
        if (pGPS->Speed > 1.0) {
            pGPS->TrackBearing = AngleLimit360(StrToDouble(params[12], nullptr)); // course
        }

        pGPS->Vario = StrToDouble(params[13], nullptr); // climbrate
        pGPS->VarioAvailable = TRUE;

        const double abs_press = StrToDouble(params[17], nullptr); // rawpressure
        UpdateBaroSource(pGPS, 0, d, StaticPressureToQNHAltitude(abs_press * 100));


        pGPS->ExtBatt1_Voltage = _tcstol(params[18], nullptr, 10) + 1000; // batteryindication

    } else {
        
        pGPS->NAVWarning = true;
        
    }

    d->nmeaParser.gpsValid = !pGPS->NAVWarning;
    d->nmeaParser.connected = true;

    if(!pGPS->NAVWarning) {
        TriggerGPSUpdate();
    }
    


    return TRUE;
}

bool LXWP0(PDeviceDescriptor_t d, TCHAR **params, size_t nparams, NMEA_INFO *pGPS)
{
    // $LXWP0,logger_stored, airspeed, airaltitude,
    //   v1[0],v1[1],v1[2],v1[3],v1[4],v1[5], hdg, windspeed*CS<CR><LF>
    //
    // 1 logger_stored : [Y|N] (not used XCTracer)
    // 2 IAS [km/h]
    // 3 baroaltitude [m]
    // 4-9 vario values [m/s] (only first are available in XCTracer)
    // 10 heading of plane
    // 11 windcourse [deg]
    // 12 windspeed [km/h]
    //
    // e.g.:
    // $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

    double alt=0, airspeed=0;

    if (ReadChecked(params[2], airspeed))
    {
        airspeed /= TOKPH;
        pGPS->TrueAirspeed = airspeed;
        pGPS->AirspeedAvailable = TRUE;
    }

    if (ReadChecked(params[3], alt))
    {
        if (airspeed>0) {
            LKASSERT(AirDensityRatio(alt)!=0);
            pGPS->IndicatedAirspeed = airspeed / AirDensityRatio(alt);
        }

        UpdateBaroSource(pGPS, 0,d,  QNEAltitudeToQNHAltitude(alt));
    }

    for(int i = 4; i <= 9; ++i) {
        if (ReadChecked(params[i], pGPS->Vario)) { /* take the last value to be more recent */
            pGPS->VarioAvailable = TRUE;
            TriggerVarioUpdate();
        }
    }

    if (ReadChecked(params[10], pGPS->MagneticHeading))
        pGPS->MagneticHeadingAvailable=TRUE;

    if (ReadChecked(params[11], pGPS->ExternalWindDirection) &&
            ReadChecked(params[12], pGPS->ExternalWindSpeed))
    {
        pGPS->ExternalWindSpeed /= TOKPH;  /* convert to m/s */
        pGPS->ExternalWindAvailable = TRUE;
    }

    return(true);
} // LXWP0()

static BOOL XCTracerParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO) {
    TCHAR ctemp[MAX_NMEA_LEN];
    TCHAR * params[MAX_NMEA_PARAMS];

    size_t n_params = NMEAParser::ValidateAndExtract(String, ctemp, MAX_NMEA_LEN, params, MAX_NMEA_PARAMS);
    if (n_params>0) {
        if(_tcscmp(params[0], TEXT("$XCTRC")) == 0) {
            return XTRC(d, params, n_params, _INFO);
        }
        if(_tcscmp(params[0], TEXT("$LXWP0")) == 0) {
            return LXWP0(d, params, n_params, _INFO);
        }
    }
    return FALSE;
}

static BOOL XCTracerInstall(PDeviceDescriptor_t d) {

    _tcscpy(d->Name, DeviceName);
    d->ParseNMEA = XCTracerParseNMEA;
    d->PutMacCready = NULL;
    d->PutBugs = NULL;
    d->PutBallast = NULL;
    d->Open = NULL;
    d->Close = NULL;
    d->Init = NULL;
    d->LinkTimeout = NULL;
    d->Declare = NULL;
    d->IsGPSSource = XCTracerTrue;
    d->IsBaroSource = XCTracerTrue;

    return (TRUE);
}

BOOL XCTracerRegister(void) {
    return devRegister(DeviceName, (1l << dfBaroAlt) | (1l << dfGPS) | (1l << dfVario), XCTracerInstall);
}