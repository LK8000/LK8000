/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include <devFlyNet.h>
#include <externs.h>
#include <device.h>
#include <Parser.h>

static BOOL _PRS(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO){
	(void)d;
	if(UpdateBaroSource(_INFO, FLYNET, StaticPressureToAltitude((HexStrToInt(String)*1.0)))){
		TriggerVarioUpdate();
	}
	return TRUE;
}

static BOOL _BAT(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO){
	(void)d;
	_INFO->ExtBatt1_Voltage = (String[0]=='*')?999.0:((HexStrToInt(String)*10.0)+1000.0);
	return TRUE;
}

static BOOL FlyNetParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO){
  if(_tcsncmp(TEXT("_PRS "), String, 5)==0){
	  return _PRS(d, &String[5], _INFO);
  }
  if(_tcsncmp(TEXT("_BAT "), String, 5)==0){
	  return _BAT(d, &String[5], _INFO);
  }
  return FALSE;
}

static BOOL FlyNetIsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}

static BOOL FlyNetLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}

static BOOL FlyNetInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("FlyNet"));
  d->ParseNMEA = FlyNetParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = FlyNetLinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = NULL;
  d->IsBaroSource = FlyNetIsBaroSource;

  return(TRUE);
}

BOOL FlyNetRegister(void){
	return devRegister(TEXT("FlyNet"), (1l << dfBaroAlt), FlyNetInstall);
}



