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

#include "devVolkslogger.h"

#include "Volkslogger/vlapi2.h"
#include "Volkslogger/vlapihlp.h"

#include "utils/heapcheck.h"
using std::min;
using std::max;

// RMN: Volkslogger
// Source data:
// $PGCS,1,0EC0,FFF9,0C6E,02*61
// $PGCS,1,0EC0,FFFA,0C6E,03*18
BOOL vl_PGCS1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO)
{
    
  TCHAR ctemp[80];
  double InternalAltitude;
  
  NMEAParser::ExtractParameter(String,ctemp,2); 
  // four characers, hex, barometric altitude
  InternalAltitude = HexStrToDouble(ctemp,NULL);

  if (d == pDevPrimaryBaroSource) {

    if(InternalAltitude > 60000)
      GPS_INFO->BaroAltitude = 
        AltitudeToQNHAltitude(InternalAltitude - 65535);  
    // Assuming that altitude has wrapped around.  60 000 m occurs at
    // QNH ~2000 hPa
    else
      GPS_INFO->BaroAltitude = 
        AltitudeToQNHAltitude(InternalAltitude);  
    // typo corrected 21.04.07
    // Else the altitude is good enough.

    GPS_INFO->BaroAltitudeAvailable = TRUE;
  }
	
  // ExtractParameter(String,ctemp,3);		
  // four characters, hex, constant.  Value 1371 (dec)

  // nSatellites = (int)(min(12,HexStrToDouble(ctemp, NULL)));
  
  if (GPS_INFO->SatellitesUsed <= 0) {
    GPS_INFO->SatellitesUsed = 4; 
    // just to make XCSoar quit complaining.  VL doesn't tell how many
    // satellites it uses.  Without this XCSoar won't do wind
    // measurements.
  }
  
  return FALSE;
}


BOOL VLParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
  (void)d;
  
  if (!NMEAParser::NMEAChecksum(String) || (GPS_INFO == NULL)){
    return FALSE;
  }
  
  if(_tcsstr(String,TEXT("$PGCS,")) == String){
    return vl_PGCS1(d, &String[6], GPS_INFO);
  }

  return FALSE;

}


VLAPI vl;

static int nturnpoints = 0;

BOOL VLDeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp);


