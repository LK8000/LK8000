/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

//__Version_1.0_ Ulrich Heynen 13.01.2022


//_____________________________________________________________________includes_


#include <time.h>
#include "externs.h"
#include "utils/stringext.h"
#include "devRCFenix.h"


#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "dlgIGCProgress.h"
#include "Util/Clamp.hpp"
#include "OS/Sleep.h"


#include "externs.h"
#include "Baro.h"
#include "Utils.h"
#include "LKInterface.h"
#include "InputEvents.h"
#include "McReady.h"
#include "Time/PeriodClock.hpp"
#include "Calc/Vario.h"
#include <queue>
#include "Thread/Mutex.hpp"
#include "Thread/Cond.hpp"
#include "Radio.h"

unsigned int uiFenixDebugLevel = 1;
extern bool UpdateQNH(const double newqnh);

#define NANO_PROGRESS_DLG
#define BLOCK_SIZE 32
#define _deb (0)

PDeviceDescriptor_t DevRCFenix::m_pDevice=NULL;
BOOL DevRCFenix::m_bShowValues = false;
BOOL DevRCFenix::bIGC_Download = false;
BOOL DevRCFenix::m_bDeclare = false;
BOOL DevRCFenix::m_bRadioEnabled = true;
BOOL DevRCFenix::m_bTriggered = false;
#define TIMEOUTCHECK



#define MAX_NMEA_PAR_LEN    30
#define MAX_VAL_STR_LEN    60


BOOL RCFenix_bValid = false;




typedef union{
  uint16_t val;
  uint8_t byte[2];
} ConvUnion;     


extern Mutex  CritSec_LXDebugStr;
extern TCHAR LxValueStr[_LAST][ MAX_VAL_STR_LEN];

extern BOOL IsDirInput( DataBiIoDir IODir);
extern BOOL IsDirOutput( DataBiIoDir IODir);
extern void UpdateValueTxt(WndProperty *wp,  ValueStringIndex Idx);




//____________________________________________________________class_definitions_

// #############################################################################
// *****************************************************************************
//
//   DevRCFenix
//
// *****************************************************************************
// #############################################################################


//  NANO 3Commands:

/// LOGBOOK
/// Reading logbook info (number of flights)
//  Query : "$PLXVC,LOGBOOK,R*<checksum><cr><lf>"
//  Answer: "$PLXVC,LOGBOOK,A,<number_of_flights>*<checksum><cr><lf>"
/// Reading logbook (number of flights)
//  Query : "$PLXVC,LOGBOOK,R,<startflight>,<endflight>*<checksum><cr><lf>"
//  Answer: "$PLXVC,LOGBOOK,A,<fl>,<n>,<name>,<date>,<takeoff>,<landing>*<checksum><cr><lf>"
//  fl      : flight number
//  n       : number of flights in logbook
//  name    : filename, it can be igc or kml
//  date    : date of flight
//  takeoff : hhmmss
//  landing : hhmmss
//  This line is repeated for n times, where n==(endflight-startflight)

/// FLIGHT
/// Reading flight
//  Query : "$PLXVC,FLIGHT,R,<filename>,<startrow>,<endrow>*<checksum><cr><lf>"
//  Answer: "$PLXVC,FLIGHT,A,<filename>,<row>,<number of rows>,<content of row>*<checksum><cr><lf>"
//  This line is repeated for n times, where n==(endrow-startrow)

/// DECL
/// Reading declaration
//  Query : "$PLXVC,DECL,R,<startrow>,<endrow>*<checksum><cr><lf>"
//  Answer: "$PLXVC,DECL,A,<row>,<number of rows>,<content of row>*<checksum><cr><lf>"
//  This line is repeated for n times, where n==(endrow-startrow)
/// Writing declaration
//  Query : "$PLXVC,DECL,W,<row>,<number of rows>,<content of row>*<checksum><cr><lf>"
//  Output file is closed, when row==number of rows.
//  Confirm: "$PLXVC,DECL,C,<row>*<checksum><cr><lf>"

/// INFO
/// Reading basic info about NANO
//  Query : "$PLXVC,INFO,R*<checksum><cr><lf>"
//  Answer: "$PLXVC,INFO,A,<name>,<version>,<ver.date>,<HW ident>,<batt voltage>,
//           <backupbatt voltage>,<pressure alt>,<ENL>,<status>*<checksum><cr><lf>"
//  ENL   : 0..999
//  status: Stop=0,CanStop=1,Start=2


BOOL  DevRCFenix::Values( PDeviceDescriptor_t d)
{
  bool res = false;

    if(d != NULL)
      if( Port() == d->PortNumber)
      {
        if( Port() >= 0)   res = true;
      }

  return res;

}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
void DevRCFenix::Install(PDeviceDescriptor_t d) {
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = FenixPutMacCready;
  d->PutBugs      = FenixPutBugs;
  d->PutBallast   = FenixPutBallast;
  d->Open         = Open;
  d->LinkTimeout  = GetTrue;
  d->Declare      = DeclareTask;
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  d->Config       = Config;

  d->IsRadio        = FenixRadioEnabled;
  d->PutVolume      = FenixPutVolume;
  d->PutSquelch     = FenixPutSquelch;
  d->PutFreqActive  = FenixPutFreqActive;
  d->PutFreqStandby = FenixPutFreqStandby;
  d->StationSwap    = FenixStationSwap;
  d->PutRadioMode   = FenixRadioMode;
  d->ParseStream    = FenixParseStream;

  StartupStore(_T(". %s installed (platform=%s test=%u)%s"),
    GetName(),
    PlatfEndian::IsBE() ? _T("be") : _T("le"),
    PlatfEndian::To32BE(0x01000000), NEWLINE);
#ifndef  TESTBENCH
	uiFenixDebugLevel = 0;
#endif	
	
} // Install()


namespace {
  std::queue<uint8_t> Fenixbuffered_data;
  Mutex Fenixmutex;
  Cond Fenixcond;
  bool bFenixBinMode= false;
}

bool FenixBlockReceived() {
  ScopeLock lock(Fenixmutex);
  return (!Fenixbuffered_data.empty());
}
  
bool IsFenixInBinaryMode() {
  ScopeLock lock(Fenixmutex);
  return bFenixBinMode;
}

bool SetFenixBinaryModeFlag(bool bBinMode) {
  ScopeLock lock(Fenixmutex);
  bool OldVal = bFenixBinMode;
  bFenixBinMode = bBinMode;
  if(!bFenixBinMode) {
    // same as clear() but free allocated memory.
    Fenixbuffered_data = std::queue<uint8_t>();
  }
  return OldVal;
}



