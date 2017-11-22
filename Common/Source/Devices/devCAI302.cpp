/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: devCAI302.cpp,v 8.2 2010/12/13 10:03:50 root Exp root $
*/


// CAUTION!
// cai302ParseNMEA is called from com port read thread
// all other functions are called from windows message loop thread


#include "externs.h"
#include "McReady.h"
#include "Baro.h"
#include "Calc/Vario.h"
#include "Comm/UpdateQNH.h"
#include "devCAI302.h"
#include "OS/Sleep.h"

using std::min;
using std::max;

#define  CtrlC  0x03
#define  swap(x)      x = ((((x<<8) & 0xff00) | ((x>>8) & 0x00ff)) & 0xffff)

#pragma pack(push, 1)                  // force byte allignement

/* UNUSED
typedef struct{
  unsigned char result[3];
  unsigned char reserved[15];
  unsigned char ID[3];
  unsigned char Type;
  unsigned char Version[5];
  unsigned char reserved2[6];
  unsigned char cai302ID;
  unsigned char reserved3;
}cai302_Wdata_t;
*/

typedef struct{
  unsigned char result[3];
  unsigned char PilotCount;
  unsigned char PilotRecordSize;
}cai302_OdataNoArgs_t;

typedef struct{
  unsigned char  result[3];
  char           PilotName[24];
  unsigned char  OldUnit;                                       // old unit
  unsigned char  OldTemperaturUnit;                             //0=Celcius 1=Farenheight
  unsigned char  SinkTone;
  unsigned char  TotalEnergyFinalGlide;
  unsigned char  ShowFinalGlideAltitude;
  unsigned char  MapDatum;  // ignored on IGC version
  unsigned short ApproachRadius;
  unsigned short ArrivalRadius;
  unsigned short EnrouteLoggingInterval;
  unsigned short CloseTpLoggingInterval;
  unsigned short TimeBetweenFlightLogs;                     // [Minutes]
  unsigned short MinimumSpeedToForceFlightLogging;          // (Knots)
  unsigned char  StfDeadBand;                                // (10ths M/S)
  unsigned char  ReservedVario;                           // multiplexed w/ vario mode:  Tot Energy, SuperNeto, Netto
  unsigned short UnitWord;
  unsigned short Reserved2;
  unsigned short MarginHeight;                              // (10ths of Meters)
  unsigned char  Spare[60];                                 // 302 expect more data than the documented filed
                                                            // be shure there is space to hold the data
}cai302_OdataPilot_t;

/* UNUSED
typedef struct{
  unsigned char result[3];
  unsigned char GliderRecordSize;
}cai302_GdataNoArgs_t;

typedef struct{
  unsigned char  result[3];
  unsigned char  GliderType[12];
  unsigned char  GliderID[12];
  unsigned char  bestLD;
  unsigned char  BestGlideSpeed;
  unsigned char  TwoMeterSinkAtSpeed;
  unsigned char  Reserved1;
  unsigned short WeightInLiters;
  unsigned short BallastCapacity;
  unsigned short Reserved2;
  unsigned short ConfigWord;                                //locked(1) = FF FE.  unlocked(0) = FF FF
  unsigned short WingArea;                                  // 100ths square meters
  unsigned char  Spare[60];                                 // 302 expect more data than the documented filed
                                                            // be shure there is space to hold the data
}cai302_Gdata_t;
*/

#pragma pack(pop)

//static cai302_Wdata_t cai302_Wdata;
static cai302_OdataNoArgs_t cai302_OdataNoArgs;
static cai302_OdataPilot_t cai302_OdataPilot;

// Additional sentance for CAI302 support
static BOOL cai_w(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS);
static BOOL cai_PCAIB(const char *String, NMEA_INFO *pGPS);
static BOOL cai_PCAID(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS);

BOOL cai302ParseNMEA(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS){

  if (!NMEAParser::NMEAChecksum(String) || (pGPS == NULL)){
    return FALSE;
  }

  if(strstr(String, "$PCAIB") == String){
    return cai_PCAIB(&String[7], pGPS);
  }

  if(strstr(String, "$PCAID") == String){
    return cai_PCAID(d, &String[7], pGPS);
  }

  if(strstr(String, "!w") == String){
    return cai_w(d, &String[3], pGPS);
  }

  return FALSE;

}

static BOOL cai302PutMacCready(DeviceDescriptor_t* d, double MacCready) {
  TCHAR szTmp[32];

  _stprintf(szTmp, TEXT("!g,m%d\r"), static_cast<int>(round(Units::ToUser(unKnots, MacCready) * 10.0)));
  d->Com->WriteString(szTmp);
  return (TRUE);
}

