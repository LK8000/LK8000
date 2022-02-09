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
#include "devLXV7_EXP.h"
#include "LKInterface.h"
#include "InputEvents.h"
#include "utils/printf.h"

extern bool UpdateQNH(const double newqnh);


int iLXV7_EXP_RxUpdateTime=0;
double LXV7_EXP_oldMC = MACCREADY;
int  LXV7_EXP_MacCreadyUpdateTimeout = 0;
int  LXV7_EXP_BugsUpdateTimeout = 0;
int  LXV7_EXP_BallastUpdateTimeout =0;
int LXV7_EXP_iGPSBaudrate = 0;
int LXV7_EXP_iPDABaudrate = 0;

//double fPolar_a=0.0, fPolar_b=0.0, fPolar_c=0.0, fVolume=0.0;
BOOL LXV7_EXP_bValid = false;
int LXV7_EXPNMEAddCheckSumStrg( TCHAR szStrg[] );
BOOL LXV7_EXPPutMacCready(PDeviceDescriptor_t d, double MacCready);
BOOL LXV7_EXPPutBallast(PDeviceDescriptor_t d, double Ballast);
BOOL LXV7_EXPPutBugs(PDeviceDescriptor_t d, double Bugs);

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
void DevLXV7_EXP::Install(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = LXV7_EXPPutMacCready;
  d->PutBugs      = LXV7_EXPPutBugs; // removed to prevent cirvular updates
  d->PutBallast   = LXV7_EXPPutBallast;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  d->DirectLink   = LXV7_EXP_DirectLink;
} // Install()



BOOL DevLXV7_EXP::LXV7_EXP_DirectLink(PDeviceDescriptor_t d, BOOL bLinkEnable)
{
TCHAR  szTmp[254];
#define CHANGE_DELAY 10

if(LXV7_EXP_iGPSBaudrate ==0)
{
  _stprintf(szTmp, TEXT("$PLXV0,BRGPS,R"));
  LXV7_EXPNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
  Poco::Thread::sleep(CHANGE_DELAY);
  d->Com->WriteString(szTmp);
  Poco::Thread::sleep(CHANGE_DELAY);
}


  if(bLinkEnable)
  {
	#if TESTBENCH
	StartupStore(TEXT("V7: enable LX V7 direct Link %s"), NEWLINE);
	#endif
	LXV7_EXP_iPDABaudrate = d->Com->GetBaudrate();

	_stprintf(szTmp, TEXT("$PLXV0,CONNECTION,W,DIRECT"));
	LXV7_EXPNMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
	Poco::Thread::sleep(CHANGE_DELAY);
    if(LXV7_EXP_iPDABaudrate != LXV7_EXP_iGPSBaudrate)
    {
	  d->Com->SetBaudrate(LXV7_EXP_iGPSBaudrate);
	#if TESTBENCH
	  StartupStore(TEXT("V7: Set Baudrate %i %s"),LXV7_EXP_iGPSBaudrate, NEWLINE);
	#endif
	  Poco::Thread::sleep(CHANGE_DELAY);
    }
	Poco::Thread::sleep(CHANGE_DELAY);
  }
  else
  {
	Poco::Thread::sleep(CHANGE_DELAY);

    if(LXV7_EXP_iPDABaudrate != LXV7_EXP_iGPSBaudrate)
    {
	#if TESTBENCH
	  StartupStore(TEXT("V7: Set Baudrate %i %s"),LXV7_EXP_iPDABaudrate, NEWLINE);
	#endif
	  d->Com->SetBaudrate(LXV7_EXP_iPDABaudrate);
	  Poco::Thread::sleep(CHANGE_DELAY);
    }

	#if TESTBENCH
	StartupStore(TEXT("v7: Return from V7 link %s"), NEWLINE);
	#endif
	_stprintf(szTmp, TEXT("$PLXV0,CONNECTION,W,VSEVEN"));
	LXV7_EXPNMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
	Poco::Thread::sleep(CHANGE_DELAY);
	Poco::Thread::sleep(CHANGE_DELAY);

  }

  return true;
}

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