BOOL DevRCFenix::FenixParseStream(DeviceDescriptor_t *d, char *String, int len, NMEA_INFO *GPS_INFO) {
  if ((!d) || (!String) || (!len)) {
    return FALSE;
  }
  
  static BOOL slowdown = false;
    if (!IsFenixInBinaryMode()) {    
      if(slowdown)
      {
         SendNmea(d, TEXT("PFLX0,LXWP0,1,LXWP1,5,LXWP2,1,LXWP3,1,GPRMB,5"));

         SendNmea( d, _T("RCDT,SET,BC_INT,AHRS,0.5,SENS,2.0"));
      //   SendNmea( d, _T("RCDT,GET,BC_INT"));
        StartupStore(TEXT("NMEA SLOWDOWN OFF!!"));      
        slowdown = false;
      }       
      return FALSE;      
   }
  
  if(!slowdown)
  {
     SendNmea(d, TEXT("PFLX0,LXWP0,100,LXWP1,100,LXWP2,100,LXWP3,100,GPRMB,100"));      
     SendNmea( d, _T("RCDT,SET,BC_INT,ALL,0.0"));

     StartupStore(TEXT("NMEA SLOWDOWN"));
     slowdown = true;
  }    
   
   
  ScopeLock lock(Fenixmutex);

  for (int i = 0; i < len; i++) {
    Fenixbuffered_data.push((uint8_t)String[i]);
  }
  Fenixcond.Broadcast();

  return  true;
}




uint8_t FenixRecChar( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout) {
  ScopeLock lock(Fenixmutex);

  while(Fenixbuffered_data.empty()) {
    Poco::Thread::sleep(1);
    Poco::Thread::yield();

    if(!Fenixcond.Wait(Fenixmutex, Timeout)) 
    {
      return REC_TIMEOUT_ERROR;
    }
  }
  if(inchar) {
    *inchar = Fenixbuffered_data.front();
   
  }
  Fenixbuffered_data.pop();

  return REC_NO_ERROR;
}

uint8_t FenixRecChar16(DeviceDescriptor_t *d, uint16_t *inchar, uint16_t Timeout) {
  ConvUnion tmp;
  int error = FenixRecChar(d, &(tmp.byte[0]), Timeout);
  if (error == REC_NO_ERROR) {
    error = FenixRecChar(d, &(tmp.byte[1]), Timeout);
  }
  *inchar = tmp.val;
  return error;
}
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


BOOL DevRCFenix::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{
 if (Declare()) return false ;  // do not configure during declaration
 
 
 if( IsFenixInBinaryMode()) return false;
static char lastSec =0;
  if( /*!Declare() &&*/ (info->Second != lastSec))  // execute every second only if no task is declaring
  {
    lastSec = info->Second;
    if((info->Second % 10) ==0) // config every 10s (not on every xx $GPGGA as there are 10Hz GPS now
    {
      SetupFenix_Sentence(d);
    }

    static int old_overindex = -1;    // call every 10s or on change
    static int old_overmode = -1;
    if( ( ((info->Second+5) %10) ==0) || (OvertargetMode != old_overmode) || (GetOvertargetIndex() != old_overindex))
    {
      PutTarget(d);
      old_overindex = GetOvertargetIndex();;
      old_overmode  = OvertargetMode;
    }
    if( ((info->Second+2) %4) ==0)
      SendNmea(d, TEXT("RCDT,GET,SENS"));
    if( ((info->Second+4) %4) ==0) 
      SendNmea(d, TEXT("RCDT,GET,NAVIGATE,0"));

    static double oldQNH= -1.0;

    if(IsDirOutput(PortIO[d->PortNumber].QNHDir))
    {      
      if(fabs( oldQNH - QNH) > 0.9)   
      { TCHAR szTmp[MAX_NMEA_LEN];
        _stprintf(szTmp,  TEXT("RCDT,SET,MC_BAL,,,,,,,%4u"),(int) (QNH) );
        SendNmea(d, szTmp);
        oldQNH = QNH;
      }
    }
  }

  if(IsDirOutput(PortIO[d->PortNumber].STFDir))
  {

    bool circling = MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING);
    static  int   Thermalmode = -1; 
    if((circling != (bool)Thermalmode) || (Thermalmode <0))
    {
      if(circling)
      {
        SendNmea(d, TEXT("RCDT,SET,SC_VAR,0")); 
        Thermalmode = 1;
      }
      else
      {
        SendNmea(d, TEXT("RCDT,SET,SC_VAR,1"));
        Thermalmode = 0;
      }
    }
  }

    if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
      return FALSE;
    }
    if (_tcsncmp(_T("$RCDT"), sentence, 5) == 0)
      return RCDT(d, sentence + 6, info);
    else
      if (_tcsncmp(_T("$LXBC"), sentence, 5) == 0)
        return LXBC(d, sentence + 6, info);
      else
        if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0)
          return LXWP2(d, sentence + 7, info);
        else
          if (_tcsncmp(_T("$LXWP0"), sentence, 6) == 0)
            return LXWP0(d, sentence + 7, info);
          else
              if(_tcsncmp(_T("$GPRMB"), sentence, 6) == 0)
                return GPRMB(d, sentence + 7, info);
              else
                if (_tcsncmp(_T("$LXWP1"), sentence, 6) == 0)
                {
                  return LXWP1(d, sentence + 7, info);
                }
return false;
} // ParseNMEA()



BOOL DevRCFenix::Open( PDeviceDescriptor_t d) {

  return TRUE;
}




CallBackTableEntry_t DevRCFenix::CallBackTable[]={

  EndCallBackEntry()
};



BOOL DevRCFenix::SetupFenix_Sentence(PDeviceDescriptor_t d)
{
#ifdef TIMEOUTCHECK
  static int i=0;
 if (Declare()) return false ;  // do not configure during declaration
  if(IsFenixInBinaryMode()) return false;
  if((i++%10)==0)
  {
    SendNmea(d, TEXT("PFLX0,LXWP0,1,LXWP1,5,LXWP2,1,LXWP3,1,GPRMB,5"));

    SendNmea( d, _T("RCDT,SET,BC_INT,AHRS,0.5,SENS,2.0"));      
  }
  else
    if(!RCFenix_bValid)
    {
     SendNmea(d,_T("RCDT,GET,MC_BAL"));
     StartupStore(TEXT("Config: RCDT"));      
    }
#endif

  return true;
}


BOOL DevRCFenix::SetDataText( ValueStringIndex Idx,  const TCHAR ValueText[])
{
  CritSec_LXDebugStr.lock();
  _tcsncpy(LxValueStr[Idx] , ValueText, MAX_VAL_STR_LEN);
  CritSec_LXDebugStr.unlock();
  return true;
}

