/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devLX.h"
#include "utils/stringext.h"
#include "utils/printf.h"
#include "utils/charset_helper.h"

//____________________________________________________________class_definitions_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
void DevLX::Install(DeviceDescriptor_t* d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
} // Install()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWPn sentences.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevLX::ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }

  if (strncmp("$LXWP0", sentence, 6) == 0)
      return LXWP0(d, sentence + 7, info);
  else if (strncmp("$LXWP1", sentence, 6) == 0)
      return LXWP1(d, sentence + 7, info);
  else if (strncmp("$LXWP2", sentence, 6) == 0)
      return LXWP2(d, sentence + 7, info);
  else if (strncmp("$LXWP3", sentence, 6) == 0)
      return LXWP3(d, sentence + 7, info);

  return(false);
} // ParseNMEA()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP0 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevLX::LXWP0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  // $LXWP0,logger_stored, airspeed, airaltitude,
  //   v1[0],v1[1],v1[2],v1[3],v1[4],v1[5], hdg, windspeed*CS<CR><LF>
  //
  // 0 loger_stored : [Y|N] (not used in LX1600)
  // 1 IAS [km/h] ----> Condor uses TAS!
  // 2 baroaltitude [m]
  // 3-8 vario values [m/s] (last 6 measurements in last second)
  // 9 heading of plane (not used in LX1600)
  // 10 windcourse [deg] (not used in LX1600)
  // 11 windspeed [km/h] (not used in LX1600)
  //
  // e.g.:
  // $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

  double alt=0, airspeed=0;

  if (ParToDouble(sentence, 1, &airspeed))
  {
    airspeed = Units::From(unKiloMeterPerHour, airspeed);
    info->TrueAirspeed = airspeed;
    info->AirspeedAvailable = TRUE;
  }

  if (ParToDouble(sentence, 2, &alt))
  {
    if (airspeed>0) {
      info->IndicatedAirspeed = IndicatedAirSpeed(airspeed, alt);
    }
    UpdateBaroSource( info, d, QNEAltitudeToQNHAltitude(alt));
  } else {
    if (airspeed>0) {
      info->IndicatedAirspeed = IndicatedAirSpeed(airspeed, QNHAltitudeToQNEAltitude(info->Altitude));
    }
  }

  double Vario = 0;
  if (ParToDouble(sentence, 8, &Vario)) { /* take the last value to be more recent */
    UpdateVarioSource(*info, *d, Vario);
  }

  if (ParToDouble(sentence, 9, &info->MagneticHeading))
      info->MagneticHeadingAvailable=TRUE;

  if (ParToDouble(sentence, 10, &info->ExternalWindDirection) &&
      ParToDouble(sentence, 11, &info->ExternalWindSpeed))
  {
    /* convert to m/s */
    info->ExternalWindSpeed = Units::From(unKiloMeterPerHour, info->ExternalWindSpeed);
    info->ExternalWindAvailable = TRUE;
  }

  return(true);
} // LXWP0()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP1 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevLX::LXWP1(DeviceDescriptor_t* d, const char* String, NMEA_INFO* pGPS)
{
  // $LXWP1,serial number,instrument ID, software version, hardware
  //   version,license string,NU*SC<CR><LF>
  //
  // instrument ID ID of LX1600
  // serial number unsigned serial number
  // software version float sw version
  // hardware version float hw version
  // license string (option to store a license of PDA SW into LX1600)
 // ParToDouble(sentence, 1, &MACCREADY);
//	$LXWP1,LX5000IGC-2,15862,11.1 ,2.0*4A
#ifdef DEVICE_SERIAL
char ctemp[180];
static int NoMsg=0;
static int oldSerial=0;
if(strlen(String) < 180)
  if((( d->SerialNumber == 0)  || ( d->SerialNumber != oldSerial)) && (NoMsg < 5))
  {
    NoMsg++ ;
    NMEAParser::ExtractParameter(String, ctemp, 0);
    from_unknown_charset(ctemp, d->Name);
    lk::snprintf(d->Name, _T("%s"), ctemp);
    StartupStore(_T(". %s\n"), d->Name);

    NMEAParser::ExtractParameter(String, ctemp, 1);
    oldSerial = d->SerialNumber = StrToDouble(ctemp, nullptr);
    StartupStore(_T(". %s Serial Number %i"), d->Name, d->SerialNumber);

    NMEAParser::ExtractParameter(String, ctemp, 2);
  	d->SoftwareVer = StrToDouble(ctemp, nullptr);
    StartupStore(_T(". %s Software Vers.: %3.2f"), d->Name, d->SoftwareVer);

    NMEAParser::ExtractParameter(String, ctemp, 3);
    d->HardwareId = StrToDouble(ctemp, nullptr) * 10;
    StartupStore(_T(". %s Hardware Vers.: %3.2f"), d->Name, (d->HardwareId) / 10.0);

    TCHAR str[255];
    _stprintf(str, _T("%s (#%i) DETECTED"), d->Name, d->SerialNumber);
    DoStatusMessage(str);

    _stprintf(str, _T("SW Ver: %3.2f HW Ver: %3.2f "),  d->SoftwareVer, (double)(d->HardwareId)/10.0);
    DoStatusMessage(str);
  }
  // nothing to do
#endif
  return true;
} // LXWP1()

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP2 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevLX::LXWP2(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  // $LXWP2,mccready,ballast,bugs,polar_a,polar_b,polar_c, audio volume
  //   *CS<CR><LF>
  //
  // Mccready: float in m/s
  // Ballast: float 1.0 ... 1.5
  // Bugs: 0 - 100%
  // polar_a: float polar_a=a/10000 w=a*v2+b*v+c
  // polar_b: float polar_b=b/100 v=(km/h/100) w=(m/s)
  // polar_c: float polar_c=c
  // audio volume 0 - 100%

  double value;
  ParToDouble(sentence, 0, &value);
  d->RecvMacCready(value);

  return(true);
} // LXWP2()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP3 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevLX::LXWP3(DeviceDescriptor_t*, const char*, NMEA_INFO*)
{
  // $LXWP3,altioffset, scmode, variofil, tefilter, televel, varioavg,
  //   variorange, sctab, sclow, scspeed, SmartDiff,
  //   GliderName, time offset*CS<CR><LF>
  //
  // altioffset //offset necessary to set QNE in ft default=0
  // scmode // methods for automatic SC switch index 0=EXTERNAL, 1=ON CIRCLING
  //   2=auto IAS default=1
  // variofil // filtering of vario in seconds (float) default=1
  // tefilter // filtering of TE compensation in seconds (float) 0 = no
  //   filtering (default=0)
  // televel // level of TE compensation from 0 to 250 default=0 (%) default=0
  // varioavg // averaging time in seconds for integrator default=25
  // variorange // 2.5 5 or 10 (m/s or kts) (float) default=5.0
  // sctab // area of silence in SC mode (float) 0-5.0 1.0= silence between
  //   +1m/s and -1m/s default=1
  // sclow // external switch/taster function 0=NORMAL 1=INVERTED 2=TASTER
  //   default=1
  // scspeed // speed of automatic switch from vario to sc mode if SCMODE==2 in
  //   (km/h) default=110
  // SmartDiff float (m/s/s) (Smart VARIO filtering)
  // GliderName // Glider name string max. 14 characters
  // time offset int in hours

  // nothing to do
  return(true);
} // LXWP3()




