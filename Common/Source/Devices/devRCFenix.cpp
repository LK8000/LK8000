/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

//__Version_1.0_ Ulrich Heynen 13.01.2022


//_____________________________________________________________________includes_
#include "externs.h"
#include "devRCFenix.h"
#include "resource.h"
#include "LKInterface.h"
#include "McReady.h"
#include "Comm/wait_ack.h"
#include "utils/printf.h"

#define MAX_VAL_STR_LEN    60

//____________________________________________________________class_definitions_

// #############################################################################
// *****************************************************************************
//
//   DevRCFenix
//
// *****************************************************************************
// #############################################################################




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
void DevRCFenix::Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, GetName());
  d->Open = Open;
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = FenixPutMacCready;
  d->PutBugs      = FenixPutBugs;
  d->PutBallast   = FenixPutBallast;
  d->PutQNH       = PutQNH;
  d->Declare      = DeclareTask;

  d->Config       = Config;
  d->PutTarget    = PutTarget;
} // Install()

//static 
BOOL DevRCFenix::Open(DeviceDescriptor_t* d) {
  if (!d) {
    return false;
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  tstring setMcBal = _T("RCDT,SET,MC_BAL,");
  

  // mc (float)
  if (IsDirOutput(PortIO.MCDir)) {
    setMcBal += to_tstring(static_cast<float>(MACCREADY));
  }
  setMcBal += _T(",");
  
  // ballast (uint16_t kg)
  if (IsDirOutput(PortIO.BALDir)) {
    setMcBal += to_tstring(static_cast<uint16_t>(GlidePolar::BallastLitres));
  }
  setMcBal += _T(",");
  
  // bugs (uint8_t %)
  if (IsDirOutput(PortIO.BUGDir)) {
    setMcBal += to_tstring(static_cast<uint8_t>(CalculateLXBugs(BUGS)));
  }
  // brightness,vario_vol,sc_vol
  setMcBal += _T(",,,,");
  
  // qnh (uint16_t mbar)
  if (IsDirOutput(PortIO.QNHDir)) {
    uint16_t mb_qnh = QNH;
    setMcBal += to_tstring(mb_qnh);
  }

  SendNmea(d, setMcBal.c_str());
  SendNmea(d, _T("RCDT,GET,MC_BAL"));
  SendNmea(d, _T("PFLX0,LXWP0,1,LXWP1,1,LXWP2,1,LXWP3,1,GPRMB,5"));

  ResetMultitargetSync();

  return TRUE;
}


extern BOOL LX_EOS_ERA_bValid;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWPn sentences.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevRCFenix::ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info) {

  auto wait_ack = d->lock_wait_ack();
  if (wait_ack && wait_ack->check(sentence)) {
    return TRUE;
  }

  if (!info) {
    return FALSE;
  }

  if (!NMEAParser::NMEAChecksum(sentence)){
    return FALSE;
  }
  if (strncmp("$RCDT", sentence, 5) == 0) {
    return LXDT(d, sentence + 6, info);
  }
  if (strncmp("$LXBC", sentence, 5) == 0) {
    return LXBC(d, sentence + 6, info);
  }
  if (strncmp("$LXWP2", sentence, 6) == 0) {
    return LXWP2(d, sentence + 7, info);
  }
  if (strncmp("$LXWP0", sentence, 6) == 0) {
    return LXWP0(d, sentence + 7, info);
  }
  if(strncmp("$GPRMB", sentence, 6) == 0) {
    return GPRMB(d, sentence + 7, info);
  }
  if (strncmp("$LXWP1", sentence, 6) == 0) {
    return LXWP1(d, sentence + 7, info);
  }

  // do not configure during declaration
  if (Declare()) {
    return FALSE;
  }

  static char lastSec =0;
  if (info->Second != lastSec) {
    // execute every second only if no task is declaring
    lastSec = info->Second;

    if (((info->Second + 2) % 4) == 0) {
      SendNmea(d, _T("RCDT,GET,SENS"));
    }
    if (((info->Second + 4) % 4) == 0) {
      SendNmea(d, _T("RCDT,GET,NAVIGATE,0"));
    }
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if (IsDirOutput(PortIO.STFDir)) {
    bool circling = MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING);
    static bool Thermalmode = !circling;
    if (circling != Thermalmode) {
      if (circling) {
        SendNmea(d, _T("RCDT,SET,SC_VAR,0")); 
      }
      else {
        SendNmea(d, _T("RCDT,SET,SC_VAR,1"));
      }
      Thermalmode = circling;
    }
  }
  return FALSE;
} // ParseNMEA()

