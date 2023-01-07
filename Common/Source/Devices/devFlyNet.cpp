/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include <externs.h>
#include "Baro.h"
#include "Calc/Vario.h"
#include "devFlyNet.h"

static BOOL _PRS(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO){
	UpdateBaroSource(_INFO, d, StaticPressureToQNHAltitude(HexStrToInt(String)));
  return TRUE;
}

static BOOL _BAT(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO){
	(void)d;
	_INFO->ExtBatt1_Voltage = (String[0]=='*')?999.0:((HexStrToInt(String)*10.0)+1000.0);
	return TRUE;
}

BOOL FlyNetParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO){
  if(_tcsncmp(TEXT("_PRS "), String, 5)==0){
	  return _PRS(d, &String[5], _INFO);
  }
  if(_tcsncmp(TEXT("_BAT "), String, 5)==0){
	  return _BAT(d, &String[5], _INFO);
  }
  return FALSE;
}

void FlyNetInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("FlyNet"));
  d->ParseNMEA = FlyNetParseNMEA;
}
