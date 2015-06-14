/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#include "devDigifly.h"

extern bool UpdateBaroSource(NMEA_INFO* pGPS, const short parserid, const PDeviceDescriptor_t d, const double fAlt);
extern bool UpdateQNH(const double newqnh);

extern double LowPassFilter(double y_last, double x_in, double fact);

static BOOL PDGFTL1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

// Leonardo Pro & Catesio
static BOOL D(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS);

static BOOL DigiflyParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){

  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }


  if(_tcsncmp(TEXT("$PDGFTL1"), String, 8)==0)
    {
      return PDGFTL1(d, &String[9], pGPS);
    } 
  
  if(_tcsncmp(TEXT("$D"), String, 2) == 0) {
      return D(d, &String[3], pGPS);
  }

  return FALSE;
}

/*
static BOOL DigiflyIsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);
}
*/

static BOOL DigiflyIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL DigiflyIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL DigiflyLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL DigiflyInstall(PDeviceDescriptor_t d){

  StartupStore(_T(". DIGIFLY device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("Digifly"));
  d->ParseNMEA = DigiflyParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = DigiflyLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = DigiflyIsGPSSource;
  d->IsBaroSource = DigiflyIsBaroSource;

  return(TRUE);

}


BOOL DigiflyRegister(void){
  return(devRegister(
    TEXT("Digifly"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfSpeed)
    | (1l << dfVario),
    DigiflyInstall
  ));
}


static BOOL PDGFTL1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{
/*
	$PDGFTL1		     field     example
	QNE 1013.25 altitude		0	2025  meters
	QNH altitude			1	2000  meters
	vario cm/s	 		2	 250  2.5m/s
	netto vario			3	-14   dm/s
	IAS				4	45    kmh
	ground efficiency 		5	134   13:4 GR
	Wind speed			6	28    kmh
	Wind direction			7	65    degrees
	Main lithium battery 3,82 v	8	382   0.01v
	Backup AA battery 1,53 v 	9	153   0.01v

	100209 Paolo Ventafridda
	netto and ias optionals
	
*/

  TCHAR ctemp[80];
  double vtas, vias;
  double altqne, altqnh;
  static bool initqnh=true;



  NMEAParser::ExtractParameter(String,ctemp,0);
  altqne = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,1);
  altqnh = StrToDouble(ctemp,NULL);

  // AutoQNH will take care of setting an average QNH if nobody does it for a while
  if (initqnh) {
	// if digifly has qnh set by user qne and qnh are of course different
	if (altqne != altqnh) {
		UpdateQNH(FindQNH(altqne,altqnh));
		StartupStore(_T(". Using Digifly QNH %f%s"),QNH,NEWLINE);
		initqnh=false;
	} else {
		// if locally QNH was set, either by user of by AutoQNH, stop processing QNH from digifly
		if ( (QNH <= 1012) || (QNH>=1014)) initqnh=false;
		// else continue entering initqnh until somebody changes qnh in either digifly or lk8000
	}
  }
  UpdateBaroSource( pGPS,0, d,  AltitudeToQNHAltitude(altqne));


  NMEAParser::ExtractParameter(String,ctemp,2);
#if 1
  pGPS->Vario = StrToDouble(ctemp,NULL)/100;
#else
  double newVario = StrToDouble(ctemp,NULL)/100;
  pGPS->Vario = LowPassFilter(pGPS->Vario,newVario,0.1);
#endif
  pGPS->VarioAvailable = TRUE;


  NMEAParser::ExtractParameter(String,ctemp,3);
  if (ctemp[0] != '\0') {
	pGPS->NettoVario = StrToDouble(ctemp,NULL)/10; // dm/s
	pGPS->NettoVarioAvailable = TRUE;
  } else
	pGPS->NettoVarioAvailable = FALSE;


  NMEAParser::ExtractParameter(String,ctemp,4);
  if (ctemp[0] != '\0') {
	// we store m/s  , so we convert it from kmh
	vias = StrToDouble(ctemp,NULL)/3.6;

	if (vias >1) {
		vtas = vias*AirDensityRatio(altqne);
		pGPS->TrueAirspeed = vtas;
		pGPS->IndicatedAirspeed = vias;
		pGPS->AirspeedAvailable = TRUE;
  	}
  } else {
	pGPS->AirspeedAvailable = FALSE;
  }


  NMEAParser::ExtractParameter(String,ctemp,8);
  pGPS->ExtBatt1_Voltage = StrToDouble(ctemp,NULL)/100;
  NMEAParser::ExtractParameter(String,ctemp,9);
  pGPS->ExtBatt2_Voltage = StrToDouble(ctemp,NULL)/100;

  TriggerVarioUpdate();

  return TRUE;
}

static BOOL D(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS) {
/*
 * 00 : vario ist           in dm/sec
 * 01 : pressure            in cents of mB
 * 02 : nettovario          in dm/sec
 * 03 : anemometer          in km/h
 * 04 : temperature         in °C
 * 05 : trk compass (dis)   in °
 * 06 : speed (dis)         in km/h
 * 07 : mcr equ             in cm/sec
 * 08 : wind speed          in km/h
 * 09 : goto goal           in tenth of mt
 * 10 : effic to ground     in tenth
 * 11 : effic to goal       in tenth
 *
 *   $D,+0,100554,+25,18,+31,,0,-356,+25,+11,115,96*6A    
 */
    TCHAR ctemp[80];

    // Vario
    NMEAParser::ExtractParameter(String,ctemp,0);
    if (ctemp[0] != '\0') {
        pGPS->Vario = StrToDouble(ctemp,NULL)/100;
        pGPS->VarioAvailable = TRUE;
    } else {
        pGPS->VarioAvailable = FALSE;
    }
    
    // Pressure
    NMEAParser::ExtractParameter(String,ctemp,1);
    if (ctemp[0] != '\0') {
        double abs_press = StrToDouble(ctemp,NULL);
        UpdateBaroSource(pGPS, 0, d, StaticPressureToAltitude(abs_press));
    }

    // Netto Vario
    NMEAParser::ExtractParameter(String,ctemp,2);
    if (ctemp[0] != '\0') {
        pGPS->NettoVario = StrToDouble(ctemp,NULL)/10;
        pGPS->NettoVarioAvailable = TRUE;
    } else {
        pGPS->NettoVarioAvailable = FALSE;
    }

    // airspeed
    NMEAParser::ExtractParameter(String,ctemp,3);
    if (ctemp[0] != '\0') {
        pGPS->TrueAirspeed = StrToDouble(ctemp,NULL) / 3600 * 1000;
        pGPS->IndicatedAirspeed = pGPS->TrueAirspeed / AirDensityRatio(pGPS->Altitude);
    } else {
        pGPS->TrueAirspeed = 0;
        pGPS->IndicatedAirspeed = 0;
    }

    // temperature
    NMEAParser::ExtractParameter(String,ctemp,4);
    if (ctemp[0] != '\0') {
        pGPS->OutsideAirTemperature = StrToDouble(ctemp,NULL);
        pGPS->TemperatureAvailable = TRUE;
    } else {
        pGPS->TemperatureAvailable = FALSE;
    }

    return TRUE;
}