static
BOOL cai302PutBugs(DeviceDescriptor_t* d, double Bugs) {
  TCHAR szTmp[32];
  _stprintf(szTmp, TEXT("!g,u%d\r"), int((Bugs * 100) + 0.5));
  d->Com->WriteString(szTmp);
  return (TRUE);
}

static
BOOL cai302PutBallast(DeviceDescriptor_t* d, double Ballast) {
  TCHAR szTmp[32];
  _stprintf(szTmp, TEXT("!g,b%d\r"), int((Ballast * 10) + 0.5));
  d->Com->WriteString(szTmp);
  return (TRUE);
}

#if 0
void test(void){

  DWORD KeyType;
  TCHAR Buffer[MAX_PATH];
  DWORD BufSize = MAX_PATH;
  int   retries;
  HKEY hKey = NULL;

  Buffer[0] = '\0';

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE ,
      TEXT("\\Software\\Microsoft\\Today\\Items\\XCSoar"),
      0, 0, &hKey
    ) == ERROR_SUCCESS){

    if (RegQueryValueEx(hKey ,
        TEXT("DLL"),
        NULL,
        &KeyType,
        (unsigned char *)&Buffer,
        &BufSize
      ) == ERROR_SUCCESS){


    }
    else Buffer[0] = '\0';

    RegCloseKey(hKey);

    if (Buffer[0] != '\0'){

      RegDeleteKey(HKEY_LOCAL_MACHINE,
                   TEXT("\\Software\\Microsoft\\Today\\Items\\XCSoar"));

      for (retries=0; retries < 10 && DeleteFile(Buffer) == 0; retries++){
        SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0xF2, 0);
        Sleep(250*retries);
      }

    }

  }

}
#endif


BOOL cai302Open(DeviceDescriptor_t* d){
  if (d && d->Com) {
    d->Com->WriteString("\x03");
    d->Com->WriteString("LOG 0\r");
  }
  return TRUE;
}

static int DeclIndex = 128;
static int nDeclErrorCode;


BOOL cai302DeclAddWayPoint(DeviceDescriptor_t* d, const WAYPOINT *wp);


