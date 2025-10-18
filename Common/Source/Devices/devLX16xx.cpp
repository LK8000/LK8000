/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include "externs.h"
#include "devLX16xx.h"
#include "LKInterface.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Utils.h"
#include "utils/printf.h"
#include "utils/charset_helper.h"
#include "OS/Sleep.h"
#include "Comm/ExternalWind.h"

int  LX166AltitudeUpdateTimeout =0;
int  LX16xxAlt=0;
double fPolar_a=0.0, fPolar_b=0.0, fPolar_c=0.0, fVolume=0.0;
BOOL bValid = false;
int LX16xxNMEAddCheckSumStrg( TCHAR szStrg[] );
BOOL LX16xxPutMacCready(DeviceDescriptor_t* d, double MacCready);
BOOL LX16xxPutBallast(DeviceDescriptor_t* d, double Ballast);
BOOL LX16xxPutBugs(DeviceDescriptor_t* d, double Bugs);

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
void DevLX16xx::Install(DeviceDescriptor_t* d)
{
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = LX16xxPutMacCready;
  d->PutBugs      = LX16xxPutBugs; // removed to prevent cirvular updates
  d->PutBallast   = LX16xxPutBallast;

  d->DirectLink   = LX16xxDirectLink;
} // Install()




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



int LX16xxNMEAddCheckSumStrg( TCHAR szStrg[] )
{
int i,iCheckSum=0;
TCHAR  szCheck[254];

 if(szStrg[0] != '$')
   return -1;

 iCheckSum = szStrg[1];
  for (i=2; i < (int)_tcslen(szStrg); i++)
  {
	//  if(szStrgi0] != ' ')
	    iCheckSum ^= szStrg[i];
  }
  _stprintf(szCheck,TEXT("*%02X\r\n"),iCheckSum);
  _tcscat(szStrg,szCheck);
  return iCheckSum;
}


BOOL DevLX16xx::LX16xxDirectLink(DeviceDescriptor_t* d, BOOL bLinkEnable)
{
TCHAR  szTmp[254];
  if(bLinkEnable)
  {
	StartupStore(TEXT("enable LX 16xx direct Link %s"), NEWLINE);
	_stprintf(szTmp, TEXT("$PFLX0,COLIBRI"));
	LX16xxNMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
	Sleep(100);
  }
  else
  {
	// exit transfer mode
	// and return to normal LX16xx  communication
	StartupStore(TEXT("return from LX 16xx link %s"), NEWLINE);
	_stprintf(szTmp, TEXT("$PFLX0,LX1600"));
	LX16xxNMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
	unsigned long lOldBR =   d->Com->GetBaudrate();

	/* switch to 4k8 for new Firmware versions of LX1600 */
	d->Com->SetBaudrate(4800);
	Sleep(100);
	d->Com->WriteString(szTmp);
	Sleep(100);
	d->Com->WriteString(szTmp);
	Sleep(100);

	/* return to previous original */
	d->Com->SetBaudrate(lOldBR);
	Sleep(100);
	d->Com->WriteString(szTmp);
	Sleep(100);
	d->Com->WriteString(szTmp);
	Sleep(100);
  }
  return true;
}


bool DevLX16xx::SetupLX_Sentence(DeviceDescriptor_t* d)
{
TCHAR  szTmp[254];

  _stprintf(szTmp, TEXT("$PFLX0,GPGGA,1,GPRMC,1,LXWP0,1,LXWP1,0,LXWP2,5,LXWP3,17,LXWP4,20,LXWP5,50"));

  LX16xxNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  static int oldQNH=0;
  int iQNH= (int) QNH *1000;
  if (BaroAltitudeAvailable(GPS_INFO))
  {
    if(iQNH != oldQNH)
    {
	  oldQNH  = iQNH;
      _stprintf(szTmp, TEXT("$PFLX3,%i,,,,,,,,,,,,"), (int)Units::To(unFeet, QFEAltitudeOffset-(double)LX16xxAlt) );
      LX16xxNMEAddCheckSumStrg(szTmp);
      LX166AltitudeUpdateTimeout = 5;
      d->Com->WriteString(szTmp);
    }
  }
  return true;
}




BOOL LX16xxPutMacCready(DeviceDescriptor_t* d, double MacCready){
  TCHAR  szTmp[254];
  if(bValid == false) {
    return false;
  }
  _stprintf(szTmp, TEXT("$PFLX2,%3.1f,%4.2f,%.0f,%4.2f,%4.2f,%4.2f,%d"), MacCready ,CalculateLXBalastFactor(BALLAST),CalculateLXBugs(BUGS),fPolar_a, fPolar_b, fPolar_c,(int) fVolume);
  LX16xxNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
  return true;
}


BOOL LX16xxPutBallast(DeviceDescriptor_t* d, double Ballast){
  TCHAR  szTmp[254];
  if(bValid == false) {
    return false;
  }

  _stprintf(szTmp, TEXT("$PFLX2,%3.1f,%4.2f,%.0f,%4.2f,%4.2f,%4.2f,%d"), MACCREADY ,CalculateLXBalastFactor(Ballast),CalculateLXBugs(BUGS),fPolar_a, fPolar_b, fPolar_c,(int) fVolume);

  LX16xxNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  return (TRUE);
}

