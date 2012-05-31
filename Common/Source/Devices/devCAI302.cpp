/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: devCAI302.cpp,v 8.2 2010/12/13 10:03:50 root Exp root $
*/


// CAUTION!
// cai302ParseNMEA is called from com port read thread
// all other functions are called from windows message loop thread


#define  LOGSTREAM 0


#include "externs.h"
#include "Utils.h"
#include "Parser.h"
#include "Port.h"
#include "McReady.h"

#include "devCAI302.h"

extern bool UpdateBaroSource(NMEA_INFO* GPS_INFO, const short parserid, const PDeviceDescriptor_t d, const double fAlt);


using std::min;
using std::max;

#define  CtrlC  0x03
#define  swap(x)      x = ((((x<<8) & 0xff00) | ((x>>8) & 0x00ff)) & 0xffff)

#pragma pack(push, 1)                  // force byte allignement

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


#pragma pack(pop)

//static cai302_Wdata_t cai302_Wdata;
static cai302_OdataNoArgs_t cai302_OdataNoArgs;
static cai302_OdataPilot_t cai302_OdataPilot;

// Additional sentance for CAI302 support
static BOOL cai_w(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL cai_PCAIB(TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL cai_PCAID(PDeviceDescriptor_t d,TCHAR *String, NMEA_INFO *GPS_INFO);
static BOOL cai302Install(PDeviceDescriptor_t d); 

static int  MacCreadyUpdateTimeout = 0;
static int  BugsUpdateTimeout = 0;
static int  BallastUpdateTimeout = 0;


BOOL cai302ParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
  
  if (!NMEAParser::NMEAChecksum(String) || (GPS_INFO == NULL)){
    return FALSE;
  }
  
  if(_tcsstr(String,TEXT("$PCAIB")) == String){
    return cai_PCAIB(&String[7], GPS_INFO);
  }

  if(_tcsstr(String,TEXT("$PCAID")) == String){
    return cai_PCAID(d,&String[7], GPS_INFO);
  }

  if(_tcsstr(String,TEXT("!w")) == String){
    return cai_w(d, &String[3], GPS_INFO);
  }

  return FALSE;

}


BOOL cai302PutMacCready(PDeviceDescriptor_t d, double MacCready){

  TCHAR  szTmp[32];

  _stprintf(szTmp, TEXT("!g,m%d\r"), int(((MacCready * 10) / KNOTSTOMETRESSECONDS) + 0.5));

  d->Com->WriteString(szTmp);

  MacCreadyUpdateTimeout = 2;

  return(TRUE);

}


BOOL cai302PutBugs(PDeviceDescriptor_t d, double Bugs){

  TCHAR  szTmp[32];

  _stprintf(szTmp, TEXT("!g,u%d\r"), int((Bugs * 100) + 0.5));

  d->Com->WriteString(szTmp);

  BugsUpdateTimeout = 2;

  return(TRUE);

}


BOOL cai302PutBallast(PDeviceDescriptor_t d, double Ballast){

  TCHAR  szTmp[32];

  _stprintf(szTmp, TEXT("!g,b%d\r"), int((Ballast * 10) + 0.5));

  d->Com->WriteString(szTmp);

  BallastUpdateTimeout = 2;

  return(TRUE);

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


BOOL cai302Open(PDeviceDescriptor_t d, int Port){

//test();

  if (!SIMMODE) {
    d->Com->WriteString(TEXT("\x03"));
    d->Com->WriteString(TEXT("LOG 0\r"));
  }

  return(TRUE);
}

BOOL cai302Close(PDeviceDescriptor_t d){
  (void)d;
  return(TRUE);
}


static int DeclIndex = 128;
static int nDeclErrorCode; 


BOOL cai302DeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp);


BOOL cai302Declare(PDeviceDescriptor_t d, Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[])
{
  TCHAR PilotName[25];
  TCHAR GliderType[13];
  TCHAR GliderID[13];
  TCHAR szTmp[255];
  nDeclErrorCode = 0;

  d->Com->StopRxThread();

  d->Com->SetRxTimeout(500);
  d->Com->WriteString(TEXT("\x03"));
  ExpectString(d, TEXT("$$$"));  // empty rx buffer (searching for
                                 // pattern that never occure)

  d->Com->WriteString(TEXT("\x03"));
  if (!ExpectString(d, TEXT("cmd>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  d->Com->WriteString(TEXT("upl 1\r"));
  if (!ExpectString(d, TEXT("up>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  ExpectString(d, TEXT("$$$"));

  d->Com->WriteString(TEXT("O\r"));
  Sleep(500); // some params come up 0 if we don't wait!
  d->Com->Read(&cai302_OdataNoArgs, sizeof(cai302_OdataNoArgs));
  
  if (!ExpectString(d, TEXT("up>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  d->Com->WriteString(TEXT("O 0\r"));  // 0=active pilot
  Sleep(1000); // some params come up 0 if we don't wait!
  d->Com->Read(&cai302_OdataPilot, min(sizeof(cai302_OdataPilot), (unsigned)cai302_OdataNoArgs.PilotRecordSize+3));
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

  d->Com->WriteString(TEXT("\x03"));
  if (!ExpectString(d, TEXT("cmd>"))){
    nDeclErrorCode = 1;
    return(FALSE);
  }

  d->Com->WriteString(TEXT("dow 1\r"));
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
            (int)(GlidePolar::Vbestld * TOKPH),
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

  d->Com->WriteString(TEXT("\x03"));
  ExpectString(d, TEXT("cmd>"));

  d->Com->WriteString(TEXT("LOG 0\r"));

  d->Com->SetRxTimeout(RXTIMEOUT);
  d->Com->StartRxThread();

  return(nDeclErrorCode == 0);

}


BOOL cai302DeclAddWayPoint(PDeviceDescriptor_t d, const WAYPOINT *wp){

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


BOOL cai302IsLogger(PDeviceDescriptor_t d){
	 (void)d;
  return(TRUE);
}


BOOL cai302IsGPSSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}

BOOL cai302IsBaroSource(PDeviceDescriptor_t d){
	(void)d;
  return(TRUE);
}

BOOL cai302Install(PDeviceDescriptor_t d){

  _tcscpy(d->Name, TEXT("CAI 302"));
  d->ParseNMEA = cai302ParseNMEA;
  d->PutMacCready = cai302PutMacCready;
  d->PutBugs = cai302PutBugs;
  d->PutBallast = cai302PutBallast;
  d->Open = cai302Open;
  d->Close = cai302Close;
  d->Init = NULL;
  d->LinkTimeout = NULL;
  d->Declare = cai302Declare;
  d->IsLogger = cai302IsLogger;
  d->IsGPSSource = cai302IsGPSSource;
  d->IsBaroSource = cai302IsBaroSource;

  return(TRUE);

}


BOOL cai302Register(void){
  return(devRegister(
    TEXT("CAI 302"), 
    (1l << dfGPS)
      | (1l << dfLogger)
      | (1l << dfSpeed)
      | (1l << dfVario)
      | (1l << dfBaroAlt)
      | (1l << dfWind),
    cai302Install
  ));
}

// local stuff

/*
$PCAIB,<1>,<2>,<CR><LF>
<1> Destination Navpoint elevation in meters, format XXXXX (leading zeros will be transmitted)
<2> Destination Navpoint attribute word, format XXXXX (leading zeros will be transmitted)
*/

BOOL cai_PCAIB(TCHAR *String, NMEA_INFO *GPS_INFO){
  (void)GPS_INFO;
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

BOOL cai_PCAID(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){
/*
	(void)GPS_INFO;
	(void)String;
*/
  TCHAR ctemp[80];

  NMEAParser::ExtractParameter(String,ctemp,1);
  double ps = StrToDouble(ctemp,NULL);
  UpdateBaroSource( GPS_INFO ,0, d,  AltitudeToQNHAltitude(ps));


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

BOOL cai_w(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *GPS_INFO){

  TCHAR ctemp[80];

  
  NMEAParser::ExtractParameter(String,ctemp,1);
  GPS_INFO->ExternalWindAvailable = TRUE;
  GPS_INFO->ExternalWindSpeed = (StrToDouble(ctemp,NULL) / 10.0);
  NMEAParser::ExtractParameter(String,ctemp,0);
  GPS_INFO->ExternalWindDirection = StrToDouble(ctemp,NULL);  


  NMEAParser::ExtractParameter(String,ctemp,4);

  UpdateBaroSource( GPS_INFO , 0, d,   AltitudeToQNHAltitude( StrToDouble(ctemp, NULL) - 1000));

//  ExtractParameter(String,ctemp,5);
//  GPS_INFO->QNH = StrToDouble(ctemp, NULL) - 1000;
  
  NMEAParser::ExtractParameter(String,ctemp,6);
  GPS_INFO->AirspeedAvailable = TRUE;
  GPS_INFO->TrueAirspeed = (StrToDouble(ctemp,NULL) / 100.0);
  GPS_INFO->IndicatedAirspeed = GPS_INFO->TrueAirspeed / AirDensityRatio(GPS_INFO->BaroAltitude);
  
  NMEAParser::ExtractParameter(String,ctemp,7);
  GPS_INFO->VarioAvailable = TRUE;
  GPS_INFO->Vario = ((StrToDouble(ctemp,NULL) - 200.0) / 10.0) * KNOTSTOMETRESSECONDS;

  NMEAParser::ExtractParameter(String,ctemp,10);
  GPS_INFO->MacReady = (StrToDouble(ctemp,NULL) / 10.0) * KNOTSTOMETRESSECONDS;
  if (MacCreadyUpdateTimeout <= 0)
    MACCREADY = GPS_INFO->MacReady;
  else
    MacCreadyUpdateTimeout--; 

  NMEAParser::ExtractParameter(String,ctemp,11);
  GPS_INFO->Ballast = StrToDouble(ctemp,NULL) / 100.0;
  if (BallastUpdateTimeout <= 0)
    BALLAST = GPS_INFO->Ballast;
  else 
    BallastUpdateTimeout--;


  NMEAParser::ExtractParameter(String,ctemp,12);
  GPS_INFO->Bugs = StrToDouble(ctemp,NULL) / 100.0;
  if (BugsUpdateTimeout <= 0)
    BUGS = GPS_INFO->Bugs;
  else 
    BugsUpdateTimeout--;

  // JMW update audio functions etc.
  TriggerVarioUpdate();

  return TRUE;

}

