/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


// adding baro alt sentance parser to support baro source priority  if (d == pDevPrimaryBaroSource){...}

#include "StdAfx.h"


#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"

#include "devEW.h"

#include "utils/heapcheck.h"


#define  USESHORTTPNAME   1       // hack, soulf be configurable

// Additional sentance for EW support

static BOOL fDeclarationPending = FALSE;
static unsigned long lLastBaudrate = 0;
static int nDeclErrorCode = 0;
static int ewDecelTpIndex = 0;


BOOL EWParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
  (void)d;
  (void)String;
  (void)GPS_INFO;
  // no propriatary sentence
  
  return FALSE;

}


void appendCheckSum(TCHAR *String){
  int i;
  unsigned char CalcCheckSum = 0;
  TCHAR sTmp[6];

  for(i=1; String[i] != '\0'; i++){
    CalcCheckSum = (unsigned char)(CalcCheckSum ^ (unsigned char)String[i]);
  }

  _stprintf(sTmp, TEXT("%02X\r\n"), CalcCheckSum);
	_tcscat(String, sTmp);

}


BOOL EWTryConnect(PDeviceDescriptor_t d) {
  int retries=10;
  while (--retries){

    d->Com->WriteString(TEXT("##\r\n"));         // send IO Mode command
    if (ExpectString(d, TEXT("IO Mode.\r"))) 
      return TRUE;

    ExpectString(d, TEXT("$$$"));                 // empty imput buffer 
  }

  nDeclErrorCode = 1;
  return(FALSE);
}


BOOL EWDeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp);


BOOL EWDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  TCHAR sTmp[72];
  TCHAR sPilot[13];
  TCHAR sGliderType[9];
  TCHAR sGliderID[9];

  nDeclErrorCode = 0;
  ewDecelTpIndex = 0;
  fDeclarationPending = TRUE;

  d->Com->StopRxThread();

  lLastBaudrate = d->Com->SetBaudrate(9600L);    // change to IO Mode baudrate

  d->Com->SetRxTimeout(500);                     // set RX timeout to 500[ms]

  if (!EWTryConnect(d)) {
    return FALSE;
  }

  _stprintf(sTmp, TEXT("#SPI"));                  // send SetPilotInfo
  appendCheckSum(sTmp);
  d->Com->WriteString(sTmp);
  Sleep(50);

  _tcsncpy(sPilot, decl->PilotName, 12);               // copy and strip fields
  sPilot[12] = '\0';
  _tcsncpy(sGliderType, decl->AircraftType, 8);
  sGliderType[8] = '\0';
  _tcsncpy(sGliderID, decl->AircraftRego, 8);
  sGliderID[8] = '\0';
 
  // build string (field 4-5 are GPS info, no idea what to write)
  _stprintf(sTmp, TEXT("%-12s%-8s%-8s%-12s%-12s%-6s\r"), 
           sPilot, 
           sGliderType, 
           sGliderID, 
           TEXT(""),                              // GPS Model
           TEXT(""),                              // GPS Serial No.
           TEXT("")                               // Flight Date,
                                                  // format unknown,
                                                  // left blank (GPS
                                                  // has a RTC)
  );
  d->Com->WriteString(sTmp);

  if (!ExpectString(d, TEXT("OK\r"))){
    nDeclErrorCode = 1;
    return(FALSE);
  };
  

  /*  
  _stprintf(sTmp, TEXT("#SUI%02d"), 0);           // send pilot name
  appendCheckSum(sTmp);
  d->Com->WriteString(sTmp);
  Sleep(50);
  d->Com->WriteString(PilotsName);
  d->Com->WriteString(TEXT("\r"));

  if (!ExpectString(d, TEXT("OK\r"))){
    nDeclErrorCode = 1;
    return(FALSE);
  };

  _stprintf(sTmp, TEXT("#SUI%02d"), 1);           // send type of aircraft
  appendCheckSum(sTmp);
  d->Com->WriteString(sTmp);
  Sleep(50);
  d->Com->WriteString(Class);
  d->Com->WriteString(TEXT("\r"));

  if (!ExpectString(d, TEXT("OK\r"))){
    nDeclErrorCode = 1;
    return(FALSE);
  };

  _stprintf(sTmp, TEXT("#SUI%02d"), 2);           // send aircraft ID
  appendCheckSum(sTmp);
  d->Com->WriteString(sTmp);
  Sleep(50);
  d->Com->WriteString(ID);
  d->Com->WriteString(TEXT("\r"));

  if (!ExpectString(d, TEXT("OK\r"))){
    nDeclErrorCode = 1;
    return(FALSE);
  };
  */

  for (int i=0; i<6; i++){                        // clear all 6 TP's
    _stprintf(sTmp, TEXT("#CTP%02d"), i);
    appendCheckSum(sTmp);
    d->Com->WriteString(sTmp);
    if (!ExpectString(d, TEXT("OK\r"))){
      nDeclErrorCode = 1;
      return(FALSE);
    };
  }

  for (int j = 0; j < decl->num_waypoints; j++)
    EWDeclAddWayPoint(d, decl->waypoint[j]);
  
  d->Com->WriteString(TEXT("NMEA\r\n"));         // switch to NMEA mode
  
  d->Com->SetBaudrate(lLastBaudrate);            // restore baudrate

  d->Com->SetRxTimeout(RXTIMEOUT);                       // clear timeout
  d->Com->StartRxThread();                       // restart RX thread

  fDeclarationPending = FALSE;                    // clear decl pending flag

  return(nDeclErrorCode == 0);                    // return() TRUE on success

}