BOOL cai302Declare(DeviceDescriptor_t* d, const Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  TCHAR PilotName[25];
  TCHAR GliderType[13];
  TCHAR GliderID[13];
  TCHAR szTmp[255];
  nDeclErrorCode = 0;

  {
    ScopeUnlock unlock(CritSec_Comm); // required to avoid deadlock In StopRxThread
    d->Com->StopRxThread();
  }

  d->Com->SetRxTimeout(500);
  d->Com->WriteString("\x03");
  ExpectString(d, TEXT("$$$"));  // empty rx buffer (searching for
                                 // pattern that never occure)

  d->Com->WriteString("\x03");
  if (!ExpectString(d, TEXT("cmd>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  d->Com->WriteString("upl 1\r");
  if (!ExpectString(d, TEXT("up>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  ExpectString(d, TEXT("$$$"));

  d->Com->WriteString("O\r");
  Sleep(500); // some params come up 0 if we don't wait!
  d->Com->Read(&cai302_OdataNoArgs, sizeof(cai302_OdataNoArgs));

  if (!ExpectString(d, TEXT("up>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  d->Com->WriteString("O 0\r");  // 0=active pilot
  Sleep(1000); // some params come up 0 if we don't wait!
  d->Com->Read(&cai302_OdataPilot, min(sizeof(cai302_OdataPilot), (size_t)cai302_OdataNoArgs.PilotRecordSize+3));
  if (!ExpectString(d, TEXT("up>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  swap(cai302_OdataPilot.ApproachRadius);
  swap(cai302_OdataPilot.ArrivalRadius);
  swap(cai302_OdataPilot.EnrouteLoggingInterval);
  swap(cai302_OdataPilot.CloseTpLoggingInterval);
  swap(cai302_OdataPilot.TimeBetweenFlightLogs);
  swap(cai302_OdataPilot.MinimumSpeedToForceFlightLogging);
  swap(cai302_OdataPilot.UnitWord);
  swap(cai302_OdataPilot.MarginHeight);

  d->Com->SetRxTimeout(1500);

  d->Com->WriteString("\x03");
  if (!ExpectString(d, TEXT("cmd>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  d->Com->WriteString("dow 1\r");
  if (!ExpectString(d, TEXT("dn>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  LK_tcsncpy(PilotName, decl->PilotName, 24);
  LK_tcsncpy(GliderType, decl->AircraftType, 12);
  LK_tcsncpy(GliderID, decl->AircraftRego, 12);

  _stprintf(szTmp, TEXT("O,%-24s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r"),
    PilotName,
    cai302_OdataPilot.OldUnit,
    cai302_OdataPilot.OldTemperaturUnit,
    cai302_OdataPilot.SinkTone,
    cai302_OdataPilot.TotalEnergyFinalGlide,
    cai302_OdataPilot.ShowFinalGlideAltitude,
    cai302_OdataPilot.MapDatum,
    cai302_OdataPilot.ApproachRadius,
    cai302_OdataPilot.ArrivalRadius,
    cai302_OdataPilot.EnrouteLoggingInterval,
    cai302_OdataPilot.CloseTpLoggingInterval,
    cai302_OdataPilot.TimeBetweenFlightLogs,
    cai302_OdataPilot.MinimumSpeedToForceFlightLogging,
    cai302_OdataPilot.StfDeadBand,
    cai302_OdataPilot.ReservedVario,
    cai302_OdataPilot.UnitWord,
    cai302_OdataPilot.MarginHeight
  );

  d->Com->WriteString(szTmp);
  if (!ExpectString(d, TEXT("dn>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  _stprintf(szTmp, TEXT("G,%-12s,%-12s,%d,%d,%d,%d,%d,%d,%d,%d\r"),
            GliderType,
            GliderID,
            (int)GlidePolar::bestld,
            (int)(GlidePolar::Vbestld() * TOKPH),
            (int)(GlidePolar::FindSpeedForSinkRateAccurate(-2.0) * TOKPH),
            (int)(WEIGHTS[0] + WEIGHTS[1]),
            (int)WEIGHTS[2],
            0,
            1, //cai302_Gdata.ConfigWord,
            (int)(GlidePolar::WingArea * 1000)
            );

  d->Com->WriteString(szTmp);
  if (!ExpectString(d, TEXT("dn>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  DeclIndex = 128;

  for (int i = 0; i < decl->num_waypoints; i++)
    cai302DeclAddWayPoint(d, decl->waypoint[i]);

  if (nDeclErrorCode == 0){

    _stprintf(szTmp, TEXT("D,%d\r"), 255 /* end of declaration */);
    d->Com->WriteString(szTmp);

    d->Com->SetRxTimeout(1500);            // D,255 takes more than 800ms

    if (!ExpectString(d, TEXT("dn>"))){
      nDeclErrorCode = 1;
    }

    // todo error checking
  }

  d->Com->SetRxTimeout(500);

  d->Com->WriteString("\x03");
  ExpectString(d, TEXT("cmd>"));

  d->Com->WriteString("LOG 0\r");

  d->Com->SetRxTimeout(RXTIMEOUT);
  d->Com->StartRxThread();

  return(nDeclErrorCode == 0);

}


BOOL cai302DeclAddWayPoint(DeviceDescriptor_t* d, const WAYPOINT *wp){

  TCHAR Name[13];
  TCHAR  szTmp[128];
  int DegLat, DegLon;
  double MinLat, MinLon;
  char NoS, EoW;

  if (nDeclErrorCode != 0)
    return(FALSE);

  LK_tcsncpy(Name, wp->Name, 12);

  DegLat = (int)wp->Latitude;
  MinLat = wp->Latitude - DegLat;
  NoS = 'N';
  if(MinLat<0)
    {
      NoS = 'S';
      DegLat *= -1;
      MinLat *= -1;
    }
  MinLat *= 60;


  DegLon = (int)wp->Longitude ;
  MinLon = wp->Longitude  - DegLon;
  EoW = 'E';
  if(MinLon<0)
    {
      EoW = 'W';
      DegLon *= -1;
      MinLon *= -1;
    }
  MinLon *=60;

  _stprintf(szTmp, TEXT("D,%d,%02d%07.4f%c,%03d%07.4f%c,%s,%d\r"),
    DeclIndex,
    DegLat, MinLat, NoS,
    DegLon, MinLon, EoW,
    Name,
    (int)wp->Altitude
  );

  DeclIndex++;

  d->Com->WriteString(szTmp);

  if (!ExpectString(d, TEXT("dn>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  return(TRUE);

}

void cai302Install(DeviceDescriptor_t* d){

  _tcscpy(d->Name, TEXT("CAI 302"));
  d->ParseNMEA = cai302ParseNMEA;
  d->PutMacCready = cai302PutMacCready;
  d->PutBugs = cai302PutBugs;
  d->PutBallast = cai302PutBallast;
  d->Open = cai302Open;
  d->Declare = cai302Declare;
}

// local stuff

/*
$PCAIB,<1>,<2>,<CR><LF>
<1> Destination Navpoint elevation in meters, format XXXXX (leading zeros will be transmitted)
<2> Destination Navpoint attribute word, format XXXXX (leading zeros will be transmitted)
*/

BOOL cai_PCAIB(const char* String, NMEA_INFO *pGPS){
  (void)pGPS;
  (void)String;
  return TRUE;
}


/*
$PCAID,<1>,<2>,<3>,<4>*hh<CR><LF>
<1> Logged 'L' Last point Logged 'N' Last Point not logged
<2> Barometer Altitude in meters (Leading zeros will be transmitted)
<3> Engine Noise Level
<4> Log Flags
*hh Checksum, XOR of all bytes of the sentence after the �$� and before the �*�
*/

static bool have_Qnhaltitude=false;

BOOL cai_PCAID(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS){

  char ctemp[80];
  static short waitinit=3;

  if (waitinit>0) {
      waitinit--;
      return TRUE;
  }


  NMEAParser::ExtractParameter(String,ctemp,1);
  // This is in conflict with !w sentence providing true altitude and the relative QNH.
  // The idea is to use this value, which would require a manual setup of QNH) only if no baro altitude
  // is available  from the !w sentence (and in such case we ignore QNH as well).
  // We use a local flag to make it easier.
  // We must wait for at least the first run to see if the sequencing pcaid-!w is done, no matter the order.
  if (!have_Qnhaltitude) {
      double ps = StrToDouble(ctemp,NULL);
      UpdateBaroSource( pGPS , d, QNEAltitudeToQNHAltitude(ps));
  }

  return TRUE;
}

/*
!w,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>*hh<CR><LF>
<1>  Vector wind direction in degrees
<2>  Vector wind speed in 10ths of meters per second
<3>  Vector wind age in seconds
<4>  Component wind in 10ths of Meters per second + 500 (500 = 0, 495 = 0.5 m/s tailwind)
<5>  True altitude in Meters + 1000
<6>  Instrument QNH setting
<7>  True airspeed in 100ths of Meters per second
<8>  Variometer reading in 10ths of knots + 200
<9>  Averager reading in 10ths of knots + 200
<10> Relative variometer reading in 10ths of knots + 200
<11> Instrument MacCready setting in 10ths of knots
<12> Instrument Ballast setting in percent of capacity
<13> Instrument Bug setting
*hh  Checksum, XOR of all bytes
*/

BOOL cai_w(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pGPS){

  char ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,1);
  pGPS->ExternalWindAvailable = TRUE;
  pGPS->ExternalWindSpeed = (StrToDouble(ctemp,NULL) / 10.0);
  NMEAParser::ExtractParameter(String,ctemp,0);
  pGPS->ExternalWindDirection = AngleLimit360(StrToDouble(ctemp,NULL)+180);


  NMEAParser::ExtractParameter(String,ctemp,4);

  // this is true altitude, already corrected for non-standard temp and pressure.
  // It is the altitude relative to the current (following) QNH
  // So we dont do any qnh conversion!
  double qnhalt=StrToDouble(ctemp,NULL)-1000;

  // minimalistic check, to be sure we are not excluding PCAID without a real qnh
  if (qnhalt!=0) {
      have_Qnhaltitude=true;
      UpdateBaroSource( pGPS , d, qnhalt);
  }

  // We DO need to set the QNH altitude as well, if we use the previous baro altitude!
  NMEAParser::ExtractParameter(String,ctemp,5);
  if (have_Qnhaltitude) {
      UpdateQNH(StrToDouble(ctemp, NULL));
  }

  NMEAParser::ExtractParameter(String,ctemp,6);
  pGPS->AirspeedAvailable = TRUE;

  pGPS->TrueAirspeed = (StrToDouble(ctemp,NULL) / 100.0);
  // if qnhalt is zero, IAS is the TAS, more or less, so no problems
  pGPS->IndicatedAirspeed = IndicatedAirSpeed(pGPS->TrueAirspeed, QNHAltitudeToQNEAltitude(qnhalt));


  NMEAParser::ExtractParameter(String,ctemp,7);
  double Vario = Units::ToSys(unKnots, (StrToDouble(ctemp,NULL) - 200.0) / 10.0);;
  UpdateVarioSource(*pGPS, *d, Vario);

  NMEAParser::ExtractParameter(String,ctemp,10);
  d->RecvMacCready(Units::ToSys(unKnots, StrToDouble(ctemp, nullptr) / 10.0));

  NMEAParser::ExtractParameter(String,ctemp,11);
  d->RecvBallast(StrToDouble(ctemp, nullptr) / 100.0);

  NMEAParser::ExtractParameter(String,ctemp,12);
  d->RecvBugs(StrToDouble(ctemp, nullptr) / 100.0);

  return TRUE;
}