bool DevLX::GPRMB(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{

  char  szTmp[MAX_NMEA_LEN];
  double fTmp;

  ParToDouble(sentence, 5, &fTmp);
  double DegLat = (double)((int) (fTmp/100.0));
  double MinLat =  fTmp- (100.0*DegLat);
  double Latitude = DegLat+MinLat/60.0;

  NMEAParser::ExtractParameter(sentence,szTmp,6);
  if (szTmp[0]==_T('S')) {
    Latitude *= -1;
  }

  ParToDouble(sentence, 7, &fTmp);
  double DegLon =  (double) ((int) (fTmp/100.0));
  double MinLon =  fTmp- (100.0*DegLon);
  double Longitude = DegLon+MinLon/60.0;

  NMEAParser::ExtractParameter(sentence,szTmp,8);
  if (szTmp[0]==_T('W')) {
    Longitude *= -1;
  }
	
  NMEAParser::ExtractParameter(sentence,szTmp,4);
  tstring tname = from_unknown_charset(szTmp);

  LockTaskData();
  {
  	lk::snprintf(WayPointList[RESWP_EXT_TARGET].Name, NAME_SIZE, TEXT("^%s"), tname.c_str());
    WayPointList[RESWP_EXT_TARGET].Latitude = Latitude;
    WayPointList[RESWP_EXT_TARGET].Longitude = Longitude;
    WayPointList[RESWP_EXT_TARGET].Altitude = RESWP_INVALIDNUMBER;  // GPRMB has no elevation information
    Alternate2 = RESWP_EXT_TARGET;
  }
  UnlockTaskData();

  return false;
}

/**
 * Converts TCHAR[] string into US-ASCII string with characters safe for
 * writing to LX devices.
 *
 * Characters are converted into their most similar representation
 * in ASCII. Nonconvertable characters are replaced by '?'.
 *
 * Output string will always be terminated by '\0'.
 *
 * @param input    input string (must be terminated with '\0')
 * @param outSize  output buffer size
 * @param output   output buffer
 */
void DevLX::Wide2LxAscii(const TCHAR* input, int outSize, char* output) {
  assert(output && (outSize > 0));

  if (output && (outSize > 0)) {

    to_usascii(input, output, outSize);

    // replace all non-ascii characters with '?' - LX Colibri is very sensitive
    // on non-ascii chars - the electronic seal can be broken
    // (to_usascii() should be enough, but to be sure that someone has not
    // incorrectly changed to_usascii())
    for (; *output != '\0'; ++output) {
      if (*output < 32 || *output > 126) {
        *output = '?';
      }
    }
  }
} // Wide2LxAscii()
