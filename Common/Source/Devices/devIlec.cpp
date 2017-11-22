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
  _tcscpy(d->Name, TEXT("Ilec SN10"));
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

	NMEAParser::ExtractParameter(String,ctemp,3); // wind direction, integer
	double wfrom;
	wfrom = StrToDouble(ctemp,NULL); //@ could also be the NMEA checksum!
	NMEAParser::ExtractParameter(String,ctemp,4); // wind speed kph integer

	if (strlen(ctemp)!=0) {

		#if 1 // 120424 fix correct wind setting

			double wspeed, wconfidence;
			wspeed = StrToDouble(ctemp,NULL);
			NMEAParser::ExtractParameter(String,ctemp,5); // confidence  0-100 percentage
			wconfidence = StrToDouble(ctemp,NULL);

			pGPS->ExternalWindAvailable = TRUE;
			if (wconfidence>=1) {
				pGPS->ExternalWindSpeed = Units::ToSys(unKiloMeterPerHour, wspeed);
				pGPS->ExternalWindDirection = wfrom;
			}

		#else

			double wspeed, wconfidence;
			wspeed = StrToDouble(ctemp,NULL);
			NMEAParser::ExtractParameter(String,ctemp,5); // confidence  0-100 percentage
			wconfidence = StrToDouble(ctemp,NULL);
			// StartupStore(_T(".... WIND from %.0f speed=%.0f kph  confidence=%.0f\n"),wfrom,wspeed,wconfidence);

			SetWindEstimate(wspeed/TOKPH, wfrom,(int)(wconfidence/100)*10);
	                CALCULATED_INFO.WindSpeed=wspeed/TOKPH;
	                CALCULATED_INFO.WindBearing=wfrom;
		#endif


	} else	pGPS->ExternalWindAvailable = FALSE;

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