BOOL VLDeclare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  vl.set_device(d);
  nturnpoints = 0;
  
  const unsigned BUFF_LEN = 128;
  TCHAR buffer[BUFF_LEN];
  int err = VLA_ERR_NOERR;
  
  // LKTOKEN  _@M1400_ = "Task declaration" 
  // LKTOKEN  _@M1404_ = "Opening connection" 
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M1404_")));
  CreateProgressDialog(buffer);
  if((err = vl.open(1, 20, 1, 38400L)) != VLA_ERR_NOERR) {
    // LKTOKEN  _@M1411_ = "Device not connected!" 
    _sntprintf(errBuffer, errBufferLen, gettext(_T("_@M1411_")));
    return FALSE;
  }
  
  // LKTOKEN  _@M1400_ = "Task declaration" 
  // LKTOKEN  _@M1405_ = "Testing connection" 
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M1405_")));
  CreateProgressDialog(buffer);
  if((err = vl.read_info()) != VLA_ERR_NOERR) {
    // LKTOKEN  _@M1414_ = "Device not responsive!" 
    _sntprintf(errBuffer, errBufferLen, gettext(_T("_@M1414_")));
    return FALSE;
  }
  
  char temp[100];
  sprintf(temp, "%S", decl->PilotName);
  strncpy(vl.declaration.flightinfo.pilot, temp, 64);

  sprintf(temp, "%S", decl->AircraftRego);
  strncpy(vl.declaration.flightinfo.gliderid, temp, 7);
  vl.declaration.flightinfo.gliderid[7]='\0'; 

  sprintf(temp, "%S", decl->AircraftType);
  strncpy(vl.declaration.flightinfo.glidertype, temp, 12);
  vl.declaration.flightinfo.glidertype[12]='\0'; 

  sprintf(temp, "%S", decl->CompetitionID);
  strncpy(vl.declaration.flightinfo.competitionid, temp, 3);
  vl.declaration.flightinfo.competitionid[3]='\0'; // BUGFIX 100331
 
  sprintf(temp, "%S", decl->CompetitionClass);
  strncpy(vl.declaration.flightinfo.competitionclass, temp, 12);
  vl.declaration.flightinfo.competitionclass[12]='\0'; // BUGFIX 100331
  
  if (ValidWayPoint(HomeWaypoint)) {
    sprintf(temp, "%S", WayPointList[HomeWaypoint].Name);
    
    strncpy(vl.declaration.flightinfo.homepoint.name, temp, 6);
    vl.declaration.flightinfo.homepoint.name[6]='\0'; // BUGFIX 100331
    vl.declaration.flightinfo.homepoint.lon = 
      WayPointList[HomeWaypoint].Longitude;
    vl.declaration.flightinfo.homepoint.lat = 
      WayPointList[HomeWaypoint].Latitude;
  }
  
  int i;
  for (i = 0; i < decl->num_waypoints; i++)
    VLDeclAddWayPoint(d, decl->waypoint[i]);

  vl.declaration.task.nturnpoints = max(min(nturnpoints-2, 12), 0);

  LockTaskData();

  // start..
  switch(StartLine) {
  case 0: // cylinder
    vl.declaration.task.startpoint.oztyp = VLAPI_DATA::DCLWPT::OZTYP_CYLSKT;
    vl.declaration.task.startpoint.lw = min(1500,(int)StartRadius);
    vl.declaration.task.startpoint.rz = min(1500,(int)StartRadius);
    vl.declaration.task.startpoint.rs = 0;
    break;
  case 1: // line
    vl.declaration.task.startpoint.oztyp = VLAPI_DATA::DCLWPT::OZTYP_LINE;
    vl.declaration.task.startpoint.lw = StartRadius * 2;
    vl.declaration.task.startpoint.rs = 0;
    vl.declaration.task.startpoint.rz = 0;
    break;
  case 2: // fai sector
    vl.declaration.task.startpoint.oztyp = VLAPI_DATA::DCLWPT::OZTYP_CYLSKT;
    vl.declaration.task.startpoint.lw = min(15000,(int)StartRadius);
    vl.declaration.task.startpoint.rz = 0;
    vl.declaration.task.startpoint.rs = min(15000,(int)StartRadius);
    break;
  }
  vl.declaration.task.startpoint.ws = 360;

  // rest of task...
  for (i=0; i<nturnpoints; i++) {
    // note this is for non-aat only!
    switch (SectorType) {
    case 0: // cylinder
      vl.declaration.task.turnpoints[i].oztyp = VLAPI_DATA::DCLWPT::OZTYP_CYLSKT;
      vl.declaration.task.turnpoints[i].rz = min(1500,(int)SectorRadius);
      vl.declaration.task.turnpoints[i].rs = 0;
      vl.declaration.task.turnpoints[i].lw = 0;
      break;
    case 1: // sector
      vl.declaration.task.turnpoints[i].oztyp = VLAPI_DATA::DCLWPT::OZTYP_CYLSKT;
      vl.declaration.task.turnpoints[i].rz = 0;
      vl.declaration.task.turnpoints[i].rs = min(15000,(int)SectorRadius);
      vl.declaration.task.turnpoints[i].lw = 0;
      break;
    case 2: // German DAe 0.5/10
      vl.declaration.task.turnpoints[i].oztyp = VLAPI_DATA::DCLWPT::OZTYP_CYLSKT;
      vl.declaration.task.turnpoints[i].rz = 500;
      vl.declaration.task.turnpoints[i].rs = 10000;
      vl.declaration.task.turnpoints[i].lw = 0;
      break;
    };
    vl.declaration.task.turnpoints[i].ws = 360; // auto direction

  }

  // Finish
  switch(FinishLine) {
  case 0: // cylinder
    vl.declaration.task.finishpoint.oztyp = VLAPI_DATA::DCLWPT::OZTYP_CYLSKT;
    vl.declaration.task.finishpoint.lw = min(1500,(int)FinishRadius);
    vl.declaration.task.finishpoint.rz = min(1500,(int)FinishRadius);
    vl.declaration.task.finishpoint.rs = 0;
    break;
  case 1: // line
    vl.declaration.task.finishpoint.oztyp = VLAPI_DATA::DCLWPT::OZTYP_LINE;
    vl.declaration.task.finishpoint.lw = FinishRadius*2;
    vl.declaration.task.finishpoint.rz = 0;
    vl.declaration.task.finishpoint.rs = 0;
    break;
  case 2: // fai sector
    vl.declaration.task.finishpoint.oztyp = VLAPI_DATA::DCLWPT::OZTYP_CYLSKT;
    vl.declaration.task.finishpoint.lw = min(15000,(int)FinishRadius);
    vl.declaration.task.finishpoint.rz = 0;
    vl.declaration.task.finishpoint.rs = min(15000,(int)FinishRadius);
    break;
  }
  vl.declaration.task.finishpoint.ws = 360;
  
  UnlockTaskData();
  
  // LKTOKEN  _@M1400_ = "Task declaration" 
  // LKTOKEN  _@M1403_ = "Sending  declaration"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M1403_")));
  CreateProgressDialog(buffer);
  if((err = vl.write_db_and_declaration()) != VLA_ERR_NOERR) {
    // LKTOKEN  _@M1415_ = "Declaration not accepted!" 
    _sntprintf(errBuffer, errBufferLen, gettext(_T("_@M1415_")));
  }
  
  // LKTOKEN  _@M1400_ = "Task declaration" 
  // LKTOKEN  _@M1406_ = "Closing connection" 
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), gettext(_T("_@M1400_")), gettext(_T("_@M1406_")));
  CreateProgressDialog(buffer);
  vl.close(1);
  
  return err == VLA_ERR_NOERR;
}