static bool OnTimer(WndForm* pWnd)
{
  WndForm * wf = pWnd->GetParentWndForm();
  WndProperty *wp = NULL;

  if(wf)
  {
    wp = (WndProperty*)wf->FindByName(TEXT("prpQNHDir")    ); UpdateValueTxt( wp,  _QNH   );
    wp = (WndProperty*)wf->FindByName(TEXT("prpMCDir")     ); UpdateValueTxt( wp,  _MC    );
    wp = (WndProperty*)wf->FindByName(TEXT("prpBUGDir")    ); UpdateValueTxt( wp,  _BUGS  );
    wp = (WndProperty*)wf->FindByName(TEXT("prpBALDir")    ); UpdateValueTxt( wp,  _BAL   );
    wp = (WndProperty*)wf->FindByName(TEXT("prpSTFDir")    ); UpdateValueTxt( wp,  _STF   );
    wp = (WndProperty*)wf->FindByName(TEXT("prpWINDDir")   ); UpdateValueTxt( wp,  _WIND  );
    wp = (WndProperty*)wf->FindByName(TEXT("prpBARODir")   ); UpdateValueTxt( wp,  _BARO  );
    wp = (WndProperty*)wf->FindByName(TEXT("prpVARIODir")  ); UpdateValueTxt( wp,  _VARIO );
    wp = (WndProperty*)wf->FindByName(TEXT("prpSPEEDDir")  ); UpdateValueTxt( wp,  _SPEED );
    wp = (WndProperty*)wf->FindByName(TEXT("prpR_TRGTDir") ); UpdateValueTxt( wp,  _R_TRGT);
    wp = (WndProperty*)wf->FindByName(TEXT("prpT_TRGTDir") ); UpdateValueTxt( wp,  _T_TRGT);
    wp = (WndProperty*)wf->FindByName(TEXT("prpGFORCEDir") ); UpdateValueTxt( wp,  _GFORCE);
    wp = (WndProperty*)wf->FindByName(TEXT("prpOATDir")    ); UpdateValueTxt( wp,  _OAT   );
    wp = (WndProperty*)wf->FindByName(TEXT("prpBAT1Dir")   ); UpdateValueTxt( wp,  _BAT1  );
    wp = (WndProperty*)wf->FindByName(TEXT("prpBAT2Dir")   ); UpdateValueTxt( wp,  _BAT2  );
    wp = (WndProperty*)wf->FindByName(TEXT("prpPOLARDir")  ); UpdateValueTxt( wp,  _POLAR );
    wp = (WndProperty*)wf->FindByName(TEXT("prpDirectLink")); UpdateValueTxt( wp,  _DIRECT);    
  }
  return true;
}

BOOL DevRCFenix::Config(PDeviceDescriptor_t d){

  WndForm*  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_DEV_LXNAV_L : IDR_XML_DEV_LXNAV_P);
  if(wf) {
    Device(d);
    WndButton *wBt = NULL;

    wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
    if(wBt){
      wBt->SetOnClickNotify(OnCloseClicked);
    }

    wBt = (WndButton *)wf->FindByName(TEXT("cmdIGCDownload"));		
#ifndef IGC_DOWNLOAD	
		// IGC Download not yet supported by Fenix 
		wBt->SetVisible(false);  // hide IGC Download button
#else
    if(wBt){
      wBt->SetOnClickNotify(OnIGCDownloadClicked);
    }
#endif
    wBt = (WndButton *)wf->FindByName(TEXT("cmdValues"));
    if(wBt){
      wBt->SetOnClickNotify(OnValuesClicked);
    }
    ShowValues(false);

    wf->SetCaption(_T("RC Fenix Config"));
    ShowData(wf, d);
    wf->ShowModal();
    wf->SetTimerNotify(0, NULL);
    Device(NULL);

    delete wf;
    wf=NULL;
  }
  return TRUE;
}


extern int DeviceASCIIConvert(TCHAR *pDest,const TCHAR *pSrc, int size=11);


