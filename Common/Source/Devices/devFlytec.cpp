/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devFlytec.h"

extern double EastOrWest(double in, TCHAR EoW);
extern double NorthOrSouth(double in, TCHAR NoS);
extern double MixedFormatToDegrees(double mixed);

static
BOOL FLYSEN(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS);

static
BOOL FlytecParseNMEA(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS){
  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }

  if(strncmp("$FLYSEN", String, 7)==0) {
    return FLYSEN(d, &String[8], pGPS);
  }

  return FALSE;
}


void FlytecInstall(DeviceDescriptor_t* d) {
  d->ParseNMEA = FlytecParseNMEA;
}

static
BOOL FLYSEN(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS)
{

  char ctemp[80];
  static int offset=-1;

  d->nmeaParser.connected = true;

  // firmware 3.31h no offset
  // firmware 3.32  1 offset
  // Determine firmware version, assuming it will not change in the session!
  if (offset<0) {
	NMEAParser::ExtractParameter(String,ctemp,8);
	if ( (strcmp(ctemp, "A")==0) || (strcmp(ctemp, "V")==0))
		offset=0;
	else {
		NMEAParser::ExtractParameter(String,ctemp,9);
		if ( (strcmp(ctemp, "A")==0) || (strcmp(ctemp, "V")==0))
			offset=1;
		else
			return TRUE;
	}
  }

  // VOID GPS SIGNAL
  NMEAParser::ExtractParameter(String,ctemp,8+offset);
  if (strcmp(ctemp, "V")==0) {
	pGPS->NAVWarning=true;
	// GPSCONNECT=false; // 121127 NO!!
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
        pGPS->Latitude = tmplat;
        pGPS->Longitude = tmplon;
	pGPS->NAVWarning=false;
  }

  // GPS SPEED
  NMEAParser::ExtractParameter(String,ctemp,6+offset);
  pGPS->Speed = StrToDouble(ctemp,NULL)/10;

  // TRACK BEARING
  if (pGPS->Speed>1.0) {
	NMEAParser::ExtractParameter(String,ctemp,5+offset);
	pGPS->TrackBearing = AngleLimit360(StrToDouble(ctemp, NULL));
  }

  // HGPS
  NMEAParser::ExtractParameter(String,ctemp,7+offset);
  pGPS->Altitude = StrToDouble(ctemp,NULL);

  // ------------------------
  label_nogps:

  // SATS
  NMEAParser::ExtractParameter(String,ctemp,9+offset);
  pGPS->SatellitesUsed = (int) StrToDouble(ctemp,NULL);

  // DATE
  // Firmware 3.32 has got the date
  if (offset>0) {
	NMEAParser::ExtractParameter(String,ctemp,0);

  long gy = strtol(&ctemp[4], nullptr, 10) + 2000;
  ctemp[4] = '\0';
  long gm = strtol(&ctemp[2], nullptr, 10);
  ctemp[2] = '\0';
  long gd = strtol(&ctemp[0], nullptr, 10);

	if ( ((gy > 1980) && (gy <2100) ) && (gm != 0) && (gd != 0) ) {
		pGPS->Year = gy;
		pGPS->Month = gm;
		pGPS->Day = gd;
	}
  }

  // TIME
  // ignoring 00:00.00
  // We need to manage UTC time
  static int StartDay=-1;
  if (pGPS->SatellitesUsed>0) {
      NMEAParser::ExtractParameter(String,ctemp,0+offset);
      pGPS->Time = TimeModify(ctemp, pGPS, StartDay);
  }
  // TODO : check if TimeHasAdvanced check is needed (cf. Parser.cpp)

  // HPA from the pressure sensor
  //   NMEAParser::ExtractParameter(String,ctemp,10+offset);
  //   double ps = StrToDouble(ctemp,NULL)/100;
  //   pGPS->BaroAltitude = (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69;

  // HBAR 1013.25
  NMEAParser::ExtractParameter(String,ctemp,11+offset);
  double qne_altitude = StrToDouble(ctemp,NULL);
  UpdateBaroSource(pGPS, d, QNEAltitudeToQNHAltitude(qne_altitude));

  // VARIO
  NMEAParser::ExtractParameter(String,ctemp,12+offset);
  double Vario = StrToDouble(ctemp,NULL)/100;
  UpdateVarioSource(*pGPS, *d, Vario);

  // TAS
  NMEAParser::ExtractParameter(String,ctemp,13+offset);
  double vtas = StrToDouble(ctemp,NULL) / 10;
  pGPS->IndicatedAirspeed = IndicatedAirSpeed(vtas, qne_altitude);
  pGPS->TrueAirspeed = vtas;
  pGPS->AirspeedAvailable = (pGPS->IndicatedAirspeed >0);

  // ignore n.14 airspeed source

  // OAT
  NMEAParser::ExtractParameter(String,ctemp,15+offset);
  pGPS->OutsideAirTemperature = StrToDouble(ctemp,NULL);
  pGPS->TemperatureAvailable=TRUE;

  // ignore n.16 baloon temperature

  // BATTERY PERCENTAGES
  NMEAParser::ExtractParameter(String,ctemp,17+offset);
  pGPS->ExtBatt1_Voltage = StrToDouble(ctemp,NULL)+1000;
  NMEAParser::ExtractParameter(String,ctemp,18+offset);
  pGPS->ExtBatt2_Voltage = StrToDouble(ctemp,NULL)+1000;

  TriggerGPSUpdate();

  return TRUE;
}
