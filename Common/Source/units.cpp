/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: units.cpp,v 8.5 2010/12/13 17:37:35 root Exp root $
*/

//default       EU   UK   US   AUS
//altitude      m    ft   ft   m
//verticalspeed m/s  kts  kts  kts
//wind speed    km/  kts  mp   kts
//IAS           km/  kts  mp   kts
//distance      km   nm   ml   nm


#include "externs.h"
#include "Library/Utm.h"
#include "utils/printf.h"
#include "Settings/read.h"
#include "Settings/write.h"

namespace {

CoordinateFormats_t CoordinateFormat;
Units_t DistanceUnit = unKiloMeter;
Units_t AltitudeUnit = unMeter;
Units_t HorizontalSpeedUnit = unKiloMeterPerHour;
Units_t VerticalSpeedUnit = unMeterPerSecond;
Units_t WindSpeedUnit = unKiloMeterPerHour;
Units_t TaskSpeedUnit = unKiloMeterPerHour;
Units_t PressureUnit = unhPa;

const char szRegistrySpeedUnitsValue[] = "SpeedUnits";
const char szRegistryTaskSpeedUnitsValue[] = "TaskSpeedUnits";
const char szRegistryVerticalSpeedUnit[] = "LiftUnits";
const char szRegistryAltitudeUnitsValue[] = "AltitudeUnits";
const char szRegistryDistanceUnitsValue[] = "DistanceUnits";
const char szRegistryLatLonUnits[] = "LatLonUnits";
const char szRegistryPressureHg[] = "PressureHg";

} // namespace

// Units configurable in system config
int Units::SpeedUnit_Config = 2;		      // default is kmh
int Units::TaskSpeedUnit_Config = 2;	    // default is kph
int Units::DistanceUnit_Config = 2;	    // default is km
int Units::VerticalSpeedUnit_Config = 1; // default m/s
int Units::AltitudeUnit_Config = 1;	    // default m
int Units::LatLonUnits_Config = 0;       // default is  DDMMSS;
int Units::PressureUnits_Config = 0; // default is hPa

void Units::ResetSettings() {
  SpeedUnit_Config = 2;		      // default is kmh
  TaskSpeedUnit_Config = 2;	    // default is kph
  DistanceUnit_Config = 2;	    // default is km
  VerticalSpeedUnit_Config = 1; // default m/s
  AltitudeUnit_Config = 1;	    // default m
  LatLonUnits_Config = 0;       // default is  DDMMSS;
  PressureUnits_Config = 0;     // default is hPa
}

bool Units::LoadSettings(const char *key, const char *value) {
  return settings::read(key, value, szRegistrySpeedUnitsValue, SpeedUnit_Config)
      || settings::read(key, value, szRegistryTaskSpeedUnitsValue, TaskSpeedUnit_Config)
      || settings::read(key, value, szRegistryVerticalSpeedUnit, VerticalSpeedUnit_Config)
      || settings::read(key, value, szRegistryAltitudeUnitsValue, AltitudeUnit_Config)
      || settings::read(key, value, szRegistryDistanceUnitsValue, DistanceUnit_Config)
      || settings::read(key, value, szRegistryLatLonUnits, LatLonUnits_Config)
      || settings::read(key, value, szRegistryPressureHg, PressureUnits_Config);
}

void Units::SaveSettings(settings::writer& write_settings) {
  write_settings(szRegistrySpeedUnitsValue, SpeedUnit_Config);
  write_settings(szRegistryTaskSpeedUnitsValue, TaskSpeedUnit_Config);
  write_settings(szRegistryVerticalSpeedUnit, VerticalSpeedUnit_Config);
  write_settings(szRegistryAltitudeUnitsValue, AltitudeUnit_Config);
  write_settings(szRegistryDistanceUnitsValue, DistanceUnit_Config);
  write_settings(szRegistryLatLonUnits, LatLonUnits_Config);
  write_settings(szRegistryPressureHg, PressureUnits_Config);
}