int LXV7_EXPNMEAddCheckSumStrg( TCHAR szStrg[] )
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


bool DevLXV7_EXP::SetupLX_Sentence(PDeviceDescriptor_t d)
{
TCHAR  szTmp[254];


_stprintf(szTmp, TEXT("$PLXV0,NMEARATE,W,2,5,0,10,1,0,0"));
  LXV7_EXPNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);


  return true;
}

long LXV7_EXPBaudrate(int iIdx)
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



BOOL LXV7_EXPPutMacCready(PDeviceDescriptor_t d, double MacCready){
  TCHAR  szTmp[254];
  if(LXV7_EXP_bValid == false) {
    return false;
  }

  _stprintf(szTmp, TEXT("$PLXV0,MC,W,%3.1f"), MacCready );

  LXV7_EXPNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);


  LXV7_EXP_MacCreadyUpdateTimeout = 5;


  return true;

}


BOOL LXV7_EXPPutBallast(PDeviceDescriptor_t d, double Ballast){
TCHAR  szTmp[254];
if(LXV7_EXP_bValid == false)
 {
  return false;
 }
   double fLXBalFact = CalculateLXBalastFactor(Ballast);
  _stprintf(szTmp, TEXT("$PLXV0,BAL,W,%4.2f"),fLXBalFact);

 LXV7_EXPNMEAddCheckSumStrg(szTmp);
 d->Com->WriteString(szTmp);

 LXV7_EXP_BallastUpdateTimeout =10;
 return(TRUE);

}


BOOL LXV7_EXPPutBugs(PDeviceDescriptor_t d, double Bugs){
TCHAR  szTmp[254];

if(LXV7_EXP_bValid == false)
  return false;


    //  _stprintf(szTmp, TEXT("$PLXV0,BUGS,W,%3.1f"),(1.00-Bugs)*100.0);
    _stprintf(szTmp, TEXT("$PLXV0,BUGS,W,%3.1f"),CalculateLXBugs(Bugs));
    LXV7_EXPNMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);

    LXV7_EXP_BugsUpdateTimeout = 5;
    return(TRUE);

}


