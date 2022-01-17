/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Dialogs/dlgProgress.h"
#include "Baro.h"
#include "devVolkslogger.h"
#include "Volkslogger/vlapi2.h"
#include "Volkslogger/vlapihlp.h"
#include "utils/stringext.h"


// RMN: Volkslogger
// Source data:
// $PGCS,1,0EC0,FFF9,0C6E,02*61
// $PGCS,1,0EC0,FFFA,0C6E,03*18
BOOL vl_PGCS1(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS)
{

  TCHAR ctemp[80];
  double InternalAltitude;

  NMEAParser::ExtractParameter(String,ctemp,2);
  // four characers, hex, barometric altitude
  InternalAltitude = HexStrToDouble(ctemp,NULL);
  double fBaroAltitude =0;

    if(InternalAltitude > 60000)
	fBaroAltitude =
        QNEAltitudeToQNHAltitude(InternalAltitude - 65535);
    // Assuming that altitude has wrapped around.  60 000 m occurs at
    // QNH ~2000 hPa
    else
	fBaroAltitude =
        QNEAltitudeToQNHAltitude(InternalAltitude);
    // typo corrected 21.04.07
    // Else the altitude is good enough.
    UpdateBaroSource( pGPS, 0,d,  fBaroAltitude);

  // ExtractParameter(String,ctemp,3);
  // four characters, hex, constant.  Value 1371 (dec)

  // nSatellites = (int)(min(12,HexStrToDouble(ctemp, NULL)));

  if (pGPS->SatellitesUsed <= 0) {
    pGPS->SatellitesUsed = 4;
  }

  return FALSE;
}


BOOL VLParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *pGPS){
  (void)d;

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }

  if(_tcsstr(String,TEXT("$PGCS,")) == String){
    return vl_PGCS1(d, &String[6], pGPS);
  }

  return FALSE;

}


VLAPI vl;

static int nturnpoints = 0;

BOOL VLDeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp);


BOOL VLDeclare(PDeviceDescriptor_t d, const Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  vl.set_device(d);
  nturnpoints = 0;

  const unsigned BUFF_LEN = 128;
  TCHAR buffer[BUFF_LEN];
  int err = VLA_ERR_NOERR;

  // LKTOKEN  _@M1400_ = "Task declaration"
  // LKTOKEN  _@M1404_ = "Opening connection"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(1404));
  CreateProgressDialog(buffer);
  if((err = vl.open(1, 20, 1, 38400L)) != VLA_ERR_NOERR) {
    // LKTOKEN  _@M1411_ = "Device not connected!"
    _tcsncpy(errBuffer, MsgToken(1411), errBufferLen);
    return FALSE;
  }

  // LKTOKEN  _@M1400_ = "Task declaration"
  // LKTOKEN  _@M1405_ = "Testing connection"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(1405));
  CreateProgressDialog(buffer);
  if((err = vl.read_info()) != VLA_ERR_NOERR) {
    // LKTOKEN  _@M1414_ = "Device not responsive!"
    _tcsncpy(errBuffer, MsgToken(1414), errBufferLen);
    return FALSE;
  }

  to_usascii(decl->PilotName, vl.declaration.flightinfo.pilot);
  to_usascii(decl->AircraftRego, vl.declaration.flightinfo.gliderid);
  to_usascii(decl->AircraftType, vl.declaration.flightinfo.glidertype);
  to_usascii(decl->CompetitionID, vl.declaration.flightinfo.competitionid);
  to_usascii(decl->CompetitionClass, vl.declaration.flightinfo.competitionclass);

  if (ValidWayPoint(HomeWaypoint)) {

    to_usascii(WayPointList[HomeWaypoint].Name, vl.declaration.flightinfo.homepoint.name);

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
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(1403));
  CreateProgressDialog(buffer);
  if((err = vl.write_db_and_declaration()) != VLA_ERR_NOERR) {
    // LKTOKEN  _@M1415_ = "Declaration not accepted!"
    _tcsncpy(errBuffer, MsgToken(1415), errBufferLen);
  }

  // LKTOKEN  _@M1400_ = "Task declaration"
  // LKTOKEN  _@M1406_ = "Closing connection"
  _sntprintf(buffer, BUFF_LEN, _T("%s: %s..."), MsgToken(1400), MsgToken(1406));
  CreateProgressDialog(buffer);
  vl.close(1);

  return err == VLA_ERR_NOERR;
}


BOOL VLDeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp){

  if (nturnpoints == 0) {
    to_usascii(wp->Name, vl.declaration.task.startpoint.name);

    vl.declaration.task.startpoint.lon =
      wp->Longitude;
    vl.declaration.task.startpoint.lat =
      wp->Latitude;
    nturnpoints++;
  } else {
    to_usascii(wp->Name, vl.declaration.task.turnpoints[nturnpoints-1].name);

    vl.declaration.task.turnpoints[nturnpoints-1].lon =
      wp->Longitude;
    vl.declaration.task.turnpoints[nturnpoints-1].lat =
      wp->Latitude;
    nturnpoints++;
  }
  to_usascii(wp->Name, vl.declaration.task.finishpoint.name);
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


void vlInstall(PDeviceDescriptor_t d){
  _tcscpy(d->Name, TEXT("Volkslogger"));
  d->ParseNMEA = VLParseNMEA;
  d->Declare = VLDeclare;
  d->IsLogger = VLIsLogger;
  d->IsGPSSource = VLIsGPSSource;
  d->IsBaroSource = VLIsBaroSource; // 100315 XCSOAR BUG missing
}
