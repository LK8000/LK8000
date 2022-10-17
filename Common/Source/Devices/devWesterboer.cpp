/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "McReady.h"
#include "devWesterboer.h"
#include "InputEvents.h"
#include "Baro.h"
#include "Calc/Vario.h"

#define VW_BIDIRECTIONAL
static BOOL PWES0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);
static BOOL PWES1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);
static BOOL PWES2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);
BOOL devWesterboerPutMacCready(PDeviceDescriptor_t d, double Mc);
BOOL devWesterboerPutBallast(PDeviceDescriptor_t d, double Ballast);
BOOL devWesterboerPutBugs(PDeviceDescriptor_t d, double Bus);
BOOL devWesterboerPutWingload(PDeviceDescriptor_t d, double fWingload);
extern bool UpdateQNH(const double newqnh);


int oldSerial;
int SerialNumber =0;
int iReceiveSuppress = 0;

int iWEST_RxUpdateTime=0;
int iWEST_TxUpdateTime=0;

int NMEAddCheckSumStrg( TCHAR szStrg[] )
{
int i,iCheckSum=0;
TCHAR  szCheck[254];

 if(szStrg[0] != '$')
   return -1;

 iCheckSum = szStrg[1];
  for (i=2; i < (int)_tcslen(szStrg); i++)
  {
	    iCheckSum ^= szStrg[i];
  }
  _stprintf(szCheck,TEXT("*%X\r\n"),iCheckSum);
  _tcscat(szStrg,szCheck);
  return iCheckSum;
}

bool RequestInfos(PDeviceDescriptor_t d)
{
#ifdef  VW_BIDIRECTIONAL
TCHAR  szTmp[254];
static int i =0;
if (i++ > 5)
{
	i=0;

  _stprintf(szTmp, TEXT("$PWES4,1,,,,,,,,"));
  NMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
}
#ifdef DEVICE_SERIAL
else
{
  static int j =0;
  if(SerialNumber == 0)
  { // request hardware/serial informations
	if(j++> 10)
	{ j=0;
	  _stprintf(szTmp, TEXT("$PWES4,2,,,,,,,,"));
	  NMEAddCheckSumStrg(szTmp);
	  d->Com->WriteString(szTmp);
	}
  }
}
#endif
#endif
  return true;
}



static BOOL WesterboerParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){

  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }



/* this is for auto MC calculation, because we do not get a notification on changed
 * MC while changed by auto calac                                                     */

if(_tcsncmp(TEXT("$PWES0"), String, 6)==0)
{
  if(iWEST_RxUpdateTime > 0)
  {
	iWEST_RxUpdateTime--;
  }
  else
  {
	static  double oldMC =0;
    if(fabs(oldMC - MACCREADY)> 0.01f)
    {
      oldMC =  MACCREADY;
	  devWesterboerPutMacCready( d, MACCREADY);
    }

    static  double fOldWingLoad= -1.0;
    if( fabs(fOldWingLoad - GlidePolar::WingLoading)> 0.05f)
    {
      fOldWingLoad = GlidePolar::WingLoading;
      devWesterboerPutWingload( d, GlidePolar::WingLoading );
    }
  }
}



  if(_tcsncmp(TEXT("$PWES0"), String, 6)==0)
    {
	  RequestInfos(d);
      return PWES0(d, &String[7], pGPS);
    }
  else
    if(_tcsncmp(TEXT("$PWES1"), String, 6)==0)
    {
	  if( iReceiveSuppress > 0)
	  {
		iReceiveSuppress--;
		return false;
	  }
      return PWES1(d, &String[7], pGPS);
    }
    else
      if(_tcsncmp(TEXT("$PWES2"), String, 6)==0)
      {
	return PWES2(d, &String[7], pGPS);
      }
  return FALSE;

}

static BOOL WesterboerIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


