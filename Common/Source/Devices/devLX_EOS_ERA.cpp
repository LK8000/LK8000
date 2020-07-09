/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

//__Version_1.0_ Ulrich Heynen 29.06.2020


//_____________________________________________________________________includes_


#include <time.h>
#include "externs.h"
#include "utils/stringext.h"
#include "devLX_EOS_ERA.h"


#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "dlgIGCProgress.h"
#include "Util/Clamp.hpp"
#include "OS/Sleep.h"
#include "dlgLXIGCDownload.h"
#include "externs.h"
#include "Baro.h"
#include "Utils.h"
#include "LKInterface.h"
#include "InputEvents.h"
#include "McReady.h"
#include "Time/PeriodClock.hpp"
#include "Calc/Vario.h"

static FILE *f= NULL;

unsigned int uiEOSDebugLevel = 1;
extern bool UpdateQNH(const double newqnh);

#define NANO_PROGRESS_DLG
#define BLOCK_SIZE 32


PDeviceDescriptor_t DevLX_EOS_ERA::m_pDevice=NULL;
BOOL DevLX_EOS_ERA::m_bShowValues = false;
BOOL DevLX_EOS_ERA::bIGC_Download = false;
BOOL DevLX_EOS_ERA::m_bDeclare = false;

uint uEOS_ERA_Timeout =0;
#define TIMEOUTCHECK



#define MAX_NMEA_PAR_LEN    30
#define MAX_VAL_STR_LEN    60


BOOL LX_EOS_ERA_bValid = false;





extern Mutex  CritSec_LXDebugStr;
extern TCHAR LxValueStr[_LAST][ MAX_VAL_STR_LEN];

extern BOOL IsDirInput( DataBiIoDir IODir);
extern BOOL IsDirOutput( DataBiIoDir IODir);
extern void UpdateValueTxt(WndProperty *wp,  ValueStringIndex Idx);




//____________________________________________________________class_definitions_

// #############################################################################
// *****************************************************************************
//
//   DevLX_EOS_ERA
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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Registers device into device subsystem.
///
/// @retval true  when device has been registered successfully
/// @retval false device cannot be registered
///
//static
bool DevLX_EOS_ERA::Register(){
  #ifdef UNIT_TESTS
    Wide2LxAsciiTest();
  #endif
  return(devRegister(GetName(),
    cap_gps | cap_baro_alt | cap_speed | cap_vario | cap_logger, Install));
} // Register()




BOOL  DevLX_EOS_ERA::Values( PDeviceDescriptor_t d)
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
BOOL DevLX_EOS_ERA::Install(PDeviceDescriptor_t d) {
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = EOSPutMacCready;
  d->PutBugs      = EOSPutBugs;
  d->PutBallast   = EOSPutBallast;
  d->Open         = Open;
  d->LinkTimeout  = GetTrue;
  d->Declare      = DeclareTask;
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  d->Config       = Config;
  d->DirectLink   = NULL;

  d->IsRadio        = GetTrue;
  d->PutVolume      = EOSPutVolume;
  d->PutSquelch     = EOSPutSquelch;
  d->PutFreqActive  = EOSPutFreqActive;
  d->PutFreqStandby = EOSPutFreqStandby;
  d->StationSwap    = EOSStationSwap;
  d->PutRadioMode   = EOSRadioMode;

  StartupStore(_T(". %s installed (platform=%s test=%u)%s"),
    GetName(),
    PlatfEndian::IsBE() ? _T("be") : _T("le"),
    PlatfEndian::To32BE(0x01000000), NEWLINE);
  return(true);
} // Install()



extern long  StrTol(const  TCHAR *buff) ;