BOOL EWDeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp){

  TCHAR EWRecord[100];
  TCHAR IDString[12];
  int DegLat, DegLon;
  double MinLat, MinLon;
  char NoS, EoW;
  short EoW_Flag, NoS_Flag, EW_Flags;	

  if (nDeclErrorCode != 0)                        // check for error
    return(FALSE);

  if (ewDecelTpIndex > 6){                        // check for max 6 TP's
    return(FALSE);
  }

  _tcsncpy(IDString, wp->Name, 6);                // copy at least 6 chars

  while (_tcslen(IDString) < 6)                   // fill up with spaces
    _tcscat(IDString, TEXT(" "));
  
  #if USESHORTTPNAME > 0
    _tcscpy(&IDString[3], TEXT("   "));           // truncate to short name
  #endif

  DegLat = (int)wp->Latitude;                    // preparte lat
  MinLat = wp->Latitude - DegLat;
  NoS = 'N';
  if(MinLat<0)
    {
      NoS = 'S';
      DegLat *= -1; MinLat *= -1;
    }
  MinLat *= 60;
  MinLat *= 1000;


  DegLon = (int)wp->Longitude ;                  // prepare long
  MinLon = wp->Longitude  - DegLon;
  EoW = 'E';
  if(MinLon<0)
    {
      EoW = 'W';
      DegLon *= -1; MinLon *= -1;
    }
  MinLon *=60;
  MinLon *= 1000;

  //	Calc E/W and N/S flags

  //	Clear flags
  EoW_Flag = 0;                                   // prepare flags
  NoS_Flag = 0;
  EW_Flags = 0;

  if (EoW == 'W')
    {
      EoW_Flag = 0x08;
    }
  else 
    { 
      EoW_Flag = 0x04;
    }
  if (NoS == 'N')
    {
      NoS_Flag = 0x01;
    }
  else 
    {	
      NoS_Flag = 0x02;
    }
  //  Do the calculation
  EW_Flags = (short)(EoW_Flag | NoS_Flag);

                                                  // setup command string
  _stprintf(EWRecord,TEXT("#STP%02X%02X%02X%02X%02X%02X%02X%02X%02X%04X%02X%04X"), 
                      ewDecelTpIndex, 
                      IDString[0], 
                      IDString[1],
                      IDString[2], 
                      IDString[3], 
                      IDString[4], 
                      IDString[5], 
                      EW_Flags, 
                      DegLat, (int)MinLat/10, 
                      DegLon, (int)MinLon/10);
    
  appendCheckSum(EWRecord);                       // complete package with CS and CRLF
    
  d->Com->WriteString(EWRecord);                 // put it to the logger

  if (!ExpectString(d, TEXT("OK\r"))){            // wait for response
    nDeclErrorCode = 1;
    return(FALSE);
  }
    
  ewDecelTpIndex = ewDecelTpIndex + 1;            // increase TP index

  return(TRUE);

}


BOOL EWIsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL EWIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL EWLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  if (!fDeclarationPending)
    d->Com->WriteString(TEXT("NMEA\r\n"));

  return(TRUE);
}


BOOL ewInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("EW Logger"));
  d->ParseNMEA = EWParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = EWLinkTimeout;
  d->Declare = EWDeclare;
  d->IsLogger = EWIsLogger;
  d->IsGPSSource = EWIsGPSSource;

  return(TRUE);

}


BOOL ewRegister(void){
  return(devRegister(
    TEXT("EW Logger"), 
    1l << dfGPS
      | 1l << dfLogger,
    ewInstall
  ));
}

