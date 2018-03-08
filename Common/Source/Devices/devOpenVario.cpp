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


int OpenVarioNMEAddCheckSumStrg(TCHAR szStrg[]);
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
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
BOOL DevOpenVario::Install(PDeviceDescriptor_t d) {
  _tcscpy(d->Name, GetName());
  d->ParseNMEA = ParseNMEA;
  d->PutMacCready = OpenVarioPutMacCready;
  d->PutBugs = OpenVarioPutBugs; // removed to prevent cirvular updates
  d->PutBallast = OpenVarioPutBallast;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = GetTrue;
  d->Declare = NULL;
  d->IsLogger = NULL;
  d->IsGPSSource = GetTrue;
  d->IsBaroSource = GetTrue;
  d->DirectLink = NULL;



  return (true);
} // Install()




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Returns device name (max length is @c DEVNAMESIZE).
///
//static
const TCHAR* DevOpenVario::GetName() {
  return (_T("OpenVario"));
} // GetName()

int OpenVarioNMEAddCheckSumStrg(TCHAR szStrg[]) {
  int i, iCheckSum = 0;
  TCHAR szCheck[254];

  if (szStrg[0] != '$')
    return -1;

  iCheckSum = szStrg[1];
  for (i = 2; i < (int) _tcslen(szStrg); i++) {
    //  if(szStrgi0] != ' ')
    iCheckSum ^= szStrg[i];
  }
  _stprintf(szCheck, TEXT("*%02X\r\n"), iCheckSum);
  _tcscat(szStrg, szCheck);
  return iCheckSum;
}

BOOL OpenVarioPutMacCready(PDeviceDescriptor_t d, double MacCready) {
  TCHAR szTmp[254];

  _stprintf(szTmp, TEXT("$POV,C,MC,%0.2f"), (double) MacCready);
  OpenVarioNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  return true;

}

BOOL OpenVarioPutBallast(PDeviceDescriptor_t d, double Ballast) {
  TCHAR szTmp[254];

  _stprintf(szTmp, TEXT("$POV,C,WL,%3f"), (1.0 + Ballast));
  OpenVarioNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  return (TRUE);

}

BOOL OpenVarioPutBugs(PDeviceDescriptor_t d, double Bugs) {
  TCHAR szTmp[254];

  _stprintf(szTmp, TEXT("$POV,C,BU,%0.2f"), Bugs);
  OpenVarioNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  return (TRUE);

}






//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses POV sentences.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
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
    if (OV_DebugLevel > 0){
      StartupStore(TEXT(" OpenVario POV"));
    }
    return true;
  }


  return (false);
} // ParseNMEA()


// altitude= 44330* ( (1-(p/p0)^(1/5.255) )
double StaticPressureToAltitude(double ps) {

  const double k1 = 0.190263; // 1/5.255
  return 44330.0 * (1.0 - pow(ps / PRESSURE_STANDARD, k1));
}

bool DevOpenVario::POV(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info) {
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

  NMEAParser::ExtractParameter(sentence, szTmp1, 0);
  char type = szTmp1[0];

  double value = 0;

  switch (type) {
    case 'E':
      if (ParToDouble(sentence, 1, &value)) {
        info->Vario = value;
        info->VarioAvailable = TRUE;
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario Vario :%5.2fm/s"), value);
        }
        TriggerVarioUpdate();
      }
      break;

    case 'P':
      if (ParToDouble(sentence, 1, &value)) {
        info->BaroAltitude = StaticPressureToAltitude(value);
        info->BaroAltitudeAvailable = true;
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario QNH %6.1fhPa Altitude :%6.1fm GPS: %6.1fm"), value, info->BaroAltitude, info->Altitude);
        }
        UpdateBaroSource(info, 0, d, info->BaroAltitude);

      }
      break;
    case 'Q':
      break;

    case 'R':
      break;

    case 'S':
      if (ParToDouble(sentence, 1, &value)) {
        info->IndicatedAirspeed = value;
        info->AirspeedAvailable = TRUE;
        if (value > 0) info->TrueAirspeed = value * AirDensityRatio(info->BaroAltitude);
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario Airspeed :%5.2fkm/h"), value);
        }
      }
      break;

    case 'T':
      if (ParToDouble(sentence, 1, &value)) {
        info->OutsideAirTemperature = value;
        info->TemperatureAvailable = TRUE;
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario OAT :%5.2f°C"), value);
        }
      }
      break;

    case 'V':
      if (ParToDouble(sentence, 1, &value)) {
        info->ExtBatt1_Voltage = value;
        if (OV_DebugLevel > 0) {
          StartupStore(TEXT(" OpenVario Voltage :%5.2fV"), value);
        }
      }
      break;
    default:
      if (ParToDouble(sentence, 1, &value)) {
        info->ExtBatt1_Voltage = value;
        StartupStore(TEXT(" OpenVario unsupported command %s :%7.2f"), szTmp1, value);
      }
      break;

  }
  return (true);
} // POV()