long LX_EOS_ERABaudrate(int iIdx)
{
//  indexes are following:
//  enum { br4800=0, br9600, br19200, br38400, br57600,
//  br115200,br230400,br256000,br460800, br500k, br1M};
long lBaudrate = -1;
  switch (iIdx)
  {
    case 0:  lBaudrate = 4800   ; break;
    case 1:  lBaudrate = 9600   ; break;
    case 2:  lBaudrate = 19200  ; break;
    case 3:  lBaudrate = 38400  ; break;
    case 4:  lBaudrate = 56800  ; break;
    case 5:  lBaudrate = 115200 ; break;
    case 6:  lBaudrate = 230400 ; break;
    case 7:  lBaudrate = 256000 ; break;
    case 8:  lBaudrate = 460800 ; break;
    case 9:  lBaudrate = 500000 ; break;
    case 10: lBaudrate = 1000000; break;
    default: lBaudrate = -1     ; break;
  }
return lBaudrate;
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


BOOL DevLX_EOS_ERA::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{
 if (Declare()) return false ;  // do not configure during declaration
static char lastSec =0;
  if( /*!Declare() &&*/ (info->Second != lastSec))  // execute every second only if no task is declaring
  {
    lastSec = info->Second;
    if((info->Second % 10) ==0) // config every 10s (not on every xx $GPGGA as there are 10Hz GPS now
    {
      SetupLX_Sentence(d);
    }

    static int old_overindex = -1;    // call every 10s or on change
    static int old_overmode = -1;
    if( ( ((info->Second+5) %10) ==0) || (OvertargetMode != old_overmode) || (GetOvertargetIndex() != old_overindex))
    {
      PutTarget(d);
      old_overindex = GetOvertargetIndex();;
      old_overmode  = OvertargetMode;
    }
  }


 

    if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
      return FALSE;
    }
    if (_tcsncmp(_T("$LXDT"), sentence, 5) == 0)
      return LXDT(d, sentence + 6, info);
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



BOOL DevLX_EOS_ERA::Open( PDeviceDescriptor_t d) {

  return TRUE;
}




CallBackTableEntry_t DevLX_EOS_ERA::CallBackTable[]={

  EndCallBackEntry()
};



BOOL DevLX_EOS_ERA::SetupLX_Sentence(PDeviceDescriptor_t d)
{
#ifdef TIMEOUTCHECK
  static int i=0;
 if (Declare()) return false ;  // do not configure during declaration
  if((i++%2)==0)
  {
    SendNmea(d, TEXT("PFLX0,LXWP0,1,LXWP1,5,LXWP2,1,LXWP3,1,LXDT,1,LXBC,1,GPRMB,5"));
  }
  else
    if(!LX_EOS_ERA_bValid)
    {
     SendNmea(d,_T("LXDT,GET,MC_BAL"));
     StartupStore(TEXT("Config: LXDT"));      
    }
#endif

  return true;
}


BOOL DevLX_EOS_ERA::SetDataText( ValueStringIndex Idx,  const TCHAR ValueText[])
{
  CritSec_LXDebugStr.Lock();
  _tcsncpy(LxValueStr[Idx] , ValueText, MAX_VAL_STR_LEN);
  CritSec_LXDebugStr.Unlock();
  return true;
}


BOOL DevLX_EOS_ERA::ShowData(WndForm* wf ,PDeviceDescriptor_t d)
{
WndProperty *wp;
if(!wf) return false;
wp = (WndProperty*)wf->FindByName(TEXT("prpMCDir"));
int PortNum = d->PortNumber;
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491));  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken(2453)); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken(2454)); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO[PortNum].MCDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBUGDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491));  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken(2453)); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken(2454)); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO[PortNum].BUGDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBALDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491));  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken(2453)); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken(2454)); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO[PortNum].BALDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSTFDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken(2453)); // LKTOKEN  _@M2453_ "OUT"
    dfe->Set((uint) PortIO[PortNum].STFDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWINDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken(2453)); // LKTOKEN  _@M2453_ "OUT"
    dfe->Set((uint) PortIO[PortNum].WINDDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBARODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO[PortNum].BARODir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpSPEEDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO[PortNum].SPEEDDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVARIODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"

    dfe->Set((uint) PortIO[PortNum].VARIODir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpR_TRGTDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(_T("$PLXVTARG")); // "IN" = $PLXVTARG
    dfe->addEnumText(_T("$GPRMB")); //  "OUT" = $GPRMB
    dfe->Set((uint) PortIO[PortNum].R_TRGTDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpT_TRGTDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(_T("$PLXVTARG")); // "IN" = $PLXVTARG
    dfe->addEnumText(_T("$GPRMB")); //  "OUT" = $GPRMB
    dfe->Set((uint) PortIO[PortNum].T_TRGTDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGFORCEDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO[PortNum].GFORCEDir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpOATDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO[PortNum].OATDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBAT1Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO[PortNum].BAT1Dir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBAT2Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO[PortNum].BAT2Dir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPOLARDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2452)); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO[PortNum].POLARDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDirectLink"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(894)); // LKTOKEN  _@M894_": "ON"
    dfe->Set((uint) PortIO[PortNum].DirLink);
    wp->RefreshDisplay();
  }

  return true;
}



 bool  DevLX_EOS_ERA::OnIGCTimeout(WndForm* pWnd){

  if(Device() == NULL) return false;


  StartupStore(_T(" ******* LX_EOS_ERA  OnIGCTimeout resend last Block request ***** %s") , NEWLINE);

  return true;
}