void Units::LongitudeToDMS(double Longitude,
                           int *dd,
                           int *mm,
                           int *ss,
                           bool *east) {

  int sign = Longitude<0 ? 0 : 1;
  Longitude = fabs(Longitude);

  *dd = (int)Longitude;
  Longitude = (Longitude - (*dd)) * 60.0;
  *mm = (int)(Longitude);
  Longitude = (Longitude - (*mm)) * 60.0;
  *ss = (int)(Longitude + 0.5);
  if (*ss >= 60) {
    (*mm)++;
    (*ss) -= 60;
  }
  if ((*mm) >= 60) {
    (*dd)++;
    (*mm) -= 60;
  }
  *east = (sign == 1);
}


void Units::LatitudeToDMS(double Latitude,
                          int *dd,
                          int *mm,
                          int *ss,
                          bool *north) {

  int sign = Latitude<0 ? 0 : 1;
  Latitude = fabs(Latitude);

  *dd = (int)Latitude;
  Latitude = (Latitude - (*dd)) * 60.0;
  *mm = (int)(Latitude);
  Latitude = (Latitude - (*mm)) * 60.0;
  *ss = (int)(Latitude + 0.5);
  if (*ss >= 60) {
    (*mm)++;
    (*ss) -= 60;
  }
  if ((*mm) >= 60) {
    (*dd)++;
    (*mm) -= 60;
  }
  *north = (sign==1);
}

bool Units::CoordinateToString(double Longitude, double Latitude, TCHAR *Buffer, size_t size) {
	if(CoordinateFormat == cfUTM) {
		int utmzone=0;
		char utmchar=0;
		double easting=0, northing=0;
		LatLonToUtmWGS84 ( utmzone, utmchar, easting, northing, Latitude, Longitude );
		lk::snprintf(Buffer, size, _T("UTM %d%c  %.0f  %.0f"), utmzone, utmchar, easting, northing);
	} else {
		TCHAR sLongitude[16];
		TCHAR sLatitude[16];

		Units::LongitudeToString(Longitude, sLongitude);
		Units::LatitudeToString(Latitude, sLatitude);

		lk::snprintf(Buffer,size,_T("%s  %s"), sLatitude, sLongitude);
	}
	return true;
}


bool Units::LongitudeToString(double Longitude, TCHAR *Buffer, size_t size){

  TCHAR EW[] = _T("WE");
  int dd, mm, ss;

  int sign = Longitude<0 ? 0 : 1;
  Longitude = fabs(Longitude);

  switch(CoordinateFormat){
    case cfDDMMSS:
      dd = (int)Longitude;
      Longitude = (Longitude - dd) * 60.0;
      mm = (int)(Longitude);
      Longitude = (Longitude - mm) * 60.0;
      ss = (int)(Longitude + 0.5);
      if (ss >= 60) {
        mm++;
        ss -= 60;
      }
      if (mm >= 60) {
        dd++;
        mm -= 60;
      }
      lk::snprintf(Buffer, size, _T("%c%03d°%02d'%02d\""), EW[sign], dd, mm, ss);
      break;
    case cfDDMMSSss:
      dd = (int)Longitude;
      Longitude = (Longitude - dd) * 60.0;
      mm = (int)(Longitude);
      Longitude = (Longitude - mm) * 60.0;
      lk::snprintf(Buffer, size, _T("%c%03d°%02d'%05.2f\""), EW[sign], dd, mm, Longitude);
    break;
    case cfDDMMmmm:
      dd = (int)Longitude;
      Longitude = (Longitude - dd) * 60.0;
      lk::snprintf(Buffer, size, _T("%c%03d°%06.3f'"), EW[sign], dd, Longitude);
    break;
    case cfDDdddd:
      lk::snprintf(Buffer, size, _T("%c%08.4f°"), EW[sign], Longitude);
    break;
    case cfUTM:
      lk::strcpy(Buffer, _T(""), size);
      break;
    default:
      assert(false);
      break;
  }

  return true;

}


