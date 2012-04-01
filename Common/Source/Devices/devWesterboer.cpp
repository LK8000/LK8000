/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"
#include "McReady.h"
#include "devWesterboer.h"
#include "InputEvents.h"

static BOOL PWES0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL PWES1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
BOOL devWesterboerPutMacCready(PDeviceDescriptor_t d, double Mc);
BOOL devWesterboerPutBallast(PDeviceDescriptor_t d, double Ballast);
BOOL devWesterboerPutBugs(PDeviceDescriptor_t d, double Bus);
BOOL devWesterboerPutWingload(PDeviceDescriptor_t d, double fWingload);

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
static int i =0;
if (i++ > 10)
	i=0;
TCHAR  szTmp[254];
  _stprintf(szTmp, TEXT("$PWES4,1,,,,,,,,"));
  NMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  return true;
}



static BOOL WesterboerParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  (void)d;





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
      return PWES0(d, &String[7], GPS_INFO);

    } 
  else
    if(_tcsncmp(TEXT("$PWES1"), String, 6)==0)
    {
	  if( iReceiveSuppress > 0)
	  {
		iReceiveSuppress--;
		return false;
	  }
      return PWES1(d, &String[7], GPS_INFO);
    }
  return FALSE;

}

static BOOL WesterboerIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL WesterboerLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL WesterboerInstall(PDeviceDescriptor_t d){

  #if ALPHADEBUG
  StartupStore(_T(". WESTERBOER device installed%s"),NEWLINE);
  #endif

  _tcscpy(d->Name, TEXT("Westerboer"));
  d->ParseNMEA = WesterboerParseNMEA;
  d->PutMacCready = devWesterboerPutMacCready;
  d->PutBugs = devWesterboerPutBugs;
  d->PutBallast = devWesterboerPutBallast;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = WesterboerLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = NULL;
  d->IsBaroSource = WesterboerIsBaroSource;

  return(TRUE);

}

BOOL WesterboerRegister(void){
  return(devRegister(
    TEXT("Westerboer VW1150"),
    (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    WesterboerInstall
  ));
}


static BOOL PWES0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
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

  TCHAR ctemp[80];
  double vtas, vias;
  double altqne, altqnh;
  static bool initqnh=true;


  // instant vario
  NMEAParser::ExtractParameter(String,ctemp,1);
  GPS_INFO->Vario = StrToDouble(ctemp,NULL)/10;
  GPS_INFO->VarioAvailable = TRUE;

  // netto vario
  NMEAParser::ExtractParameter(String,ctemp,3);
  if (ctemp[0] != '\0') {
	GPS_INFO->NettoVario = StrToDouble(ctemp,NULL)/10;
	GPS_INFO->NettoVarioAvailable = TRUE;
  } else
	GPS_INFO->NettoVarioAvailable = FALSE;


  // Baro altitudes. To be verified, because I have no docs from Westerboer of any kind.
  NMEAParser::ExtractParameter(String,ctemp,6);
  altqne = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,7);
  altqnh = StrToDouble(ctemp,NULL);

  // AutoQNH will take care of setting an average QNH if nobody does it for a while
  if (initqnh) {
	// if wester has qnh set by user qne and qnh are of course different
	if (altqne != altqnh) {
		QNH=FindQNH(altqne,altqnh);
        CAirspaceManager::Instance().QnhChangeNotify(QNH);
		StartupStore(_T(". Using WESTERBOER QNH %f%s"),QNH,NEWLINE);
		initqnh=false;
	} else {
		// if locally QNH was set, either by user of by AutoQNH, stop processing QNH from Wester
		if ( (QNH <= 1012) || (QNH>=1014)) initqnh=false;
		// else continue entering initqnh until somebody changes qnh in either Wester or lk8000
	}
  }

  if (d == pDevPrimaryBaroSource) {
      UpdateBaroSource( GPS_INFO, WESTERBOER,  AltitudeToQNHAltitude(altqne));
  }


  // IAS and TAS
  NMEAParser::ExtractParameter(String,ctemp,8);
  vias = StrToDouble(ctemp,NULL)/36;
  NMEAParser::ExtractParameter(String,ctemp,9);
  vtas = StrToDouble(ctemp,NULL)/36;

  if (vias >1) {
	GPS_INFO->TrueAirspeed = vtas;
	GPS_INFO->IndicatedAirspeed = vias;
	GPS_INFO->AirspeedAvailable = TRUE;
  } else
	GPS_INFO->AirspeedAvailable = FALSE;

  // external battery voltage
  NMEAParser::ExtractParameter(String,ctemp,10);
  GPS_INFO->ExtBatt1_Voltage = StrToDouble(ctemp,NULL)/10;

  // OAT
  NMEAParser::ExtractParameter(String,ctemp,11);
  GPS_INFO->OutsideAirTemperature = StrToDouble(ctemp,NULL)/10;
  GPS_INFO->TemperatureAvailable=TRUE;


  TriggerVarioUpdate();

  return TRUE;
}



//GEXTERN double BUGS;
//GEXTERN double BALLAST;
//GEXTERN int POLARID;
//GEXTERN double POLAR[POLARSIZE];
//GEXTERN double WEIGHTS[POLARSIZE];
//GEXTERN double POLARV[POLARSIZE];
//GEXTERN double POLARLD[POLARSIZE];

static BOOL PWES1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
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
    MACCREADY = fTemp;
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
    BUGS = fTemp;
    iWEST_RxUpdateTime = 5;
  }

  return TRUE;
}




BOOL devWesterboerPutMacCready(PDeviceDescriptor_t d, double Mc){
	  (void)d;


TCHAR  szTmp[254];
  _stprintf(szTmp, TEXT("$PWES4,,%d,,,,,,,"),(int)(Mc*10.0f+0.49f));
  NMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);

  return(TRUE);

}


BOOL devWesterboerPutWingload(PDeviceDescriptor_t d, double fWingload){
	  (void)d;

  TCHAR  szTmp[254];
    _stprintf(szTmp, TEXT("$PWES4,,,,%d,,,,,"),(int)(fWingload *10.0f+0.5f));
    NMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);

  return(TRUE);

}


BOOL devWesterboerPutBallast(PDeviceDescriptor_t d, double Ballast){
	  (void)d;

  return(TRUE);

}

BOOL devWesterboerPutBugs(PDeviceDescriptor_t d, double Bug){
	  (void)d;

iReceiveSuppress = 2;
  TCHAR  szTmp[254];
    _stprintf(szTmp, TEXT("$PWES4,,,,,%d,,,,"),(int)((1.0-Bug)*100.0+0.5));
    NMEAddCheckSumStrg(szTmp);
    d->Com->WriteString(szTmp);
  return(TRUE);

}


