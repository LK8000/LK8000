/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "devLK8EX1.h"

extern bool UpdateBaroSource(NMEA_INFO* pGPS, const short parserid, const PDeviceDescriptor_t d, const double fAlt);

static BOOL LK8EX1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

BOOL LK8EX1ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){

  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  if(_tcsncmp(TEXT("$LK8EX1"), String, 7)==0)
    {
      return LK8EX1(d, &String[8], pGPS);
    } 

  return FALSE;

}

BOOL LK8EX1IsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


BOOL LK8EX1LinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL LK8EX1Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("LK8EX1"));
  d->ParseNMEA = LK8EX1ParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = LK8EX1LinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = NULL;
  d->IsBaroSource = LK8EX1IsBaroSource;

  return(TRUE);

}


BOOL LK8EX1Register(void){
  return(devRegister(
    TEXT("LK8EX1"),
    (1l << dfBaroAlt)
    | (1l << dfVario),
    LK8EX1Install
  ));
}

/*
	LK8000 EXTERNAL INSTRUMENT SERIES 1 - NMEA SENTENCE: LK8EX1
	VERSION A, 110217

	LK8EX1,pressure,altitude,vario,temperature,battery,*checksum

	Field 0, raw pressure in hPascal:
		hPA*100 (example for 1013.25 becomes  101325) 
		no padding (987.25 becomes 98725, NOT 098725)
		If no pressure available, send 999999 (6 times 9)
		If pressure is available, field 1 altitude will be ignored

	Field 1, altitude in meters, relative to QNH 1013.25
		If raw pressure is available, this value will be IGNORED (you can set it to 99999
		but not really needed)!
		(if you want to use this value, set raw pressure to 999999)
		This value is relative to sea level (QNE). We are assuming that
		currently at 0m altitude pressure is standard 1013.25.
		If you cannot send raw altitude, then send what you have but then
		you must NOT adjust it from Basic Setting in LK.
		Altitude can be negative
		If altitude not available, and Pressure not available, set Altitude
		to 99999  (5 times 9)
		LK will say "Baro altitude available" if one of fields 0 and 1 is available.

	Field 2, vario in cm/s
		If vario not available, send 9999  (4 times 9)
		Value can also be negative

	Field 3, temperature in C , can be also negative
		If not available, send 99

	Field 4, battery voltage or charge percentage
		Cannot be negative
		If not available, send 999 (3 times 9)
		Voltage is sent as float value like: 0.1 1.4 2.3  11.2 
		To send percentage, add 1000. Example 0% = 1000
		14% = 1014 .  Do not send float values for percentages.
		Percentage should be 0 to 100, with no decimals, added by 1000!

*/

static BOOL LK8EX1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{

  TCHAR ctemp[80];
  bool havebaro=false;

  // HPA from the pressure sensor
  NMEAParser::ExtractParameter(String,ctemp,0);
  double ps = StrToDouble(ctemp,NULL);
  if (ps!=999999) {
	ps/=100;
	UpdateBaroSource( pGPS, 0,d, (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69);
  }

  // QNE
  if (!havebaro) {
	NMEAParser::ExtractParameter(String,ctemp,1);
	double ba = StrToDouble(ctemp,NULL);
	if (ba!=99999) {
	    UpdateBaroSource( pGPS, 0,d,  AltitudeToQNHAltitude(ba));
	}
  }


  // VARIO
  NMEAParser::ExtractParameter(String,ctemp,2);
  double va = StrToDouble(ctemp,NULL);
  if (va != 9999) {
	pGPS->Vario = va/100;
	pGPS->VarioAvailable = TRUE;
  } else
	pGPS->VarioAvailable = FALSE;

  // OAT
  NMEAParser::ExtractParameter(String,ctemp,3);
  double ta = StrToDouble(ctemp,NULL);
  if (ta != 99) {
	pGPS->OutsideAirTemperature = ta;
	pGPS->TemperatureAvailable=TRUE;
  }

  // BATTERY PERCENTAGES
  NMEAParser::ExtractParameter(String,ctemp,4);
  double voa = StrToDouble(ctemp,NULL);
  if (voa!=999) {
	pGPS->ExtBatt1_Voltage = voa;
  }



  // currently unused in LK, but ready for next future
  TriggerVarioUpdate();

  return TRUE;
}