bool  DevLXV7_EXP::PutGPRMB(PDeviceDescriptor_t d)
{

//RMB - The recommended minimum navigation sentence is sent whenever a route or a goto is active.
//      On some systems it is sent all of the time with null data.
//      The Arrival alarm flag is similar to the arrival alarm inside the unit and can be decoded to
//      drive an external alarm.
//      Note: the use of leading zeros in this message to preserve the character spacing.
//      This is done, I believe, because some autopilots may depend on exact character spacing.
//
//     $GPRMB,A,0.66,L,003,004,4917.24,N,12309.57,W,001.3,052.5,000.5,V*20
//where:
//           RMB          Recommended minimum navigation information
//           A            Data status A = OK, V = Void (warning)
//           0.66,L       Cross-track error (nautical miles, 9.99 max),
//                                steer Left to correct (or R = right)
//           003          Origin waypoint ID
//           004          Destination waypoint ID
//           4917.24,N    Destination waypoint latitude 49 deg. 17.24 min. N
//           12309.57,W   Destination waypoint longitude 123 deg. 09.57 min. W
//           001.3        Range to destination, nautical miles (999.9 max)
//           052.5        True bearing to destination
//           000.5        Velocity towards destination, knots
//           V            Arrival alarm  A = arrived, V = not arrived
//           *20          checksum
static int old_overindex = -99;
static int old_overmode = -99;
int overindex = GetOvertargetIndex();
int overmode  = OvertargetMode;

bool bTaskpresent = false; //ValidTaskPoint(0);
if(bTaskpresent)
  if(ValidTaskPoint(ActiveTaskPoint))
    overindex = Task[ActiveTaskPoint].Index;


#define SEND_ON_CHANGE_ONLY
#ifdef SEND_ON_CHANGE_ONLY
if(overindex < 0)               /* vaslid waypoint */
  return -1;
if(overindex == old_overindex)  /* same as before */
  if(overmode == old_overmode)  /* and same mode  */
    return 0;
#endif

old_overindex = overindex;
old_overmode  = overmode;
TCHAR  szTmp[512];


int DegLat, DegLon;
double MinLat, MinLon;
char NoS, EoW;

if (!ValidWayPoint(overindex)) return TRUE;

DegLat = (int)WayPointList[overindex].Latitude;
MinLat = WayPointList[overindex].Latitude - DegLat;
NoS = 'N';
if((MinLat<0) || ((MinLat-DegLat==0) && (DegLat<0)))
  {
    NoS = 'S';
    DegLat *= -1; MinLat *= -1;
  }
MinLat *= 60;

DegLon = (int)WayPointList[overindex].Longitude ;
MinLon = WayPointList[overindex].Longitude  - DegLon;
EoW = 'E';
if((MinLon<0) || ((MinLon-DegLon==0) && (DegLon<0)))
  {
    EoW = 'W';
    DegLon *= -1; MinLon *= -1;
  }
MinLon *=60;

#define SET_VALUES_BY_RMB
#ifdef SET_VALUES_BY_RMB
  if(bTaskpresent)
  {
#ifdef NO_RMB_BUT_PLXVTARG
	                     //  $GPRMB,A,0.66,L,003,004 ,4917.24   ,N ,12309.57  ,W ,01.3,52.5,00.5,V*20
    _stprintf( szTmp,  TEXT("$GPRMB,A,0.00,R,XXX,%s%s,%02d%05.2f,%c,%03d%05.2f,%c,%.1f,%.1f,%.1f,V"),
               MsgToken(1323), // LKTOKEN _@M1323_ "T>"
	           WayPointList[overindex].Name,
               DegLat, MinLat, NoS, DegLon, MinLon, EoW,
               WayPointCalc[overindex].Distance * TONAUTICALMILES,
               WayPointCalc[overindex].Bearing,
               WayPointCalc[overindex].VGR * TOKNOTS
             );
    LXV7_EXPNMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);
#if TESTBENCH
    StartupStore(TEXT("V7: %s"),szTmp);
#endif
#else
    _stprintf( szTmp,  TEXT("$PLXVTARG,%s%s,%02d%05.2f,%c,%03d%05.2f,%c,%i "),
               MsgToken(1323), // LKTOKEN _@M1323_ "T>"
	           WayPointList[overindex].Name,
               DegLat, MinLat, NoS, DegLon, MinLon, EoW,
               (int) (WayPointList[overindex].Altitude +0.5)
             );
    LXV7_EXPNMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);
#endif
#if TESTBENCH
    StartupStore(TEXT("V7: %s"),szTmp);
#endif

  }
  else
  {                          //  $GPRMB,A,0.66,L,003,004 ,4917.24   ,N ,12309.57  ,W ,01.3,52.5,00.5,V*20
#ifdef NO_RMB_BUT_PLXVTARG
    _stprintf( szTmp,  TEXT("$GPRMB,A,0.00,R,XXX,%s%s,%02d%05.2f,%c,%03d%05.2f,%c,%.1f,%.1f,%.1f,V"),
	           GetOvertargetHeader(),
		       WayPointList[overindex].Name,
	           DegLat, MinLat, NoS, DegLon, MinLon, EoW,
	           WayPointCalc[overindex].Distance * TONAUTICALMILES,
	           WayPointCalc[overindex].Bearing,
	           WayPointCalc[overindex].VGR * TOKNOTS
	         );
	LXV7_EXPNMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
#if TESTBENCH
	StartupStore(TEXT("V7: %s"),szTmp);
#endif
#else
	_stprintf( szTmp,  TEXT("$PLXVTARG,%s%s,%02d%05.2f,%c,%03d%05.2f,%c,%i "),
	           GetOvertargetHeader(),
		       WayPointList[overindex].Name,
	           DegLat, MinLat, NoS, DegLon, MinLon, EoW,
	           (int)(WayPointList[overindex].Altitude +0.5)
	         );
	LXV7_EXPNMEAddCheckSumStrg(szTmp);
	d->Com->WriteString(szTmp);
#endif
#if TESTBENCH
	StartupStore(TEXT("V7: %s"),szTmp);
#endif
  }