static bool OnTimer(WndForm* pWnd)
{
  WndForm * wf = pWnd->GetParentWndForm();
  WndProperty *wp = NULL;

  if(wf)
  {
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

BOOL DevLX_EOS_ERA::Config(PDeviceDescriptor_t d){

  WndForm*  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_DEV_LXNAV_L : IDR_XML_DEV_LXNAV_P);
  if(wf) {
    Device(d);
    WndButton *wBt = NULL;

    wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
    if(wBt){
      wBt->SetOnClickNotify(OnCloseClicked);
    }

    wBt = (WndButton *)wf->FindByName(TEXT("cmdIGCDownload"));
    if(wBt){
      wBt->SetOnClickNotify(OnIGCDownloadClicked);
    }

    wBt = (WndButton *)wf->FindByName(TEXT("cmdValues"));
    if(wBt){
      wBt->SetOnClickNotify(OnValuesClicked);
    }
    ShowValues(false);

    wf->SetCaption(_T("LX ERA/EOS Config"));
    ShowData(wf, d);
    wf->ShowModal();
    wf->SetTimerNotify(0, NULL);
    Device(NULL);

    delete wf;
    wf=NULL;
  }
  return TRUE;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Returns device name (max length is @c DEVNAMESIZE).
///
//static
const TCHAR* DevLX_EOS_ERA::GetName() {
  return(_T("LX EOS/ERA"));
} // GetName()



BOOL DevLX_EOS_ERA::CeckAck(PDeviceDescriptor_t d, unsigned errBufSize, TCHAR errBuf[])
{
  return ComExpect(d, "$LXDT,ANS,OK*5c", 256, NULL, errBufSize, errBuf);
}



int DeviceASCIIConvert(TCHAR *pDest, TCHAR *pSrc, int size=11)
{
  char szTmp[size+10];
  if (pSrc && pDest)
  {
    TCHAR2usascii(pSrc , szTmp, size);
    ascii2TCHAR(szTmp, pDest,   size) ;
    return _tcslen(pDest);
  }
  return 0;
}


BOOL FormatTP( TCHAR* DeclStrings, int num, int total,const WAYPOINT *wp)
{
TCHAR Name[60];
DeviceASCIIConvert(Name, (TCHAR*)wp->Name,20) ;

  if(wp && DeclStrings)
  {
    _stprintf(DeclStrings, TEXT("LXDT,SET,TP,%i,%i,%i,%i,%s"),num,total+2,
                                                             ( int)(wp->Latitude*60000.0),
                                                             ( int)(wp->Longitude*60000.0),
                                                             Name );
  }
return true;
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
BOOL DevLX_EOS_ERA::DeclareTask(PDeviceDescriptor_t d,
  Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]) {
  Decl  decl;
  Class lxClass;

  bool Good  = true;


  if (!CheckWPCount(*lkDecl,Decl::min_wp_count - 2, Decl::max_wp_count - 2, errBufSize, errBuf)){
    return(false);
  }
  ShowProgress(decl_enable);
  ShowProgress(decl_send);
  Declare(true);

    

  TCHAR PilotName[12];
  TCHAR PilotSurName[12];
  TCHAR* NamePtr= NULL;
  NamePtr = _tcstok (lkDecl->PilotName, _T(" ,.-:_"));
  DeviceASCIIConvert(PilotName,  NamePtr ,11  );

  TCHAR* SurNamePtr = _tcstok (NULL,    _T(" ,.-:_"));
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

  _stprintf(DeclStrings[i++], TEXT("LXDT,SET,PILOT,%s,%s"),(PilotName), (PilotSurName));
  _stprintf(DeclStrings[i++], TEXT("LXDT,SET,GLIDER,%s,%s,%s,%s"), AircraftType, AircraftReg, AircraftCompID, AircraftClass);

  if(AATEnabled)
    _stprintf(DeclStrings[i++], TEXT("LXDT,SET,TSK_PAR,0,0,%02i:%02i"), (int)AATTaskLength/60,  (int)(AATTaskLength-((int)(AATTaskLength/60)*60)));
  else
    _stprintf(DeclStrings[i++], TEXT("LXDT,SET,TSK_PAR,0,0,00:00"));
  int num=0;

  int dir=0,autonxt=1,isline=0,a1=45,a2=45,a21=5000,r1=5000,r2=500, elev = WayPointList[HomeWaypoint].Altitude;
  FormatTP( (TCHAR*) &DeclStrings[i++], num , wpCount, &WayPointList[HomeWaypoint]);  // Takeoff

  num++;
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
    _stprintf(DeclStrings[i++], TEXT("LXDT,SET,ZONE,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i"),num,dir,autonxt,isline,a1,a2,a21,r1,r2, elev);
    num++;
  }
  FormatTP( (TCHAR*) &DeclStrings[i++], num , wpCount,  &WayPointList[HomeWaypoint]);   // Landing

  num++;

  // Send complete declaration to logger
  int  orgRxTimeout;
  SetRxTimeout(d, 4000,orgRxTimeout,  errBufSize , errBuf);
  int attemps =0;
  char RecBuf [4096] = "";  
  do    
  {
    for (int ii = 0; ii < i ; ii++){

      if (Good)
        Good = SendNmea(d, DeclStrings[ii]);
     
      StartupStore(_T(". EOS/ERA Decl: %s %s "),   DeclStrings[ii], NEWLINE);
      if (Good)
        Good = ComExpect(d, "$LXDT,ANS,OK*5c", 4095, RecBuf, errBufSize, errBuf);

    }
    attemps++;
    if(!Good)
      Poco::Thread::sleep(500);
  } while ((!Good) && (attemps < 3));

 
  int tmp ;
  SetRxTimeout(d, orgRxTimeout, tmp, errBufSize , errBuf);

  Declare(false);
  ShowProgress(decl_disable);
  return(Good);
} // DeclareTask()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Converts TCHAR[] string into US-ASCII string with characters safe for
/// writing to LX devices.
///
/// Characters are converted into their most similar representation
/// in ASCII. Nonconvertable characters are replaced by '?'.
///
/// Output string will always be terminated by '\0'.
///
/// @param input    input string (must be terminated with '\0')
/// @param outSize  output buffer size
/// @param output   output buffer
///
/// @retval true  all characters copied
/// @retval false some characters could not be copied due to buffer size
///
//static
bool DevLX_EOS_ERA::Wide2LxAscii(const TCHAR* input, int outSize, char* output){
  if (outSize == 0)
    return(false);
  int res = TCHAR2usascii(input, output, outSize);
  // replace all non-ascii characters with '?' - LX devices is very sensitive
  // on non-ascii chars - the electronic seal can be broken
  // (unicode2usascii() should be enough, but to be sure that someone has not
  // incorrectly changed unicode2usascii())
  output--;
  while (*++output != '\0') {
    if (*output < 32 || *output > 126) *output = '?';
  }
  return(res >= 0);
} // Wide2LxAscii()



// #############################################################################
// *****************************************************************************
//
//   DevLX_EOS_ERA::Decl
//
// *****************************************************************************
// #############################################################################


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constructor - sets all data to 0.
///
DevLX_EOS_ERA::Decl::Decl(){
  memset(this, 0, sizeof(*this));
} // Decl()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Format waypoint data
///
/// @param buf  [out] buffer
/// @param wp    waypoint data (if @c NULL, LAT/LON will be set to 0)
/// @param type  waypoint type
/// @param idx   waypoint index
///
void DevLX_EOS_ERA::Decl::WpFormat(TCHAR buf[], const WAYPOINT* wp, WpType type, int totalNum ){
  int DegLat, DegLon;

  double MinLat, MinLon;
  char NS, EW;
  int iAlt =0.0;
  const TCHAR* wpName = _T("");
  if (wp != NULL) {
   iAlt =(int) wp->Altitude ;
    DegLat = (int)wp->Latitude;
    MinLat = wp->Latitude - DegLat;
    if((MinLat<0) || ((MinLat-DegLat==0) && (DegLat<0))) {
        NS = 'S'; DegLat *= -1; MinLat *= -1;
    } else NS = 'N';
    DegLon = (int)wp->Longitude ;
    MinLon = wp->Longitude - DegLon;
    if((MinLon<0) || ((MinLon-DegLon==0) && (DegLon<0))) {
        EW = 'W'; DegLon *= -1; MinLon *= -1;
    } else EW = 'E';
    wpName = wp->Name;
  } else {
    DegLat = MinLat = DegLon = MinLon = 0;
    NS = 'N'; EW = 'E';
    // set TP name
    switch (type) {
      case tp_takeoff: wpName = _T("TAKEOFF"); break;
      case tp_landing: wpName = _T("LANDING"); break;
      case tp_regular: wpName = _T("WP");      break;
      default:         wpName = _T("");        break;
    }
  }
  _stprintf(buf, TEXT("C%02d%05.0f%c%03d%05.0f%c%s::%i.0"),
             DegLat, MinLat*60000,NS,DegLon, MinLon*60000, EW,wpName,iAlt );
} // WpFormat()


// #############################################################################
// *****************************************************************************
//
//   DevLX_EOS_ERA::Class
//
// *****************************************************************************
// #############################################################################



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constructor - sets all data to 0.
///
DevLX_EOS_ERA::Class::Class(){
  memset(this, 0, sizeof(*this));
} // Class()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the value of @c name member.
///
/// @param text  string to be set (will be converted into ASCII)
///
void DevLX_EOS_ERA::Class::SetName(const TCHAR* text){
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
bool DevLX_EOS_ERA::SendNmea(PDeviceDescriptor_t d, const TCHAR buf[], unsigned errBufSize, TCHAR errBuf[]){

  ScopeLock Lock(CritSec_Comm);
  if(!d || !d->Com) {
    return false;
  }

  char asciibuf[256];
  DevLX_EOS_ERA::Wide2LxAscii(buf, 256, asciibuf);
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


bool DevLX_EOS_ERA::SendNmea(PDeviceDescriptor_t d, const TCHAR buf[]){
TCHAR errBuf[10]= _T("");
TCHAR errBufSize=10;
  DevLX_EOS_ERA::SendNmea(d,  buf,errBufSize,errBuf);
  if(_tcslen (errBuf) > 1)
  {
    DoStatusMessage(errBuf);
    return false;
  }
 // StartupStore(_T(" LX_EOS_ERA SenadNmea %s %s"),buf, NEWLINE);
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
bool DevLX_EOS_ERA::SendDecl(PDeviceDescriptor_t d, unsigned row, unsigned n_rows,
                   TCHAR content[], unsigned errBufSize, TCHAR errBuf[]){
  TCHAR szTmp[MAX_NMEA_LEN];

  char retstr[20];
  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PLXVC,DECL,W,%u,%u,%s"), row, n_rows, content);
  bool status = DevLX_EOS_ERA::SendNmea(d, szTmp, errBufSize, errBuf);
  if (status) {
    sprintf(retstr, "$PLXVC,DECL,C,%u", row);
    status = status && ComExpect(d, retstr, 512, NULL, errBufSize, errBuf);

  }
  return status;

} // SendDecl()




void DevLX_EOS_ERA::GetDirections(WndButton* pWnd){
  if(pWnd) {
    WndForm * wf = pWnd->GetParentWndForm();
    if(wf) {
    int PortNum = Port();

      WndProperty *wp;
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

void DevLX_EOS_ERA::OnCloseClicked(WndButton* pWnd)
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


void DevLX_EOS_ERA::OnValuesClicked(WndButton* pWnd) {

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
    SendNmea(Device(),_T("LXDT,GET,MC_BAL"));  // request new data
  }

}

void DevLX_EOS_ERA::OnIGCDownloadClicked(WndButton* pWnd) {
(void)pWnd;
LockFlightData();
bool bInFlight    = CALCULATED_INFO.Flying;
UnlockFlightData();

  if(bInFlight) {
    MessageBoxX(MsgToken(2418), MsgToken(2397), mbOk);
    return;
  }

    SendNmea(Device(), _T("LXDT,GET,FLIGHTS_NO"));

    dlgLX_IGCSelectListShowModal();
}




 bool  DevLX_EOS_ERA::OnStartIGC_FileRead(TCHAR Filename[]) {

TCHAR IGCFilename[MAX_PATH];
LocalPath(IGCFilename, _T(LKD_LOGS), Filename);

  f = _tfopen( IGCFilename, TEXT("w"));
  if(f == NULL)   return false;
  fclose(f);
  // SendNmea(Device(), _T("PLXVC,KEEP_ALIVE,W"), errBufSize, errBuf);
  StartupStore(_T(" ******* LX_EOS_ERA  IGC Download START ***** %s") , NEWLINE);
  /*
   * TCHAR szTmp[MAX_NMEA_LEN];
  _sntprintf(szTmp,MAX_NMEA_LEN, _T("PLXVC,FLIGHT,R,%s,1,%u"),Filename,BLOCK_SIZE+1);
  _sntprintf(m_Filename, array_size(m_Filename), _T("%s"),Filename);
  SendNmea(Device(), szTmp);
  StartupStore(_T("> %s %s") ,szTmp, NEWLINE);
  IGCDownload(true);
#ifdef  NANO_PROGRESS_DLG
  CreateIGCProgressDialog();
#endif*/
return true;

}



BOOL DevLX_EOS_ERA::AbortLX_IGC_FileRead(void)
{

  if(f != NULL)
  {
    fclose(f); f= NULL;
  }
  bool bWasInProgress = IGCDownload() ;
  IGCDownload ( false );

#ifdef  NANO_PROGRESS_DLG
  CloseIGCProgressDialog();
#endif
  return bWasInProgress;
}


BOOL DevLX_EOS_ERA::Close (PDeviceDescriptor_t d) {
  Device(NULL);
  return TRUE;
}







//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP0 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevLX_EOS_ERA::LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
  // $LXWP0,logger_stored, airspeed, airaltitude,
  //   v1[0],v1[1],v1[2],v1[3],v1[4],v1[5], hdg, windspeed*CS<CR><LF>
  //
  // 0 loger_stored : [Y|N] (not used in LX1600)
  // 1 IAS [km/h] ----> Condor uses TAS!
  // 2 baroaltitude [m]
  // 3-8 vario values [m/s] (last 6 measurements in last second)
  // 9 heading of plane (not used in LX1600)
  // 10 windcourse [deg] (not used in LX1600)
  // 11 windspeed [km/h] (not used in LX1600)
  //
  // e.g.:
  // $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

double fDir,fTmp,airspeed=0;

 // if( !devGetAdvancedMode(d))
  {
    if( ParToDouble(sentence, 1, &fTmp))
    {

      if(Values(d))
      { TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp, MAX_NMEA_LEN,_T("%5.1fkm/h ($LXWP0)"),fTmp);
        SetDataText(_SPEED,   szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].SPEEDDir  ))
      {
        airspeed = fTmp/TOKPH;
        info->IndicatedAirspeed = airspeed;
        info->AirspeedAvailable = TRUE;
      }
    }

    if (ParToDouble(sentence, 2, &fTmp))
    {
      if(Values(d))
      { TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp, MAX_NMEA_LEN, _T("%5.1fm ($LXWP0)"),fTmp);
        SetDataText( _BARO,   szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].BARODir  ))
      {
        if (airspeed>0) info->IndicatedAirspeed = airspeed / AirDensityRatio(fTmp);
        UpdateBaroSource( info, 0, d,fTmp);
      }
    }

    if (ParToDouble(sentence, 3, &fTmp))
    {
      if(Values(d))
      { TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.1fm ($LXWP0)"),fTmp/TOKPH);
        SetDataText( _VARIO,   szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].VARIODir  ))
      {
        UpdateVarioSource(*info, *d, fTmp);
      }
    }
  }

  if (ParToDouble(sentence, 10, &fDir) &&
  ParToDouble(sentence, 11, &fTmp))
  {

    if(Values(d))
    { TCHAR szTmp[MAX_NMEA_LEN];
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.1fkm/h %3.0f° ($LXWP0)"),fTmp,fDir);
      SetDataText( _WIND,   szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].WINDDir  ))
    {
      info->ExternalWindDirection = fDir;
      info->ExternalWindSpeed =  fTmp/TOKPH;
      info->ExternalWindAvailable = TRUE;
    }
  }

  return(false);
} // LXWP0()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP1 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevLX_EOS_ERA::LXWP1(PDeviceDescriptor_t d, const TCHAR* String, NMEA_INFO* pGPS)
{
  // $LXWP1,serial number,instrument ID, software version, hardware
  //   version,license string,NU*SC<CR><LF>
  //
  // instrument ID ID of LX1600
  // serial number unsigned serial number
  // software version float sw version
  // hardware version float hw version
  // license string (option to store a license of PDA SW into LX1600)
 // ParToDouble(sentence, 1, &MACCREADY);
//  $LXWP1,LX5000IGC-2,15862,11.1 ,2.0*4A
#ifdef DEVICE_SERIAL
TCHAR ctemp[MAX_NMEA_LEN];
static int NoMsg=0;
static int oldSerial=0;
if(_tcslen(String) < 180)
  if((( pGPS->SerialNumber == 0)  || ( pGPS->SerialNumber != oldSerial)) && (NoMsg < 5))
  {
    NoMsg++ ;
    NMEAParser::ExtractParameter(String,ctemp,0);
    if(_tcslen(ctemp) < DEVNAMESIZE)
      _tcsncpy(d->Name, ctemp, DEVNAMESIZE);
    StartupStore(_T(". %s\n"),ctemp);

    NMEAParser::ExtractParameter(String,ctemp,1);
    pGPS->SerialNumber= (int)StrToDouble(ctemp,NULL);
    oldSerial = pGPS->SerialNumber;
    _stprintf(ctemp, _T("%s Serial Number %i"), d->Name, pGPS->SerialNumber);
    StartupStore(_T(". %s\n"),ctemp);

    NMEAParser::ExtractParameter(String,ctemp,2);
    pGPS->SoftwareVer= StrToDouble(ctemp,NULL);
    _stprintf(ctemp, _T("%s Software Vers.: %3.2f"), d->Name, pGPS->SoftwareVer);
    StartupStore(_T(". %s\n"),ctemp);

    NMEAParser::ExtractParameter(String,ctemp,3);
    pGPS->HardwareId= (int)(StrToDouble(ctemp,NULL)*10);
    _stprintf(ctemp, _T("%s Hardware Vers.: %3.2f"), d->Name, (double)(pGPS->HardwareId)/10.0);
    StartupStore(_T(". %s\n"),ctemp);
    _stprintf(ctemp, _T("%s (#%i) DETECTED"), d->Name, pGPS->SerialNumber);
    DoStatusMessage(ctemp);
    _stprintf(ctemp, _T("SW Ver: %3.2f HW Ver: %3.2f "),  pGPS->SoftwareVer, (double)(pGPS->HardwareId)/10.0);
    DoStatusMessage(ctemp);
  }
  // nothing to do
#endif
  return(true);
} // LXWP1()


