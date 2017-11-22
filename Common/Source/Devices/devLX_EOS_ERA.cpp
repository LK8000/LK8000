/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "devLX_EOS_ERA.h"
#include "dlgEOSIGCDownload.h"
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
#include "utils/charset_helper.h"
#include "utils/printf.h"
#include "Comm/UpdateQNH.h"


unsigned int uiEOSDebugLevel = 1;

BOOL DevLX_EOS_ERA::m_bShowValues = false;
BOOL DevLX_EOS_ERA::bIGC_Download = false;
BOOL DevLX_EOS_ERA::m_bTriggered = false;

#define TIMEOUTCHECK

#define MAX_VAL_STR_LEN    60

BOOL LX_EOS_ERA_bValid = false;

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
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
void DevLX_EOS_ERA::Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, GetName());
  d->Open         = Open;
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = EOSPutMacCready;
  d->PutBugs      = EOSPutBugs;
  d->PutBallast   = EOSPutBallast;
  d->PutQNH       = PutQNH;
  d->Declare      = DeclareTask;

  d->Config       = Config;

  d->PutVolume      = EOSPutVolume;
  d->PutSquelch     = EOSPutSquelch;
  d->PutFreqActive  = EOSPutFreqActive;
  d->PutFreqStandby = EOSPutFreqStandby;
  d->StationSwap    = EOSStationSwap;
  d->PutRadioMode   = EOSRadioMode;
  d->ParseStream    = EOSParseStream;
  d->PutTarget      = PutTarget;
} // Install()

BOOL DevLX_EOS_ERA::Open(DeviceDescriptor_t* d) {
  ResetMultitargetSync();
  return TRUE;
}

namespace {
  std::queue<uint8_t> EOSbuffered_data;
  Mutex EOSmutex;
  Cond EOScond;
  bool bEOSBinMode= false;
}

bool EOSBlockReceived() {
  ScopeLock lock(EOSmutex);
  return (!EOSbuffered_data.empty());
}
  
bool IsEOSInBinaryMode() {
  ScopeLock lock(EOSmutex);
  return bEOSBinMode;
}

bool SetEOSBinaryModeFlag(bool bBinMode) {
  ScopeLock lock(EOSmutex);
  bool OldVal = bEOSBinMode;
  bEOSBinMode = bBinMode;
  if(!bEOSBinMode) {
    // same as clear() but free allocated memory.
    EOSbuffered_data = std::queue<uint8_t>();
  }
  return OldVal;
}



BOOL DevLX_EOS_ERA::EOSParseStream(DeviceDescriptor_t* d, char *String, int len, NMEA_INFO *GPS_INFO) {
  if ((!d) || (!String) || (!len)) {
    return FALSE;
  }
  
  static BOOL slowdown = false;
  if (!IsEOSInBinaryMode()) {    
    if(slowdown) {
      SendNmea(d, _T("PFLX0,LXWP0,1,LXWP1,5,LXWP2,1,LXWP3,1,GPRMB,5"));

      SendNmea(d, _T("LXDT,SET,BC_INT,AHRS,0.5,SENS,2.0"));
      TestLog(TEXT("NMEA SLOWDOWN OFF!!"));      
      slowdown = false;
    }       
    return FALSE;      
  }
  
  if(!slowdown) {
     SendNmea(d, _T("PFLX0,LXWP0,100,LXWP1,100,LXWP2,100,LXWP3,100,GPRMB,100"));      
     SendNmea(d, _T("LXDT,SET,BC_INT,ALL,0.0"));

     TestLog(_T("NMEA SLOWDOWN"));
     slowdown = true;
  }

  ScopeLock lock(EOSmutex);
  for (int i = 0; i < len; i++) {
    EOSbuffered_data.push((uint8_t)String[i]);
  }
  EOScond.Broadcast();

  return  true;
}




uint8_t EOSRecChar(DeviceDescriptor_t* d, uint8_t *inchar, uint16_t Timeout) {
  ScopeLock lock(EOSmutex);

  while(EOSbuffered_data.empty()) {
    Sleep(1);
    Poco::Thread::yield();

    if(!EOScond.Wait(EOSmutex, Timeout)) 
    {
      return REC_TIMEOUT_ERROR;
    }
  }
  if(inchar) {
    *inchar = EOSbuffered_data.front();
  }
  EOSbuffered_data.pop();

  return REC_NO_ERROR;
}