BOOL DevRCFenix::Config(DeviceDescriptor_t* d) {

  std::unique_ptr<WndForm>  wf(dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_DEV_LXNAV_L : IDR_XML_DEV_LXNAV_P));
  if(wf) {
    Device(d);
    WndButton *wBt = wf->FindByName<WndButton>(TEXT("cmdClose"));
    if(wBt){
      wBt->SetOnClickNotify(OnCloseClicked);
    }

    wBt = wf->FindByName<WndButton>(TEXT("cmdIGCDownload"));		
#ifndef IGC_DOWNLOAD	
		// IGC Download not yet supported by Fenix 
		wBt->SetVisible(false);  // hide IGC Download button
#else
    if(wBt){
      wBt->SetOnClickNotify(OnIGCDownloadClicked);
    }
#endif
    wBt = wf->FindByName<WndButton>(TEXT("cmdValues"));
    if(wBt){
      wBt->SetOnClickNotify(OnValuesClicked);
    }
    ShowValues(false);

    wf->SetCaption(_T("RC Fenix Config"));
    ShowData(wf.get(), d);
    wf->ShowModal();
    wf->SetTimerNotify(0, nullptr);
    Device(nullptr);
  }
  return TRUE;
}

extern int DeviceASCIIConvert(TCHAR *pDest,const TCHAR *pSrc, int size=11);

