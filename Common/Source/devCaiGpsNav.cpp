/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: devCaiGpsNav.cpp,v 8.2 2010/12/13 10:04:35 root Exp root $
*/


// CAUTION!
// caiGpsNavParseNMEA is called from com port read thread
// all other functions are called from windows message loop thread


#define  LOGSTREAM 0


#include <windows.h>
#include <tchar.h>


#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"

#include "devCaiGpsNav.h"

#if NOSIM

#else
#ifdef _SIM_
static BOOL fSimMode = TRUE;
#else
static BOOL fSimMode = FALSE;
#endif
#endif


#define  CtrlC  0x03
#define  swap(x)      x = ((((x<<8) & 0xff00) | ((x>>8) & 0x00ff)) & 0xffff)


BOOL caiGpsNavOpen(PDeviceDescriptor_t d, int Port){

  #if NOSIM
  if (!SIMMODE) {
  #else
  if (!fSimMode){
  #endif
	  d->Com->WriteString(TEXT("\x03"));
	  Sleep(50);
	  d->Com->WriteString(TEXT("NMEA\r"));
	  
	  // This is for a slightly different mode, that
	  // apparently outputs pressure info too...
	  //(d->Com.WriteString)(TEXT("PNP\r\n"));
	  //(d->Com.WriteString)(TEXT("LOG 0\r\n"));
  }
  
  return(TRUE);
}

BOOL caiGpsNavClose(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}




BOOL caiGpsNavIsLogger(PDeviceDescriptor_t d){
  // There is currently no support for task declaration
  // from XCSoar
	(void)d; // TODO feature: CAI GPS NAV declaration
  return(FALSE);
}


BOOL caiGpsNavIsGPSSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}


BOOL caiGpsNavInstall(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("CAI GPS-NAV"));
  d->ParseNMEA = NULL;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = caiGpsNavOpen;
  d->Close = caiGpsNavClose;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = NULL;
  d->IsLogger = caiGpsNavIsLogger;
  d->IsGPSSource = caiGpsNavIsGPSSource;

  return(TRUE);

}


BOOL caiGpsNavRegister(void){
  return(devRegister(
    TEXT("CAI GPS-NAV"), 
    (1l << dfGPS),
    caiGpsNavInstall
  ));
}