BOOL DevRCFenix::FormatTP( TCHAR* DeclStrings, int num, int total,const WAYPOINT *wp)
{
  if(DeclStrings)
  {
    int  lat =0; 
    int  lon =0; 
    TCHAR Name[60] =_T("");
    if(wp)
    {
      lat = ( int)(wp->Latitude*60000.0);
      lon = (int) (wp->Longitude*60000.0);  
      DeviceASCIIConvert(Name, wp->Name,20) ;  
    }

      _stprintf(DeclStrings, TEXT("RCDT,SET,TP,%i,%i,%i,%i,%s"),num,
                                                               total+2,
                                                               lat,
                                                               lon,
                                                               Name );
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
BOOL DevRCFenix::DeclareTask(PDeviceDescriptor_t d,
    const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]) {
  Decl  decl;
  Class lxClass;

  bool Good  = true;


  if (!CheckWPCount(*lkDecl,Decl::min_wp_count - 2, Decl::max_wp_count - 2, errBufSize, errBuf)){
    return(false);
  }
  ShowProgress(decl_enable);
  ShowProgress(decl_send);
  Declare(true);

  TCHAR Pilot[64];
  _tcscpy(Pilot , lkDecl->PilotName); //copy to local instance (Multi driver support)

    TCHAR PilotName[12]=_T("");
    TCHAR PilotSurName[12]=_T("");;
    TCHAR* NamePtr= _tcstok (Pilot, _T(" ,.-:_"));
    if(NamePtr !=NULL)
        DeviceASCIIConvert(PilotName,  NamePtr ,11  );

    TCHAR* SurNamePtr = _tcstok (NULL,    _T(" ,.-:_"));
    if(SurNamePtr !=NULL)
        DeviceASCIIConvert(PilotSurName,  SurNamePtr ,11  );

  TCHAR AircraftType[12];   DeviceASCIIConvert(AircraftType,  lkDecl->AircraftType    ,11);
  TCHAR AircraftReg[12];    DeviceASCIIConvert(AircraftReg,   lkDecl->AircraftRego    ,11);
  TCHAR AircraftCompID[12]; DeviceASCIIConvert(AircraftCompID,lkDecl->CompetitionID   ,11);
  TCHAR AircraftClass[12];  DeviceASCIIConvert(AircraftClass, lkDecl->CompetitionClass,11);

  int wpCount = lkDecl->num_waypoints;
  int totalLines =  wpCount*2 + 2 + 4; // N * w(p + zone) + takeoff + landing + header
  TCHAR DeclStrings[totalLines][256];
  INT i = 0;

  double SecRadius;
  int Type;

  _stprintf(DeclStrings[i++], TEXT("RCDT,SET,PILOT,%s,%s"),(PilotName), (PilotSurName));
  _stprintf(DeclStrings[i++], TEXT("RCDT,SET,GLIDER,%s,%s,%s,%s"), AircraftType, AircraftReg, AircraftCompID, AircraftClass);

  if(UseAATTarget())
    _stprintf(DeclStrings[i++], TEXT("RCDT,SET,TSK_PAR,0,0,%02i:%02i"), (int)AATTaskLength/60,  (int)(AATTaskLength-((int)(AATTaskLength/60)*60)));
  else
    _stprintf(DeclStrings[i++], TEXT("RCDT,SET,TSK_PAR,0,0,00:00"));
  int num=0;

  int dir=0,autonxt=1,isline=0,a1=45,a2=45,a21=5000,r1=5000,r2=500, elev = WayPointList[HomeWaypoint].Altitude;

  WAYPOINT* pTakeOff = NULL;
  if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint) && DeclTakeoffLanding)
  {
    pTakeOff = &WayPointList[HomeWaypoint];
  }
  FormatTP( (TCHAR*) &DeclStrings[i++], num++ , wpCount, pTakeOff);   // Takeoff

  

  for (int ii = 0; ii < wpCount; ii++)
  {
    FormatTP( (TCHAR*) &DeclStrings[i++], num, wpCount, lkDecl->waypoint[ii]);   //  Task waypoints

    GetTaskSectorParameter(ii, &Type, &SecRadius);
    switch(Type)
    {
      case LINE  : isline=1; r1= SecRadius; a1 = 0  ; a2=180; r2=0  ; a21 =0; break;
      case SECTOR: isline=0; r1= SecRadius; a1 = 45 ; a2=180; r2=0  ; a21 =0; break;
      case CIRCLE: isline=0; r1= SecRadius; a1 =180 ; a2=180; r2=0  ; a21 =0; break;
      case DAe   : isline=0; r1= SecRadius; a1 = 45 ; a2=180; r2=500; a21 =0; break;
      case CONE  : isline=0; r1= SecRadius; a1 =180 ; a2=0  ; r2=0  ; a21 =0; break;
    }

    elev = WayPointList[HomeWaypoint].Altitude;
    r1=(int)SecRadius;
    _stprintf(DeclStrings[i++], TEXT("RCDT,SET,ZONE,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i"),num,dir,autonxt,isline,a1,a2,a21,r1,r2, elev);
    num++;
  }

  FormatTP( (TCHAR*) &DeclStrings[i++], num++ , wpCount, pTakeOff);   // Landing

  bool status= false;
  if ( StopRxThread(d, errBufSize, errBuf)) {
    // Send complete declaration to logger
    int orgRxTimeout;
    StartupStore(_T(". RC Fenix SetRxTimeout%s "), NEWLINE);
    status = SetRxTimeout(d, 500, orgRxTimeout, errBufSize, errBuf);
    int attemps = 0;
    char RecBuf[4096] = "";

    do {
      Good = true;
      for (int ii = 0; ii < i; ii++) {
        StartupStore(_T(". RC Fenix Decl: %s %s "), DeclStrings[ii], NEWLINE);
        if (Good)
          Good = SendNmea(d, DeclStrings[ii]);


        if (Good)
          Good = ComExpect(d, "$RCDT,ANS,OK*59", 4095, RecBuf, errBufSize, errBuf);

      }
      attemps++;
      if (!Good)
        Poco::Thread::sleep(500);
    } while ((!Good) && (attemps < 3));



    // restore Rx timeout
    status = status && SetRxTimeout(d, orgRxTimeout, orgRxTimeout, status ? errBufSize : 0, errBuf);
  }
  // restart RX thread
  StartRxThread(d, status ? errBufSize : 0, errBuf);

  Declare(false);
  ShowProgress(decl_disable);
  return(Good);
} // DeclareTask()



// #############################################################################
// *****************************************************************************
//
//   DevRCFenix::Decl
//
// *****************************************************************************
// #############################################################################


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constructor - sets all data to 0.
///
DevRCFenix::Decl::Decl(){
  memset(this, 0, sizeof(*this));
} // Decl()

// #############################################################################
// *****************************************************************************
//
//   DevRCFenix::Class
//
// *****************************************************************************
// #############################################################################



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constructor - sets all data to 0.
///
DevRCFenix::Class::Class(){
  memset(this, 0, sizeof(*this));
} // Class()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the value of @c name member.
///
/// @param text  string to be set (will be converted into ASCII)
///
void DevRCFenix::Class::SetName(const TCHAR* text){
  Wide2LxAscii(text, sizeof(name), name);
} // SetName()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Send string as NMEA sentence with prefix '$', suffix '*', and CRC
///
/// @param d           device descriptor
/// @param buf         string for sending
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  NMEA sentence successfully written
/// @retval false error (description in @p errBuf)
///
// static
bool DevRCFenix::SendNmea(PDeviceDescriptor_t d, const TCHAR buf[], unsigned errBufSize, TCHAR errBuf[]){

  ScopeLock Lock(CritSec_Comm);
  if(!d || !d->Com) {
    return false;
  }

  char asciibuf[256];
  DevRCFenix::Wide2LxAscii(buf, 256, asciibuf);
  unsigned char chksum = 0;

  if (!ComWrite(d, '$', errBufSize, errBuf)) {
    return (false);
  }

  for(size_t i = 0; i < strlen(asciibuf); i++) {
    if (!ComWrite(d, asciibuf[i], errBufSize, errBuf)) {
      return (false);
    }
    chksum ^= asciibuf[i];
  }

  sprintf(asciibuf, "*%02X\r\n",chksum);
  for(size_t i = 0; i < strlen(asciibuf); i++) {
    if (!ComWrite(d, asciibuf[i], errBufSize, errBuf)) {
      return (false);
    }
  }

//  StartupStore(_T("request: $%s*%02X %s "),   buf,chksum, NEWLINE);

  return (true);
} // SendNmea()