bool Units::LatitudeToString(double Latitude, TCHAR *Buffer, size_t size){
  TCHAR EW[] = _T("SN");
  int dd, mm, ss;

  int sign = Latitude<0 ? 0 : 1;
  Latitude = fabs(Latitude);

  switch(CoordinateFormat){
    case cfDDMMSS:
      dd = (int)Latitude;
      Latitude = (Latitude - dd) * 60.0;
      mm = (int)(Latitude);
      Latitude = (Latitude - mm) * 60.0;
      ss = (int)(Latitude + 0.5);
      if (ss >= 60) {
        mm++;
        ss -= 60;
      }
      if (mm >= 60) {
        dd++;
        mm -= 60;
      }
      lk::snprintf(Buffer, size, _T("%c%02d°%02d'%02d\""), EW[sign], dd, mm, ss);
    break;
    case cfDDMMSSss:
      dd = (int)Latitude;
      Latitude = (Latitude - dd) * 60.0;
      mm = (int)(Latitude);
      Latitude = (Latitude - mm) * 60.0;
      lk::snprintf(Buffer, size, _T("%c%02d°%02d'%05.2f\""), EW[sign], dd, mm, Latitude);
    break;
    case cfDDMMmmm:
      dd = (int)Latitude;
      Latitude = (Latitude - dd) * 60.0;
      lk::snprintf(Buffer, size, _T("%c%02d°%06.3f'"), EW[sign], dd, Latitude);
    break;
    case cfDDdddd:
      lk::snprintf(Buffer, size, _T("%c%07.4f°"), EW[sign], Latitude);
    break;
    case cfUTM:
      _tcscpy(Buffer,_T(""));
    break;
    default:
      assert(false);
    break;
  }

  return true;

}

const TCHAR *Units::GetName(Units_t Unit) {
  // JMW adjusted this because units are pretty standard internationally
  // so don't need different names in different languages.
  return impl::UnitDescriptors[Unit].Name;
}

CoordinateFormats_t Units::GetCoordinateFormat() {
  return CoordinateFormat;
}

Units_t Units::GetDistanceUnit() {
  return DistanceUnit;
}

Units_t Units::GetAltitudeUnit() {
  return AltitudeUnit;
}

Units_t Units::GetAlternateAltitudeUnit() { // 100126
  return AltitudeUnit==unFeet?unMeter:unFeet;
}

Units_t Units::GetHorizontalSpeedUnit() {
  return HorizontalSpeedUnit;
}

Units_t Units::GetTaskSpeedUnit() {
  return TaskSpeedUnit;
}

Units_t Units::GetVerticalSpeedUnit() {
  return VerticalSpeedUnit;
}

Units_t Units::GetWindSpeedUnit() {
  return WindSpeedUnit;
}

Units_t Units::GetPressureUnit() {
  return PressureUnit;
}

void Units::NotifyUnitChanged() {
  switch (SpeedUnit_Config) {
    case 0 :
      HorizontalSpeedUnit = unStatuteMilesPerHour;
      WindSpeedUnit = unStatuteMilesPerHour;
      break;
    case 1 :
      HorizontalSpeedUnit = unKnots;
      WindSpeedUnit = unKnots;
      break;
    case 2 :
    default:
      HorizontalSpeedUnit = unKiloMeterPerHour;
      WindSpeedUnit = unKiloMeterPerHour;
      break;
  }

  switch(DistanceUnit_Config) {
    case 0 :
      DistanceUnit = unStatuteMiles;
      break;
    case 1 :
      DistanceUnit = unNauticalMiles;
      break;
    default:
    case 2 :
      DistanceUnit = unKiloMeter;
      break;
  }

  switch(AltitudeUnit_Config) {
    case 0 :
      AltitudeUnit = unFeet;
      break;
    default:
    case 1 :
      AltitudeUnit = unMeter;
      break;
  }

  switch(VerticalSpeedUnit_Config) {
    case 0 :
      VerticalSpeedUnit = unKnots;
      break;
    default:
    case 1 :
      VerticalSpeedUnit = unMeterPerSecond;
      break;
    case 2 :
      VerticalSpeedUnit = unFeetPerMinutes;
      break;
  }

  switch(TaskSpeedUnit_Config) {
    case 0 :
      TaskSpeedUnit = unStatuteMilesPerHour;
      break;
    case 1 :
      TaskSpeedUnit = unKnots;
      break;
    case 2 :
    default:
      TaskSpeedUnit = unKiloMeterPerHour;
      break;
  }
  
  switch(LatLonUnits_Config) {
    case 0:
    default:
      CoordinateFormat = cfDDMMSS;
      break;
    case 1:
      CoordinateFormat = cfDDMMSSss;
      break;
    case 2:
      CoordinateFormat = cfDDMMmmm;
      break;
    case 3:
      CoordinateFormat = cfDDdddd;
      break;
    case 4:
      CoordinateFormat = cfUTM;
      break;
  }

  switch(PressureUnits_Config) {
    case 0:
    default:
      PressureUnit = unhPa;
      break;
    case 1:
      PressureUnit = unInHg;
      break;
  }
}