BOOL DevLX_EOS_ERA::EOSSetMC(PDeviceDescriptor_t d,float fTmp, const TCHAR *info )
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

BOOL DevLX_EOS_ERA::EOSSetBAL(PDeviceDescriptor_t d,float fTmp, const TCHAR *info)
{
bool ret = false;


  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    _sntprintf(szTmp,MAX_NMEA_LEN,  _T("%3.0f L = %3.0f%% %s"),fTmp,(fTmp/WEIGHTS[2]*100.0), info);
    SetDataText(_BAL,  szTmp);
  }
  if(IsDirInput(PortIO[d->PortNumber].BALDir  ))
  {
    if(fabs(fTmp- GlidePolar::BallastLitres) > 1 )
    {
      GlidePolar::BallastLitres = fTmp;
      BALLAST =  GlidePolar::BallastLitres / WEIGHTS[2];
      ret = true;
    }
  }

  return ret;
}

BOOL DevLX_EOS_ERA::EOSSetBUGS(PDeviceDescriptor_t d,float fTmp, const TCHAR *info)
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP2 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevLX_EOS_ERA::LXWP2(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO*)
{
  // $LXWP2,mccready,ballast,bugs,polar_a,polar_b,polar_c, audio volume
  //   *CS<CR><LF>
  //
  // Mccready: float in m/s
  // Ballast: float 1.0 ... 1.5
  // Bugs: 0 - 100%
  // polar_a: float polar_a=a/10000 w=a*v2+b*v+c
  // polar_b: float polar_b=b/100 v=(km/h/100) w=(m/s)
  // polar_c: float polar_c=c
  // audio volume 0 - 100%
//float fBallast,fBugs, polar_a, polar_b, polar_c, fVolume;


double fTmp;
if(!LX_EOS_ERA_bValid)
{
   if (ParToDouble(sentence, 0, &fTmp))
     EOSSetMC  (d, fTmp,_T("($LXWP2)") );

   if(ParToDouble(sentence, 1, &fTmp))
     EOSSetBAL (d, fTmp,_T("($LXWP2)") );

   if(ParToDouble(sentence, 2, &fTmp))
     EOSSetBUGS(d, fTmp,_T("($LXWP2)") );
}
 double fa,fb,fc;
     if(ParToDouble(sentence, 3, &fa))
       if(ParToDouble(sentence, 4, &fb))
         if(ParToDouble(sentence, 5, &fc))
         {
            TCHAR szTmp[MAX_NMEA_LEN];
            if(Values(d))
            {
               _sntprintf(szTmp,MAX_NMEA_LEN, _T("a:%5.3f b:%5.3f c:%5.3f ($LXWP2)"),fa,fb,fc);
               SetDataText(  _POLAR,  szTmp);
            }

            if(IsDirInput(PortIO[d->PortNumber].POLARDir ))
            {
              extern bool PolarWinPilot2XCSoar(double dPOLARV[3], double dPOLARW[3], double ww[2]);

              double v;
              for (int i=0; i < 3; i++)
              {
                v=POLARV[i]/100;
                POLARLD[i] = -(fa*v*v + fb*v + fc);
#ifdef POLAR_DEBUG
                _sntprintf(szTmp,MAX_NMEA_LEN, _T("V[%i]:%5.0f    s[%i]:%6.2f  ($LXWP2)"),i,POLARV[i],i,POLARLD[i] );
                StartupStore(TEXT("EOS/ERA Polar: %s"), szTmp);
#endif
              }
              _stprintf (szPolarName, _T("%s"), d->Name );
              PolarWinPilot2XCSoar(POLARV, POLARLD, WW);
              GlidePolar::SetBallast();
            }
          }



     if(ParToDouble(sentence, 6, &fTmp))
     {
       // volume
     }




  return(true);
} // LXWP2()


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Parses LXWP3 sentence.
///
/// @param d         device descriptor
/// @param sentence  received NMEA sentence
/// @param info      GPS info to be updated
///
/// @retval true if the sentence has been parsed
///
//static
BOOL DevLX_EOS_ERA::LXWP3(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
double fTmp;


  if(ParToDouble(sentence, 1, &fTmp))  // SC mode
  {
    int  iTmp = (int)(fTmp+0.1);
    if(Values(d))
    {
      switch(iTmp)
      {
        case 0:  SetDataText( _STF, _T("MANUAL ($LXWP3)"));
        case 1:  SetDataText( _STF, _T("VARIO ($LXWP3)"));
        case 2:  SetDataText( _STF, _T("SPEED ($LXWP3)"));
      }
    }

    static int  iOldVarioSwitch=0;
    if(IsDirInput(PortIO[d->PortNumber].STFDir))
    {
      EnableExternalTriggerCruise = true;
      if(iTmp != iOldVarioSwitch)
      {
        iOldVarioSwitch = iTmp;
        if(iTmp==2)
        {
          ExternalTriggerCruise = true;
          ExternalTriggerCircling = false;
        }
        else
        {
          ExternalTriggerCruise = false;
          ExternalTriggerCircling = true;
        }
      }
    }
  }

  // $LXWP3,altioffset, scmode, variofil, tefilter, televel, varioavg,
  //   variorange, sctab, sclow, scspeed, SmartDiff,
  //   GliderName, time offset*CS<CR><LF>
  //
  // altioffset //offset necessary to set QNE in ft default=0
  // scmode // methods for automatic SC switch index 0=EXTERNAL, 1=ON CIRCLING
  //   2=auto IAS default=1
  // variofil // filtering of vario in seconds (float) default=1
  // tefilter // filtering of TE compensation in seconds (float) 0 = no
  //   filtering (default=0)
  // televel // level of TE compensation from 0 to 250 default=0 (%) default=0
  // varioavg // averaging time in seconds for integrator default=25
  // variorange // 2.5 5 or 10 (m/s or kts) (float) default=5.0
  // sctab // area of silence in SC mode (float) 0-5.0 1.0= silence between
  //   +1m/s and -1m/s default=1
  // sclow // external switch/taster function 0=NORMAL 1=INVERTED 2=TASTER
  //   default=1
  // scspeed // speed of automatic switch from vario to sc mode if SCMODE==2 in
  //   (km/h) default=110
  // SmartDiff float (m/s/s) (Smart VARIO filtering)
  // GliderName // Glider name string max. 14 characters
  // time offset int in hours

  // nothing to do
  return(true);
} // LXWP3()