// ToDo raw 2.5% may cause circular updates due to inaccurate steps
// can be solved later update from LX to LK works
BOOL LX16xxPutBugs(DeviceDescriptor_t* d, double Bugs){
  TCHAR  szTmp[254];

  if(bValid == false) {
    return false;
  }

  if(Bugs < 0.7) {
    Bugs = 0.7;
  }

  _stprintf(szTmp, TEXT("$PFLX2,%3.1f,%4.2f,%.0f,%4.2f,%4.2f,%4.2f,%d"), MACCREADY , CalculateLXBalastFactor(BALLAST),CalculateLXBugs(Bugs),fPolar_a, fPolar_b, fPolar_c,(int) fVolume);

	LX16xxNMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
  return(TRUE);
}





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
BOOL DevLX16xx::ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  static int i=40;

  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }

  /* configure LX after 30 GPS positions */
  if (strncmp("$GPGGA", sentence, 6) == 0) {
    if (i++ > 10) {
      SetupLX_Sentence(d);
      i = 0;
    }
  }

  if (strncmp("$LXWP0", sentence, 6) == 0)
    return LXWP0(d, sentence + 7, info);
  else if (strncmp("$LXWP1", sentence, 6) == 0)
    return LXWP1(d, sentence + 7, info);
  else if (strncmp("$LXWP2", sentence, 6) == 0)
    return LXWP2(d, sentence + 7, info);
  else if (strncmp("$LXWP3", sentence, 6) == 0)
    return LXWP3(d, sentence + 7, info);
  else if (strncmp("$LXWP4", sentence, 6) == 0)
    return LXWP4(d, sentence + 7, info);

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
bool DevLX16xx::LXWP0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
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
    info->TrueAirSpeed.update(*d, airspeed);
  }
  if(LX166AltitudeUpdateTimeout >0)
	  LX166AltitudeUpdateTimeout--;
  else
    if (ParToDouble(sentence, 2, &alt))
    {
      LX16xxAlt = (int) alt;
      if (airspeed>0) {
        info->IndicatedAirSpeed.update(*d, IndicatedAirSpeed(airspeed, alt));
      }
      UpdateBaroSource( info, d, QNEAltitudeToQNHAltitude(alt));
    }

  double Vario = 0;
  if (ParToDouble(sentence, 3, &Vario)) {
    UpdateVarioSource(*info, *d, Vario);
  }

  double WindSpeed, WindDirection;
  if (ParToDouble(sentence, 10, &WindDirection) && ParToDouble(sentence, 11, &WindSpeed)) {
    UpdateExternalWind(*info, *d, Units::From(Units_t::unKiloMeterPerHour, WindSpeed), WindDirection);
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
bool DevLX16xx::LXWP1(DeviceDescriptor_t* d, const char* String, NMEA_INFO* pGPS)
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
  static tstring oldSerial;
if (strlen(String) < 180)
  if((d->SerialNumber.empty() || ( d->SerialNumber != oldSerial)) && (NoMsg < 5))
  {
    NoMsg++ ;
    NMEAParser::ExtractParameter(String, ctemp, 0);
    from_unknown_charset(ctemp, d->Name);
    lk::snprintf(d->Name, _T("%s"),ctemp);
    StartupStore(_T(". %s\n"),ctemp);

    NMEAParser::ExtractParameter(String, ctemp, 1);
    oldSerial = d->SerialNumber = from_unknown_charset(ctemp);
	  StartupStore(_T(". %s Serial Number %s"), d->Name, d->SerialNumber.c_str());

    NMEAParser::ExtractParameter(String, ctemp, 2);
    d->SoftwareVer= StrToDouble(ctemp, nullptr);
    StartupStore(_T(". %s Software Vers.: %3.2f"), d->Name, d->SoftwareVer);

    NMEAParser::ExtractParameter(String, ctemp, 3);
    d->HardwareId = StrToDouble(ctemp, nullptr) * 10;
  	StartupStore(_T(". %s Hardware Vers.: %3.2f"), d->Name, d->HardwareId / 10.0);

    TCHAR str[255];
    _stprintf(str, _T("%s (#%s) DETECTED"), d->Name, d->SerialNumber.c_str());
    DoStatusMessage(str);
    _stprintf(str, _T("SW Ver: %3.2f HW Ver: %3.2f "), d->SoftwareVer, d->HardwareId / 10.0);
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
bool DevLX16xx::LXWP2(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO*)
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
  //float fBallast,fBugs, polar_a, polar_b, polar_c, fVolume;

  double fTmp;
  if (ParToDouble(sentence, 0, &fTmp)) {
    int iTmp = (fTmp * 100.0 + 0.5f);
    bValid = true;
    d->RecvMacCready((double)(iTmp) / 100.0);
  }

  if (ParToDouble(sentence, 1, &fTmp)) {
    d->RecvBallast(CalculateBalastFromLX(fTmp));
  }

  if(ParToDouble(sentence, 2, &fTmp)) {
    d->RecvBugs(CalculateBugsFromLX(fTmp));
  }

  if (ParToDouble(sentence, 3, &fTmp))
    fPolar_a = fTmp;
  if (ParToDouble(sentence, 4, &fTmp))
    fPolar_b = fTmp;
  if (ParToDouble(sentence, 5, &fTmp))
    fPolar_c = fTmp;
  if (ParToDouble(sentence, 6, &fTmp))
  {
    fVolume = fTmp;
  }
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
bool DevLX16xx::LXWP3(DeviceDescriptor_t*, const char*, NMEA_INFO*)
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


bool DevLX16xx::LXWP4(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{

// $LXWP4 Sc, Netto, Relativ, gl.dif, leg speed, leg time, integrator, flight time, battery voltage*CS<CR><LF>
// Sc  float (m/s)
// Netto  float (m/s)
// Relativ  float (m/s)
// Distance float (m)
// gl.dif  int (ft)
// leg speed (km/h)
// leg time (km/h)
// integrator float (m/s)
// flight time unsigned in seconds
// battery voltage float (V)

  double Batt;

  if (ParToDouble(sentence, 9, &Batt))
  {
	 info->ExtBatt1_Voltage = Batt;
  }


  return(true);
} // LXWP4()
