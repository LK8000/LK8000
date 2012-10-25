/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include "externs.h"
#include "devLXV7.h"
#include "LKInterface.h"
#include "InputEvents.h"

extern bool UpdateBaroSource(NMEA_INFO* GPS_INFO, const short parserid, const PDeviceDescriptor_t d, const double fAlt);

int iLXV7_RxUpdateTime=0;
double LXV7_oldMC = MACCREADY;
int  LXV7_MacCreadyUpdateTimeout = 0;
int  LXV7_BugsUpdateTimeout = 0;
int  LXV7_BallastUpdateTimeout =0;
int LXV7_iGPSBaudrate = 0;
int LXV7_iPDABaudrate = 0;

//double fPolar_a=0.0, fPolar_b=0.0, fPolar_c=0.0, fVolume=0.0;
BOOL LXV7_bValid = false;
int LXV7NMEAddCheckSumStrg( TCHAR szStrg[] );
BOOL LXV7PutMacCready(PDeviceDescriptor_t d, double MacCready);
BOOL LXV7PutBallast(PDeviceDescriptor_t d, double Ballast);
BOOL LXV7PutBugs(PDeviceDescriptor_t d, double Bugs);


//____________________________________________________________class_definitions_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Registers device into device subsystem.
///
/// @retval true  when device has been registered successfully
/// @retval false device cannot be registered
///
//static


