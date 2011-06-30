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

#include "devFlytec.h"

#include "utils/heapcheck.h"

static BOOL FLYSEN(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);

static BOOL FlytecParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  (void)d;

  if(_tcsncmp(TEXT("$FLYSEN"), String, 7)==0)
    {
      return FLYSEN(d, &String[8], GPS_INFO);
    } 

  return FALSE;

}


static BOOL FlytecIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL FlytecIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL FlytecLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL FlytecInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("Flytec/FLYSEN"));
  d->ParseNMEA = FlytecParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = FlytecLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = FlytecIsGPSSource;
  d->IsBaroSource = FlytecIsBaroSource;

  return(TRUE);

}


BOOL FlytecRegister(void){
  return(devRegister(
    TEXT("Flytec/FLYSEN"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    FlytecInstall
  ));
}

// TODO use GPSUtils for this stuff
double EastOrWest(double in, TCHAR EoW)
{
  if(EoW == 'W')
    return -in;
  else
    return in;
}
double NorthOrSouth(double in, TCHAR NoS)
{
  if(NoS == 'S')
    return -in;
  else
    return in;
}
double MixedFormatToDegrees(double mixed)
{
  double mins, degrees;

  degrees = (int) (mixed/100);
  mins = (mixed - degrees*100)/60;

  return degrees+mins;
}



static BOOL FLYSEN(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{

  TCHAR ctemp[80];
  double vtas;
  static int offset=-1;


  // firmware 3.31h no offset
  // firmware 3.32  1 offset
  // Determine firmware version, assuming it will not change in the session!
  if (offset<0) {
	NMEAParser::ExtractParameter(String,ctemp,8);
	if ( (_tcscmp(ctemp,_T("A"))==0) || (_tcscmp(ctemp,_T("V"))==0))
		offset=0;
	else {
		NMEAParser::ExtractParameter(String,ctemp,9);
		if ( (_tcscmp(ctemp,_T("A"))==0) || (_tcscmp(ctemp,_T("V"))==0))
			offset=1;
		else 
			return TRUE;
	}
  }

  // VOID GPS SIGNAL
  NMEAParser::ExtractParameter(String,ctemp,8+offset);
  if (_tcscmp(ctemp,_T("V"))==0) {
	GPS_INFO->NAVWarning=true;
	GPSCONNECT=false;
	goto label_nogps;
  }
  // ------------------------

  double tmplat;
  double tmplon;

  NMEAParser::ExtractParameter(String,ctemp,1+offset);
  tmplat = MixedFormatToDegrees(StrToDouble(ctemp, NULL));
  NMEAParser::ExtractParameter(String,ctemp,2+offset);
  tmplat = NorthOrSouth(tmplat, ctemp[0]);

  NMEAParser::ExtractParameter(String,ctemp,3+offset);
  tmplon = MixedFormatToDegrees(StrToDouble(ctemp, NULL));
  NMEAParser::ExtractParameter(String,ctemp,4+offset);
  tmplon = EastOrWest(tmplon,ctemp[0]);

  if (!((tmplat == 0.0) && (tmplon == 0.0))) {
        GPS_INFO->Latitude = tmplat;
        GPS_INFO->Longitude = tmplon;
	GPS_INFO->NAVWarning=false;
	GPSCONNECT=true;
  }

  // GPS SPEED
  NMEAParser::ExtractParameter(String,ctemp,6+offset);
  GPS_INFO->Speed = StrToDouble(ctemp,NULL)/10;

  // TRACK BEARING
  if (GPS_INFO->Speed>1.0) {
	NMEAParser::ExtractParameter(String,ctemp,5+offset);
	GPS_INFO->TrackBearing = AngleLimit360(StrToDouble(ctemp, NULL));
  }

  // HGPS
  NMEAParser::ExtractParameter(String,ctemp,7+offset);
  GPS_INFO->Altitude = StrToDouble(ctemp,NULL);

  // ------------------------
  label_nogps: 

  // SATS
  NMEAParser::ExtractParameter(String,ctemp,9+offset);
  GPS_INFO->SatellitesUsed = (int) StrToDouble(ctemp,NULL);

  // DATE
  // Firmware 3.32 has got the date 
  if (offset>0) {
	NMEAParser::ExtractParameter(String,ctemp,0);
	long gy, gm, gd;
	TCHAR *Stop;
        gy = _tcstol(&ctemp[4], &Stop, 10) + 2000;
        ctemp[4] = '\0';
        gm = _tcstol(&ctemp[2], &Stop, 10);
        ctemp[2] = '\0';
        gd = _tcstol(&ctemp[0], &Stop, 10);

	if ( ((gy > 1980) && (gy <2100) ) && (gm != 0) && (gd != 0) ) {
		GPS_INFO->Year = gy;
		GPS_INFO->Month = gm;
		GPS_INFO->Day = gd;
	}
  }

  // TIME
  // ignoring 00:00.00
  // And no UTC, since this is local time already.
  NMEAParser::ExtractParameter(String,ctemp,0+offset);
  double fixTime = StrToDouble(ctemp,NULL);
  if (fixTime>0 && GPS_INFO->SatellitesUsed>0) {
	double hours, mins,secs;
	hours = fixTime / 10000;
	GPS_INFO->Hour = (int)hours;
	mins = fixTime / 100;
	mins = mins - (GPS_INFO->Hour*100);
	GPS_INFO->Minute = (int)mins;
	secs = fixTime - (GPS_INFO->Hour*10000) - (GPS_INFO->Minute*100);
	GPS_INFO->Second = (int)secs;
  }


  // HPA from the pressure sensor
  //   NMEAParser::ExtractParameter(String,ctemp,10+offset);
  //   double ps = StrToDouble(ctemp,NULL)/100;
  //   GPS_INFO->BaroAltitude = (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69;

  // HBAR 1013.25
  NMEAParser::ExtractParameter(String,ctemp,11+offset);
  if (d == pDevPrimaryBaroSource) {
	GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(StrToDouble(ctemp,NULL));
	GPS_INFO->BaroAltitudeAvailable = TRUE;
  }

  // VARIO
  NMEAParser::ExtractParameter(String,ctemp,12+offset);
  GPS_INFO->Vario = StrToDouble(ctemp,NULL)/100;

  // TAS
  NMEAParser::ExtractParameter(String,ctemp,13+offset);
  vtas=StrToDouble(ctemp,NULL)/10;
  GPS_INFO->IndicatedAirspeed = vtas/AirDensityRatio(GPS_INFO->BaroAltitude);
  GPS_INFO->TrueAirspeed = vtas;
  if (GPS_INFO->IndicatedAirspeed >0) 
	GPS_INFO->AirspeedAvailable = TRUE;
  else 
	GPS_INFO->AirspeedAvailable = FALSE;

  // ignore n.14 airspeed source

  // OAT
  NMEAParser::ExtractParameter(String,ctemp,15+offset);
  GPS_INFO->OutsideAirTemperature = StrToDouble(ctemp,NULL);
  GPS_INFO->TemperatureAvailable=TRUE;

  // ignore n.16 baloon temperature  

  // BATTERY PERCENTAGES
  NMEAParser::ExtractParameter(String,ctemp,17+offset);
  GPS_INFO->ExtBatt1_Voltage = StrToDouble(ctemp,NULL)+1000;
  NMEAParser::ExtractParameter(String,ctemp,18+offset);
  GPS_INFO->ExtBatt2_Voltage = StrToDouble(ctemp,NULL)+1000;



  GPS_INFO->VarioAvailable = TRUE;

  // currently unused in LK, but ready for next future
  TriggerVarioUpdate();
  TriggerGPSUpdate();

  return TRUE;
}