BOOL DevLX_EOS_ERA::LXWP4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
// $LXWP4 Sc, Netto, Relativ, gl.dif, leg speed, leg time, integrator, flight time, battery voltage*CS<CR><LF>
// Sc  float (m/s)
// Netto  float (m/s)
// Relativ  float (m/s)
// Distance float (m)
// gl.dif  int (ft)
// leg speed (km/h)
// leg time (km/h)
// integrator float (m/s)
// flight time unsigned in seconds
// battery voltage float (V)
  return(true);
} // LXWP4()


extern BOOL ValidFrequency(double Freq);

BOOL DevLX_EOS_ERA::LXDT(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
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
    if(ParToDouble(sentence, 2, &fTmp)) if(ValidFrequency(fTmp)) RadioPara.ActiveFrequency  = fTmp;
    if(ParToDouble(sentence, 3, &fTmp)) if(ValidFrequency(fTmp)) RadioPara.PassiveFrequency = fTmp;
    if(ParToDouble(sentence, 4, &fTmp)) RadioPara.Volume  = (int) fTmp;
    if(ParToDouble(sentence, 5, &fTmp)) RadioPara.Squelch = (int) fTmp;
    if(ParToDouble(sentence, 6, &fTmp)) RadioPara.Vox     = (int) fTmp;
  }
  else
  if(_tcsncmp(szTmp, _T("MC_BAL"), 6) == 0)
  {
    StartupStore(TEXT("MC_BAL %s"), sentence);
    if(ParToDouble(sentence, 2, &fTmp)) {EOSSetMC(d, fTmp,_T("($LXDT)") );}
    if(ParToDouble(sentence, 3, &fTmp)) {EOSSetBAL(d, fTmp,_T("($LXDT)") );}
    if(ParToDouble(sentence, 4, &fTmp)) {EOSSetBUGS(d, fTmp,_T("($LXDT)") );}
    if(ParToDouble(sentence, 5, &fTmp)) {}
    if(ParToDouble(sentence, 6, &fTmp)) {}
    if(ParToDouble(sentence, 7, &fTmp)) {}
    LX_EOS_ERA_bValid = true;
 
  }
  else
  if(_tcsncmp(szTmp, _T("FLIGHTS_NO"), 10) == 0)
  {
    if(ParToDouble(sentence, 2, &fTmp)) iNoFlights =(int) (fTmp+0.05);
    if(iNoFlights > 0)
    {
      _sntprintf(szTmp, MAX_NMEA_LEN, _T("LXDT,GET,FLIGHT_INFO,%i"),1);
      SendNmea(d,szTmp);
    }
  }
  else
  if(_tcsncmp(szTmp, _T("FLIGHT_INFO"), 11) == 0)
  { TCHAR FileName[50]= _T("FileName"), Pilot[50]= _T(""),Surname[50]= _T(""), Takeoff[50]= _T(""),Date[50]= _T(""),Landing[50]= _T(""),Type[50]= _T(""), Reg[50]= _T("");
    StartupStore(TEXT("FLIGHT_INFO %s"), sentence);

    NMEAParser::ExtractParameter(sentence, Date     ,2);int  iNo = (int) StrToDouble(Date,nullptr);
    NMEAParser::ExtractParameter(sentence, FileName ,3 );
    NMEAParser::ExtractParameter(sentence, Date     ,4);
    NMEAParser::ExtractParameter(sentence, Takeoff  ,5);
    NMEAParser::ExtractParameter(sentence, Landing  ,6);
    NMEAParser::ExtractParameter(sentence, Pilot    ,7);
    NMEAParser::ExtractParameter(sentence, Surname  ,8);
    NMEAParser::ExtractParameter(sentence, Type     ,9);
    NMEAParser::ExtractParameter(sentence, Reg      ,10);
    TCHAR Line[2][MAX_NMEA_LEN];
    _sntprintf( Line[0],MAX_NMEA_LEN, _T("%s %s %s  %s %s"),FileName, Pilot,Surname, Reg, Type);
    _sntprintf( Line[1],MAX_NMEA_LEN, _T("%s (%s-%s) "), Date ,Takeoff ,Landing);
    AddElement(Line[0], Line[1]);
    if(iNo < iNoFlights)
    {
      _sntprintf(szTmp, MAX_NMEA_LEN, _T("LXDT,GET,FLIGHT_INFO,%i"),iNo+1);
      SendNmea(d,szTmp);
    }
  }
  else
  if(_tcsncmp(szTmp, _T("ERROR"), 5) == 0)  // ERROR?
  {
    NMEAParser::ExtractParameter(sentence, szTmp, 2);
    DoStatusMessage(TEXT("LX EOS/ERA Error: %s"), szTmp);
    StartupStore(TEXT("LX EOS/ERA Error: %s"), szTmp);
  }
  else
  if(_tcsncmp(szTmp, _T("OK"), 2) == 0)
  {
    if(!Declare())
      SendNmea(d,_T("LXDT,GET,MC_BAL"));
    else
    {
    }
  }
  return(true);
} // LXDT()



