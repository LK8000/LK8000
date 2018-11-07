/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */
//_____________________________________________________________________includes_

#include "externs.h"
#include "devOpenVario.h"

extern bool UpdateBaroSource(NMEA_INFO* pGPS, const short parserid, const PDeviceDescriptor_t d, const double fAlt);


BOOL OpenVarioPutMacCready(PDeviceDescriptor_t d, double MacCready);
BOOL OpenVarioPutBallast(PDeviceDescriptor_t d, double Ballast);
BOOL OpenVarioPutBugs(PDeviceDescriptor_t d, double Bugs);

constexpr int OV_DebugLevel = 0;
//____________________________________________________________class_definitions_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Registers device into device subsystem.
///
/// @retval true  when device has been registered successfully
/// @retval false device cannot be registered
///
//static
bool DevOpenVario::Register() {

  return (devRegister(GetName(),
          cap_gps | cap_baro_alt | cap_speed | cap_vario, Install));
} // Register()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval TRUE  when device has been installed successfully
/// @retval FALSE device cannot be installed
///
//static
BOOL DevOpenVario::Install(PDeviceDescriptor_t d) {
  _tcscpy(d->Name, GetName());
  d->ParseNMEA = ParseNMEA;
  d->PutMacCready = OpenVarioPutMacCready;
  d->PutBugs = OpenVarioPutBugs;
  d->PutBallast = OpenVarioPutBallast;
  d->IsGPSSource = GetTrue;
  d->IsBaroSource = GetTrue;

  return TRUE;
} // Install()




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Returns device name (max length is @c DEVNAMESIZE).
///
//static
const TCHAR* DevOpenVario::GetName() {
  return (_T("OpenVario"));
} // GetName()

BOOL OpenVarioPutMacCready(PDeviceDescriptor_t d, double MacCready) {
  char szTmp[80];

  sprintf(szTmp, "$POV,C,MC,%0.2f", MacCready); // 14 + 5 + 1 char
  NMEAParser::AppendChecksum(szTmp);
  d->Com->WriteString(szTmp);

  return TRUE;
}

BOOL OpenVarioPutBallast(PDeviceDescriptor_t d, double Ballast) {
  char szTmp[80];

  sprintf(szTmp, "$POV,C,WL,%3f", (1.0 + Ballast)); // 13 + 5 + 1 char
  NMEAParser::AppendChecksum(szTmp);
  d->Com->WriteString(szTmp);

  return TRUE;
}

BOOL OpenVarioPutBugs(PDeviceDescriptor_t d, double Bugs) {
  char szTmp[80];

  sprintf(szTmp, "$POV,C,BU,%0.2f", Bugs); // 14 + 5 + 1 char
  NMEAParser::AppendChecksum(szTmp);
  d->Com->WriteString(szTmp);

  return TRUE;
}






//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses POV sentences.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval TRUE if the sentence has been parsed
///
//static
BOOL DevOpenVario::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info) {


  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)) {
    if (OV_DebugLevel > 0) {
      StartupStore(TEXT(" OpenVario Checksum Error"));
    }
    return FALSE;
  }

  if (_tcsncmp(_T("$POV"), sentence, 4) == 0) {
    return POV(d, sentence + 5, info);
  }


  return FALSE;
} // ParseNMEA()


BOOL DevOpenVario::POV(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info) {
  TCHAR szTmp1[80];


  /*
   * Type definitions:
   *
   * E: TE vario in m/s
   * P: static pressure in hPa
   * Q: dynamic pressure in Pa
   * R: total pressure in hPa
   * S: true airspeed in km/h
   * T: temperature in deg C
   */

  int FieldIndex = 0;
  do {
    NMEAParser::ExtractParameter(sentence, szTmp1, FieldIndex++);
    if (_tcslen(szTmp1) != 1) {
      break; // we are on CRC field or sentence is invalid : stop parsing
    }
    double value = 0;
    if (!ParToDouble(sentence, FieldIndex++, &value)) {
      break; // Invalid Field : stop parsing
    }

    const char& type = szTmp1[0];
    switch (type) {
      case 'E':
        info->Vario = value;
        info->VarioAvailable = TRUE;
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario Vario :%5.2fm/s"), value);
        }
        TriggerVarioUpdate();
        break;

      case 'P':
        if( value >= 0.0 && value <= 2000.0 ) {
          const double AltQNH = StaticPressureToQNHAltitude(value*100.0);
          UpdateBaroSource(info, 0, d, AltQNH);
          if (OV_DebugLevel > 0) {
            StartupStore(TEXT(" OpenVario QNE %6.1fhPa Altitude QNH :%6.1fm GPS: %6.1fm"), value, AltQNH, info->Altitude);
          }
        }
        break;

      case 'Q':
      	if ((value >= -999.0) && (value <= 9998.0)) {
          info->AirspeedAvailable = TRUE;
          info->IndicatedAirspeed = sqrt(2 * value / 1.225);
          info->TrueAirspeed = info->IndicatedAirspeed * AirDensityRatio(QNHAltitudeToQNEAltitude(info->Altitude));
          if (OV_DebugLevel > 0) {
            StartupStore(TEXT(" OpenVario Dynamic Pressure :%6.1fhPa IAS :%4.1fkm/h"), value/100.0, info->IndicatedAirspeed*3.6);
          }
        }
        break;

      case 'R':
        // 2018-03-13 : this value is never sent by sensord
        break;

      case 'S':
        // 2018-03-13 : this value is never sent by sensord
        info->TrueAirspeed = value/3.6;
        info->AirspeedAvailable = TRUE;
        {
          const double AltQNH = (info->BaroAltitudeAvailable ? info->BaroAltitude : info->Altitude);
          info->IndicatedAirspeed = info->TrueAirspeed / AirDensityRatio(QNHAltitudeToQNEAltitude(AltQNH));
        }
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario Airspeed :%5.2fkm/h"), value);
        }
        break;

      case 'T':
        info->OutsideAirTemperature = value;
        info->TemperatureAvailable = TRUE;
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario OAT :%5.2f°C"), value);
        }
        break;

      case 'V':
        info->ExtBatt1_Voltage = value;
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario Voltage :%5.2fV"), value);
        }
        break;
      default:
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario unsupported command %s :%7.2f"), szTmp1, value);
        }
        break;
    }

  } while (true); // loop is break a the begining in case of invalid input values.

  return TRUE;
} // POV()