void WesterboerInstall(PDeviceDescriptor_t d) {
  StartupStore(_T(". WESTERBOER device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("Westerboer"));
  d->ParseNMEA = WesterboerParseNMEA;
  d->PutMacCready = devWesterboerPutMacCready;
  d->PutBugs = devWesterboerPutBugs;
  d->PutBallast = devWesterboerPutBallast;
  d->IsBaroSource =  WesterboerIsBaroSource;
}

static BOOL PWES0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{
/*
	Sent by Westerboer VW1150  combining data stream from Flarm and VW1020.
	RMZ is being sent too, which is a problem.

	$PWES0,22,10,8,18,17,6,1767,1804,1073,1073,116,106
               A  B  C  D  E F  G    H    I    J   K    L


	A	Device              21 = VW1000, 21 = VW 1010, 22 = VW1020, 23 = VW1030
	B	vario *10			= 2.2 m/s
	C	average vario *10		= 1.0 m/s
	D	netto vario *10			= 1.8 m/s
	E	average netto vario *10		= 1.7 m/s
	F	 stf 		           = -999.. 999 (neg faster.. slower)
	G	baro altitude 			= 1767 m
	H	baro altitude calibrated by user?
	I	IAS kmh *10 			= 107.3 kmh
	J	TAS kmh *10  ?
	K	battery V *10			= 11.6 V
	L	OAT * 10			= 10.6 C

*/

  TCHAR ctemp[180];
  double vtas, vias;
  double altqne, altqnh;
  static bool initqnh=true;
#ifdef DEVICE_SERIAL
  static int NoMsg =0;
 // static int HardwareId = 0;



if(_tcslen(String) < 180)
  if(((d->SerialNumber == 0) || (oldSerial != SerialNumber)) && (NoMsg < 5))
  {
	NoMsg++ ;
    NMEAParser::ExtractParameter(String,ctemp,0);
    d->HardwareId= (int)StrToDouble(ctemp,NULL);
    switch (d->HardwareId)
    {
      case 21:  _tcscpy(d->Name, TEXT("VW1010")); break;
      case 22:  _tcscpy(d->Name, TEXT("VW1020")); break;
      case 23:  _tcscpy(d->Name, TEXT("VW1030")); break;
      default:  _tcscpy(d->Name, TEXT("Westerboer")); break;
    }
	StartupStore(_T(". %s\n"),ctemp);
	_stprintf(ctemp, _T("%s  DETECTED"), d->Name);
	oldSerial = d->SerialNumber;
	DoStatusMessage(ctemp);
	StartupStore(_T(". %s\n"),ctemp);
  }
#endif
  // instant vario
  NMEAParser::ExtractParameter(String,ctemp,1);
  double Vario = StrToDouble(ctemp,NULL)/10;
  UpdateVarioSource(*pGPS, *d, Vario);

  // netto vario
  NMEAParser::ExtractParameter(String,ctemp,3);
  if (ctemp[0] != '\0') {
	pGPS->NettoVario = StrToDouble(ctemp,NULL)/10;
	pGPS->NettoVarioAvailable = TRUE;
  } else
	pGPS->NettoVarioAvailable = FALSE;


  // Baro altitudes. To be verified, because I have no docs from Westerboer of any kind.
  NMEAParser::ExtractParameter(String,ctemp,6);
  altqne = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,7);
  altqnh = StrToDouble(ctemp,NULL);

  // AutoQNH will take care of setting an average QNH if nobody does it for a while
  if (initqnh) {
	// if wester has qnh set by user qne and qnh are of course different
	if (altqne != altqnh) {
		UpdateQNH(FindQNH(altqne,altqnh));
		StartupStore(_T(". Using WESTERBOER QNH %f%s"),QNH,NEWLINE);
		initqnh=false;
	} else {
		// if locally QNH was set, either by user of by AutoQNH, stop processing QNH from Wester
		if ( (QNH <= 1012) || (QNH>=1014)) initqnh=false;
		// else continue entering initqnh until somebody changes qnh in either Wester or lk8000
	}
  }

  UpdateBaroSource(pGPS, d, QNEAltitudeToQNHAltitude(altqne));


  // IAS and TAS
  NMEAParser::ExtractParameter(String,ctemp,8);
  vias = StrToDouble(ctemp,NULL)/36;
  NMEAParser::ExtractParameter(String,ctemp,9);
  vtas = StrToDouble(ctemp,NULL)/36;

  if (vias >1) {
	pGPS->TrueAirspeed = vtas;
	pGPS->IndicatedAirspeed = vias;
	pGPS->AirspeedAvailable = TRUE;
  } else
	pGPS->AirspeedAvailable = FALSE;

  // external battery voltage
  NMEAParser::ExtractParameter(String,ctemp,10);
  pGPS->ExtBatt1_Voltage = StrToDouble(ctemp,NULL)/10;

  // OAT
  NMEAParser::ExtractParameter(String,ctemp,11);
  pGPS->OutsideAirTemperature = StrToDouble(ctemp,NULL)/10;
  pGPS->TemperatureAvailable=TRUE;

  return TRUE;
}



//GEXTERN double BUGS;
//GEXTERN double BALLAST;
//GEXTERN int POLARID;
//GEXTERN double POLAR[POLARSIZE];
//GEXTERN double WEIGHTS[POLARSIZE];
//GEXTERN double POLARV[POLARSIZE];
//GEXTERN double POLARLD[POLARSIZE];

static BOOL PWES1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{
/*
	Sent by Westerboer VW1150  combining data stream from Flarm and VW1020.
	RMZ is being sent too, which is a problem.

	$PWES1,22,25,1,18,1,6,385,5
           A  B  C  D E F  G    H    I


0	A	Device              21 = VW1000, 21 = VW 1010, 22 = VW1020, 23 = VW1030
1	B	Mc *10			    25 = 2.5 m/s
2	C	Vario /STF switch 	0= Vario , 1 = STF
3	D	integration time	18	= 18 s
4	E	damping             1,2,3   ???
5	F	volume    		    0..8
6	G	Wing load 			200..999, 385 = 38,5kg/m2
7	H	Bugs                0..20%


*/

  TCHAR ctemp[80];
  double fTemp;
  int iTmp;

  // Get MC Ready
  NMEAParser::ExtractParameter(String,ctemp,1);
  iTmp = (int)StrToDouble(ctemp,NULL);
  fTemp = (double)iTmp/10.0f;
  if(fabs(fTemp-MACCREADY)> 0.05)
  {
    CheckSetMACCREADY(fTemp);
    iWEST_RxUpdateTime = 5;
  }

  // Get STF switch
  NMEAParser::ExtractParameter(String,ctemp,2);
  iTmp = (int)StrToDouble(ctemp,NULL);
#ifdef STF_SWITCH
  EnableExternalTriggerCruise = true;
static int  iOldVarioSwitch=0;
  if(iTmp != iOldVarioSwitch)
  {
	iOldVarioSwitch = iTmp;
    if(iTmp)
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
#endif



  NMEAParser::ExtractParameter(String,ctemp,6);
  iTmp = (int)StrToDouble(ctemp,NULL);
  fTemp = (double)iTmp/ 10.0f;
  if(fabs(fTemp-GlidePolar::WingLoading )> 0.05)
  {
    GlidePolar::WingLoading = fTemp;
    iWEST_RxUpdateTime = 5;
  }

  NMEAParser::ExtractParameter(String,ctemp,7);
  iTmp = (int) StrToDouble(ctemp,NULL);
  fTemp = (double)(100-iTmp)/100.0f;
  if(fabs(fTemp-BUGS)> 0.005)
  {
    CheckSetBugs(fTemp);
    iWEST_RxUpdateTime = 5;
  }

  return TRUE;
}




BOOL devWesterboerPutMacCready(PDeviceDescriptor_t d, double Mc){
	  (void)d;
#ifdef  VW_BIDIRECTIONAL
iReceiveSuppress = 1;
TCHAR  szTmp[254];
  _stprintf(szTmp, TEXT("$PWES4,,%d,,,,,,,"),(int)(Mc*10.0f+0.49f));
  NMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
#endif
  return(TRUE);

}


BOOL devWesterboerPutWingload(PDeviceDescriptor_t d, double fWingload){
	  (void)d;
#ifdef  VW_BIDIRECTIONAL
  TCHAR  szTmp[254];
  iReceiveSuppress = 1;
    _stprintf(szTmp, TEXT("$PWES4,,,,%d,,,,,"),(int)(fWingload *10.0f+0.5f));
    NMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);
#endif
  return(TRUE);

}


BOOL devWesterboerPutBallast(PDeviceDescriptor_t d, double Ballast){
	  (void)d;

  return(TRUE);

}

BOOL devWesterboerPutBugs(PDeviceDescriptor_t d, double Bug){
	  (void)d;
#ifdef  VW_BIDIRECTIONAL
iReceiveSuppress = 1;
  TCHAR  szTmp[254];
    _stprintf(szTmp, TEXT("$PWES4,,,,,%d,,,,"),(int)((1.0-Bug)*100.0+0.5));
    NMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);
#endif
  return(TRUE);

}