bool DevRCFenix::SendNmea(PDeviceDescriptor_t d, const TCHAR buf[]){
TCHAR errBuf[10]= _T("");
TCHAR errBufSize=10;
  DevRCFenix::SendNmea(d,  buf,errBufSize,errBuf);
  if(_tcslen (errBuf) > 1)
  {
    DoStatusMessage(errBuf);
    return false;
  }
 // StartupStore(_T(" RCFenix SenadNmea %s %s"),buf, NEWLINE);
  return true;
} // SendNmea()

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Send one line of declaration to logger
///
/// @param d           device descriptor
/// @param row         row number
/// @param row         number of rows
/// @param content     row content
/// @param errBufSize  error message buffer size
/// @param errBuf[]    [out] error message
///
/// @retval true  row successfully written
/// @retval false error (description in @p errBuf)
///
// static
bool DevRCFenix::SendDecl(PDeviceDescriptor_t d, unsigned row, unsigned n_rows,
                   TCHAR content[], unsigned errBufSize, TCHAR errBuf[]){
  TCHAR szTmp[MAX_NMEA_LEN];

  char retstr[20];
  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PLXVC,DECL,W,%u,%u,%s"), row, n_rows, content);
  bool status = DevRCFenix::SendNmea(d, szTmp, errBufSize, errBuf);
  if (status) {
    sprintf(retstr, "$PLXVC,DECL,C,%u", row);
    status = status && ComExpect(d, retstr, 512, NULL, errBufSize, errBuf);

  }
  return status;

} // SendDecl()




void DevRCFenix::GetDirections(WndButton* pWnd){
  if(pWnd) {
    WndForm * wf = pWnd->GetParentWndForm();
    if(wf) {
    int PortNum = Port();

      WndProperty *wp;
      
      wp = (WndProperty*)wf->FindByName(TEXT("prpQNHDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].QNHDir = (DataBiIoDir) dfe->GetAsInteger();
      }     
      wp = (WndProperty*)wf->FindByName(TEXT("prpMCDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].MCDir = (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBUGDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].BUGDir =  (DataBiIoDir) dfe->GetAsInteger();
      }

      wp = (WndProperty*)wf->FindByName(TEXT("prpBALDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].BALDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpSTFDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].STFDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpWINDDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].WINDDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBARODir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].BARODir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpVARIODir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].VARIODir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpSPEEDDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].SPEEDDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpR_TRGTDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].R_TRGTDir =  (DataTP_Type) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpGFORCEDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].GFORCEDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpOATDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].OATDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBAT1Dir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].BAT1Dir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBAT2Dir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].BAT2Dir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpPOLARDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].POLARDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpDirectLink"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].DirLink =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpT_TRGTDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO[PortNum].T_TRGTDir =  (DataTP_Type) dfe->GetAsInteger();
      }
    }
  }
}

void DevRCFenix::OnCloseClicked(WndButton* pWnd)
{
  WndForm * wf = pWnd->GetParentWndForm();

  if(!ShowValues())
    GetDirections(pWnd);
  if(wf)
  {
    wf->SetTimerNotify(0, NULL);
    wf->SetModalResult(mrOK);
  }
}


void DevRCFenix::OnValuesClicked(WndButton* pWnd) {

  WndForm * wf = pWnd->GetParentWndForm();
  if(wf)
  {
    ShowValues (!ShowValues());
    WndButton *wBt = (WndButton *) wf->FindByName(TEXT("cmdValues"));
    if (wBt) {
      if (ShowValues()) {
        wf->SetTimerNotify(250, OnTimer); // update values 4 times a second
        GetDirections(pWnd);
        wBt->SetCaption(MsgToken(2468));//  _@M2468_ "Direction"
      } else {
        wf->SetTimerNotify(0, NULL);    // turn Off the timer
        wBt->SetCaption(MsgToken(2467));// _@M2467_ "Values"
        if (wf) ShowData(wf, Device());
      }
    }
    SetDataText( _QNH,   _T(""));
    SetDataText( _MC,    _T(""));
    SetDataText( _BUGS,  _T(""));
    SetDataText( _BAL,   _T(""));
    SetDataText( _STF,   _T(""));
    SetDataText( _WIND,  _T(""));
    SetDataText( _BARO,  _T(""));
    SetDataText( _VARIO, _T(""));
    SetDataText( _SPEED, _T(""));
    SetDataText( _R_TRGT,_T(""));
    SetDataText( _GFORCE,_T(""));
    SetDataText( _OAT,   _T(""));
    SetDataText( _BAT1,  _T(""));
    SetDataText( _BAT2,  _T(""));
    SetDataText( _POLAR, _T(""));
    SetDataText( _DIRECT,_T(""));
    SetDataText( _T_TRGT,_T(""));    
    SendNmea(Device(),_T("RCDT,GET,MC_BAL"));  // request new data
  }

}



BOOL DevRCFenix::Close (PDeviceDescriptor_t d) {
  Device(NULL);
  return TRUE;
}




BOOL DevRCFenix::FenixSetMC(PDeviceDescriptor_t d,float fTmp, const TCHAR *info )
{
bool ret = false;


  int iTmp =(int) (fTmp*100.0+0.5f);
  fTmp = (double)(iTmp)/100.0;
  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.2fm/s %s"),fTmp,info);
    SetDataText( _MC,   szTmp);
  }
  if(IsDirInput(PortIO[d->PortNumber].MCDir))
  {
    if(fabs(MACCREADY - fTmp)> 0.001)
    {
      CheckSetMACCREADY(fTmp);
      ret = true;
    }
  }

  return ret;
}



BOOL DevRCFenix::FenixSetSTF(PDeviceDescriptor_t d,int  iTmp, const TCHAR *info)
{
bool ret = false;


  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    if(iTmp > 0)
      _sntprintf(szTmp,MAX_NMEA_LEN,  _T("STF %s"), info);
    else
      _sntprintf(szTmp,MAX_NMEA_LEN,  _T("VARIO %s"), info);      
    SetDataText(_STF,  szTmp);
  }
  if(IsDirInput(PortIO[d->PortNumber].STFDir  ))
  {
    static int  iOldVarioSwitch=-1;
    if(iTmp != iOldVarioSwitch)
    {
      iOldVarioSwitch = iTmp;
      if(iTmp==1)
      {
        ExternalTriggerCruise = true;
        ExternalTriggerCircling = false;
         MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_CRUISE);
      }
      else
      {
        ExternalTriggerCruise = false;
        ExternalTriggerCircling = true;
        MapWindow::mode.UserForcedMode(MapWindow::Mode::MODE_FLY_CIRCLING);        
      }
    }
  }

  return ret;
}

BOOL DevRCFenix::FenixSetBAL(PDeviceDescriptor_t d,float fTmp, const TCHAR *info)
{
bool ret = false;


  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    if(WEIGHTS[2] > 0)
    {
      _sntprintf(szTmp,MAX_NMEA_LEN,  _T("%3.0f L = %3.0f%% %s"),fTmp,(fTmp/WEIGHTS[2]*100.0), info);
      SetDataText(_BAL,  szTmp);
    }
  }
  if(IsDirInput(PortIO[d->PortNumber].BALDir  ))
  {
    if((fabs(fTmp- GlidePolar::BallastLitres) > 1 )  &&   (WEIGHTS[2] > 0))
    {
      GlidePolar::BallastLitres = fTmp;
      BALLAST =  GlidePolar::BallastLitres / WEIGHTS[2];
      ret = true;
    }
  }

  return ret;
}