static
BOOL FormatTP(TCHAR* DeclStrings, int num, int total,const WAYPOINT *wp) {
  if(DeclStrings) {
    int  lat = 0;
    int  lon = 0;
    TCHAR Name[60] =_T("");
    if(wp) {
      lat = (int)(wp->Latitude*60000.0);
      lon = (int)(wp->Longitude*60000.0);
      DeviceASCIIConvert(Name, wp->Name,20);
    }
    _stprintf(DeclStrings,
              TEXT("RCDT,SET,TP,%i,%i,%i,%i,%s"),
              num, total+2, lat, lon, Name);
    return true;
  }
  return false;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Writes declaration into the logger.
///
/// @param d           device descriptor to be installed
/// @param lkDecl      LK task declaration data
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  declaration has been written successfully
/// @retval false error during declaration (description in @p errBuf)
///
//static
BOOL DevRCFenix::DeclareTask(DeviceDescriptor_t* d,
    const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]) {

  if (!CheckWPCount(*lkDecl,Decl::min_wp_count - 2, Decl::max_wp_count - 2, errBufSize, errBuf)) {
    return false;
  }
  ShowProgress(decl_enable);
  ShowProgress(decl_send);
  Declare(true);

  TCHAR Pilot[64];
  _tcscpy(Pilot , lkDecl->PilotName); //copy to local instance (Multi driver support)

  TCHAR PilotName[12] = _T("");
  TCHAR PilotSurName[12] = _T("");;
  TCHAR* NamePtr= _tcstok(Pilot, _T(" ,.-:_"));
  if (NamePtr) {
    DeviceASCIIConvert(PilotName, NamePtr ,11);
  }
  TCHAR* SurNamePtr = _tcstok (nullptr, _T(" ,.-:_"));
  if (SurNamePtr) {
    DeviceASCIIConvert(PilotSurName, SurNamePtr ,11);
  }

  TCHAR AircraftType[12];   DeviceASCIIConvert(AircraftType,  lkDecl->AircraftType    ,11);
  TCHAR AircraftReg[12];    DeviceASCIIConvert(AircraftReg,   lkDecl->AircraftRego    ,11);
  TCHAR AircraftCompID[12]; DeviceASCIIConvert(AircraftCompID,lkDecl->CompetitionID   ,11);
  TCHAR AircraftClass[12];  DeviceASCIIConvert(AircraftClass, lkDecl->CompetitionClass,11);

  int wpCount = lkDecl->num_waypoints;
  int totalLines = (wpCount * 2) + 2 + 4; // N * w(p + zone) + takeoff + landing + header
  auto DeclStrings = std::make_unique<TCHAR[][256]>(totalLines);
  INT i = 0;

  _stprintf(DeclStrings[i++], _T("RCDT,SET,PILOT,%s,%s"), PilotName, PilotSurName);
  _stprintf(DeclStrings[i++], _T("RCDT,SET,GLIDER,%s,%s,%s,%s"), 
                              AircraftType, AircraftReg, AircraftCompID, AircraftClass);

  if (UseAATTarget()) {
    _stprintf(DeclStrings[i++], TEXT("RCDT,SET,TSK_PAR,0,0,%02i:%02i"), 
                (int)(AATTaskLength / 60),  (int)(AATTaskLength-((int)(AATTaskLength/60)*60)));
  }
  else {
    _stprintf(DeclStrings[i++], TEXT("RCDT,SET,TSK_PAR,0,0,00:00"));
  }
  int num=0;

  int dir=0,autonxt=1,isline=0,a1=45,a2=45,a21=5000,r1=5000,r2=500, elev = WayPointList[HomeWaypoint].Altitude;

  WAYPOINT* pTakeOff = nullptr;
  if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint) && DeclTakeoffLanding) {
    pTakeOff = &WayPointList[HomeWaypoint];
  }
  FormatTP(DeclStrings[i++], num++ , wpCount, pTakeOff);   // Takeoff

  

  for (int ii = 0; ii < wpCount; ii++) {
    FormatTP(DeclStrings[i++], num, wpCount, lkDecl->waypoint[ii]);   //  Task waypoints

    sector_param param = GetTaskSectorParameter(ii);
    switch (param.type) {
      case sector_type_t::LINE  : isline=1; r1=param.radius; a1 = 0  ; a2=180; r2=0  ; a21 =0; break;
      case sector_type_t::SECTOR: isline=0; r1=param.radius; a1 = 45 ; a2=180; r2=0  ; a21 =0; break;
      case sector_type_t::CIRCLE: isline=0; r1=param.radius; a1 =180 ; a2=180; r2=0  ; a21 =0; break;
      case sector_type_t::DAe   : isline=0; r1=param.radius; a1 = 45 ; a2=180; r2=500; a21 =0; break;
      case sector_type_t::ESS_CIRCLE: isline=0; r1=param.radius; a1 =180 ; a2=180; r2=0  ; a21 =0; break;
      default:
        assert(false);
        break;
    }

    elev = WayPointList[HomeWaypoint].Altitude;
    _stprintf(DeclStrings[i++], _T("RCDT,SET,ZONE,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i"),
                num, dir, autonxt, isline, a1, a2, a21, r1, r2, elev);
    num++;
  }

  FormatTP(DeclStrings[i++], num++ , wpCount, pTakeOff);   // Landing

  bool success  = false;

  wait_ack_shared_ptr wait_ack = d->make_wait_ack("$RCDT,ANS,OK*59\n");

  for (int ii = 0; ii < i; ii++) {

    TestLog(_T(". RC Fenix Decl: > %s"), DeclStrings[ii]);

    success =  SendNmea(d, DeclStrings[ii]);
    if (success) {
      success = wait_ack->wait(20000);
    }

    TestLog(_T(". RC Fenix Decl: < %s"), success ? _T("$RCDT,ANS,OK*59"): _T("failed"));

    if (!success) {
      break; // failed ...
    }
  }

  wait_ack = nullptr;

  Declare(false);
  ShowProgress(decl_disable);
  return success;
} // DeclareTask()

