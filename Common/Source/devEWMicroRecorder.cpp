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

#include "devEWMicroRecorder.h"


// Additional sentance for EW support

static int nDeclErrorCode = 0;
#define MAX_USER_SIZE 2500
static int user_size = 0;
static TCHAR user_data[MAX_USER_SIZE];


BOOL ExpectStringWait(PDeviceDescriptor_t d, TCHAR *token) {

  int i=0, ch;
  int j=0;

  if (!d->Com)
    return FALSE;

  while (j<500) {

    ch = d->Com->GetChar();

    if (ch != EOF) {

      if (token[i] == ch) 
        i++;
      else
        i=0;

      if ((unsigned)i == _tcslen(token))
        return(TRUE);
    }
    j++;
  }

  #if debugIGNORERESPONCE > 0
  return(TRUE);
  #endif
  return(FALSE);

}



BOOL EWMicroRecorderParseNMEA(PDeviceDescriptor_t d, 
                              TCHAR *String, NMEA_INFO *GPS_INFO){
  TCHAR ctemp[80], *params[5];
  int nparams = NMEAParser::ValidateAndExtract(String, ctemp, 80, params, 5);
  if (nparams < 1)
    return FALSE;

  if (!_tcscmp(params[0], TEXT("$PGRMZ")) && nparams >= 3) {
    if (d == pDevPrimaryBaroSource) {
      double altitude = NMEAParser::ParseAltitude(params[1], params[2]);

      GPS_INFO->BaroAltitude = AltitudeToQNHAltitude(altitude);
      GPS_INFO->BaroAltitudeAvailable = true;
    }

    return TRUE;
  }

  return FALSE;
}


BOOL EWMicroRecorderTryConnect(PDeviceDescriptor_t d) {
  int retries=10;
  TCHAR ch;

  while (--retries){

    d->Com->WriteString(TEXT("\x02"));         // send IO Mode command

    user_size = 0;
    bool started = false;

    while ((ch = d->Com->GetChar()) != _TEOF) {
      if (!started) {
        if (ch == _T('-')) {
          started = true;
        }
      }
      if (started) {
        if (ch == 0x13) {
          d->Com->WriteString(TEXT("\x16"));
          user_data[user_size] = 0;
          // found end of file
          return TRUE;
        } else {
          if (user_size<MAX_USER_SIZE-1) {
            user_data[user_size] = ch;
            user_size++;
          }
        }
      }
    }

  }

  nDeclErrorCode = 1;
  return(FALSE);
}


static void EWMicroRecorderWriteWayPoint(PDeviceDescriptor_t d, 
                                         const WAYPOINT *wp,
                                         TCHAR* EWType) {
  TCHAR EWRecord[128];
  int DegLat, DegLon;
  double MinLat, MinLon;
  TCHAR NoS, EoW;

  DegLat = (int)wp->Latitude;                    // prepare lat
  MinLat = wp->Latitude - DegLat;
  NoS = _T('N');
  if(MinLat<0)
    {
      NoS = _T('S');
      DegLat *= -1; MinLat *= -1;
    }
  MinLat *= 60;
  MinLat *= 1000;

  DegLon = (int)wp->Longitude ;                  // prepare long
  MinLon = wp->Longitude  - DegLon;
  EoW = _T('E');
  if(MinLon<0)
    {
      EoW = _T('W');
      DegLon *= -1; MinLon *= -1;
    }
  MinLon *= 60;
  MinLon *= 1000;

  _stprintf(EWRecord,
            TEXT("%s%02d%05d%c%03d%05d%c %s\r\n"),
            EWType,
            DegLat, (int)MinLat, NoS, 
            DegLon, (int)MinLon, EoW, 
            wp->Name);
  d->Com->WriteString(EWRecord);                 // put it to the logger
}


BOOL EWMicroRecorderDeclare(PDeviceDescriptor_t d, Declaration_t *decl)
{
  const WAYPOINT *wp;
  nDeclErrorCode = 0;

  // Must have at least two, max 12 waypoints
  if (decl->num_waypoints < 2 || decl->num_waypoints > 12)
    return FALSE;

  d->Com->StopRxThread();

  d->Com->SetRxTimeout(500);                     // set RX timeout to 500[ms]

  if (!EWMicroRecorderTryConnect(d)) {
    return FALSE;
  }

  d->Com->WriteString(TEXT("\x18"));         // start to upload file
  d->Com->WriteString(user_data);

  TCHAR EWRecord[128];
  _stprintf(EWRecord, TEXT("Pilot Name:     %s\r\n"), decl->PilotName);
  d->Com->WriteString(EWRecord);
  _stprintf(EWRecord, TEXT("Competition ID: %s\r\n"), decl->CompetitionID);
  d->Com->WriteString(EWRecord);
  _stprintf(EWRecord, TEXT("Aircraft Type:  %s\r\n"), decl->AircraftType);
  d->Com->WriteString(EWRecord);
  _stprintf(EWRecord, TEXT("Aircraft ID:    %s\r\n"), decl->AircraftRego);
  d->Com->WriteString(EWRecord);

  d->Com->WriteString(TEXT("Description:      Declaration\r\n"));

  for (int i = 0; i < 11; i++) {
    wp = decl->waypoint[i];
    if (i == 0) {
      EWMicroRecorderWriteWayPoint(d, wp, TEXT("Take Off LatLong: "));
      EWMicroRecorderWriteWayPoint(d, wp, TEXT("Start LatLon:     "));
    } else if (i + 1 < decl->num_waypoints) {
      EWMicroRecorderWriteWayPoint(d, wp, TEXT("TP LatLon:        "));
    } else {
      d->Com->WriteString(TEXT("TP LatLon:        0000000N00000000E TURN POINT\r\n"));
    }
  }

  wp = decl->waypoint[decl->num_waypoints - 1];
  EWMicroRecorderWriteWayPoint(d, wp, TEXT("Finish LatLon:    "));
  EWMicroRecorderWriteWayPoint(d, wp, TEXT("Land LatLon:      "));
  
  d->Com->WriteString(TEXT("\x03"));         // finish sending user file

  if (!ExpectStringWait(d, TEXT("uploaded successfully"))) {
    // error!
    nDeclErrorCode = 1;
  }
  d->Com->WriteString(TEXT("!!\r\n"));         // go back to NMEA mode

  d->Com->SetRxTimeout(RXTIMEOUT);                       // clear timeout
  d->Com->StartRxThread();                       // restart RX thread

  return(nDeclErrorCode == 0);                    // return() TRUE on success
}



BOOL EWMicroRecorderIsTrue(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL ewMicroRecorderInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("EW MicroRecorder"));
  d->ParseNMEA = EWMicroRecorderParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = EWMicroRecorderDeclare;
  d->IsLogger = EWMicroRecorderIsTrue;
  d->IsGPSSource = EWMicroRecorderIsTrue;
  d->IsBaroSource = EWMicroRecorderIsTrue;

  return(TRUE);

}


BOOL ewMicroRecorderRegister(void){
  return(devRegister(
    TEXT("EW MicroRecorder"), 
    1l << dfGPS
      | 1l << dfLogger
      | 1l << dfBaroAlt,
    ewMicroRecorderInstall
  ));
}



