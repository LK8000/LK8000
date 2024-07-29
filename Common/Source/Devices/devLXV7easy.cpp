/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "devBase.h"
#include "devLXV7easy.h"
#include "utils/printf.h"
#include "utils/charset_helper.h"

static bool LXWP1(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

static bool PLXVF(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);
static bool PLXVS(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

extern BOOL LXV7easyParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info);

static 
bool ParToDouble(const char* sentence, unsigned int parIdx, double* value) {
  char  temp[80];
  const char* stop;

  NMEAParser::ExtractParameter(sentence, temp, parIdx);

  stop = NULL;
  *value = StrToDouble(temp, &stop);

  if (stop == NULL || *stop != '\0')
    return(false);

  return(true);
} 

void LXV7easyInstall(DeviceDescriptor_t* d) {
  d->ParseNMEA    = LXV7easyParseNMEA;
}



BOOL LXV7easyParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{

    if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)) {
        return FALSE;
    }

    //
    // We ignore the followings
    //
    if (strncmp("$PLXV0", sentence, 6) == 0) {
	return TRUE;
    }
    if (strncmp("$LXWP0", sentence, 6) == 0) {
	return TRUE;
    }
    if (strncmp("$LXWP2", sentence, 6) == 0) {
	return TRUE;
    }
    if (strncmp("$LXWP3", sentence, 6) == 0) {
	return TRUE;
    }
    if (strncmp("$LXWP4", sentence, 6) == 0) {
	return TRUE;
    }
    if (strncmp("$LXWP5", sentence, 6) == 0) {
	return TRUE;
    }

    //
    // We manage the followings
    //
    if (strncmp("$PLXVF", sentence, 6) == 0)
        return PLXVF(d, sentence + 7, info);

    if (strncmp("$PLXVS", sentence, 6) == 0)
        return PLXVS(d, sentence + 7, info);

    if (strncmp("$LXWP1", sentence, 6) == 0)
        return LXWP1(d, sentence + 7, info);


    return(FALSE);
}


bool LXWP1(DeviceDescriptor_t* d, const char* String, NMEA_INFO* pGPS)
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
    char ctemp[180];
    static int NoMsg=0;
    static int oldSerial=0;

    if (strlen(String) >= 180) {
        return true;
    }
    if((( d->SerialNumber == 0)  || ( d->SerialNumber != oldSerial)) && (NoMsg < 5)) {
        NoMsg++ ;
        NMEAParser::ExtractParameter(String, ctemp, 0);
        from_unknown_charset(ctemp, d->Name);
        StartupStore(_T(". %s"),d->Name);

        NMEAParser::ExtractParameter(String, ctemp, 1);
        d->SerialNumber = StrToDouble(ctemp, nullptr);
        oldSerial = d->SerialNumber;
        StartupStore(_T(". %s Serial Number %i"), d->Name, d->SerialNumber);

        NMEAParser::ExtractParameter(String, ctemp, 2);
        d->SoftwareVer = StrToDouble(ctemp, nullptr);
        StartupStore(_T(". %s Software Vers.: %3.2f"), d->Name, d->SoftwareVer);

        NMEAParser::ExtractParameter(String, ctemp, 3);
        d->HardwareId = StrToDouble(ctemp,NULL) * 10;
        StartupStore(_T(". %s Hardware Vers.: %3.2f"), d->Name, (double)(d->HardwareId)/10.0);

        TCHAR str[255];
        _stprintf(str, _T("%s (#%i) DETECTED"), d->Name, d->SerialNumber);
        DoStatusMessage(str);
        _stprintf(str, _T("SW Ver: %3.2f HW Ver: %3.2f "),  d->SoftwareVer, d->HardwareId / 10.0);
        DoStatusMessage(str);
    }
#endif
    return true;
}



bool PLXVF(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{

    double alt=0, airspeed=0;

    if (ParToDouble(sentence, 5, &airspeed)) {
        info->IndicatedAirspeed = airspeed;
        info->AirspeedAvailable = TRUE;
    }

    if (ParToDouble(sentence, 6, &alt)) {
	    UpdateBaroSource(info, d, QNEAltitudeToQNHAltitude(alt));
        if (airspeed>0) {
            info->TrueAirspeed = TrueAirSpeed(airspeed, alt);
        }
    }

    double Vario = 0;
    if (ParToDouble(sentence, 4, &Vario)) {
        UpdateVarioSource(*info, *d, Vario);
    }

    return(true);
} 



bool PLXVS(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info) {

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




