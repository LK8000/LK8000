/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devDigifly.h"
#include "MathFunctions.h"

extern bool UpdateQNH(const double newqnh);

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

static BOOL DigiflyIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}

void DigiflyInstall(PDeviceDescriptor_t d) {

  StartupStore(_T(". DIGIFLY device installed%s"),NEWLINE);

  _tcscpy(d->Name, TEXT("Digifly"));
  d->ParseNMEA = DigiflyParseNMEA;
  d->IsBaroSource = DigiflyIsBaroSource;
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
  UpdateBaroSource( pGPS,0, d,  QNEAltitudeToQNHAltitude(altqne));


  NMEAParser::ExtractParameter(String,ctemp,2);
  UpdateVarioSource(*pGPS, *d, StrToDouble(ctemp,NULL)/100);

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
		vtas = TrueAirSpeed(vias, altqne);
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
        UpdateVarioSource(*pGPS, *d, StrToDouble(ctemp,NULL)/100);
    }
    // Pressure
    NMEAParser::ExtractParameter(String,ctemp,1);
    if (ctemp[0] != '\0') {
        double abs_press = StrToDouble(ctemp,NULL);
        UpdateBaroSource(pGPS, 0, d, StaticPressureToQNHAltitude(abs_press));
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
        pGPS->TrueAirspeed = StrToDouble(ctemp,NULL) / TOKPH;
        pGPS->IndicatedAirspeed = IndicatedAirSpeed(pGPS->TrueAirspeed, QNHAltitudeToQNEAltitude(pGPS->Altitude));
        pGPS->AirspeedAvailable = true;
    } else {
        pGPS->TrueAirspeed = 0;
        pGPS->IndicatedAirspeed = 0;
        pGPS->AirspeedAvailable = false;
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
