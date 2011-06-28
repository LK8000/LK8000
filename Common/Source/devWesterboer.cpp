/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"

#include "devWesterboer.h"

#include "utils/heapcheck.h"

static BOOL PWES0(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL WesterboerParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  (void)d;

  if(_tcsncmp(TEXT("$PWES0"), String, 6)==0)
    {
      return PWES0(d, &String[7], GPS_INFO);
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
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
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
  #if ALPHADEBUG
  StartupStore(_T(". Westerboer device registered%s"),NEWLINE);
  #endif
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


	A	??
	B	vario *10			= 2.2 m/s
	C	average vario *10		= 1.0 m/s
	D	netto vario *10			= 1.8 m/s
	E	average netto vario *10		= 1.7 m/s
	F	??? MC *10 stf ???		= 0.6 MC
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
	GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(altqne);
	GPS_INFO->BaroAltitudeAvailable = TRUE;
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