bool DevLXV7::Register()
{

  return(devRegister(GetName(),
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
BOOL DevLXV7::Install(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = LXV7PutMacCready;
  d->PutBugs      = LXV7PutBugs; // removed to prevent cirvular updates
  d->PutBallast   = LXV7PutBallast;
  d->Open         = NULL;
  d->Close        = NULL;
  d->Init         = NULL;
  d->LinkTimeout  = GetTrue;
  d->Declare      = NULL;
  d->IsLogger     = NULL;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  d->DirectLink   = LXV7DirectLink;



  return(true);
} // Install()



BOOL DevLXV7::LXV7DirectLink(PDeviceDescriptor_t d, BOOL bLinkEnable)
{
TCHAR  szTmp[254];
#define CHANGE_DELAY 10

if(LXV7_iGPSBaudrate ==0)
{
  _stprintf(szTmp, TEXT("$PLXV0,BRGPS,R"));
  LXV7NMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
  Sleep(CHANGE_DELAY);
  d->Com->WriteString(szTmp);
  Sleep(CHANGE_DELAY);
}


  if(bLinkEnable)
  {
	LockComm();
	#if TESTBENCH
	StartupStore(TEXT("enable LX V7 direct Link %s"), NEWLINE);
	#endif
	LXV7_iPDABaudrate = d->Com->GetBaudrate();

	_stprintf(szTmp, TEXT("$PLXV0,CONNECTION,W,DIRECT"));
	LXV7NMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
	Sleep(CHANGE_DELAY);
    if(LXV7_iPDABaudrate != LXV7_iGPSBaudrate)
    {
	  d->Com->SetBaudrate(LXV7_iGPSBaudrate);
	#if TESTBENCH
	  StartupStore(TEXT("Set Baudrate %i %s"),LXV7_iGPSBaudrate, NEWLINE);
	#endif
	  Sleep(CHANGE_DELAY);
    }
	Sleep(CHANGE_DELAY);
  }
  else
  {
	Sleep(CHANGE_DELAY);

    if(LXV7_iPDABaudrate != LXV7_iGPSBaudrate)
    {
	#if TESTBENCH
	  StartupStore(TEXT("Set Baudrate %i %s"),LXV7_iPDABaudrate, NEWLINE);
	#endif
	  d->Com->SetBaudrate(LXV7_iPDABaudrate);
	  Sleep(CHANGE_DELAY);
    }

	#if TESTBENCH
	StartupStore(TEXT("Return from V7 link %s"), NEWLINE);
	#endif
	_stprintf(szTmp, TEXT("$PLXV0,CONNECTION,W,VSEVEN"));
	LXV7NMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
	Sleep(CHANGE_DELAY);
	UnlockComm();
	Sleep(CHANGE_DELAY);

  }

  return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Returns device name (max length is @c DEVNAMESIZE).
///
//static
const TCHAR* DevLXV7::GetName()
{
  return(_T("LXV7"));
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



int LXV7NMEAddCheckSumStrg( TCHAR szStrg[] )
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


bool DevLXV7::SetupLX_Sentence(PDeviceDescriptor_t d)
{
TCHAR  szTmp[254];


_stprintf(szTmp, TEXT("$PLXV0,NMEARATE,W,2,5,0,0,1,0,0"));
  LXV7NMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);


  return true;
}

long Baudrate(int iIdx)
{
//	indexes are following:
//	enum { br4800=0, br9600, br19200, br38400, br57600,
//	br115200,br230400,br256000,br460800, br500k, br1M};
long lBaudrate = -1;
switch (iIdx)
{
  case 0:  lBaudrate = 4800   ; break;
  case 1:  lBaudrate = 9600   ; break;
  case 2:  lBaudrate = 19200  ; break;
  case 3:  lBaudrate = 38400  ; break;
  case 4:  lBaudrate = 56800  ; break;
  case 5:  lBaudrate = 115200 ; break;
  case 6:  lBaudrate = 230400 ; break;
  case 7:  lBaudrate = 256000 ; break;
  case 8:  lBaudrate = 460800 ; break;
  case 9:  lBaudrate = 500000 ; break;
  case 10: lBaudrate = 1000000; break;
  default: lBaudrate = -1     ; break;
}
return lBaudrate;
}



BOOL LXV7PutMacCready(PDeviceDescriptor_t d, double MacCready){
TCHAR  szTmp[254];
if(LXV7_bValid == false)
  return false;

  _stprintf(szTmp, TEXT("$PLXV0,MC,W,%3.1f"), MacCready );

  LXV7NMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);


  LXV7_MacCreadyUpdateTimeout = 5;


  return true;

}


BOOL LXV7PutBallast(PDeviceDescriptor_t d, double Ballast){
TCHAR  szTmp[254];
if(LXV7_bValid == false)
  return false;


  _stprintf(szTmp, TEXT("$PLXV0,BAL,W,%4.2f"),(1.0+Ballast));

 LXV7NMEAddCheckSumStrg(szTmp);
 d->Com->WriteString(szTmp);


 //DevLXV7::PutGPRMB(d);

 LXV7_BallastUpdateTimeout =10;
 return(TRUE);

}


BOOL LXV7PutBugs(PDeviceDescriptor_t d, double Bugs){
TCHAR  szTmp[254];

if(LXV7_bValid == false)
  return false;


	  _stprintf(szTmp, TEXT("$PLXV0,BUGS,W,%3.1f"),(1.00-Bugs)*100.0);

	LXV7NMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);

	LXV7_BugsUpdateTimeout = 5;
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
BOOL DevLXV7::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{
  static int i=40;
  TCHAR  szTmp[256];


  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }

  if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0)
  {
	if(iLXV7_RxUpdateTime > 0)
	{
	  iLXV7_RxUpdateTime--;
	}
	else
	{
	  if(fabs(LXV7_oldMC - MACCREADY)> 0.005f)
	  {
		LXV7PutMacCready( d,  MACCREADY);
		LXV7_oldMC = MACCREADY;
		LXV7_MacCreadyUpdateTimeout = 2;
      }
	}
  }

  /* configure LX after 10 GPS positions */
  if (_tcsncmp(_T("$GPGGA"), sentence, 6) == 0)
  {
    if(i++ > 4)
    {
      SetupLX_Sentence(d);
	  i=0;
    }


    static int oldQFEOff =0;
    static int iOldQNH   =0;



    int iQNH = (int)(QNH*100.0);
    if(iQNH != iOldQNH)
    {
  	  iOldQNH = iQNH;
      _stprintf(szTmp, TEXT("$PLXV0,QNH,W,%i"),(int)iQNH);
      LXV7NMEAddCheckSumStrg(szTmp);
      d->Com->WriteString(szTmp);
    }

    int QFE = (int)QFEAltitudeOffset;
    if(QFE != oldQFEOff)
    {
  	  oldQFEOff = QFE;
      _stprintf(szTmp, TEXT("$PLXV0,ELEVATION,W,%i"),(int)(QFEAltitudeOffset));
      LXV7NMEAddCheckSumStrg(szTmp);
  //    d->Com->WriteString(szTmp);
    }
  }
  if(LXV7_iGPSBaudrate ==0)
  {
    _stprintf(szTmp, TEXT("$PLXV0,BRGPS,R"));
    LXV7NMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);
  }

if (_tcsncmp(_T("$PLXVF"), sentence, 6) == 0)
  return PLXVF(d, sentence + 7, info);
else
  if (_tcsncmp(_T("$PLXVS"), sentence, 6) == 0)
    return PLXVS(d, sentence + 7, info);
  else
	if (_tcsncmp(_T("$PLXV0"), sentence, 6) == 0)
	  return PLXV0(d, sentence + 7, info);
	else
      if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0)
        return LXWP2(d, sentence + 7, info);