BOOL DevLX_EOS_ERA::LXBC(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR szTmp[MAX_NMEA_LEN];

devSetAdvancedMode(d,true);


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
          _sntprintf(szTmp, MAX_NMEA_LEN, _T("gX:%5.2f gY:%5.2f gZ:%5.2f Pitch:%5.2f Roll:%5.2f Yaw:%5.2f Slip:%5.2f($LXDT)"),fX,fY,fZ, fPitch, fRoll, fYaw, fSlip);
        else
          _sntprintf(szTmp, MAX_NMEA_LEN, _T("gX:%5.2f gY:%5.2f gZ:%5.2f($LXDT)"),fX,fY,fZ);
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


BOOL DevLX_EOS_ERA::EOSPutMacCready(PDeviceDescriptor_t d, double MacCready){
TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;

  if(!IsDirOutput(PortIO[d->PortNumber].MCDir)) return false;

  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("LXDT,SET,MC_BAL,%.1f,,,,,"), MacCready);
  StartupStore(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);
 /* Poco::Thread::sleep(REQ_DEADTIME);
  SendNmea(d,_T("LXDT,GET,MC_BAL"));*/
return true;

}


BOOL DevLX_EOS_ERA::EOSPutBallast(PDeviceDescriptor_t d, double Ballast){
TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;
  if(LX_EOS_ERA_bValid == false) return false;
  if(!IsDirOutput(PortIO[d->PortNumber].BALDir)) return false;
  
  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("LXDT,SET,MC_BAL,,%.0f,,,,"),GlidePolar::BallastLitres);
  StartupStore(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);
 /* Poco::Thread::sleep(REQ_DEADTIME);
  SendNmea(d,_T("LXDT,GET,MC_BAL"));*/