static BOOL PWES2(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{
//	$PWES2: Datenausgabe, Ger�teparameter
//	$PWES2,DD,SSSS,YY,FFFF*CS<CR><LF>
//	Symbol Inhalt Einheit Wertebereich Beispiel
//	DD Device 20=VW1000,
//	21=VW1010,
//	22=VW1020,
//	23=VW1030,
//	60=VW1150
//	22 f�r VW1020
//	SSSS Seriennummer 0 .. 9999
//	YY Baujahr 0 .. 99 10 = 2010
//	FFFF Firmware * 100 100 .. 9999 101 = 1.01
//	$PWES2,60,1234,12,3210*22
#ifdef DEVICE_SERIAL
TCHAR ctemp[180];
static int NoMsg=0;

if(_tcslen(String) < 180)
  if(((d->SerialNumber == 0) || (oldSerial	!= SerialNumber)) && (NoMsg < 5))
  {
	NoMsg++ ;
    NMEAParser::ExtractParameter(String,ctemp,0);
    d->HardwareId= (int)StrToDouble(ctemp,NULL);
    switch (d->HardwareId)
    {
      case 21:  _tcscpy(d->Name, TEXT("VW1010")); break;
      case 22:  _tcscpy(d->Name, TEXT("VW1020")); break;
      case 23:  _tcscpy(d->Name, TEXT("VW1030")); break;
      case 60:  _tcscpy(d->Name, TEXT("VW1150")); break;
      default:  _tcscpy(d->Name, TEXT("Westerboer")); break;
    }


	NMEAParser::ExtractParameter(String,ctemp,1);
	d->SerialNumber= (int)StrToDouble(ctemp,NULL);
	SerialNumber = d->SerialNumber;

	NMEAParser::ExtractParameter(String,ctemp,2);
	int Year = (int)(StrToDouble(ctemp,NULL));

	NMEAParser::ExtractParameter(String,ctemp,3);
	d->SoftwareVer= StrToDouble(ctemp,NULL)/100.0;



    _stprintf(ctemp, _T("%s (#%i) DETECTED"), d->Name, d->SerialNumber);
    DoStatusMessage(ctemp);
	StartupStore(_T(". %s\n"),ctemp);
    _stprintf(ctemp, _T("SW Ver:%3.2f  HW Ver:%i "),  d->SoftwareVer, Year);
    DoStatusMessage(ctemp);
	StartupStore(_T(". %s\n"),ctemp);
	oldSerial	=SerialNumber;
  }
  // nothing to do
#endif
  return(true);
} // PWES2()