void Units::FormatAltitude(double Altitude, TCHAR *Buffer, size_t size){
  lk::snprintf(Buffer, size, _T("%.0f%s"), ToAltitude(Altitude), GetAltitudeName());
}

void Units::FormatAlternateAltitude(double Altitude, TCHAR *Buffer, size_t size){
  lk::snprintf(Buffer, size, _T("%.0f%s"), ToAlternateAltitude(Altitude), GetAlternateAltitudeName());
}

void Units::FormatArrival(double Altitude, TCHAR *Buffer, size_t size){
  FormatAltitude(Altitude, Buffer, size);
}

void Units::FormatDistance(double Distance, TCHAR *Buffer, size_t size) {
  int prec = 0;

  Units_t UnitIdx = GetDistanceUnit();
  double value = To(UnitIdx, Distance);
  if (value >= 100) {
    prec = 0;
  } else if (value > 10) {
    prec = 1;
  } else if (value > 1) {
    prec = 2;
  } else {
    prec = 3;
    if (UnitIdx == unKiloMeter) {
      prec = 0;
      UnitIdx = unMeter;
      value = To(UnitIdx, Distance);
    }
    if (UnitIdx == unNauticalMiles
            || UnitIdx == unStatuteMiles) {

      const double ftValue = To(unFeet, Distance);  
      if (value < 1000) {
        prec = 0;
        UnitIdx = unFeet;
        value = ftValue;
      } else {
        prec = 1;
      }
    }
  }

  lk::snprintf(Buffer, size, _T("%.*f%s"), prec, value, GetName(UnitIdx));
}

void Units::FormatMapScale(double Distance, TCHAR *Buffer, size_t size){

  int prec = 0;
  Units_t UnitIdx = GetDistanceUnit();

  double value = To(UnitIdx, Distance);

  if (value >= 9.999) {
    prec = 0;
  } else if (value >= 0.999) {
    prec = 1;
  } else {
    if (UnitIdx == unKiloMeter){
      prec = 0;
      UnitIdx = unMeter;
      value = To(UnitIdx, Distance);
    } else if ((UnitIdx == unNauticalMiles || UnitIdx == unStatuteMiles) && (value < 0.160)) {
      prec = 0;
      UnitIdx = unFeet;
      value = To(UnitIdx, Distance);
    } else {
      prec = 2;
    }
  }

  lk::snprintf(Buffer, size, _T("%.*f%s"), prec, value, GetName(UnitIdx));
}

void Units::TimeToText(TCHAR* text, size_t cb, int d) {
  bool negative = (d < 0);
  int dd = abs(d) % (3600 * 24);
  int hours = (dd / 3600);
  int mins = (dd / 60 - hours * 60);
  hours = hours % 24;

  lk::snprintf(text, cb, _T("%s%02d:%02d"), (negative ? _T("-") : _T("")), hours, mins);
}

