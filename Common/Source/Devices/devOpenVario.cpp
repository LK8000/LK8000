/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */
//_____________________________________________________________________includes_

#include "externs.h"
#include "devOpenVario.h"
#include "LKInterface.h"
#include "InputEvents.h"
#include <iostream>

#include <fcntl.h>

void LKBeep(int freq, int delay) {
#ifdef __linux__

  //  ioctl(fd, KDMKTONE, (delay<<16 | 1193180/freq));
#else
  Beep(freq, delay);
#endif
}

extern bool UpdateBaroSource(NMEA_INFO* pGPS, const short parserid, const PDeviceDescriptor_t d, const double fAlt);
extern bool UpdateQNH(const double newqnh);


int iOpenVario_RxUpdateTime = 0;
double OpenVario_oldMC = MACCREADY;
int OpenVario_MacCreadyUpdateTimeout = 0;
int OpenVario_BugsUpdateTimeout = 0;
int OpenVario_BallastUpdateTimeout = 0;


//double fPolar_a=0.0, fPolar_b=0.0, fPolar_c=0.0, fVolume=0.0;
BOOL OpenVario_bValid = false;
int OpenVarioNMEAddCheckSumStrg(TCHAR szStrg[]);
BOOL OpenVarioPutMacCready(PDeviceDescriptor_t d, double MacCready);
BOOL OpenVarioPutBallast(PDeviceDescriptor_t d, double Ballast);
BOOL OpenVarioPutBugs(PDeviceDescriptor_t d, double Bugs);

int OV_DebugLevel = 0;
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
//                                 Polar
//        MC     BAL    BUG%     a      b      c     Volume
//$PFLX2,1.80,  1.00,   30,    1.71,  -2.43,  1.46, 0*15


//GEXTERN double BUGS;
//GEXTERN double BALLAST;
//GEXTERN int POLARID;
//GEXTERN double POLAR[POLARSIZE];
//GEXTERN double WEIGHTS[POLARSIZE];
//GEXTERN double POLARV[POLARSIZE];
//GEXTERN double POLARLD[POLARSIZE];

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
  if (OpenVario_bValid == false)
    return false;

  _stprintf(szTmp, TEXT("POV,C,MC,%0.2f"), (double) MacCready);
  OpenVarioNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);


  OpenVario_MacCreadyUpdateTimeout = 5;


  return true;

}

BOOL OpenVarioPutBallast(PDeviceDescriptor_t d, double Ballast) {
  TCHAR szTmp[254];
  if (OpenVario_bValid == false)
    return false;

  _stprintf(szTmp, TEXT("POV,C,WL,%3f"), (1.0 + Ballast));
  OpenVarioNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);




  OpenVario_BallastUpdateTimeout = 10;
  return (TRUE);

}

BOOL OpenVarioPutBugs(PDeviceDescriptor_t d, double Bugs) {
  TCHAR szTmp[254];

  if (OpenVario_bValid == false)
    return false;



  _stprintf(szTmp, TEXT("$POV,C,BU,%0.2f"), Bugs);
  OpenVarioNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  OpenVario_BugsUpdateTimeout = 5;
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
    if (OV_DebugLevel > 0) StartupStore(TEXT(" OpenVario Checksum Error %s"), NEWLINE);
    return FALSE;
  }

  if (_tcsncmp(_T("$POV"), sentence, 4) == 0) {
    return POV(d, sentence + 5, info);
    if (OV_DebugLevel > 0) StartupStore(TEXT(" OpenVario POV %s"), NEWLINE);
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
        if (OV_DebugLevel > 0) StartupStore(TEXT(" OpenVario Vario :%5.2fm/s %s"), value, NEWLINE);
        TriggerVarioUpdate();
        if ((info->Vario) > 0.1)
          LKBeep(1500 + info->Vario * 100, 100);
        else
          LKBeep(1500 + info->Vario * 100, 200);
      }
      break;

    case 'P':
      if (ParToDouble(sentence, 1, &value)) {
        info->BaroAltitude = StaticPressureToAltitude(value);
        info->BaroAltitudeAvailable = true;
        if (OV_DebugLevel > 0) StartupStore(TEXT(" OpenVario QNH %6.1fhPa Altitude :%6.1fm GPS: %6.1fm %s"), value, info->BaroAltitude, info->Altitude, NEWLINE);
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
        if (OV_DebugLevel > 0) StartupStore(TEXT(" OpenVario Airspeed :%5.2fkm/h %s"), value, NEWLINE);
      }
      break;

    case 'T':
      if (ParToDouble(sentence, 1, &value)) {
        info->OutsideAirTemperature = value;
        info->TemperatureAvailable = TRUE;
        if (OV_DebugLevel > 0) StartupStore(TEXT(" OpenVario OAT :%5.2f°C %s"), value, NEWLINE);
      }
      break;

    case 'V':
      if (ParToDouble(sentence, 1, &value)) {
        info->ExtBatt1_Voltage = value;
        if (OV_DebugLevel > 0) StartupStore(TEXT(" OpenVario Voltage :%5.2fV %s"), value, NEWLINE);
      }
      break;
    default:
      if (ParToDouble(sentence, 1, &value)) {
        info->ExtBatt1_Voltage = value;
        StartupStore(TEXT(" OpenVario unsupported command %s :%7.2f %s"), szTmp1, value, NEWLINE);
      }
      break;

  }
  return (true);
} // POV()