BOOL DevRCFenix::FenixSetBUGS(PDeviceDescriptor_t d,float fTmp, const TCHAR *info)
{
bool ret = false;


  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.0f%% %s"),fTmp, info);
    SetDataText(_BUGS,  szTmp);
  }
  if(IsDirInput(PortIO[d->PortNumber].BUGDir ))
  {
    if(  fabs(fTmp -BUGS) >= 0.01)
    {
      fTmp = CalculateBugsFromLX(fTmp);
      CheckSetBugs(fTmp);
      if(  fabs(fTmp -BUGS) >= 0.01) // >= 1% difference
      {
        BUGS = fTmp;
        ret = true;
      }
    }
  }

  return ret;
}


BOOL DevRCFenix::RCDT(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR szTmp[MAX_NMEA_LEN];

double fTmp=0.0;

devSetAdvancedMode(d,true);
static int iNoFlights=0;
  NMEAParser::ExtractParameter(sentence, szTmp, 0);
  if(_tcsncmp(szTmp, _T("ANS"), 3) != 0)  // really an Answer?
    return 0;

  NMEAParser::ExtractParameter(sentence, szTmp, 1);  // Command?
  if(_tcsncmp(szTmp, _T("RADIO"), 5) == 0)
  {
    m_bRadioEnabled = true;
    if(ParToDouble(sentence, 2, &fTmp)) if(ValidFrequency(fTmp)) RadioPara.ActiveFrequency  = fTmp;
    if(ParToDouble(sentence, 3, &fTmp)) if(ValidFrequency(fTmp)) RadioPara.PassiveFrequency = fTmp;
    if(ParToDouble(sentence, 4, &fTmp)) RadioPara.Volume  = (int) fTmp;
    if(ParToDouble(sentence, 5, &fTmp)) RadioPara.Squelch = (int) fTmp;
    if(ParToDouble(sentence, 6, &fTmp)) RadioPara.Vox     = (int) fTmp;
  }
  else
  if(_tcsncmp(szTmp, _T("MC_BAL"), 6) == 0)
  {
    if(_deb)  StartupStore(TEXT("MC_BAL %s"), sentence);
    if(ParToDouble(sentence, 2, &fTmp)) {FenixSetMC(d, fTmp,_T("($RCDT)") );}
    if(ParToDouble(sentence, 3, &fTmp)) {FenixSetBAL(d, fTmp,_T("($RCDT)") );}
    if(ParToDouble(sentence, 4, &fTmp)) {FenixSetBUGS(d, fTmp,_T("($RCDT)") );}
    if(ParToDouble(sentence, 5, &fTmp)) {}  // Screen brightness in percent
    if(ParToDouble(sentence, 6, &fTmp)) {}  // Variometer volume in percent
    if(ParToDouble(sentence, 7, &fTmp)) {}  // SC volume in percent
    if(ParToDouble(sentence, 8, &fTmp))   // QNH in hPa (NEW)
    {
      if(IsDirInput(PortIO[d->PortNumber].QNHDir))
      { 
        _stprintf( szTmp, _T("%4.0f hPa"),fTmp);      
        if(Values(d)) SetDataText( _QNH,   szTmp);
        static double oldQNH = -1;
        if ( fabs( oldQNH - fTmp) > 0.1)
        {
          UpdateQNH( fTmp);
          oldQNH = fTmp;
        }
      }  
    }
    RCFenix_bValid = true;    
  }
  else
  if(_tcsncmp(szTmp, _T("FLIGHTS_NO"), 10) == 0)
  {
    if(ParToDouble(sentence, 2, &fTmp)) iNoFlights =(int) (fTmp+0.05);
    if((iNoFlights > 0)
      && m_bTriggered)  // call next if triggerd from here only
    {
//      FenixListFilled(false);
      _sntprintf(szTmp, MAX_NMEA_LEN, _T("RCDT,GET,FLIGHT_INFO,%i"),1);
      SendNmea(d,szTmp);
    }
  }
  else
  if(_tcsncmp(szTmp, _T("SENS"), 4) == 0)  // Sensor Data?
  {
    SENS(d, sentence,  info, 2);
  }
  else
  if(_tcsncmp(szTmp, _T("SC_VAR"), 6) == 0)  // Vario / STF
  {
    if(ParToDouble(sentence, 2, &fTmp)) {
      FenixSetSTF(d, (int)fTmp,_T(" ($RCDT,SC_VAR)") );
    }
  }
  else
  if(_tcsncmp(szTmp, _T("NAVIGATE"), 7) == 0)  // Navigation Target
  {
    GetTarget( d, sentence,  info);    
  }

  if(_tcsncmp(szTmp, _T("ERROR"), 5) == 0)  // ERROR?
  {
    NMEAParser::ExtractParameter(sentence, szTmp, 2);
    if(_tcsncmp(szTmp, _T("Radio not enabled"), 17) == 0)  
    {
      m_bRadioEnabled = false;
    }
    else
    {
      DoStatusMessage(TEXT("RC Fenix Error:"), szTmp, false);
      StartupStore(TEXT("RC Fenix Error: %s"), szTmp);
    }
  }
  else
  if(_tcsncmp(szTmp, _T("OK"), 2) == 0)
  {
    if(!Declare())
      SendNmea(d,_T("RCDT,GET,MC_BAL"));
    else
    {
    }
  }
  return(true);
} // RCDT()




BOOL DevRCFenix::SENS(PDeviceDescriptor_t d,  const TCHAR* sentence, NMEA_INFO* info, int ParNo)
{ 
TCHAR szTmp[MAX_NMEA_LEN];
double fTmp;

  if(ParToDouble(sentence, ParNo++, &fTmp)) { // Outside air temperature in °C. Left empty if OAT value not valid
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%4.2f°C ($RCDT)"),fTmp);
     if(Values(d)) SetDataText(_OAT,  szTmp);
    if(IsDirInput(PortIO[d->PortNumber].OATDir))
    {
      info->OutsideAirTemperature = fTmp;
      info->TemperatureAvailable  = TRUE;
    }
  }
  if(ParToDouble(sentence, ParNo++, &fTmp)) { // main power supply voltage
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%4.2fV ($RCDT)"),fTmp);
    if(Values(d)) SetDataText(_BAT1,  szTmp);
    if(IsDirInput(PortIO[d->PortNumber].BAT1Dir))
    {
      info->ExtBatt1_Voltage = fTmp;	
    }
  }
  if(ParToDouble(sentence, ParNo++, &fTmp)) { // Backup battery voltage
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%4.2fV ($RCDT)"),fTmp);
     if(Values(d)) SetDataText(_BAT2,  szTmp);
    if(IsDirInput(PortIO[d->PortNumber].BAT2Dir))
    {
      info->ExtBatt2_Voltage = fTmp;	
    }
  }
  NMEAParser::ExtractParameter(sentence, szTmp, ParNo++); 
  {  // Current flap setting
  }
  NMEAParser::ExtractParameter(sentence, szTmp, ParNo++);
  { // Recommended flap setting
  }
  if(ParToDouble(sentence, ParNo++, &fTmp)) {  // Current landing gear position (0 = out, 1 = inside, left empty if gear input not configured)
  }
  if(ParToDouble(sentence, ParNo++, &fTmp)) {  // SC/Vario mode (0 = Vario, 1 = SC)
    FenixSetSTF(d, (int)fTmp,_T(" ($RCDT,SENS)"));
  }
  return true;
}