BOOL DevRCFenix::FenixPutMacCready(DeviceDescriptor_t* d, double MacCready) {
  if (!d) {
    return false;
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if (!IsDirOutput(PortIO.MCDir)) {
    return false;
  }

  TCHAR  szTmp[MAX_NMEA_LEN];
  lk::snprintf(szTmp, _T("RCDT,SET,MC_BAL,%.1f,,,,,,"), MacCready);

  SendNmea(d,szTmp);
  return TRUE;
}


BOOL DevRCFenix::FenixPutBallast(DeviceDescriptor_t* d, double Ballast) {
  if (!d) {
    return false;
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if (!IsDirOutput(PortIO.BALDir)) {
    return false;
  }

  TCHAR szTmp[MAX_NMEA_LEN];
  lk::snprintf(szTmp, _T("RCDT,SET,MC_BAL,,%.0f,,,,,"), GlidePolar::BallastLitres);

  SendNmea(d, szTmp);

  return TRUE;
}


BOOL DevRCFenix::FenixPutBugs(DeviceDescriptor_t* d, double Bugs) {
  if (!d) {
    return false;
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(!IsDirOutput(PortIO.BUGDir)) {
    return false;
  }
  TCHAR szTmp[MAX_NMEA_LEN];

  double fLXBugs = CalculateLXBugs(Bugs);
  lk::snprintf(szTmp, _T("RCDT,SET,MC_BAL,,,%.0f,,,,"), fLXBugs);

  SendNmea(d,szTmp);
  return TRUE;
}


BOOL DevRCFenix::PutQNH(DeviceDescriptor_t* d, double qnh_mb) {
  if (!d) {
    return false;
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if (!IsDirOutput(PortIO.QNHDir)) {
    return false;
  }
  TCHAR szTmp[MAX_NMEA_LEN];
  lk::snprintf(szTmp, _T("RCDT,SET,MC_BAL,,,,,,,%.0f"), qnh_mb);

  SendNmea(d, szTmp);

  return true;
}


BOOL DevRCFenix::PutTarget(DeviceDescriptor_t* d, const WAYPOINT& wpt) {
  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(PortIO.T_TRGTDir == TP_Off) {
    return false;
  }

  TCHAR  szTmp[MAX_NMEA_LEN];

  if (PortIO.T_TRGTDir  == TP_VTARG) {                                  
    int rwdir = 0; 
    int landable =0;

    if ((wpt.Flags & LANDPOINT) > 0) {
      landable = 1;
      rwdir    = wpt.RunwayDir;
    }

    _sntprintf( szTmp,MAX_NMEA_LEN, TEXT("RCDT,SET,NAVIGATE,1,%s,%i,%i,%i,%i,%s,%i"),
                wpt.Name, 
                (int) (wpt.Latitude * 60000.0), 
                (int) (wpt.Longitude * 60000.0),
                (int) (wpt.Altitude + 0.5),
                landable, wpt.Freq, rwdir);
    
    // $RCDT,SET,NAVIGATE,0,MARIBOR,2788794,941165,267,1,119.200,14*2A<CR><LF>
    SendNmea(d, szTmp);
    TestLog(TEXT("Send navigation Target Fenix: ($RCDT) %s"), wpt.Name);
    SetDataText(d, _T_TRGT,  wpt.Name);
  }
  else if (PortIO.T_TRGTDir  == TP_GPRMB) {
    // GPRMB,A,,,,H>TAKEOFF,5144.78,N,00616.70,E,,,A

    int DegLat = wpt.Latitude;
    double MinLat = wpt.Latitude - DegLat;
    char NoS = 'N';
    if ((MinLat < 0) || (((MinLat - DegLat) == 0) && (DegLat < 0))) {
      NoS = 'S';
      DegLat *= -1; 
      MinLat *= -1;
    }
    MinLat *= 60;

    int DegLon = wpt.Longitude ;
    double MinLon = wpt.Longitude - DegLon;
    char EoW = 'E';
    if ((MinLon < 0) || (((MinLon - DegLon) == 0) && (DegLon < 0))) {
      EoW = 'W';
      DegLon *= -1;
      MinLon *= -1;
    }
    MinLon *=60;

    lk::snprintf(szTmp, _T("GPRMB,A,,,%s,%02d%05.2f,%c,%03d%05.2f,%c,,,,A"),
                                    wpt.Name, DegLat, MinLat, NoS, DegLon, MinLon, EoW);

    SendNmea(d, szTmp);
    TestLog(TEXT("Send navigation Target Fenix: ($GPRMB) %s"), wpt.Name);
    SetDataText(d, _T_TRGT,  wpt.Name);
  }


  return TRUE;
}
