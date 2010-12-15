/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"

#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"

#include "devFlymasterF1.h"

static BOOL VARIO(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);

static BOOL FlymasterF1ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  (void)d;

  if(_tcsncmp(TEXT("$VARIO"), String, 6)==0)
    {
      return VARIO(d, &String[7], GPS_INFO);
    } 

  return FALSE;

}

/*
static BOOL FlymasterF1IsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(FALSE);
}
*/

static BOOL FlymasterF1IsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE); 
}


static BOOL FlymasterF1IsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


static BOOL FlymasterF1LinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static BOOL flymasterf1Install(PDeviceDescriptor_t d){

  #if ALPHADEBUG
  StartupStore(_T(". FlymasterF1 device installed%s"),NEWLINE);
  #endif

  _tcscpy(d->Name, TEXT("FlymasterF1"));
  d->ParseNMEA = FlymasterF1ParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = FlymasterF1LinkTimeout;
  d->Declare = NULL;
  d->IsGPSSource = FlymasterF1IsGPSSource;
  d->IsBaroSource = FlymasterF1IsBaroSource;

  return(TRUE);

}


BOOL flymasterf1Register(void){
  #if ALPHADEBUG
  StartupStore(_T(". FlymasterF1 device registered%s"),NEWLINE);
  #endif
  return(devRegister(
    TEXT("FlymasterF1"),
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfVario),
    flymasterf1Install
  ));
}


// *****************************************************************************
// local stuff

static BOOL VARIO(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
  // $VARIO,fPressure,fVario,Bat1Volts,Bat2Volts,BatBank,TempSensor1,TempSensor2*CS

  TCHAR ctemp[80];
  NMEAParser::ExtractParameter(String,ctemp,0);
  double ps = StrToDouble(ctemp,NULL);
  GPS_INFO->BaroAltitude = (1 - pow(fabs(ps / QNH),  0.190284)) * 44307.69;
  NMEAParser::ExtractParameter(String,ctemp,1);
  GPS_INFO->Vario = StrToDouble(ctemp,NULL)/10.0;
  // JMW vario is in dm/s

  NMEAParser::ExtractParameter(String,ctemp,2);
  GPS_INFO->ExtBatt1_Voltage = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,3);
  GPS_INFO->ExtBatt2_Voltage = StrToDouble(ctemp,NULL);
  NMEAParser::ExtractParameter(String,ctemp,4);
  GPS_INFO->ExtBatt_Bank = (int)StrToDouble(ctemp,NULL);

/*
  StartupStore(_T("++++++ BATTBANK: "));
  StartupStore(ctemp);
  StartupStore(_T("\n"));
*/

  GPS_INFO->VarioAvailable = TRUE;
  GPS_INFO->BaroAltitudeAvailable = TRUE;

  TriggerVarioUpdate();

  return TRUE;
}
