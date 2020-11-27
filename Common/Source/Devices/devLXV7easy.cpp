/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devBase.h"
#include "devLXV7easy.h"

static bool LXWP1(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

static bool PLXVF(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);
static bool PLXVS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info);

extern BOOL LXV7easyInstall(PDeviceDescriptor_t d);
extern BOOL LXV7easyParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info);

static BOOL isTrue(PDeviceDescriptor_t) {
     return TRUE;
}

static bool ParToDouble(const TCHAR* sentence, unsigned int parIdx, double* value)
{
  TCHAR  temp[80];
  const TCHAR* stop;

  NMEAParser::ExtractParameter(sentence, temp, parIdx);

  stop = NULL;
  *value = StrToDouble(temp, &stop);

  if (stop == NULL || *stop != '\0')
    return(false);

  return(true);
} 

bool LXV7easyRegister(void){
    return(devRegister(
        TEXT("LXV7 easy"),
        (1l << dfGPS)
        | (1l << dfBaroAlt)
        | (1l << dfSpeed)
        | (1l << dfVario),
        LXV7easyInstall
    ));
}

BOOL LXV7easyInstall(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, TEXT("LXV7 easy"));
  d->ParseNMEA    = LXV7easyParseNMEA;
  d->LinkTimeout  = isTrue;
  d->IsGPSSource  = isTrue;
  d->IsBaroSource = isTrue;

  return(TRUE);
} 



BOOL LXV7easyParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{

    if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)) {
        return FALSE;
    }

    //
    // We ignore the followings
    //
    if (_tcsncmp(_T("$PLXV0"), sentence, 6) == 0) {
	return TRUE;
    }
    if (_tcsncmp(_T("$LXWP0"), sentence, 6) == 0) {
	return TRUE;
    }
    if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0) {
	return TRUE;
    }
    if (_tcsncmp(_T("$LXWP3"), sentence, 6) == 0) {
	return TRUE;
    }
    if (_tcsncmp(_T("$LXWP4"), sentence, 6) == 0) {
	return TRUE;
    }
    if (_tcsncmp(_T("$LXWP5"), sentence, 6) == 0) {
	return TRUE;
    }

    //
    // We manage the followings
    //
    if (_tcsncmp(_T("$PLXVF"), sentence, 6) == 0)
        return PLXVF(d, sentence + 7, info);

    if (_tcsncmp(_T("$PLXVS"), sentence, 6) == 0)
        return PLXVS(d, sentence + 7, info);

    if (_tcsncmp(_T("$LXWP1"), sentence, 6) == 0)
        return LXWP1(d, sentence + 7, info);


    return(FALSE);
}


bool LXWP1(PDeviceDescriptor_t d, const TCHAR* String, NMEA_INFO* pGPS)
{
    // $LXWP1,serial number,instrument ID, software version, hardware
    //   version,license string,NU*SC<CR><LF>
    //
    // instrument ID ID of LX1600
    // serial number unsigned serial number
    // software version float sw version
    // hardware version float hw version
    // license string (option to store a license of PDA SW into LX1600)
    //	$LXWP1,LX5000IGC-2,15862,11.1 ,2.0*4A

#ifdef DEVICE_SERIAL
    TCHAR ctemp[180];
    static int NoMsg=0;
    static int oldSerial=0;

    if(_tcslen(String) >= 180) return true;

    if((( d->SerialNumber == 0)  || ( d->SerialNumber != oldSerial)) && (NoMsg < 5)) {
        NoMsg++ ;
        NMEAParser::ExtractParameter(String,ctemp,0);
        if(_tcslen(ctemp) < DEVNAMESIZE)
	    _stprintf(d->Name, _T("%s"),ctemp);

        StartupStore(_T(". %s\n"),ctemp);

	NMEAParser::ExtractParameter(String,ctemp,1);
	d->SerialNumber= (int)StrToDouble(ctemp,NULL);
	oldSerial = d->SerialNumber;
	_stprintf(ctemp, _T("%s Serial Number %i"), d->Name, d->SerialNumber);
	StartupStore(_T(". %s\n"),ctemp);

	NMEAParser::ExtractParameter(String,ctemp,2);
	d->SoftwareVer= StrToDouble(ctemp,NULL);
	_stprintf(ctemp, _T("%s Software Vers.: %3.2f"), d->Name, d->SoftwareVer);
	StartupStore(_T(". %s\n"),ctemp);

	NMEAParser::ExtractParameter(String,ctemp,3);
        d->HardwareId= (int)(StrToDouble(ctemp,NULL)*10);
	_stprintf(ctemp, _T("%s Hardware Vers.: %3.2f"), d->Name, (double)(d->HardwareId)/10.0);
	StartupStore(_T(". %s\n"),ctemp);
        _stprintf(ctemp, _T("%s (#%i) DETECTED"), d->Name, d->SerialNumber);

        DoStatusMessage(ctemp);
        _stprintf(ctemp, _T("SW Ver: %3.2f HW Ver: %3.2f "),  d->SoftwareVer, (double)(d->HardwareId)/10.0);
        DoStatusMessage(ctemp);
    }
#endif
    return(true);  
} 



bool PLXVF(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{

    double alt=0, airspeed=0;

    if (ParToDouble(sentence, 5, &airspeed)) {
        info->IndicatedAirspeed = airspeed;
        info->AirspeedAvailable = TRUE;
    }

    if (ParToDouble(sentence, 6, &alt)) {
	UpdateBaroSource( info, 0, d, QNEAltitudeToQNHAltitude(alt));
        if (airspeed>0) info->TrueAirspeed =  airspeed * AirDensityRatio(alt);
    }

    double Vario = 0;
    if (ParToDouble(sentence, 4, &Vario)) {
        UpdateVarioSource(*info, *d, Vario);
    }

    return(true);
} 



bool PLXVS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info) {

    double Batt;
    double OAT;

    if (ParToDouble(sentence, 0, &OAT)) {
	 info->OutsideAirTemperature = OAT;
	 info->TemperatureAvailable  = TRUE;
    }


    if (ParToDouble(sentence, 2, &Batt)) {
	 info->ExtBatt1_Voltage = Batt;
    }

    return(true);
} // PLXVS()




