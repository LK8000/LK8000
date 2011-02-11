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

#include "devIlec.h"

#include "utils/heapcheck.h"

static BOOL PILC(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);

static BOOL IlecParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  (void)d;

  if(_tcsncmp(TEXT("$PILC"), String, 5)==0)
    {
      return PILC(d, &String[6], GPS_INFO);
    } 

  return FALSE;

}


static BOOL IlecIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL IlecIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL IlecLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL IlecInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("Ilec SN10"));
  d->ParseNMEA = IlecParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = IlecLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = IlecIsGPSSource;
  d->IsBaroSource = IlecIsBaroSource;

  return(TRUE);

}


BOOL IlecRegister(void){
  return(devRegister(
    TEXT("Ilec SN10"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    IlecInstall
  ));
}


static BOOL PILC(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{

  TCHAR ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);

  if (_tcscmp(ctemp,_T("PDA1"))==0) {

	NMEAParser::ExtractParameter(String,ctemp,1);
	#if DUALBARO
	if (d == pDevPrimaryBaroSource) {
		GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(StrToDouble(ctemp,NULL));
		GPS_INFO->BaroAltitudeAvailable = TRUE;
	}
	#else
	GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(StrToDouble(ctemp,NULL));
	GPS_INFO->BaroAltitudeAvailable = TRUE;
	#endif

	NMEAParser::ExtractParameter(String,ctemp,2);
	GPS_INFO->Vario = StrToDouble(ctemp,NULL);

	NMEAParser::ExtractParameter(String,ctemp,3); // wind direction, integer
	double wfrom;
	wfrom = StrToDouble(ctemp,NULL); //@ could also be the NMEA checksum!
	NMEAParser::ExtractParameter(String,ctemp,4); // wind speed kph integer

	if (_tcslen(ctemp)!=0) {
		double wspeed, wconfidence;
		wspeed = StrToDouble(ctemp,NULL);
		NMEAParser::ExtractParameter(String,ctemp,5); // confidence  0-100 percentage
		wconfidence = StrToDouble(ctemp,NULL);
		// StartupStore(_T(".... WIND from %.0f speed=%.0f kph  confidence=%.0f\n"),wfrom,wspeed,wconfidence);

		SetWindEstimate(wspeed/TOKPH, wfrom,(int)(wconfidence/100)*10);
                CALCULATED_INFO.WindSpeed=wspeed/TOKPH;
                CALCULATED_INFO.WindBearing=wfrom;


	} 
		// else StartupStore(_T(".. NO WIND\n"));
	
	
	GPS_INFO->VarioAvailable = TRUE;
	TriggerVarioUpdate();
	return TRUE;
  }

  if (_tcscmp(ctemp,_T("SET"))==0) {
	NMEAParser::ExtractParameter(String,ctemp,1);
	QNH = StrToDouble(ctemp,NULL);
	// StartupStore(_T("... SET QNH= %.1f\n"),QNH);
#ifdef LKAIRSPACE
	CAirspaceManager::instance()->QnhChangeNotify(QNH);
#else
	AirspaceQnhChangeNotify(QNH);
#endif

	return TRUE;
  }

  // Using the polar requires recalculating polar coefficients internally each time.. no thanks.
  // if (_tcscmp(ctemp,_T("POLAR"))==0) { } 

  return TRUE;
}
