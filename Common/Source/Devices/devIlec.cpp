/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devIlec.h"

extern bool UpdateQNH(const double newqnh);

static BOOL PILC(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

static BOOL IlecParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){

  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  if(_tcsncmp(TEXT("$PILC"), String, 5)==0)
    {
      return PILC(d, &String[6], pGPS);
    }

  return FALSE;

}

static BOOL IlecIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


void IlecInstall(PDeviceDescriptor_t d) {
  _tcscpy(d->Name, TEXT("Ilec SN10"));
  d->ParseNMEA = IlecParseNMEA;
  d->IsBaroSource = IlecIsBaroSource;
}

static BOOL PILC(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{

  TCHAR ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,0);

  if (_tcscmp(ctemp,_T("PDA1"))==0) {

	NMEAParser::ExtractParameter(String,ctemp,1);
	UpdateBaroSource( pGPS, 0,d, QNEAltitudeToQNHAltitude(StrToDouble(ctemp, NULL)));

	NMEAParser::ExtractParameter(String,ctemp,2);
	double Vario = StrToDouble(ctemp,NULL);
	UpdateVarioSource(*pGPS, *d, Vario);

	NMEAParser::ExtractParameter(String,ctemp,3); // wind direction, integer
	double wfrom;
	wfrom = StrToDouble(ctemp,NULL); //@ could also be the NMEA checksum!
	NMEAParser::ExtractParameter(String,ctemp,4); // wind speed kph integer

	if (_tcslen(ctemp)!=0) {

		#if 1 // 120424 fix correct wind setting

			double wspeed, wconfidence;
			wspeed = StrToDouble(ctemp,NULL);
			NMEAParser::ExtractParameter(String,ctemp,5); // confidence  0-100 percentage
			wconfidence = StrToDouble(ctemp,NULL);

			pGPS->ExternalWindAvailable = TRUE;
			if (wconfidence>=1) {
				pGPS->ExternalWindSpeed = wspeed/TOKPH;
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

  if (_tcscmp(ctemp,_T("SET"))==0) {
	NMEAParser::ExtractParameter(String,ctemp,1);
	UpdateQNH(StrToDouble(ctemp,NULL));
	// StartupStore(_T("... SET QNH= %.1f\n"),QNH);

	return TRUE;
  }

  // Using the polar requires recalculating polar coefficients internally each time.. no thanks.
  // if (_tcscmp(ctemp,_T("POLAR"))==0) { }

  return TRUE;
}
