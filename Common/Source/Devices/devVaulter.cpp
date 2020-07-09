/*
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include "externs.h"
#include "devVaulter.h"
#include "LKInterface.h"
#include "Baro.h"
#include "Calc/Vario.h"

int iVaulter_RxUpdateTime=0;
double oldVaulterMC = MACCREADY;
int  VaulterMacCreadyUpdateTimeout = 0;
int  VaulterBugsUpdateTimeout = 0;
int  VaulterBallastUpdateTimeout =0;
int  VaulterAltitudeUpdateTimeout =0;
int  VaulterAlt=0;

BOOL bVaulterValid = false;
int VaulterNMEAddCheckSumStrg( TCHAR szStrg[] );
BOOL VaulterPutMacCready(PDeviceDescriptor_t d, double MacCready);
BOOL VaulterPutBallast(PDeviceDescriptor_t d, double Ballast);


//____________________________________________________________class_definitions_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Registers device into device subsystem.
///
/// @retval true  when device has been registered successfully
/// @retval false device cannot be registered
///
//static
bool DevVaulter::Register()
{

  return(devRegister(GetName(),
    cap_gps | cap_baro_alt | cap_speed | cap_vario, Install));
} // Register()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
BOOL DevVaulter::Install(PDeviceDescriptor_t d)
{
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = VaulterPutMacCready;
  d->PutBallast   = VaulterPutBallast;
  d->LinkTimeout  = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  return(true);
} // Install()




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Returns device name (max length is @c DEVNAMESIZE).
///
//static
const TCHAR* DevVaulter::GetName()
{
  return(_T("Vaulter"));
} // GetName()


int VaulterNMEAddCheckSumStrg( TCHAR szStrg[] )
{
int i,iCheckSum=0;
TCHAR  szCheck[254];

 if(szStrg[0] != '$')
   return -1;

 iCheckSum = szStrg[1];
  for (i=2; i < (int)_tcslen(szStrg); i++)
  {
	//  if(szStrgi0] != ' ')
	    iCheckSum ^= szStrg[i];
  }
  _stprintf(szCheck,TEXT("*%02X\r\n"),iCheckSum);
  _tcscat(szStrg,szCheck);
  return iCheckSum;
}




BOOL VaulterPutMacCready(PDeviceDescriptor_t d, double MacCready){
TCHAR  szTmp[254];
if(bVaulterValid == false)
  return false;

  _stprintf(szTmp, TEXT("$PITV1,MC=%4.2f"), MacCready);
  VaulterNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
  VaulterMacCreadyUpdateTimeout = 5;
  return true;

}


BOOL VaulterPutBallast(PDeviceDescriptor_t d, double Ballast){
TCHAR  szTmp[254];
if(bVaulterValid == false)
  return false;


  _stprintf(szTmp, TEXT("$PITV1,WL=%4.2f"), (1.0+Ballast));
 VaulterNMEAddCheckSumStrg(szTmp);
 d->Com->WriteString(szTmp);

 VaulterBallastUpdateTimeout =5;
 return(TRUE);

}







//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWPn sentences.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevVaulter::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{


  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }


  if (_tcsncmp(_T("$PITV3"), sentence, 6) == 0)
    return PITV3(d, sentence + 7, info);
  else if (_tcsncmp(_T("$PITV4"), sentence, 6) == 0)
    return PITV4(d, sentence + 7, info);
  else if (_tcsncmp(_T("$PITV5"), sentence, 6) == 0)
    return PITV5(d, sentence + 7, info);


  return(false);
} // ParseNMEA()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP0 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevVaulter::PITV3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
//  $PITV3,20.0,-5.3,280.2,33.0,1.1*44
//  Feld Beispiel Beschreibung
//  1 20.0 Rollwinkel (Grad, positiv nach rechts)
//  2 -5.3 Anstellwinkel (Grad, positiv nach oben)
//  3 280.2 Kurs (Grad, bezogen auf rechtweisend Nord)
//  4 33.0 Angezeigte Geschwindigkeit (m/s)
//  5 1.1 Lastenvielfaches (g)
  double alt=0, tmp=0;

  if (ParToDouble(sentence, 0, &tmp))
  {
    info->Roll = tmp;
    info->GyroscopeAvailable = true;
  }

  if (ParToDouble(sentence, 1, &tmp))
    info->Pitch = tmp;

  if (ParToDouble(sentence, 2, &tmp))
  {
    info->MagneticHeading  = tmp;
    info->MagneticHeadingAvailable = true;
  }

  if (ParToDouble(sentence, 3, &tmp))
  {
    bVaulterValid = true;
    info->IndicatedAirspeed = tmp;
    info->TrueAirspeed = tmp * AirDensityRatio(alt);;
    info->AirspeedAvailable = TRUE;
  }


  if(VaulterAltitudeUpdateTimeout >0)
	  VaulterAltitudeUpdateTimeout--;




  return(true);
} // LXWP0()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP1 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevVaulter::PITV4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
//  $PITV4,2.0,2.8,2.2,430.2,460.2,460.4*44
//  Feld Beispiel Beschreibung
//  1 2.0 Absoultes Variometer (m/s, positiv steigen)
//  2 2.8 Netto Variometer (m/s, positiv steigen)
//  3 2.2 Relatives Variometer (m/s, positiv steigen)
//  4 430.2 Barometrische Höhe (m)
//  5 460.2 Barometrische Energiehöhe (m)
//  6 460.4 Inertiale Energiehöhe (m)
  double  tmp=0;
  if (ParToDouble(sentence, 0, &tmp))
  {
    UpdateVarioSource(*info, *d, tmp);
  }
  if (ParToDouble(sentence, 1, &tmp))
  {
    info->NettoVario = tmp;
    info->NettoVarioAvailable = true;
  }
  if (ParToDouble(sentence, 3, &tmp))
  {
    info->BaroAltitude = tmp;
    info->BaroAltitudeAvailable = true;
  }
  return(true);

} // PITV1()




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP2 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
bool DevVaulter::PITV5(PDeviceDescriptor_t, const TCHAR* sentence, NMEA_INFO* info )
{
//  $PITV5,5.0,30.0,0.950,0.15,0,2.30*44
//  Feld Beispiel Beschreibung
//  1 5.0 Windgeschwindigkeit (m/s)
//  2 30.0 Windrichtung ind Grad, bezogen au  rechtweisend Nord)
//  3 0.950 Quadratwurzel des Verhältnisses von  Ludichte zu ISA Meeresspiegel
//  4 0.15 Turbulenzmessung
//  5 0 Sollfahrt/Vario:
//  0=Vario
//  1=Sollfahrt
//  6 2.30 MacCready-Wert (m/s)
double fTmp;

  if (ParToDouble(sentence, 0, &fTmp))
  {
      info->ExternalWindSpeed = fTmp;

    if (ParToDouble(sentence, 1, &fTmp))
    {
      info->ExternalWindDirection = fTmp;
      info->ExternalWindAvailable = true;
    }
  }

  if(VaulterMacCreadyUpdateTimeout > 0)
    VaulterMacCreadyUpdateTimeout--;
  else
  {
    if (ParToDouble(sentence, 5, &fTmp))
    {
        info->MacReady = fTmp;
    }
  }
  return(true);
} // PITV5()