uint8_t EOSRecChar16(DeviceDescriptor_t* d, uint16_t *inchar, uint16_t Timeout) {
  ConvUnion tmp;
  int error = EOSRecChar(d, &(tmp.byte[0]), Timeout);
  if (error == REC_NO_ERROR) {
    error = EOSRecChar(d, &(tmp.byte[1]), Timeout);
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


BOOL DevLX_EOS_ERA::ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  if (Declare()) return false ;  // do not configure during declaration
  if( IsEOSInBinaryMode()) return false;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  static char lastSec =0;
  if( /*!Declare() &&*/ (info->Second != lastSec))  // execute every second only if no task is declaring
  {
    lastSec = info->Second;
    if((info->Second % 10) ==0) // config every 10s (not on every xx $GPGGA as there are 10Hz GPS now
    {
      SetupLX_Sentence(d);
    }

    if( ((info->Second+2) %4) ==0)
      SendNmea(d, TEXT("LXDT,GET,SENS"));
    if( ((info->Second+4) %4) ==0) 
      SendNmea(d, TEXT("LXDT,GET,NAVIGATE,0"));
  }

  if(IsDirOutput(PortIO.STFDir))
  {

    bool circling = MapWindow::mode.Is(MapWindow::Mode::MODE_CIRCLING);
    static  int   Thermalmode = -1; 
    if((circling != (bool)Thermalmode) || (Thermalmode <0))
    {
      if(circling)
      {
        SendNmea(d, TEXT("LXDT,SET,SC_VAR,0")); 
        Thermalmode = 1;
      }
      else
      {
        SendNmea(d, TEXT("LXDT,SET,SC_VAR,1"));
        Thermalmode = 0;
      }
    }
  }

  if ((info == nullptr) || !NMEAParser::NMEAChecksum(sentence)) {
    return FALSE;
  }
  if (strncmp("$LXDT", sentence, 5) == 0) {
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
  if (strncmp("$GPRMB", sentence, 6) == 0) {
    return GPRMB(d, sentence + 7, info);
  }
  if (strncmp("$LXWP1", sentence, 6) == 0) {
    return LXWP1(d, sentence + 7, info);
  }
  return false;
} // ParseNMEA()


BOOL DevLX_EOS_ERA::SetupLX_Sentence(DeviceDescriptor_t* d)
{
#ifdef TIMEOUTCHECK
  static int i=0;
 if (Declare()) return false ;  // do not configure during declaration
  if(IsEOSInBinaryMode()) return false;
  if((i++%2)==0)
  {
    SendNmea(d, TEXT("PFLX0,LXWP0,1,LXWP1,5,LXWP2,1,LXWP3,1,GPRMB,5"));

    SendNmea( d, _T("LXDT,SET,BC_INT,AHRS,0.5,SENS,2.0"));      
  }
  else
    if(!LX_EOS_ERA_bValid)
    {
     SendNmea(d,_T("LXDT,GET,MC_BAL"));
     TestLog(TEXT("Config: LXDT"));
    }
#endif

  return true;
}




BOOL DevLX_EOS_ERA::ShowData(WndForm* wf , DeviceDescriptor_t* d)
{
  WndProperty *wp;
  if(!wf) return false;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

//  int PortNum = d->PortNumber;

  wp = (WndProperty*)wf->FindByName(TEXT("prpQNHDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.QNHDir);
    wp->RefreshDisplay();
  }
  
  wp = (WndProperty*)wf->FindByName(TEXT("prpMCDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.MCDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBUGDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.BUGDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBALDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.BALDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSTFDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.STFDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWINDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->Set((uint) PortIO.WINDDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBARODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.BARODir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpSPEEDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.SPEEDDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVARIODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"

    dfe->Set((uint) PortIO.VARIODir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpR_TRGTDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(_T("$xxDT,GET,NAVIGATE")); // "IN" 
    dfe->addEnumText(_T("$GPRMB")); //  "OUT" = $GPRMB
    dfe->Set((uint) PortIO.R_TRGTDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpT_TRGTDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(_T("$xxDT,SET,NAVIGATE")); // "IN"
    dfe->addEnumText(_T("$GPRMB")); //  "OUT" = $GPRMB
    dfe->Set((uint) PortIO.T_TRGTDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpGFORCEDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.GFORCEDir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpOATDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.OATDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBAT1Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.BAT1Dir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBAT2Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.BAT2Dir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPOLARDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.POLARDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpDirectLink"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<894>()); // LKTOKEN  _@M894_": "ON"
    dfe->Set((uint) PortIO.DirLink);
    wp->RefreshDisplay();
  }
  

  return true;
}


bool DevLX_EOS_ERA::OnTimer(WndForm* pWnd)
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

BOOL DevLX_EOS_ERA::Config(DeviceDescriptor_t* d){

  CallBackTableEntry_t CallBackTable[] = {
    EndCallBackEntry()
  };

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
    Device(nullptr);

    delete wf;
  }
  return TRUE;
}


int DeviceASCIIConvert(TCHAR *pDest, const TCHAR *pSrc, int size=11)
{
  if (pSrc && pDest) {
    char szTmp[size+10];
    to_usascii(pSrc , szTmp, size);
    from_utf8(szTmp, pDest, size);
    return _tcslen(pDest);
  }
  return 0;
}

static
BOOL FormatTP(TCHAR* DeclStrings, int num, int total,const WAYPOINT *wp)
{
  if(DeclStrings) {

    int  lat =0; 
    int  lon =0; 
    TCHAR Name[60] =_T("");
    if(wp)
    {
      lat = ( int)(wp->Latitude*60000.0);
      lon = (int) (wp->Longitude*60000.0);  
      DeviceASCIIConvert(Name, wp->Name,20) ;  
    }

    _stprintf(DeclStrings, TEXT("LXDT,SET,TP,%i,%i,%i,%i,%s"),
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
BOOL DevLX_EOS_ERA::DeclareTask(DeviceDescriptor_t* d, const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]) {

  bool Good  = true;

  if (!CheckWPCount(*lkDecl,Decl::min_wp_count - 2, Decl::max_wp_count - 2, errBufSize, errBuf)){
    return(false);
  }

  ShowProgress(decl_enable);
  ShowProgress(decl_send);
  Declare(true);

  TCHAR Pilot[64];
  _tcscpy(Pilot , lkDecl->PilotName); //copy to local instance (Multi driver support)

  TCHAR PilotName[12];
  TCHAR PilotSurName[12];
  TCHAR* NamePtr = _tcstok (Pilot, _T(" ,.-:_"));
  DeviceASCIIConvert(PilotName, NamePtr, 11);

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


  _stprintf(DeclStrings[i++], TEXT("LXDT,SET,PILOT,%s,%s"),(PilotName), (PilotSurName));
  _stprintf(DeclStrings[i++], TEXT("LXDT,SET,GLIDER,%s,%s,%s,%s"), AircraftType, AircraftReg, AircraftCompID, AircraftClass);

  if(UseAATTarget())
    _stprintf(DeclStrings[i++], TEXT("LXDT,SET,TSK_PAR,0,0,%02i:%02i"), (int)AATTaskLength/60,  (int)(AATTaskLength-((int)(AATTaskLength/60)*60)));
  else
    _stprintf(DeclStrings[i++], TEXT("LXDT,SET,TSK_PAR,0,0,00:00"));
  int num=0;

  int dir=0,autonxt=1,isline=0,a1=45,a2=45,a21=5000,r1=5000,r2=500, elev = WayPointList[HomeWaypoint].Altitude;

  WAYPOINT* pTakeOff = NULL;
  if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint) && DeclTakeoffLanding)
  {
    pTakeOff = &WayPointList[HomeWaypoint];
  }
  FormatTP(DeclStrings[i++], num++ , wpCount, pTakeOff);   // Takeoff

  

  for (int ii = 0; ii < wpCount; ii++)
  {
    FormatTP(DeclStrings[i++], num, wpCount, lkDecl->waypoint[ii]);   //  Task waypoints

    sector_param param = GetTaskSectorParameter(ii);
    switch(param.type) {
      case sector_type_t::LINE  : isline=1; r1= param.radius; a1 = 0  ; a2=180; r2=0  ; a21 =0; break;
      case sector_type_t::SECTOR: isline=0; r1= param.radius; a1 = 45 ; a2=180; r2=0  ; a21 =0; break;
      case sector_type_t::CIRCLE: isline=0; r1= param.radius; a1 =180 ; a2=180; r2=0  ; a21 =0; break;
      case sector_type_t::DAe   : isline=0; r1= param.radius; a1 = 45 ; a2=180; r2=500; a21 =0; break;
      default:
        assert(false);
        break;
    }

    elev = WayPointList[HomeWaypoint].Altitude;
    _stprintf(DeclStrings[i++], TEXT("LXDT,SET,ZONE,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i"),num,dir,autonxt,isline,a1,a2,a21,r1,r2, elev);
    num++;
  }

  FormatTP(DeclStrings[i++], num++ , wpCount, pTakeOff);   // Landing

  bool status = false;
  {
    ScopeUnlock unlock(CritSec_Comm); // required to avoid deadlock In StopRxThread
    status = StopRxThread(d, errBufSize, errBuf);
  }

  if (status) {
    // Send complete declaration to logger
    int orgRxTimeout;
    TestLog(_T(". EOS/ERA SetRxTimeout"));
    status = SetRxTimeout(d, 2000, orgRxTimeout, errBufSize, errBuf);
    int attemps = 0;
    char RecBuf[4096] = "";

    do {
      Good = true;
      for (int ii = 0; ii < i; ii++) {
        TestLog(_T(". EOS/ERA Decl: %s"), DeclStrings[ii]);
        if (Good)
          Good = SendNmea(d, DeclStrings[ii]);


        if (Good)
          Good = ComExpect(d, "$LXDT,ANS,OK*5", 4095, RecBuf, errBufSize, errBuf);

      }
      attemps++;
      if (!Good)
        Sleep(500);
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


void DevLX_EOS_ERA::GetDirections(WndButton* pWnd){
  unsigned PortNum = Port();
  if (PortNum >= std::size(PortConfig)) {
    return;
  }
  auto& PortIO = PortConfig[PortNum].PortIO;

  if(pWnd) {
    WndForm * wf = pWnd->GetParentWndForm();
    if(wf) {

      WndProperty *wp;
      
      wp = (WndProperty*)wf->FindByName(TEXT("prpQNHDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.QNHDir = (DataBiIoDir) dfe->GetAsInteger();
      }     
      wp = (WndProperty*)wf->FindByName(TEXT("prpMCDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.MCDir = (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBUGDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BUGDir =  (DataBiIoDir) dfe->GetAsInteger();
      }

      wp = (WndProperty*)wf->FindByName(TEXT("prpBALDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BALDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpSTFDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.STFDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpWINDDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.WINDDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBARODir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BARODir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpVARIODir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.VARIODir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpSPEEDDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.SPEEDDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpR_TRGTDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.R_TRGTDir =  (DataTP_Type) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpGFORCEDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.GFORCEDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpOATDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.OATDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBAT1Dir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BAT1Dir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpBAT2Dir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BAT2Dir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpPOLARDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.POLARDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpDirectLink"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.DirLink =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = (WndProperty*)wf->FindByName(TEXT("prpT_TRGTDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.T_TRGTDir =  (DataTP_Type) dfe->GetAsInteger();
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
        wBt->SetCaption(MsgToken<2468>());//  _@M2468_ "Direction"
      } else {
        wf->SetTimerNotify(0, NULL);    // turn Off the timer
        wBt->SetCaption(MsgToken<2467>());// _@M2467_ "Values"
        if (wf) ShowData(wf, Device());
      }
    }
    devSetAdvancedMode(m_pDevice,false);
    ClearDataText( _QNH   );
    ClearDataText( _MC    );
    ClearDataText( _BUGS  );
    ClearDataText( _BAL   );
    ClearDataText( _STF   );
    ClearDataText( _WIND  );
    ClearDataText( _BARO  );
    ClearDataText( _VARIO );
    ClearDataText( _SPEED );
    ClearDataText( _R_TRGT);
    ClearDataText( _GFORCE);
    ClearDataText( _OAT   );
    ClearDataText( _BAT1  );
    ClearDataText( _BAT2  );
    ClearDataText( _POLAR );
    ClearDataText( _DIRECT);
    ClearDataText( _T_TRGT);
    SendNmea(Device(),_T("LXDT,GET,MC_BAL"));  // request new data
  }

}



void DevLX_EOS_ERA::OnIGCDownloadClicked(WndButton* pWnd) {
  (void)pWnd;
  LockFlightData();
  bool bInFlight    = CALCULATED_INFO.Flying;
  UnlockFlightData();

  if(bInFlight) {
    MessageBoxX(MsgToken<2418>(), MsgToken<2397>(), mbOk);
    return;
  }
  m_bTriggered = true;
  SendNmea(Device(), _T("LXDT,SET,BC_INT,AHRS,0.0,SENS,0.0"));    
  Sleep(50);
  SendNmea(Device(), _T("LXDT,GET,FLIGHTS_NO"));
  Sleep(50);
  dlgEOSIGCSelectListShowModal();
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
BOOL DevLX_EOS_ERA::LXWP0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  // $LXWP0,logger_stored, airspeed, airaltitude,
  //   v1[0],v1[1],v1[2],v1[3],v1[4],v1[5], hdg, windspeed*CS<CR><LF>
  //
  // 0 loger_stored : [Y|N] (not used in LX1600)
  // 1 TAS [km/h]  TAS!
  // 2 true Altitude [m] (already QNH corrected!!!)
  // 3-8 vario values [m/s] (last 6 measurements in last second)
  // 9 heading of plane (not used in LX1600)
  // 10 windcourse [deg] (not used in LX1600)
  // 11 windspeed [km/h] (not used in LX1600)
  //
  // e.g.:
  // $LXWP0,Y,222.3,1665.5,1.71,,,,,,239,174,10.1

  double fDir,fTmp,airspeed=0;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

 // if( !devGetAdvancedMode(d))
  {
    if( ParToDouble(sentence, 1, &fTmp))
    {

      if(Values(d)) 
      { 
        TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp, MAX_NMEA_LEN,_T("%5.1fkm/h ($LXWP0)"),fTmp);
        SetDataText( d,_SPEED,   szTmp);
      }
      if(IsDirInput(PortIO.SPEEDDir  ))
      {
        airspeed = Units::ToSys(unKiloMeterPerHour, fTmp);
        info->TrueAirspeed = airspeed;
        info->AirspeedAvailable = TRUE;
      }
    }

    if (ParToDouble(sentence, 2, &fTmp))
    {
      if(Values(d))
      { TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp, MAX_NMEA_LEN, _T("%5.1fm ($LXWP0)"),fTmp);
        SetDataText( d, _BARO,   szTmp);
      }
      if(IsDirInput(PortIO.BARODir  ))
      {
        if (airspeed > 0) {
          info->IndicatedAirspeed = IndicatedAirSpeed(airspeed, fTmp);
        }
        UpdateBaroSource( info, d, fTmp);
      }
    }

    if (ParToDouble(sentence, 3, &fTmp))
    {
      if(Values(d))
      { TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.1fm/s ($LXWP0)"), fTmp);
        SetDataText( d, _VARIO,   szTmp);
      }
      if(IsDirInput(PortIO.VARIODir  ))
      {
        UpdateVarioSource(*info, *d, fTmp);
      }
    }
  }

  if (ParToDouble(sentence, 10, &fDir) && ParToDouble(sentence, 11, &fTmp))
  {

    if(Values(d))
    { 
      TCHAR szTmp[MAX_NMEA_LEN];
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.1fkm/h %3.0f° ($LXWP0)"),fTmp,fDir);
      SetDataText( d, _WIND,   szTmp);
    }
    if(IsDirInput(PortIO.WINDDir  ))
    {
      info->ExternalWindDirection = fDir;
      info->ExternalWindSpeed =  Units::ToSys(unKiloMeterPerHour, fTmp);
      info->ExternalWindAvailable = TRUE;
    }
  }

  return(false);
} // LXWP0()




BOOL DevLX_EOS_ERA::EOSSetMC(DeviceDescriptor_t* d,float fTmp, const TCHAR *info )
{
  bool ret = false;

  int iTmp =(int) (fTmp*100.0+0.5f);
  fTmp = (double)(iTmp)/100.0;
  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.2fm/s %s"),fTmp,info);
    SetDataText( d, _MC,   szTmp);
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if (IsDirInput(PortIO.MCDir)) {
    d->RecvMacCready(fTmp);
  }

  return ret;
}



BOOL DevLX_EOS_ERA::EOSSetSTF(DeviceDescriptor_t* d,int  iTmp, const TCHAR *info)
{
  bool ret = false;


  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    if(iTmp > 0)
      _sntprintf(szTmp,MAX_NMEA_LEN,  _T("STF %s"), info);
    else
      _sntprintf(szTmp,MAX_NMEA_LEN,  _T("VARIO %s"), info);      
    SetDataText( d,_STF,  szTmp);
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(IsDirInput(PortIO.STFDir  ))
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

BOOL DevLX_EOS_ERA::EOSSetBAL(DeviceDescriptor_t* d,float fTmp, const TCHAR *info)
{
  bool ret = false;

  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    if(WEIGHTS[2] > 0)
    {
      _sntprintf(szTmp,MAX_NMEA_LEN,  _T("%3.0f L = %3.0f%% %s"),fTmp,(fTmp/WEIGHTS[2]*100.0), info);
      SetDataText( d,_BAL,  szTmp);
    }
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;
 
  if(IsDirInput(PortIO.BALDir  ))
  {
    if((fabs(fTmp- GlidePolar::BallastLitres) > 1 )  &&   (WEIGHTS[2] > 0))
    {
      GlidePolar::BallastLitres = fTmp;
      d->RecvBallast(GlidePolar::BallastLitres / WEIGHTS[2]);
      ret = true;
    }
  }

  return ret;
}

BOOL DevLX_EOS_ERA::EOSSetBUGS(DeviceDescriptor_t* d,float fTmp, const TCHAR *info)
{
  bool ret = false;

  if(Values(d))
  {
    TCHAR szTmp[MAX_NMEA_LEN];
    _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.0f%% %s"),fTmp, info);
    SetDataText( d,_BUGS,  szTmp);
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if (IsDirInput(PortIO.BUGDir)) {
    d->RecvBugs(CalculateBugsFromLX(fTmp));
    ret = true;
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
BOOL DevLX_EOS_ERA::LXWP2(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO*)
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
  if(ParToDouble(sentence, 3, &fa)) {
    if(ParToDouble(sentence, 4, &fb)) {
      if(ParToDouble(sentence, 5, &fc)) {
        TCHAR szTmp[MAX_NMEA_LEN];
        if(Values(d))
        {
            _sntprintf(szTmp,MAX_NMEA_LEN, _T("a:%5.3f b:%5.3f c:%5.3f ($LXWP2)"),fa,fb,fc);
            SetDataText( d, _POLAR,  szTmp);
        }

        const auto& PortIO = PortConfig[d->PortNumber].PortIO;

        if(IsDirInput(PortIO.POLARDir ))
        {
          for (int i=0; i < 3; i++)
          {
            double v=POLARV[i]/100;
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
BOOL DevLX_EOS_ERA::LXWP3(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  double fTmp;
  if(ParToDouble(sentence, 1, &fTmp))  // SC mode
  {
    int  iTmp = (int)(fTmp+0.1);
    if(Values(d))
    {
      switch(iTmp)
      {
        case 0:  
          SetDataText( d,_STF, _T("MANUAL ($LXWP3)"));
          break;
        case 1:
          SetDataText( d,_STF, _T("VARIO ($LXWP3)"));
          break;
        case 2:
          SetDataText( d,_STF, _T("SPEED ($LXWP3)"));
          break;
      }
    }

    const auto& PortIO = PortConfig[d->PortNumber].PortIO;

    static int  iOldVarioSwitch=0;
    if(IsDirInput(PortIO.STFDir))
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


BOOL DevLX_EOS_ERA::LXDT(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  char szTmp[MAX_NMEA_LEN];

  double fTmp=0.0;

  devSetAdvancedMode(d,true);
  static int iNoFlights=0;
  NMEAParser::ExtractParameter(sentence, szTmp, 0);
  if(strncmp(szTmp, "ANS", 3) != 0)  // really an Answer?
    return 0;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  NMEAParser::ExtractParameter(sentence, szTmp, 1);  // Command?
  if(strncmp(szTmp, "RADIO", 5) == 0)
  {
    d->IsRadio = true;
    if (!RadioPara.Enabled) {
      RadioPara.Enabled = true;
    }

    NMEAParser::ExtractParameter(sentence, szTmp, 2);  // Active frequency
    RadioPara.ActiveKhz = ExtractFrequency(szTmp);

    NMEAParser::ExtractParameter(sentence, szTmp, 3);  // Standby frequency
    RadioPara.PassiveKhz = ExtractFrequency(szTmp);

    if(ParToDouble(sentence, 4, &fTmp)) {
      RadioPara.Volume  = (int) fTmp;
    }
    if(ParToDouble(sentence, 5, &fTmp)) {
      RadioPara.Squelch = (int) fTmp;
    }
    if(ParToDouble(sentence, 6, &fTmp)) {
      RadioPara.Vox     = (int) fTmp;
    }
  }
  else if(strncmp(szTmp, "MC_BAL", 6) == 0)
  {
    if(ParToDouble(sentence, 2, &fTmp)) {EOSSetMC(d, fTmp,_T("($LXDT)") );}
    if(ParToDouble(sentence, 3, &fTmp)) {EOSSetBAL(d, fTmp,_T("($LXDT)") );}
    if(ParToDouble(sentence, 4, &fTmp)) {EOSSetBUGS(d, fTmp,_T("($LXDT)") );}
    if(ParToDouble(sentence, 5, &fTmp)) {}  // Screen brightness in percent
    if(ParToDouble(sentence, 6, &fTmp)) {}  // Variometer volume in percent
    if(ParToDouble(sentence, 7, &fTmp)) {}  // SC volume in percent
    if(ParToDouble(sentence, 8, &fTmp))   // QNH in hPa (NEW)
    {
      if(IsDirInput(PortIO.QNHDir))
      { 
        TCHAR szQNH[MAX_NMEA_LEN];
        _stprintf( szQNH, _T("%6.1fhPa ($LXDT)"),fTmp);
        if(Values(d)) SetDataText( d, _QNH,   szQNH);
        static double oldQNH = -1;
        if ( fabs( oldQNH - fTmp) > 0.1)
        {
          UpdateQNH( fTmp);
          oldQNH = fTmp;
        }
      }  
    }
    LX_EOS_ERA_bValid = true;    
  }
  else if(strncmp(szTmp, "FLIGHTS_NO", 10) == 0)
  {
    if(ParToDouble(sentence, 2, &fTmp)) iNoFlights =(int) (fTmp+0.05);
    if((iNoFlights > 0)
      && m_bTriggered)  // call next if triggerd from here only
    {
      EOSListFilled(false);
      TCHAR szNmea[MAX_NMEA_LEN];
      _sntprintf(szNmea, MAX_NMEA_LEN, _T("LXDT,GET,FLIGHT_INFO,%i"),1);
      SendNmea(d, szNmea);
    }
  }
  else if(strncmp(szTmp, "FLIGHT_INFO", 11) == 0)
  { 
    char FileName[50]= "FileName", Pilot[50]= "",Surname[50]= "", Takeoff[50]= "",Date[50]= "",Landing[50]= "",Type[50]= "", Reg[50]= "";
    TestLog(TEXT("FLIGHT_INFO %s"), sentence);

    NMEAParser::ExtractParameter(sentence, Date     ,2);int  iNo = (int) StrToDouble(Date,nullptr);
    NMEAParser::ExtractParameter(sentence, FileName ,3);
    NMEAParser::ExtractParameter(sentence, Date     ,4);
    NMEAParser::ExtractParameter(sentence, Takeoff  ,5);
    NMEAParser::ExtractParameter(sentence, Landing  ,6);
    NMEAParser::ExtractParameter(sentence, Pilot    ,7);
    NMEAParser::ExtractParameter(sentence, Surname  ,8);
    NMEAParser::ExtractParameter(sentence, Type     ,9);
    NMEAParser::ExtractParameter(sentence, Reg      ,10);
    uint32_t filesize  = 0;
    if (ParToDouble(sentence, 15, &fTmp))
       filesize = (uint32_t)fTmp;
    
    TCHAR Line[2][MAX_NMEA_LEN];
    lk::snprintf(Line[0], _T("%s %s %s  %s %s"),FileName, Pilot,Surname, Reg, Type);
    lk::snprintf(Line[1], _T("%s (%s-%s) %ukB"), Date ,Takeoff ,Landing,filesize/1024);
    AddEOSElement(Line[0], Line[1], filesize );
    if((iNo < iNoFlights) && m_bTriggered)
    {
      TCHAR szNmea[MAX_NMEA_LEN];
      _sntprintf(szNmea, MAX_NMEA_LEN, _T("LXDT,GET,FLIGHT_INFO,%i"),iNo+1);
      SendNmea(d, szNmea);
    }
    else
    {
      EOSListFilled(true);
      m_bTriggered = false;
    }
  }
  else if(strncmp(szTmp, "SENS", 4) == 0)  // Sensor Data?
  {
    SENS(d, sentence,  info, 2);
  }
  else if(strncmp(szTmp, "SC_VAR", 6) == 0)  // Vario / STF
  {
    if(ParToDouble(sentence, 2, &fTmp)) {
      EOSSetSTF(d, (int)fTmp,_T(" ($LXDT,SC_VAR)") );
    }
  }
  else if(strncmp(szTmp, "NAVIGATE", 7) == 0)  // Navigation Target
  {
    GetTarget( d, sentence,  info);    
  }

  if(strncmp(szTmp, "ERROR", 5) == 0)  // ERROR?
  {
    NMEAParser::ExtractParameter(sentence, szTmp, 2);
    if(strncmp(szTmp, "Radio not enabled", 17) == 0)  
    {
      d->IsRadio = false;
    }
    else
    {
      tstring tsError = from_unknown_charset(szTmp);
      DoStatusMessage(TEXT("LX EOS/ERA Error:"), tsError.c_str(), false);
      StartupStore(TEXT("LX EOS/ERA Error: %s"), tsError.c_str());
    }
  }
  else if(strncmp(szTmp, "OK", 2) == 0)
  {
    if(!Declare()) {
      SendNmea(d,_T("LXDT,GET,MC_BAL"));
    }
  }
  return(true);
} // LXDT()




BOOL DevLX_EOS_ERA::SENS(DeviceDescriptor_t* d,  const char* sentence, NMEA_INFO* info, int ParNo)
{ 
  TCHAR szTmp[MAX_NMEA_LEN];
  double fTmp;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(ParToDouble(sentence, ParNo++, &fTmp)) { // Outside air temperature in °C. Left empty if OAT value not valid
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%4.2f°C ($LXDT)"),fTmp);
    if(Values(d)) {
      SetDataText( d,_OAT,  szTmp);
    }
    if(IsDirInput(PortIO.OATDir)) {
      info->OutsideAirTemperature = fTmp;
      info->TemperatureAvailable  = TRUE;
    }
  }
  if(ParToDouble(sentence, ParNo++, &fTmp)) { // main power supply voltage
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%4.2fV ($LXDT)"),fTmp);
    if(Values(d)) {
      SetDataText( d,_BAT1,  szTmp);
    }
    if(IsDirInput(PortIO.BAT1Dir)) {
      info->ExtBatt1_Voltage = fTmp;	
    }
  }
  if(ParToDouble(sentence, ParNo++, &fTmp)) { // Backup battery voltage
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%4.2fV ($LXDT)"),fTmp);
    if(Values(d)) {
      SetDataText( d,_BAT2,  szTmp);
    }
    if(IsDirInput(PortIO.BAT2Dir)) {
      info->ExtBatt2_Voltage = fTmp;	
    }
  }
/*  
  NMEAParser::ExtractParameter(sentence, szTmp, ParNo++); 
  {  // Current flap setting
  }
  NMEAParser::ExtractParameter(sentence, szTmp, ParNo++);
  { // Recommended flap setting
  }
*/  
  if(ParToDouble(sentence, ParNo++, &fTmp)) {  // Current landing gear position (0 = out, 1 = inside, left empty if gear input not configured)
  }
  if(ParToDouble(sentence, ParNo++, &fTmp)) {  // SC/Vario mode (0 = Vario, 1 = SC)
    EOSSetSTF(d, (int)fTmp,_T(" ($LXDT,SENS)"));
  }
  return true;
}

BOOL DevLX_EOS_ERA::LXBC(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  TCHAR szTmp[MAX_NMEA_LEN];

  devSetAdvancedMode(d,true);
  if(strncmp(sentence, "SENS", 4) == 0) {  
    SENS(d,  sentence,  info,1);
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(strncmp(sentence, "AHRS", 4) == 0)
  {
    if(IsDirInput(PortIO.GFORCEDir)) { 
      double fX,fY,fZ, fPitch, fRoll, fYaw, fSlip;
      
      bool bAHRS = ParToDouble(sentence, 1, &fPitch); // pitch
      bAHRS = bAHRS && ParToDouble(sentence, 2, &fRoll); // Roll
      bAHRS = bAHRS && ParToDouble(sentence, 3, &fYaw); // Yaw
      bAHRS = bAHRS && ParToDouble(sentence, 4, &fSlip); // slip

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
          SetDataText( d, _GFORCE,  szTmp);
        }
        if(IsDirInput(PortIO.GFORCEDir))
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


BOOL DevLX_EOS_ERA::EOSPutMacCready(DeviceDescriptor_t* d, double MacCready){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(!IsDirOutput(PortIO.MCDir)) return false;

  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("LXDT,SET,MC_BAL,%.1f,,,,,,"), MacCready);
  TestLog(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);

  return true;
}


BOOL DevLX_EOS_ERA::EOSPutBallast(DeviceDescriptor_t* d, double Ballast){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;
  if(LX_EOS_ERA_bValid == false) return false;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(!IsDirOutput(PortIO.BALDir)) return false;
  
  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("LXDT,SET,MC_BAL,,%.0f,,,,,"),GlidePolar::BallastLitres);
  TestLog(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);
  
  return(TRUE);
}


BOOL DevLX_EOS_ERA::EOSPutBugs(DeviceDescriptor_t* d, double Bugs){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if(!IsDirOutput(PortIO.BUGDir)) return false;
  double fLXBugs = CalculateLXBugs( Bugs);

  _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("LXDT,SET,MC_BAL,,,%.0f,,,,"),fLXBugs);
  TestLog(TEXT("Send: %s"), szTmp);
  SendNmea(d,szTmp);

  return(TRUE);
}

BOOL DevLX_EOS_ERA::PutQNH(DeviceDescriptor_t* d, double qnh_mb) {
  if (!d) {
    return false;
  }

  const auto& PortIO = PortConfig[d->PortNumber].PortIO;
  if (!IsDirOutput(PortIO.QNHDir)) {
    return false;
  } 

  TCHAR szTmp[MAX_NMEA_LEN];
  _stprintf(szTmp,  TEXT("LXDT,SET,MC_BAL,,,,,,,%.0f"), qnh_mb);
  TestLog(TEXT("Send: %s"), szTmp);
  SendNmea(d, szTmp);

  return TRUE;
}

BOOL DevLX_EOS_ERA::PutTarget(DeviceDescriptor_t* d, const WAYPOINT& wpt)
{
  const auto& PortIO = PortConfig[d->PortNumber].PortIO;
  if(PortIO.T_TRGTDir == TP_Off) {
    return false;
  }

  TCHAR  szTmp[MAX_NMEA_LEN];

  if( PortIO.T_TRGTDir  == TP_VTARG) {                                  
    int rwdir = 0; 
    int landable =0;

    if((wpt.Flags & LANDPOINT)> 0) 
    {
      landable = 1;
      rwdir    = wpt.RunwayDir;
    }

    _sntprintf( szTmp,MAX_NMEA_LEN, TEXT("LXDT,SET,NAVIGATE,%i,%s,%i,%i,%i,%i,%s,%i"),
            1, 
            wpt.Name, 
            (int) (wpt.Latitude * 60000.0), 
            (int) (wpt.Longitude * 60000.0),
            (int) (wpt.Altitude + 0.5),
            landable,
            wpt.Freq,
            rwdir);

    DevLX_EOS_ERA::SendNmea(d, szTmp);
 //   $LXDT,SET,NAVIGATE,0,MARIBOR,2788794,941165,267,1,119.200,14*2A<CR><LF>
    TestLog(_T("Send navigation Target LXNav: ($LXDT,SET,NAVIGATE) %s"), wpt.Name);
  }
  else if( PortIO.T_TRGTDir  == TP_GPRMB) {
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

    int DegLon = wpt.Longitude;
    double MinLon = wpt.Longitude - DegLon;
    char EoW = 'E';
    if ((MinLon < 0) || (((MinLon - DegLon) == 0) && (DegLon < 0))) {
      EoW = 'W';
      DegLon *= -1;
      MinLon *= -1;
    }
    MinLon *=60;    

    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("GPRMB,A,,,%s,%02d%05.2f,%c,%03d%05.2f,%c,,,,A"),
                      wpt.Name, DegLat, MinLat, NoS, DegLon, MinLon, EoW);

    DevLX_EOS_ERA::SendNmea(d, szTmp);
    TestLog(_T("Send navigation Target LXNav: ($GPRMB) %s"), wpt.Name);
  }

  if(Values(d)) {
    SetDataText( d, _T_TRGT, wpt.Name);
  }

  return(true);
}




BOOL DevLX_EOS_ERA::GetTarget(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info) {
  const auto& PortIO = PortConfig[d->PortNumber].PortIO;

  if (PortIO.R_TRGTDir != TP_VTARG) {    return false;
  }

  double fLat, fLon, fAlt, fFlags;

  ParToDouble(sentence, 4, &fLat);   // latitude
  ParToDouble(sentence, 5, &fLon);   // longitude
  if (!ParToDouble(sentence, 6, &fAlt)) {   // altitude (elevation)
    fAlt = RESWP_INVALIDNUMBER;
  }
//ParToDouble(sentence, 7, &fTmp);   // distance (not needed)
//ParToDouble(sentence, 8, &fTmp);   // bearing  (not needed)
  ParToDouble(sentence, 9, &fFlags); // landable?
		
  char szTmp[MAX_NMEA_LEN];		
  NMEAParser::ExtractParameter(sentence,szTmp,3);
    // detect and fix charset
  tstring tname = from_unknown_charset(szTmp);

  LockTaskData();
  {
    if (Alternate2 == RESWP_EXT_TARGET) { // pointing to external target?
      Alternate2 = -1;                 // clear external =re-enable!
    }
  	lk::snprintf(WayPointList[RESWP_EXT_TARGET].Name, TEXT("^%s"), tname.c_str());
    WayPointList[RESWP_EXT_TARGET].Latitude  = fLat / 60000;
    WayPointList[RESWP_EXT_TARGET].Longitude = fLon / 60000;
    WayPointList[RESWP_EXT_TARGET].Altitude  = fAlt;

    if (fFlags > 0)
        WayPointList[RESWP_EXT_TARGET].Flags = LANDPOINT;
    else
        WayPointList[RESWP_EXT_TARGET].Flags = 0;

    Alternate2 = RESWP_EXT_TARGET;
  }
  UnlockTaskData();

  if(Values(d))
  {
    tname += _T(" ($LXDT)");
    SetDataText(d, _R_TRGT,  tname.c_str());
  }
  return false;
}



BOOL DevLX_EOS_ERA::EOSRequestRadioInfo(DeviceDescriptor_t* d)
{
  if (d && d->IsRadio) {
    Poco::Thread::sleep(50);
    SendNmea(d, _T("LXDT,GET,RADIO"));
    return TRUE;
  } 
  return FALSE;
}

BOOL DevLX_EOS_ERA::EOSPutVolume(DeviceDescriptor_t* d, int Volume) {
  if (d && d->IsRadio) {
    TCHAR  szTmp[255];
    _stprintf(szTmp,_T("LXDT,SET,RADIO,,,%i,,,"),Volume)  ;
    SendNmea(d,szTmp);
    if(uiEOSDebugLevel) {
      StartupStore(_T(". EOS Volume  %i"), Volume);
    }
    RadioPara.Volume = Volume;
    EOSRequestRadioInfo(d);
    return TRUE;
  }
  return FALSE;
}




BOOL DevLX_EOS_ERA::EOSPutSquelch(DeviceDescriptor_t* d, int Squelch) {
  if (d && d->IsRadio) {
    TCHAR  szTmp[255];
    _stprintf(szTmp,_T("LXDT,SET,RADIO,,,,%i,,"),Squelch)  ;
    SendNmea(d,szTmp);
    if(uiEOSDebugLevel) {
      StartupStore(_T(". EOS Squelch  %i"), Squelch);
    }
    RadioPara.Squelch = Squelch;
    EOSRequestRadioInfo(d);
    return TRUE;
  }
  return FALSE;
}



BOOL DevLX_EOS_ERA::EOSPutFreqActive(DeviceDescriptor_t* d, unsigned khz, const TCHAR* StationName) {
  if (d && d->IsRadio) {
    TCHAR  szTmp[255];
    _stprintf(szTmp,_T("LXDT,SET,RADIO,%7.3f,,,,,"), khz / 1000.);
    SendNmea(d,szTmp);
    EOSRequestRadioInfo(d);
    RadioPara.ActiveKhz = khz;
    if(StationName) {
      _sntprintf(RadioPara.ActiveName, NAME_SIZE,_T("%s"),StationName) ;
    }
    if(uiEOSDebugLevel) {
      StartupStore(_T(". EOS Active Station %7.3fMHz %s"), khz / 1000., StationName);
    } 
    return TRUE;
  }
  return FALSE;
}


BOOL DevLX_EOS_ERA::EOSPutFreqStandby(DeviceDescriptor_t* d, unsigned khz,  const TCHAR* StationName) {
  if (d && d->IsRadio) {
    TCHAR  szTmp[255];
    _stprintf(szTmp,_T("LXDT,SET,RADIO,,%7.3f,,,,"), khz / 1000.);
    SendNmea(d,szTmp);
    EOSRequestRadioInfo(d);
    RadioPara.PassiveKhz = khz;
    if(StationName) {
      _sntprintf(RadioPara.PassiveName , NAME_SIZE ,_T("%s"),StationName) ;
    }
    if(uiEOSDebugLevel) {
      StartupStore(_T(". EOS Standby Station %7.3fMHz %s"), khz / 1000., StationName);
    }
    return TRUE;
  }
  return FALSE;
}


BOOL DevLX_EOS_ERA::EOSStationSwap(DeviceDescriptor_t* d) {
  if (d && d->IsRadio) {
    SendNmea(d,_T("LXDT,SET,R_SWITCH"));
    EOSRequestRadioInfo(d);
    if(uiEOSDebugLevel) {
      StartupStore(_T(". EOS  station swap"));
    }
    return TRUE;
  }
  return FALSE;
}


BOOL DevLX_EOS_ERA::EOSRadioMode(DeviceDescriptor_t* d, int mode) {
  if (d && d->IsRadio) {
    TCHAR  szTmp[255];
    _stprintf(szTmp,_T("LXDT,SET,R_DUAL,%i"),mode);
    SendNmea(d, szTmp);
    EOSRequestRadioInfo(d);
    if(uiEOSDebugLevel) {
      StartupStore(_T(". EOS  Dual Mode: %i"), mode);
    }
    return TRUE;
  }
  return FALSE;
}
