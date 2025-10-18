/*
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
//_____________________________________________________________________includes_

#include "externs.h"
#include "devVaulter.h"
#include "LKInterface.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Comm/ExternalWind.h"

int iVaulter_RxUpdateTime=0;
double oldVaulterMC = MACCREADY;
int  VaulterBugsUpdateTimeout = 0;
int  VaulterBallastUpdateTimeout =0;
int  VaulterAltitudeUpdateTimeout =0;
int  VaulterAlt=0;

BOOL bVaulterValid = false;
int VaulterNMEAddCheckSumStrg( TCHAR szStrg[] );
BOOL VaulterPutMacCready(DeviceDescriptor_t* d, double MacCready);
BOOL VaulterPutBallast(DeviceDescriptor_t* d, double Ballast);


//____________________________________________________________class_definitions_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
void DevVaulter::Install(DeviceDescriptor_t* d)
{
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = VaulterPutMacCready;
  d->PutBallast   = VaulterPutBallast;
} // Install()


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




BOOL VaulterPutMacCready(DeviceDescriptor_t* d, double MacCready){
  TCHAR  szTmp[254];
  if(bVaulterValid == false) {
    return false;
  }

  _stprintf(szTmp, TEXT("$PITV1,MC=%4.2f"), MacCready);
  VaulterNMEAddCheckSumStrg(szTmp);
  d->Com->WriteString(szTmp);
  return true;

}


BOOL VaulterPutBallast(DeviceDescriptor_t* d, double Ballast){
  TCHAR  szTmp[254];
  if(bVaulterValid == false) {
    return false;
  }

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
BOOL DevVaulter::ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{


  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
    return FALSE;
  }


  if (strncmp("$PITV3", sentence, 6) == 0)
    return PITV3(d, sentence + 7, info);
  else if (strncmp("$PITV4", sentence, 6) == 0)
    return PITV4(d, sentence + 7, info);
  else if (strncmp("$PITV5", sentence, 6) == 0)
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
bool DevVaulter::PITV3(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
//  $PITV3,20.0,-5.3,280.2,33.0,1.1*44
//  Feld Beispiel Beschreibung
//  1 20.0 Rollwinkel (Grad, positiv nach rechts)
//  2 -5.3 Anstellwinkel (Grad, positiv nach oben)
//  3 280.2 Kurs (Grad, bezogen auf rechtweisend Nord)
//  4 33.0 Angezeigte Geschwindigkeit (m/s)
//  5 1.1 Lastenvielfaches (g)
  double tmp=0;

  if (ParToDouble(sentence, 0, &tmp))
  {
    info->Roll = tmp;
    info->GyroscopeAvailable = true;
  }

  if (ParToDouble(sentence, 1, &tmp))
    info->Pitch = tmp;

  if (ParToDouble(sentence, 2, &tmp))
  {
    info->MagneticHeading.update(*d, tmp);
  }

  if (ParToDouble(sentence, 3, &tmp))
  {
    bVaulterValid = true;
    info->IndicatedAirspeed = tmp;
    double qne_altitude = QNHAltitudeToQNEAltitude(info->Altitude);
    info->TrueAirspeed = TrueAirSpeed(tmp, qne_altitude);
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
bool DevVaulter::PITV4(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
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
    info->NettoVario.update(*d, tmp);
  }
  if (ParToDouble(sentence, 3, &tmp))
  {
    UpdateBaroSource(info, d, tmp);
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
bool DevVaulter::PITV5(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info )
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


  double WindSpeed, WindDirection;
  if (ParToDouble(sentence, 0, &WindSpeed) && ParToDouble(sentence, 1, &WindDirection)) {
    UpdateExternalWind(*info, *d, Units::From(Units_t::unMeterPerSecond, WindSpeed), WindDirection);
  }

  double fTmp;
  if (ParToDouble(sentence, 5, &fTmp)) {
    d->RecvMacCready(fTmp);
  }
  return (true);
} // PITV5()

