/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Comm/UpdateQNH.h"
#include "devIlec.h"
#include "Comm/ExternalWind.h"


static
BOOL PILC(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS);

static
BOOL IlecParseNMEA(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS){
  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }

  if(strncmp("$PILC", String, 5)==0) {
      return PILC(d, &String[6], pGPS);
  }

  return FALSE;
}


void IlecInstall(DeviceDescriptor_t* d) {
  d->ParseNMEA = IlecParseNMEA;
}

static
BOOL PILC(DeviceDescriptor_t* d, const char* String, NMEA_INFO *pGPS)
{

  char ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);

  if (strcmp(ctemp, "PDA1")==0) {

	NMEAParser::ExtractParameter(String,ctemp,1);
	UpdateBaroSource( pGPS, d, QNEAltitudeToQNHAltitude(StrToDouble(ctemp, NULL)));

	NMEAParser::ExtractParameter(String,ctemp,2);
	double Vario = StrToDouble(ctemp,NULL);
	UpdateVarioSource(*pGPS, *d, Vario);

	NMEAParser::ExtractParameter(String,ctemp,4); // wind speed kph integer
	if (strlen(ctemp)!=0) {
		double wspeed = StrToDouble(ctemp,NULL);
		NMEAParser::ExtractParameter(String,ctemp,5); // confidence  0-100 percentage
		double wconfidence = StrToDouble(ctemp,NULL);
		if (wconfidence > 0) {
			NMEAParser::ExtractParameter(String,ctemp,3); // wind direction, integer
			double wfrom = StrToDouble(ctemp,NULL); //@ could also be the NMEA checksum!
			UpdateExternalWind(*pGPS, *d, Units::From(Units_t::unKiloMeterPerHour, wspeed), wfrom);
		}
	}

	return TRUE;
  }

  if (strcmp(ctemp, "SET")==0) {
	NMEAParser::ExtractParameter(String,ctemp,1);
	UpdateQNH(StrToDouble(ctemp,NULL));
	// StartupStore(_T("... SET QNH= %.1f\n"),QNH);

	return TRUE;
  }

  // Using the polar requires recalculating polar coefficients internally each time.. no thanks.
  // if (_tcscmp(ctemp,_T("POLAR"))==0) { }

  return TRUE;
}