BOOL DevRCFenix::LXBC(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR szTmp[MAX_NMEA_LEN];

devSetAdvancedMode(d,true);
if(_tcsncmp(sentence, _T("SENS"), 4) == 0)
{  
  SENS(d,  sentence,  info,1);
}

if(_tcsncmp(sentence, _T("AHRS"), 4) == 0)
{
  if(IsDirInput(PortIO[d->PortNumber].GFORCEDir))
  { bool bAHRS = false;
    double fX,fY,fZ, fPitch, fRoll, fYaw, fSlip;
    if(ParToDouble(sentence, 1, &fPitch))  // pitch
      if(ParToDouble(sentence, 2, &fRoll)) // Roll
        if(ParToDouble(sentence, 3, &fYaw)) // Yaw
          if(ParToDouble(sentence, 4, &fSlip)) // slip
            bAHRS = true;

    if(ParToDouble(sentence, 5, &fX) &&
       ParToDouble(sentence, 6, &fY) &&
       ParToDouble(sentence, 7, &fZ))
    {
      if(Values(d))
      {
        if(bAHRS)
          _sntprintf(szTmp, MAX_NMEA_LEN, _T("gX:%5.2f gY:%5.2f gZ:%5.2f Pitch:%5.2f Roll:%5.2f Yaw:%5.2f Slip:%5.2f($RCDT)"),fX,fY,fZ, fPitch, fRoll, fYaw, fSlip);
        else
          _sntprintf(szTmp, MAX_NMEA_LEN, _T("gX:%5.2f gY:%5.2f gZ:%5.2f($RCDT)"),fX,fY,fZ);
        SetDataText( _GFORCE,  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].GFORCEDir))
      {
        info->AccelX  = fX;
        info->AccelY  = fY;
        info->AccelZ  = fZ;
        info->AccelerationAvailable = true;
        if(bAHRS)
        {
          info->Pitch = fPitch;
          info->Roll  = fRoll;
          info->GyroscopeAvailable = true;
        }
      }
    }
  }
}
return true;
}

#define  REQ_DEADTIME 500


BOOL DevRCFenix::FenixPutMacCready(PDeviceDescriptor_t d, double MacCready){
TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;

  if(!IsDirOutput(PortIO[d->PortNumber].MCDir)) return false;

  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("RCDT,SET,MC_BAL,%.1f,,,,,,"), MacCready);
  StartupStore(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);

return true;

}


BOOL DevRCFenix::FenixPutBallast(PDeviceDescriptor_t d, double Ballast){
TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;
  if(RCFenix_bValid == false) return false;
  if(!IsDirOutput(PortIO[d->PortNumber].BALDir)) return false;
  
  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("RCDT,SET,MC_BAL,,%.0f,,,,,"),GlidePolar::BallastLitres);
  StartupStore(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);



return(TRUE);
}


BOOL DevRCFenix::FenixPutBugs(PDeviceDescriptor_t d, double Bugs){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;

  if(!IsDirOutput(PortIO[d->PortNumber].BUGDir)) return false;
  double fLXBugs = CalculateLXBugs( Bugs);

  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("RCDT,SET,MC_BAL,,,%.0f,,,,"),fLXBugs);
  StartupStore(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);



return(TRUE);
}


BOOL DevRCFenix::PutTarget(PDeviceDescriptor_t d)
{
  if(PortIO[d->PortNumber].T_TRGTDir == TP_Off) {
    return false;
  }


int overindex = GetOvertargetIndex();

if(overindex < 0)               /* valid waypoint ?*/
  return -1;


bool bTaskpresent = false; //ValidTaskPoint(0);
if(bTaskpresent)
  if(ValidTaskPoint(ActiveTaskPoint))
    overindex = Task[ActiveTaskPoint].Index;

TCHAR  szTmp[MAX_NMEA_LEN];


int DegLat, DegLon;
double MinLat, MinLon;
char NoS, EoW;

if (!ValidWayPoint(overindex)) return TRUE;

DegLat = (int)WayPointList[overindex].Latitude;
MinLat = WayPointList[overindex].Latitude - DegLat;
NoS = 'N';
if((MinLat<0) || ((MinLat-DegLat==0) && (DegLat<0)))
{
  NoS = 'S';
  DegLat *= -1; MinLat *= -1;
}
MinLat *= 60;

DegLon = (int)WayPointList[overindex].Longitude ;
MinLon = WayPointList[overindex].Longitude  - DegLon;
EoW = 'E';
if((MinLon<0) || ((MinLon-DegLon==0) && (DegLon<0)))
  {
    EoW = 'W';
    DegLon *= -1; MinLon *= -1;
  }
MinLon *=60;


TCHAR szName[MAX_VAL_STR_LEN];
  if( 0 /*bTaskpresent*/)  {
    _sntprintf( szName, MAX_VAL_STR_LEN,_T("%s%s"), MsgToken(1323), WayPointList[overindex].Name); // LKTOKEN _@M1323_ "T>"
  } else {
    _sntprintf( szName, MAX_VAL_STR_LEN,_T("%s%s"),GetOvertargetHeader(), WayPointList[overindex].Name); // LKTOKEN _@M1323_ "T>"
  }

  if( PortIO[d->PortNumber].T_TRGTDir  == TP_VTARG)
  {                                  
    int rwdir = 0; 
    int landable =0;

    if((WayPointList[overindex].Flags & LANDPOINT)> 0) 
    {
      landable = 1;
      rwdir    = WayPointList[overindex].RunwayDir;
    }

    _sntprintf( szTmp,MAX_NMEA_LEN, TEXT("RCDT,SET,NAVIGATE,%i,%s,%i,%i,%i,%i,%s,%i"),
      1,
      szName, 
      (int) (WayPointList[overindex].Latitude * 60000.0), 
      (int) (WayPointList[overindex].Longitude* 60000.0),
      (int) (WayPointList[overindex].Altitude +0.5),
      landable,
      WayPointList[overindex].Freq ,
      rwdir
    );
    
 //   $RCDT,SET,NAVIGATE,0,MARIBOR,2788794,941165,267,1,119.200,14*2A<CR><LF>
      _tcsncat (szName, _T(" ($RCDT,SET,NAVIGATE)"), std::size(szName) - _tcslen(szName));
#ifdef TESTBENCH
     StartupStore(TEXT("Send navigation Target Fenix: %s"), szName);
#endif
  }
  else
  {
    if( PortIO[d->PortNumber].T_TRGTDir  == TP_GPRMB)
    {               //                      GPRMB,A,,,,H>TAKEOFF,5144.78,N,00616.70,E,,,A
      _sntprintf( szTmp,MAX_NMEA_LEN, TEXT("GPRMB,A,,,%s,%02d%05.2f,%c,%03d%05.2f,%c,,,,A"),
        szName, DegLat, MinLat, NoS, DegLon, MinLon, EoW);

      _tcsncat (szName, _T(" ($GPRMB)"), std::size(szName) - _tcslen(szName));
    }
   
#ifdef TESTBENCH
    StartupStore(TEXT("Send navigation Target Fenix: %s"), szName);
#endif
  }

  if(Values(d)) SetDataText( _T_TRGT,  szName);
  DevRCFenix::SendNmea(d,szTmp);

return(true);
}