return(TRUE);
}


BOOL DevLX_EOS_ERA::EOSPutBugs(PDeviceDescriptor_t d, double Bugs){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;

  if(!IsDirOutput(PortIO[d->PortNumber].BUGDir)) return false;
  double fLXBugs = CalculateLXBugs( Bugs);

  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("LXDT,SET,MC_BAL,,,%.0f,,,"),fLXBugs);
  StartupStore(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);
  /*
  Poco::Thread::sleep(REQ_DEADTIME);
  SendNmea(d,_T("LXDT,GET,MC_BAL"));*/


return(TRUE);
}


BOOL DevLX_EOS_ERA::PutTarget(PDeviceDescriptor_t d)
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
  {                                    // PLXVTARG,KOLN,4628.80   ,N ,01541.167 ,E ,268.0
    _sntprintf( szTmp,MAX_NMEA_LEN, TEXT("PLXVTARG,%s,%02d%05.2f,%c,%03d%05.2f,%c,%i"),
      szName, DegLat, MinLat, NoS, DegLon, MinLon, EoW,
      (int) (WayPointList[overindex].Altitude +0.5));

      _tcsncat (szName, _T(" ($PLXVTARG)"),MAX_VAL_STR_LEN);
#ifdef TESTBENCH
     StartupStore(TEXT("Send navigation Target LXNav: %s"), szName);
#endif
  }
  else
  {
    if( PortIO[d->PortNumber].T_TRGTDir  == TP_GPRMB)
    {               //                      GPRMB,A,,,,H>TAKEOFF,5144.78,N,00616.70,E,,,A
      _sntprintf( szTmp,MAX_NMEA_LEN, TEXT("GPRMB,A,,,%s,%02d%05.2f,%c,%03d%05.2f,%c,,,,A"),
        szName, DegLat, MinLat, NoS, DegLon, MinLon, EoW);

      _tcsncat (szName, _T(" ($GPRMB)"),MAX_VAL_STR_LEN);
    }
#ifdef TESTBENCH
    StartupStore(TEXT("Send navigation Target LXNav: %s"), szName);
#endif
  }

  SetDataText( _T_TRGT,  szName);
  DevLX_EOS_ERA::SendNmea(d,szTmp);