BOOL VLDeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp){
  char temp[100];
  sprintf(temp, "%S", wp->Name);

  if (nturnpoints == 0) {
    strncpy(vl.declaration.task.startpoint.name, temp, 6);
    vl.declaration.task.startpoint.lon = 
      wp->Longitude;
    vl.declaration.task.startpoint.lat = 
      wp->Latitude;
    nturnpoints++;
  } else {
    strncpy(vl.declaration.task.turnpoints[nturnpoints-1].name, temp, 6);
    vl.declaration.task.turnpoints[nturnpoints-1].lon = 
      wp->Longitude;
    vl.declaration.task.turnpoints[nturnpoints-1].lat = 
      wp->Latitude;
    nturnpoints++;
  }
  strncpy(vl.declaration.task.finishpoint.name, temp, 6);
  vl.declaration.task.finishpoint.lon = 
    wp->Longitude;
  vl.declaration.task.finishpoint.lat = 
    wp->Latitude;

  return(TRUE);

}


BOOL VLIsLogger(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL VLIsGPSSource(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL VLIsBaroSource(PDeviceDescriptor_t d){ // 100315 added
  (void)d;
  return(TRUE);
}

BOOL VLLinkTimeout(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


BOOL vlInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Volkslogger"));
  d->ParseNMEA = VLParseNMEA;
  d->PutMacCready = NULL;
  d->PutBugs = NULL;
  d->PutBallast = NULL;
  d->Open = NULL;
  d->Close = NULL;
  d->Init = NULL;
  d->LinkTimeout = VLLinkTimeout;
  d->Declare = VLDeclare;
  d->IsLogger = VLIsLogger;
  d->IsGPSSource = VLIsGPSSource;
  d->IsBaroSource = VLIsBaroSource; // 100315 XCSOAR BUG missing

  return(TRUE);

}


BOOL vlRegister(void){
  return(devRegister(
    TEXT("Volkslogger"), 
    (1l << dfGPS)
    | (1l << dfBaroAlt)
    | (1l << dfLogger),
    vlInstall
  ));
}