#else
  /****************
   * SYm style
   ****************/
  _stprintf( szTmp,  TEXT("$PFLX4,,,,%.f,%.f,,,,0,"),   WayPointCalc[overindex].Distance,  -WayPointCalc[overindex].AltReqd[AltArrivMode]*TOFEET );
  LXV7_EXPNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
#if TESTBENCH
  StartupStore(TEXT("V7: %s"),szTmp);
#endif

#endif

return(true);
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
BOOL DevLXV7_EXP::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{

  static int i=40;
  TCHAR  szTmp[256];


  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }

  if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0)
  {
	if(iLXV7_EXP_RxUpdateTime > 0)
	{
		iLXV7_EXP_RxUpdateTime--;
	}
	else
	{
	  if(fabs(LXV7_EXP_oldMC - MACCREADY)> 0.005f)
	  {
//		LXV7_EXPPutMacCready( d,  MACCREADY);
		LXV7_EXP_oldMC = MACCREADY;
		LXV7_EXP_MacCreadyUpdateTimeout = 2;
      }
	}
  }

  PutGPRMB(d);

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
      LXV7_EXPNMEAddCheckSumStrg(szTmp);
      d->Com->WriteString(szTmp);
    }


    int QFE = (int)QFEAltitudeOffset;
    if(QFE != oldQFEOff)
    {
  	  oldQFEOff = QFE;
      _stprintf(szTmp, TEXT("$PLXV0,ELEVATION,W,%i"),(int)(QFEAltitudeOffset));
      LXV7_EXPNMEAddCheckSumStrg(szTmp);
  //    d->Com->WriteString(szTmp);
    }
  }
  if(LXV7_EXP_iGPSBaudrate ==0)
  {
    _stprintf(szTmp, TEXT("$PLXV0,BRGPS,R"));
    LXV7_EXPNMEAddCheckSumStrg(szTmp);
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
      else
        if (_tcsncmp(_T("$LXWP1"), sentence, 6) == 0)
          return LXWP1(d, sentence + 7, info);
	else
	  if (_tcsncmp(_T("$LXWP0"), sentence, 6) == 0)
	    return LXWP0(d, sentence + 7, info);
#ifdef OLD_LX_SENTENCES
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
bool DevLXV7_EXP::LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
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




  if (ParToDouble(sentence, 10, &info->ExternalWindDirection) &&
      ParToDouble(sentence, 11, &info->ExternalWindSpeed))
  {
    info->ExternalWindSpeed /=  TOKPH;
    info->ExternalWindAvailable = TRUE;
  }

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
bool DevLXV7_EXP::LXWP1(PDeviceDescriptor_t d, const TCHAR* String, NMEA_INFO* pGPS)
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
TCHAR ctemp[180];
static int NoMsg=0;
static int oldSerial=0;
if(_tcslen(String) < 180)
  if((( d->SerialNumber == 0)  || ( d->SerialNumber != oldSerial)) && (NoMsg < 5))
  {
	NoMsg++ ;
    NMEAParser::ExtractParameter(String,ctemp,0);
    lk::snprintf(d->Name, _T("%s"),ctemp);
    StartupStore(_T(". %s\n"),ctemp);

	NMEAParser::ExtractParameter(String,ctemp,1);
	d->SerialNumber= (int)StrToDouble(ctemp,NULL);
	oldSerial = d->SerialNumber;
	_stprintf(ctemp, _T("%s Serial Number %i"), d->Name, d->SerialNumber);
	StartupStore(_T(". %s\n"),ctemp);

	NMEAParser::ExtractParameter(String,ctemp,2);
	d->SoftwareVer= StrToDouble(ctemp,NULL);
	_stprintf(ctemp, _T("%s Software Vers.: %3.2f"), d->Name, d->SoftwareVer);
	StartupStore(_T(". %s\n"),ctemp);

	NMEAParser::ExtractParameter(String,ctemp,3);
    d->HardwareId= (int)(StrToDouble(ctemp,NULL)*10);
	_stprintf(ctemp, _T("%s Hardware Vers.: %3.2f"), d->Name, (double)(d->HardwareId)/10.0);
	StartupStore(_T(". %s\n"),ctemp);
    _stprintf(ctemp, _T("%s (#%i) DETECTED"), d->Name, d->SerialNumber);
    DoStatusMessage(ctemp);
    _stprintf(ctemp, _T("SW Ver: %3.2f HW Ver: %3.2f "),  d->SoftwareVer, (double)(d->HardwareId)/10.0);
    DoStatusMessage(ctemp);
  }
  // nothing to do
#endif  
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
bool DevLXV7_EXP::LXWP2(PDeviceDescriptor_t, const TCHAR* sentence, NMEA_INFO*)
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
if(LXV7_EXP_MacCreadyUpdateTimeout > 0)
{
	LXV7_EXP_MacCreadyUpdateTimeout--;
}
else
  if (ParToDouble(sentence, 0, &fTmp))
  {
	iTmp =(int) (fTmp*100.0+0.5f);
	fTmp = (double)(iTmp)/100.0;
	LXV7_EXP_bValid = true;
	if(fabs(MACCREADY - fTmp)> 0.001)
	{
	  CheckSetMACCREADY(fTmp);
	  iLXV7_EXP_RxUpdateTime =5;
	}
  }


if(LXV7_EXP_BallastUpdateTimeout > 0)
{
	LXV7_EXP_BallastUpdateTimeout--;
}
else
  if (ParToDouble(sentence, 1, &fTmp))
  {
    double newBallast = CalculateBalastFromLX(fTmp);
    if(fabs(newBallast- BALLAST) > 0.01 )
    {
      CheckSetBallast(newBallast);
      iLXV7_EXP_RxUpdateTime = 5;
    }
  }

if(LXV7_EXP_BugsUpdateTimeout > 0)
{
  LXV7_EXP_BugsUpdateTimeout--;
}
else
  if(ParToDouble(sentence, 2, &fTmp))
  {
    double newBug = CalculateBugsFromLX(fTmp);
	if(  fabs(newBug -BUGS) >= 0.03)
    {
      CheckSetBugs(newBug);
      iLXV7_EXP_RxUpdateTime = 5;
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
bool DevLXV7_EXP::LXWP3(PDeviceDescriptor_t, const TCHAR*, NMEA_INFO*)
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




bool DevLXV7_EXP::LXWP4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
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


  return(true);
} // LXWP4()



bool DevLXV7_EXP::PLXVF(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{

  double alt=0, airspeed=0;


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
    UpdateBaroSource( info, 0, d, QNEAltitudeToQNHAltitude(alt));
    if (airspeed>0) {
      info->TrueAirspeed = TrueAirSpeed(airspeed, alt);
    }
  }

  double Vario = 0;
  if (ParToDouble(sentence, 4, &Vario))
  {
    UpdateVarioSource(*info, *d, Vario);
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


bool DevLXV7_EXP::PLXVS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
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



bool DevLXV7_EXP::PLXV0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR  szTmp1[80], szTmp2[80];




  NMEAParser::ExtractParameter(sentence,szTmp1,1);
  if  (_tcscmp(szTmp1,_T("W"))!=0)  // no write flag received
	 return false;

  NMEAParser::ExtractParameter(sentence,szTmp1,0);
  if  (_tcscmp(szTmp1,_T("BRGPS"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	LXV7_EXP_iGPSBaudrate = LXV7_EXPBaudrate( (int)( (StrToDouble(szTmp2,NULL))+0.1 ) );
	return true;
  }


  if (_tcscmp(szTmp1,_T("BRPDA"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	LXV7_EXP_iPDABaudrate = LXV7_EXPBaudrate( (int) StrToDouble(szTmp2,NULL));
	return true;
  }

  NMEAParser::ExtractParameter(sentence,szTmp1,0);
  if  (_tcscmp(szTmp1,_T("QNH"))==0)
  {
	NMEAParser::ExtractParameter(sentence,szTmp2,2);
	UpdateQNH((StrToDouble(szTmp2,NULL))/100.0);
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