void Units::TimeToTextSimple(TCHAR* text, size_t cb, int d) {
  bool negative = (d < 0);
  int dd = abs(d) % (3600 * 24);
  int hours = (dd / 3600);
  int mins = (dd / 60 - hours * 60);
  hours = hours % 24;

  lk::snprintf(text, cb, _T("%s%02d%02d"), (negative ? _T("-") : _T("")), hours, mins);
}

// Not for displaying a clock time, good for a countdown
// will display either
// Returns true if hours, false if minutes
bool Units::TimeToTextDown(TCHAR* text, size_t cb, int d) {
  bool negative = (d < 0);
  int dd = abs(d) % (3600 * 24);
  int hours = (dd / 3600);
  int mins = (dd / 60 - hours * 60);
  hours = hours % 24;
  int seconds = (dd - mins * 60 - hours * 3600);

  if (hours == 0) {
    lk::snprintf(text, cb, _T("%s%02d:%02d"), (negative ? _T("-") : _T("")), mins, seconds);
    return false;
  } else {
    lk::snprintf(text, cb, _T("%s%02d:%02d"), (negative ? _T("-") : _T("")), hours, mins);
    return true;
  }
}

// LK8000
void Units::TimeToTextS(TCHAR* text, size_t cb, int d) {
  bool negative = (d < 0);
  int dd = abs(d) % (3600 * 24);
  int hours = (dd / 3600);
  int mins = (dd / 60 - hours * 60);
  int seconds = (dd - mins * 60 - hours * 3600);
  hours = hours % 24;

  lk::snprintf(text, cb, _T("%s%d:%02d:%02d"), (negative ? _T("-") : _T("")), hours, mins, seconds);
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("Units") {

  #define CHECK_APPROX_EQ(x, y) CHECK_EQ(x, doctest::Approx(y))

  SUBCASE("Length") {
    CHECK_EQ(Units::From(unFeet, 1), Units::From(unMeter, 0.3048));
    CHECK_EQ(Units::From(unNauticalMiles, 1), Units::From(unMeter, 1852));
    CHECK_EQ(Units::From(unStatuteMiles, 1), Units::From(unMeter, 1609.344));
    CHECK_EQ(Units::From(unKiloMeter, 1), Units::From(unMeter, 1000));
    CHECK_APPROX_EQ(Units::From(unFligthLevel, 125), Units::From(unFeet, 12500));
  }

  SUBCASE("Speed") {
    CHECK_EQ(Units::From(unKiloMeterPerHour, 3.6), Units::From(unMeterPerSecond, 1));
    CHECK_EQ(Units::From(unKnots, 1), Units::From(unKiloMeterPerHour, 1.852));
    CHECK_APPROX_EQ(Units::From(unStatuteMilesPerHour, 1), Units::From(unMeterPerSecond, 0.44704));
    CHECK_EQ(Units::From(unFeetPerMinutes, 1), Units::From(unMeterPerSecond, 0.00508));
    CHECK_EQ(Units::From(unFeetPerSecond, 1), Units::From(unMeterPerSecond, 0.3048));
    CHECK_EQ(Units::From(unCentimeterPersecond, 100), Units::From(unMeterPerSecond, 1));
    CHECK_EQ(Units::From(unDecimeterPersecond, 10), Units::From(unMeterPerSecond, 1));
  }

  SUBCASE("Temp") {
    CHECK_APPROX_EQ(Units::From(unGradFahrenheit, 0), Units::From(unKelvin, 255.37222222));
    CHECK_APPROX_EQ(Units::From(unGradFahrenheit, 10), Units::From(unKelvin, 260.92777778));
    CHECK_EQ(Units::From(unGradCelcius, 1), Units::From(unKelvin, 274.15));
  }

  SUBCASE("Acceleration") {
    CHECK_APPROX_EQ(Units::From(unG, 1), Units::From(unMeterSquareSecond, 9.80665));
  }

  #undef CHECK_APPROX_EQ
}
#endif