#ifdef OLD_LX_SENTENCES
	else
      if (_tcsncmp(_T("$LXWP0"), sentence, 6) == 0)
        return LXWP0(d, sentence + 7, info);
      else
        if (_tcsncmp(_T("$LXWP1"), sentence, 6) == 0)
          return LXWP1(d, sentence + 7, info);
        else
          if (_tcsncmp(_T("$LXWP3"), sentence, 6) == 0)
            return LXWP3(d, sentence + 7, info);
          else
            if (_tcsncmp(_T("$LXWP4"), sentence, 6) == 0)
              return LXWP4(d, sentence + 7, info);
#endif
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
bool DevLXV7::LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
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



 /*
  if (ParToDouble(sentence, 10, &info->ExternalWindDirection) &&
      ParToDouble(sentence, 11, &info->ExternalWindSpeed))
    info->ExternalWindAvailable = TRUE;
*/
//  TriggerVarioUpdate();

  return(false);
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
bool DevLXV7::LXWP1(PDeviceDescriptor_t, const TCHAR*, NMEA_INFO*)
{
  // $LXWP1,serial number,instrument ID, software version, hardware
  //   version,license string,NU*SC<CR><LF>
  //
  // instrument ID ID of LX1600
  // serial number unsigned serial number
  // software version float sw version
  // hardware version float hw version
  // license string (option to store a license of PDA SW into LX1600)

  // nothing to do
  return(true);
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
bool DevLXV7::LXWP2(PDeviceDescriptor_t, const TCHAR* sentence, NMEA_INFO*)
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
int iTmp;
if(LXV7_MacCreadyUpdateTimeout > 0)
{
	LXV7_MacCreadyUpdateTimeout--;
}
else
  if (ParToDouble(sentence, 0, &fTmp))
  {
	iTmp =(int) (fTmp*100.0+0.5f);
	fTmp = (double)(iTmp)/100.0;
	LXV7_bValid = true;
	if(fabs(MACCREADY - fTmp)> 0.001)
	{
	  MACCREADY = fTmp;
	  iLXV7_RxUpdateTime =5;
	}
  }


if(LXV7_BallastUpdateTimeout > 0)
{
	LXV7_BallastUpdateTimeout--;
}
else
  if (ParToDouble(sentence, 1, &fTmp))
  {
	fTmp -= 1.0;
	fTmp  = (fTmp);

	if(  fabs(fTmp -BALLAST) >= 0.05)
    {
      BALLAST = fTmp;
      iLXV7_RxUpdateTime = 5;
    }
  }

if(LXV7_BugsUpdateTimeout > 0)
{
  LXV7_BugsUpdateTimeout--;
}
else
  if(ParToDouble(sentence, 2, &fTmp))
  {
	int iTmp = 100-(int)(fTmp+0.5);
	fTmp =  (double)iTmp/100.0;
	if(  fabs(fTmp -BUGS) >= 0.03)
    {
      BUGS = fTmp;
      iLXV7_RxUpdateTime = 5;
    }
  }
/*
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
*/
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
bool DevLXV7::LXWP3(PDeviceDescriptor_t, const TCHAR*, NMEA_INFO*)
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




bool DevLXV7::LXWP4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{


// $LXWP4 Sc, Netto, Relativ, gl.dif, leg speed, leg time, integrator, flight time, battery voltage*CS<CR><LF>
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


  return(true);
} // LXWP4()



bool DevLXV7::PLXVF(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{

  double alt, airspeed;


  if (ParToDouble(sentence, 1, &info->AccelX))
    if (ParToDouble(sentence, 2, &info->AccelY))
      if (ParToDouble(sentence, 3, &info->AccelZ))
        info->AccelerationAvailable = true;

  if (ParToDouble(sentence, 5, &airspeed))
  {
//	airspeed = 135.0/TOKPH;
	info->IndicatedAirspeed = airspeed;
	info->AirspeedAvailable = TRUE;

  }

  if (ParToDouble(sentence, 6, &alt))
  {
	UpdateBaroSource( info, 0, d, AltitudeToQNHAltitude(alt));
    info->TrueAirspeed =  airspeed * AirDensityRatio(alt);
  }

  if (ParToDouble(sentence, 4, &info->Vario))
  {
	info->VarioAvailable = TRUE;
	TriggerVarioUpdate();
  }


  // Get STF switch
double fTmp;
if (ParToDouble(sentence, 7, &fTmp))
{
int  iTmp = (int)(fTmp+0.1);
EnableExternalTriggerCruise = true;

static int  iOldVarioSwitch=0;
if(iTmp != iOldVarioSwitch)
{
  iOldVarioSwitch = iTmp;
  if(iTmp==1)
  {
    ExternalTriggerCruise = true;
    ExternalTriggerCircling = false;
  }
  else
  {
    ExternalTriggerCruise = false;
    ExternalTriggerCircling = true;
  }
}
}

  return(true);
} // PLXVF()


bool DevLXV7::PLXVS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
double Batt;
double OAT;
  if (ParToDouble(sentence, 0, &OAT))
  {
	 info->OutsideAirTemperature = OAT;
	 info->TemperatureAvailable  = TRUE;
  }
#ifdef SLOW_DET
	  // Get STF switch
  double fTmp;
  if (ParToDouble(sentence, 1, &fTmp))
  {
    int  iTmp = (int)(fTmp+0.1);
    EnableExternalTriggerCruise = true;

    static int  iOldVarioSwitch=0;
    if(iTmp != iOldVarioSwitch)
    {
	  iOldVarioSwitch = iTmp;
      if(iTmp==1)
      {
	    ExternalTriggerCruise = true;
	    ExternalTriggerCircling = false;
      }
      else
      {
        ExternalTriggerCruise = false;
	    ExternalTriggerCircling = true;
      }
    }
  }
#endif

  if (ParToDouble(sentence, 2, &Batt))
	 info->ExtBatt1_Voltage = Batt;


  return(true);
} // PLXVS()



bool DevLXV7::PLXV0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR  szTmp1[80], szTmp2[80];




  NMEAParser::ExtractParameter(sentence,szTmp1,1);
  if  (_tcscmp(szTmp1,_T("W"))!=0)  // no write flag received
	 return false;

  NMEAParser::ExtractParameter(sentence,szTmp1,0);
  if  (_tcscmp(szTmp1,_T("BRGPS"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	LXV7_iGPSBaudrate = Baudrate( (int)( (StrToDouble(szTmp2,NULL))+0.1 ) );
	return true;
  }

  if (_tcscmp(szTmp1,_T("BRPDA"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	LXV7_iPDABaudrate = Baudrate( (int) StrToDouble(szTmp2,NULL));
	return true;
  }
#ifdef DEBUG_PARAMETERS
  if (_tcscmp(szTmp1,_T("MC"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	iTmp =(int) StrToDouble(szTmp2,NULL);
	return true;
  }

  if (_tcscmp(szTmp1,_T("BAL"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	iTmp = (int) StrToDouble(szTmp2,NULL);
	return true;
  }

  if (_tcscmp(szTmp1,_T("BUGS"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	iTmp = (int) StrToDouble(szTmp2,NULL);
	return true;
  }


  if (_tcscmp(szTmp1,_T("VOL"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	iTmp = (int) StrToDouble(szTmp2,NULL);
	return true;
  }

  if (_tcscmp(szTmp1,_T("POLAR"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	iTmp = (int) StrToDouble(szTmp2,NULL);
	return true;
  }

  if (_tcscmp(szTmp1,_T("CONNECTION"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	iTmp = (int) StrToDouble(szTmp2,NULL);
	return true;
  }

  if (_tcscmp(szTmp1,_T("NMEARATE"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	iTmp = (int) StrToDouble(szTmp2,NULL);
	return true;
  }
#endif
  return(false);
} // PLXV0()