return(true);
}



BOOL DevLX_EOS_ERA::GPRMB(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
  if(PortIO[d->PortNumber].R_TRGTDir != TP_GPRMB) {
    return false;
  }

TCHAR  szTmp[MAX_NMEA_LEN];

double fTmp;
  NMEAParser::ExtractParameter(sentence,szTmp,4);


    if(Alternate2 == RESWP_EXT_TARGET) // pointing to external target?
      Alternate2 = -1;                 // clear external =re-enable!


  _tcscpy(WayPointList[RESWP_EXT_TARGET].Name, _T("^") );
  _tcscat(WayPointList[RESWP_EXT_TARGET].Name, szTmp );

  ParToDouble(sentence, 5, &fTmp);
  double DegLat = (double)((int) (fTmp/100.0));
  double MinLat =  fTmp- (100.0*DegLat);
  double Latitude = DegLat+MinLat/60.0;
  TCHAR NoS;
  NMEAParser::ExtractParameter(sentence,&NoS,6);
  if (NoS==_T('S')) {
    Latitude *= -1;
  }

  ParToDouble(sentence, 7, &fTmp);
  double DegLon =  (double) ((int) (fTmp/100.0));
  double MinLon =  fTmp- (100.0*DegLon);
  double Longitude = DegLon+MinLon/60.0;
  TCHAR EoW;
  NMEAParser::ExtractParameter(sentence,&EoW,8);
  if (EoW==_T('W')) {
    Longitude *= -1;
  }
  WayPointList[RESWP_EXT_TARGET].Latitude=Latitude;
  WayPointList[RESWP_EXT_TARGET].Longitude=Longitude;
  WayPointList[RESWP_EXT_TARGET].Altitude=0;  // GPRMB has no elevation information
  Alternate2 = RESWP_EXT_TARGET;

  if(Values(d))
  {
    _tcsncat(szTmp, _T(" ($GPRMB)"),MAX_NMEA_LEN );
    SetDataText( _R_TRGT,  szTmp);
  }
  return false;
}



BOOL DevLX_EOS_ERA::EOSRequestRadioInfo(PDeviceDescriptor_t d)
{

  Poco::Thread::sleep(50);
  SendNmea(d,(TCHAR*)_T("LXDT,GET,RADIO"));
  return true;
}

BOOL DevLX_EOS_ERA::EOSPutVolume(PDeviceDescriptor_t d, int Volume) {

  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("LXDT,SET,RADIO,,,%i,,,"),Volume)  ;
  SendNmea(d,szTmp);
  if(uiEOSDebugLevel) StartupStore(_T(". EOS Volume  %i%s"), Volume,NEWLINE);
  RadioPara.Volume = Volume;
  EOSRequestRadioInfo(d);
  return(TRUE);
}




BOOL DevLX_EOS_ERA::EOSPutSquelch(PDeviceDescriptor_t d, int Squelch) {

  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("LXDT,SET,RADIO,,,,%i,,"),Squelch)  ;
  SendNmea(d,szTmp);
  if(uiEOSDebugLevel) StartupStore(_T(". EOS Squelch  %i%s"), Squelch,NEWLINE);
  RadioPara.Squelch = Squelch;
  EOSRequestRadioInfo(d);
  return(TRUE);
}



BOOL DevLX_EOS_ERA::EOSPutFreqActive(PDeviceDescriptor_t d, double Freq, const TCHAR* StationName) {

  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("LXDT,SET,RADIO,%7.3f,,,,,"),Freq)  ;
  SendNmea(d,szTmp);
  EOSRequestRadioInfo(d);
  RadioPara.ActiveFrequency=  Freq;
  if(StationName != NULL)
    _sntprintf(RadioPara.ActiveName, NAME_SIZE,_T("%s"),StationName) ;
  if(uiEOSDebugLevel) StartupStore(_T(". EOS Active Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);

  return(TRUE);
}


BOOL DevLX_EOS_ERA::EOSPutFreqStandby(PDeviceDescriptor_t d, double Freq,  const TCHAR* StationName) {

  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("LXDT,SET,RADIO,,%7.3f,,,,"),Freq)  ;
  SendNmea(d,szTmp);
  EOSRequestRadioInfo(d);
  RadioPara.PassiveFrequency =  Freq;
  if(StationName != NULL)
    _sntprintf(RadioPara.PassiveName , NAME_SIZE ,_T("%s"),StationName) ;
  if(uiEOSDebugLevel) StartupStore(_T(". EOS Standby Station %7.3fMHz %s%s"), Freq, StationName,NEWLINE);

  return(TRUE);
}


BOOL DevLX_EOS_ERA::EOSStationSwap(PDeviceDescriptor_t d) {


  SendNmea(d,_T("LXDT,SET,R_SWITCH"));

  EOSRequestRadioInfo(d);

  if(uiEOSDebugLevel) StartupStore(_T(". EOS  station swap %s"), NEWLINE);

  return(TRUE);
}


BOOL DevLX_EOS_ERA::EOSRadioMode(PDeviceDescriptor_t d, int mode) {

  TCHAR  szTmp[255];
  _stprintf(szTmp,_T("LXDT,SET,R_DUAL,%i"),mode);
  SendNmea(d,(TCHAR*)szTmp);
  EOSRequestRadioInfo(d);
  if(uiEOSDebugLevel) StartupStore(_T(". EOS  Dual Mode: %i %s"), mode, NEWLINE);
  return(TRUE);
}