BOOL DevRCFenix::GetTarget(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
  if(PortIO[d->PortNumber].R_TRGTDir != TP_VTARG) {
    return false;
  }

TCHAR  szTmp[MAX_NMEA_LEN];

double fTmp;
  NMEAParser::ExtractParameter(sentence,szTmp,3);


    if(Alternate2 == RESWP_EXT_TARGET) // pointing to external target?
      Alternate2 = -1;                 // clear external =re-enable!

  _tcscpy(WayPointList[RESWP_EXT_TARGET].Name, _T("^") );
  _tcscat(WayPointList[RESWP_EXT_TARGET].Name, szTmp );


  ParToDouble(sentence, 4, &fTmp);
  WayPointList[RESWP_EXT_TARGET].Latitude= fTmp/60000;
  ParToDouble(sentence, 5, &fTmp);
  WayPointList[RESWP_EXT_TARGET].Longitude= fTmp/60000;
  ParToDouble(sentence, 6, &fTmp);
  WayPointList[RESWP_EXT_TARGET].Altitude= fTmp; 
  Alternate2 = RESWP_EXT_TARGET;
//  ParToDouble(sentence, 7, &fTmp);  // distance
//  ParToDouble(sentence, 8, &fTmp);  // bearing
  
  ParToDouble(sentence, 9, &fTmp);
  if( fTmp > 0 )
    WayPointList[RESWP_EXT_TARGET].Flags = LANDPOINT;
  else
    WayPointList[RESWP_EXT_TARGET].Flags =0;
  
  if(Values(d))
  {
    _tcsncat(szTmp, _T(" ($RCDT)"), std::size(szTmp) - _tcslen(szTmp));
    SetDataText( _R_TRGT,  szTmp);
  }
  return false;
}




BOOL DevRCFenix::FenixRequestRadioInfo(PDeviceDescriptor_t d)
{
  if(!FenixRadioEnabled(d)) return false;
  Poco::Thread::sleep(50);
  SendNmea(d,(TCHAR*)_T("RCDT,GET,RADIO"));
  return true;
}

BOOL DevRCFenix::FenixPutVolume(PDeviceDescriptor_t d, int Volume) {
  if(!FenixRadioEnabled(d)) return false;
  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("RCDT,SET,RADIO,,,%i,,,"),Volume)  ;
  SendNmea(d,szTmp);
  if(uiFenixDebugLevel) StartupStore(_T(". Fenix Volume  %i%s"), Volume,NEWLINE);
  RadioPara.Volume = Volume;
  FenixRequestRadioInfo(d);
  return(TRUE);
}




BOOL DevRCFenix::FenixPutSquelch(PDeviceDescriptor_t d, int Squelch) {
 if(!FenixRadioEnabled(d)) return false;
  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("RCDT,SET,RADIO,,,,%i,,"),Squelch)  ;
  SendNmea(d,szTmp);
  if(uiFenixDebugLevel) StartupStore(_T(". Fenix Squelch  %i%s"), Squelch,NEWLINE);
  RadioPara.Squelch = Squelch;
  FenixRequestRadioInfo(d);
  return(TRUE);
}



BOOL DevRCFenix::FenixPutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) {
 if(!FenixRadioEnabled(d)) return false;
  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("RCDT,SET,RADIO,%7.3f,,,,,"),Freq)  ;
  SendNmea(d,szTmp);
  FenixRequestRadioInfo(d);
  RadioPara.ActiveFrequency=  Freq;
  if(StationName != NULL)
    _sntprintf(RadioPara.ActiveName, NAME_SIZE,_T("%s"),StationName) ;
  if(uiFenixDebugLevel) StartupStore(_T(". Fenix Active Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);

  return(TRUE);
}


BOOL DevRCFenix::FenixPutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) {
 if(!FenixRadioEnabled(d)) return false;
  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("RCDT,SET,RADIO,,%7.3f,,,,"),Freq)  ;
  SendNmea(d,szTmp);
  FenixRequestRadioInfo(d);
  RadioPara.PassiveFrequency =  Freq;
  if(StationName != NULL)
    _sntprintf(RadioPara.PassiveName , NAME_SIZE ,_T("%s"),StationName) ;
  if(uiFenixDebugLevel) StartupStore(_T(". Fenix Standby Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);

  return(TRUE);
}


BOOL DevRCFenix::FenixStationSwap(PDeviceDescriptor_t d) {

 if(!FenixRadioEnabled(d)) return false;
  SendNmea(d,_T("RCDT,SET,R_SWITCH"));

  FenixRequestRadioInfo(d);

  if(uiFenixDebugLevel) StartupStore(_T(". Fenix  station swap %s"), NEWLINE);

  return(TRUE);
}


BOOL DevRCFenix::FenixRadioMode(PDeviceDescriptor_t d, int mode) {
 if(!FenixRadioEnabled(d)) return false;
  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("RCDT,SET,R_DUAL,%i"),mode);
  SendNmea(d,(TCHAR*)szTmp);
  FenixRequestRadioInfo(d);
  if(uiFenixDebugLevel) StartupStore(_T(". Fenix  Dual Mode: %i %s"), mode, NEWLINE);
  return(TRUE);
}

