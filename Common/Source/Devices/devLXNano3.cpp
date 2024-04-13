/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

//__Version_1.0____________________________________________Vladimir Fux 12/2015_


//_____________________________________________________________________includes_


#include <time.h>
#include "externs.h"
#include "utils/stringext.h"
#include "utils/charset_helper.h"
#include "devLXNano3.h"
#include "Calc/Vario.h"
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
#include "utils/printf.h"
#include "Comm/UpdateQNH.h"


#define NANO_PROGRESS_DLG
#define BLOCK_SIZE 32

DeviceDescriptor_t* DevLXNanoIII::m_pDevice=NULL;
BOOL DevLXNanoIII::m_bShowValues = false;
BOOL DevLXNanoIII::bIGC_Download = false;
BOOL DevLXNanoIII::m_bDeclare = false;
static FILE *f= NULL;
uint uTimeout =0;
//______________________________________________________________________defines_

/// polynom for LX data CRC
#define LX_CRC_POLY 0x69
#define QNH_OR_ELEVATION
TCHAR m_Filename[19];
uint m_CurLine =0;


#define MAX_VAL_STR_LEN    60

int iNano3_RxUpdateTime=0;
double Nano3_oldMC = MACCREADY;

int iS_SeriesTimeout =0;
int iNano3_GPSBaudrate = 0;
int iNano3_PDABaudrate = 0;
//double fPolar_a=0.0, fPolar_b=0.0, fPolar_c=0.0, fVolume=0.0;
BOOL Nano3_bValid = false;
int Nano3_NMEAddCheckSumStrg( TCHAR szStrg[] );
BOOL Nano3_PutMacCready(DeviceDescriptor_t* d, double MacCready);
BOOL Nano3_PutBallast(DeviceDescriptor_t* d, double Ballast);
BOOL Nano3_PutBugs(DeviceDescriptor_t* d, double Bugs);



Mutex  CritSec_LXDebugStr;
TCHAR LxValueStr[_LAST][ MAX_VAL_STR_LEN];

//____________________________________________________________class_definitions_

// #############################################################################
// *****************************************************************************
//
//   DevLXNanoIII
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


BOOL  DevLXNanoIII::Values(DeviceDescriptor_t* d)
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
void DevLXNanoIII::Install(DeviceDescriptor_t* d) {
  _tcscpy(d->Name, GetName());
  d->Open         = Open;
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = Nano3_PutMacCready;
  d->PutBugs      = Nano3_PutBugs;
  d->PutBallast   = Nano3_PutBallast;
  d->Declare      = DeclareTask;

  d->Config       = Config;
  d->DirectLink   = Nano3_DirectLink;
  d->PutTarget    = PutTarget;

  StartupStore(_T(". %s installed (platform=%s test=%u)%s"),
    GetName(),
    PlatfEndian::IsBE() ? _T("be") : _T("le"),
    PlatfEndian::To32BE(0x01000000), NEWLINE);
} // Install()

BOOL DevLXNanoIII::Open(DeviceDescriptor_t* d) {
  Nano3_PutMacCready(d, MACCREADY);
  Nano3_PutBallast(d, BALLAST);
  Nano3_PutBugs(d, BUGS);

  ResetMultitargetSync();
  return TRUE;
}

long  StrTol(const  TCHAR *buff) {
   return _tcstol(buff, nullptr, 10);
}


long Nano3Baudrate(int iIdx)
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




BOOL DevLXNanoIII::Nano3_DirectLink(DeviceDescriptor_t* d, BOOL bLinkEnable)
{
TCHAR  szTmp[MAX_NMEA_LEN];
#define CHANGE_DELAY 10

  const auto& Port = PortConfig[d->PortNumber];

  if(!IsDirInput(Port.PortIO.DirLink))
    return false;

  if(iNano3_GPSBaudrate ==0)
  {
    _tcscpy(szTmp, TEXT("PLXV0,BRGPS,R"));
    SendNmea(d,szTmp);
    Sleep(CHANGE_DELAY);
    SendNmea(d, szTmp);
    Sleep(CHANGE_DELAY);
  }


  if(bLinkEnable)
  {
    #if TESTBENCH
    StartupStore(TEXT("LAXNav: enable LX LAXNav direct Link %s"), NEWLINE);
    #endif
    iNano3_PDABaudrate = d->Com->GetBaudrate();

    _tcscpy(szTmp, TEXT("PLXV0,CONNECTION,W,DIRECT"));
    SendNmea(d, szTmp);
    Sleep(CHANGE_DELAY);
    if((iNano3_PDABaudrate > 0) && (iNano3_GPSBaudrate >0) && (iNano3_PDABaudrate != iNano3_GPSBaudrate))
    {
      d->Com->SetBaudrate(iNano3_GPSBaudrate);
    #if TESTBENCH
      StartupStore(TEXT("LAXNav: Set Baudrate %i %s"),iNano3_GPSBaudrate, NEWLINE);
    #endif
      Sleep(CHANGE_DELAY);
    }
    Sleep(CHANGE_DELAY);
  }
  else
  {
    Sleep(CHANGE_DELAY);

    if((iNano3_PDABaudrate > 0) && (iNano3_GPSBaudrate > 0) &&(iNano3_PDABaudrate != iNano3_GPSBaudrate))
    {
#if TESTBENCH
      StartupStore(TEXT("LAXNav: Set Baudrate %i %s"),iNano3_PDABaudrate, NEWLINE);
#endif
      d->Com->SetBaudrate(iNano3_PDABaudrate);
      Sleep(CHANGE_DELAY);
    }

    #if TESTBENCH
    StartupStore(TEXT("LAXNav: Return from LAXNav link %s"), NEWLINE);
    #endif
    _tcscpy(szTmp, TEXT("PLXV0,CONNECTION,W,VSEVEN"));
    SendNmea(d,szTmp);
    Sleep(CHANGE_DELAY);

  }

  return true;
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
// static
BOOL DevLXNanoIII::ParseNMEA(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info) {
  auto wait_ack = d->lock_wait_ack();
  if (wait_ack && wait_ack->check(sentence)) {
    return TRUE;
  }

  TCHAR szTmp[MAX_NMEA_LEN];
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if (strncmp("$LXWP2", sentence, 6) == 0) {
    Nano3_bValid = true;
    if (iNano3_RxUpdateTime > 0) {
      iNano3_RxUpdateTime--;
    } else {
      if (!IsDirOutput(PortIO.MCDir)) {
        if (fabs(Nano3_oldMC - MACCREADY) > 0.005f) {
          Nano3_PutMacCready(d, MACCREADY);
          Nano3_oldMC = MACCREADY;
        }
      }
    }
  }

  static char lastSec = 0;
  if (!Declare() && (info->Second != lastSec))  // execute every second only if no task is declaring
  {
    lastSec = info->Second;
    if ((info->Second % 10) == 0)  // config every 10s (not on every xx $GPGGA as there are 10Hz GPS now
    {
      SetupLX_Sentence(d);
    }
  }

  if (strncmp("$GPGGA", sentence, 6) == 0) {
    if (iS_SeriesTimeout-- < 0)
      devSetAdvancedMode(d, false);

#ifdef QNH_OR_ELEVATION
    static int iOldQNH = 0;
    int iQNH = (int)(QNH * 100.0);
    if (iQNH != iOldQNH) {
      iOldQNH = iQNH;
      _sntprintf(szTmp, MAX_NMEA_LEN, TEXT("PLXV0,QNH,W,%i"), (int)iQNH);
      SendNmea(d, szTmp);
    }
#else
    static int oldQFEOff = 0;
    int QFE = (int)QFEAltitudeOffset;
    if (QFE != oldQFEOff) {
      oldQFEOff = QFE;
      _sntprintf(szTmp, MAX_NMEA_LEN, TEXT("PLXV0,ELEVATION,W,%i"), (int)(QFEAltitudeOffset));
      SendNmea(d, szTmp);
    }
#endif
  }
#ifdef EEE
  if (iNano3_GPSBaudrate == 0) {
    _tcscpy(szTmp, TEXT("PLXV0,BRGPS,R"));
    SendNmea(d, szTmp);
  }
#endif
  if (strncmp("$PLXVC", sentence, 6) == 0) {
    return PLXVC(d, sentence, info);
  }

  if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)) {
    return FALSE;
  }

  if (strncmp("$PLXVF", sentence, 6) == 0)
    return PLXVF(d, sentence + 7, info);
  else if (strncmp("$PLXVS", sentence, 6) == 0)
    return PLXVS(d, sentence + 7, info);
  else if (strncmp("$PLXV0", sentence, 6) == 0)
    return PLXV0(d, sentence + 7, info);
  else if (strncmp("$LXWP2", sentence, 6) == 0)
    return LXWP2(d, sentence + 7, info);
  else if (strncmp("$LXWP0", sentence, 6) == 0)
    return LXWP0(d, sentence + 7, info);
  else if (strncmp("$PLXVTARG", sentence, 9) == 0)
    return PLXVTARG(d, sentence + 10, info);
  else if (strncmp("$GPRMB", sentence, 6) == 0)
    return GPRMB(d, sentence + 7, info);
  else if (strncmp("$LXWP1", sentence, 6) == 0) {
    Nano3_bValid = true;
    return LXWP1(d, sentence + 7, info);
  }
  return false;
}  // ParseNMEA()

CallBackTableEntry_t DevLXNanoIII::CallBackTable[]={

  EndCallBackEntry()
};



BOOL DevLXNanoIII::SetupLX_Sentence(DeviceDescriptor_t* d)
{
  SendNmea(d, TEXT("PLXV0,NMEARATE,W,2,5,10,10,1,5,5"));
  return true;
}


BOOL DevLXNanoIII::SetDataText(DeviceDescriptor_t* d, ValueStringIndex Idx,  const TCHAR ValueText[])
{
	bool res = false;
if(d)
 if( Port() == d->PortNumber)
 {
  CritSec_LXDebugStr.lock();
  _tcsncpy(LxValueStr[Idx] , ValueText, MAX_VAL_STR_LEN);
  CritSec_LXDebugStr.unlock();
	res = true;
 }
  return res;
}

BOOL DevLXNanoIII::ClearDataText( ValueStringIndex Idx )
{

 {
  CritSec_LXDebugStr.lock();
  _tcsncpy(LxValueStr[Idx] , _T(""), MAX_VAL_STR_LEN);
  CritSec_LXDebugStr.unlock();
 }
  return true;
}


BOOL DevLXNanoIII::ShowData(WndForm* wf , DeviceDescriptor_t* d)
{
  WndProperty *wp;
  if(!wf) return false;

  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

   wp = wf->FindByName<WndProperty>(TEXT("prpQNHDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
  //  dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
 //   dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.QNHDir);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpMCDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.MCDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBUGDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.BUGDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBALDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>());  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->addEnumText(MsgToken<2454>()); // LKTOKEN  _@M2454_ "IN & OUT"
    dfe->Set((uint) PortIO.BALDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpSTFDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->Set((uint) PortIO.STFDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpWINDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->addEnumText(MsgToken<2453>()); // LKTOKEN  _@M2453_ "OUT"
    dfe->Set((uint) PortIO.WINDDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBARODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.BARODir);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpSPEEDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.SPEEDDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpVARIODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"

    dfe->Set((uint) PortIO.VARIODir);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpR_TRGTDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(_T("$PLXVTARG")); // "IN" = $PLXVTARG
    dfe->addEnumText(_T("$GPRMB")); //  "OUT" = $GPRMB
    dfe->Set((uint) PortIO.R_TRGTDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpT_TRGTDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(_T("$PLXVTARG")); // "IN" = $PLXVTARG
    dfe->addEnumText(_T("$GPRMB")); //  "OUT" = $GPRMB
    dfe->Set((uint) PortIO.T_TRGTDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpGFORCEDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.GFORCEDir);
    wp->RefreshDisplay();
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpOATDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.OATDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBAT1Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.BAT1Dir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpBAT2Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.BAT2Dir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpPOLARDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<2452>()); // LKTOKEN  _@M2452_ "IN"
    dfe->Set((uint) PortIO.POLARDir);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpDirectLink"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken<491>()); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken<894>()); // LKTOKEN  _@M894_": "ON"
    dfe->Set((uint) PortIO.DirLink);
    wp->RefreshDisplay();
  }

  return true;
}


void UpdateValueTxt(WndProperty *wp,  ValueStringIndex Idx)
{
  if(wp)
  {
    DataField* dfe = wp->GetDataField();
    if(dfe)
    {
      dfe->Clear();
      CritSec_LXDebugStr.lock();
      dfe->addEnumText(LxValueStr[Idx]);
      CritSec_LXDebugStr.unlock();
      wp->RefreshDisplay();
    }
  }
}

static bool OnTimer(WndForm* pWnd)
{
  WndForm * wf = pWnd->GetParentWndForm();
  WndProperty *wp = NULL;

  if(wf)
  {
    wp = wf->FindByName<WndProperty>(TEXT("prpQNHDir")    ); UpdateValueTxt( wp,  _QNH   );    
    wp = wf->FindByName<WndProperty>(TEXT("prpMCDir")     ); UpdateValueTxt( wp,  _MC    );
    wp = wf->FindByName<WndProperty>(TEXT("prpBUGDir")    ); UpdateValueTxt( wp,  _BUGS  );
    wp = wf->FindByName<WndProperty>(TEXT("prpBALDir")    ); UpdateValueTxt( wp,  _BAL   );
    wp = wf->FindByName<WndProperty>(TEXT("prpSTFDir")    ); UpdateValueTxt( wp,  _STF   );
    wp = wf->FindByName<WndProperty>(TEXT("prpWINDDir")   ); UpdateValueTxt( wp,  _WIND  );
    wp = wf->FindByName<WndProperty>(TEXT("prpBARODir")   ); UpdateValueTxt( wp,  _BARO  );
    wp = wf->FindByName<WndProperty>(TEXT("prpVARIODir")  ); UpdateValueTxt( wp,  _VARIO );
    wp = wf->FindByName<WndProperty>(TEXT("prpSPEEDDir")  ); UpdateValueTxt( wp,  _SPEED );
    wp = wf->FindByName<WndProperty>(TEXT("prpR_TRGTDir") ); UpdateValueTxt( wp,  _R_TRGT);
    wp = wf->FindByName<WndProperty>(TEXT("prpT_TRGTDir") ); UpdateValueTxt( wp,  _T_TRGT);
    wp = wf->FindByName<WndProperty>(TEXT("prpGFORCEDir") ); UpdateValueTxt( wp,  _GFORCE);
    wp = wf->FindByName<WndProperty>(TEXT("prpOATDir")    ); UpdateValueTxt( wp,  _OAT   );
    wp = wf->FindByName<WndProperty>(TEXT("prpBAT1Dir")   ); UpdateValueTxt( wp,  _BAT1  );
    wp = wf->FindByName<WndProperty>(TEXT("prpBAT2Dir")   ); UpdateValueTxt( wp,  _BAT2  );
    wp = wf->FindByName<WndProperty>(TEXT("prpPOLARDir")  ); UpdateValueTxt( wp,  _POLAR );
    wp = wf->FindByName<WndProperty>(TEXT("prpDirectLink")); UpdateValueTxt( wp,  _DIRECT);
  }
  return true;
}

BOOL DevLXNanoIII::Config(DeviceDescriptor_t* d){

  WndForm*  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_DEV_LXNAV_L : IDR_XML_DEV_LXNAV_P);
  if(wf) {
    Device(d);
    WndButton *wBt = NULL;

    wBt = wf->FindByName<WndButton>(TEXT("cmdClose"));
    if(wBt){
      wBt->SetOnClickNotify(OnCloseClicked);
    }

    wBt = wf->FindByName<WndButton>(TEXT("cmdIGCDownload"));
    if(wBt){
      wBt->SetOnClickNotify(OnIGCDownloadClicked);
    }

    wBt = wf->FindByName<WndButton>(TEXT("cmdValues"));
    if(wBt){
      wBt->SetOnClickNotify(OnValuesClicked);
    }
    ShowValues(false);

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
// static
BOOL DevLXNanoIII::DeclareTask(DeviceDescriptor_t* d, const Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]) {

  // to declared Start, TPs, and Finish we will add Takeoff and Landing,
  // so maximum NB of declared TPs is Decl::max_wp_count - 2
  if (!CheckWPCount(*lkDecl, Decl::min_wp_count - 2, Decl::max_wp_count - 2, errBufSize, errBuf)) {
    return false;
  }

  Decl decl;
  Class lxClass;
  byte t_DD, t_MM, t_YY, t_hh, t_mm, t_ss;
  // we will use text-defined class
  lxClass.SetName(lkDecl->CompetitionClass);

  bool status = false;

  Declare(true);
  ShowProgress(decl_enable);
  // Create and send task declaration...
  ShowProgress(decl_send);

  int wpCount = lkDecl->num_waypoints;
  unsigned totalLines = 6 + 1 + wpCount + 1;
  TCHAR DeclStrings[totalLines][256];
  unsigned i = 0;

  // Metadata
  _stprintf(DeclStrings[i++], TEXT("HFPLTPILOT:%s"), lkDecl->PilotName);
  _stprintf(DeclStrings[i++], TEXT("HFGTYGGLIDERTYPE:%s"), lkDecl->AircraftType);
  _stprintf(DeclStrings[i++], TEXT("HFGIDGLIDERID:%s"), lkDecl->AircraftRego);
  _stprintf(DeclStrings[i++], TEXT("HFCIDCOMPETITIONID:%s"), lkDecl->CompetitionID);
  _stprintf(DeclStrings[i++], TEXT("HFCCLCOMPETITIONCLASS:%s"), lkDecl->CompetitionClass);

  // "C" record, first line acording to IGC GNSS specification 3.6.1
  if (!GPS_INFO.NAVWarning && GPS_INFO.SatellitesUsed > 0 && GPS_INFO.Day >= 1 && GPS_INFO.Day <= 31 &&
      GPS_INFO.Month >= 1 && GPS_INFO.Month <= 12) {
    t_DD = GPS_INFO.Day;
    t_MM = GPS_INFO.Month;
    t_YY = GPS_INFO.Year % 100;
    t_hh = GPS_INFO.Hour;
    t_mm = GPS_INFO.Minute;
    t_ss = GPS_INFO.Second;
  } else {  // use system time
    time_t sysTime = time(NULL);
    struct tm tm_temp = {};
    struct tm* utc = gmtime_r(&sysTime, &tm_temp);
    t_DD = utc->tm_mday;
    t_MM = utc->tm_mon + 1;
    t_YY = utc->tm_year % 100;
    t_hh = utc->tm_hour;
    t_mm = utc->tm_min;
    t_ss = utc->tm_sec;
  }
  _stprintf(DeclStrings[i++], TEXT("C%02d%02d%02d%02d%02d%02d000000%04d%02d"),
         // DD    MM    YY    HH    MM    SS (DD MM YY) IIII  TT
            t_DD, t_MM, t_YY, t_hh, t_mm, t_ss,         1,    wpCount - 2);

  // TakeOff point
  if ((HomeWaypoint >= 0) && ValidWayPoint(HomeWaypoint) && DeclTakeoffLanding) {
    decl.WpFormat(DeclStrings[i++], &WayPointList[HomeWaypoint], Decl::tp_takeoff);
  } else {
    decl.WpFormat(DeclStrings[i++], NULL, Decl::tp_takeoff);
  }

  // TurnPoints
  for (int ii = 0; ii < wpCount; ii++) {
    decl.WpFormat(DeclStrings[i++], lkDecl->waypoint[ii], Decl::tp_regular);
  }

  // Landing point
  if ((HomeWaypoint >= 0) && ValidWayPoint(HomeWaypoint) && DeclTakeoffLanding) {
    decl.WpFormat(DeclStrings[i++], &WayPointList[HomeWaypoint], Decl::tp_landing);
  } else {
    decl.WpFormat(DeclStrings[i++], NULL, Decl::tp_landing);
  }

  for (unsigned ii = 0; ii < i; ii++) {
    TestLog(_T(". NANO Decl: > %s"), DeclStrings[ii]);

    char wait_str[128];
    size_t out_size = lk::snprintf(wait_str, "$PLXVC,DECL,C,%u", ii + 1);
    if (out_size < 128) {
      unsigned crc = nmea_crc(wait_str + 1);
      lk::snprintf(&wait_str[out_size], 128 - out_size, "*%02X\r\n", crc);
    }
    wait_ack_shared_ptr wait_ack = d->make_wait_ack(wait_str);

    TCHAR send_str[512];
    lk::snprintf(send_str, _T("PLXVC,DECL,W,%u,%u,%s"), ii + 1, i, DeclStrings[ii]);

    status = SendNmea(d, send_str);
    if (status) {
      status = wait_ack->wait(10000);
    }

    TestLog(_T(". NANO Decl: < %s"), status ? to_tstring(wait_str).c_str() : _T("failed"));

    if (!status) {
      break;  // failed ...
    }
  }

  ShowProgress(decl_disable);
  Declare(false);

  return status;
}  // DeclareTask()

// #############################################################################
// *****************************************************************************
//
//   DevLXNanoIII::Decl
//
// *****************************************************************************
// #############################################################################


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Format waypoint data
///
/// @param buf  [out] buffer
/// @param wp    waypoint data (if @c NULL, LAT/LON will be set to 0)
/// @param type  waypoint type
/// @param idx   waypoint index
///
void DevLXNanoIII::Decl::WpFormat(TCHAR buf[], const WAYPOINT* wp, WpType type){
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
//   DevLXNanoIII::Class
//
// *****************************************************************************
// #############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the value of @c name member.
///
/// @param text  string to be set (will be converted into ASCII)
///
void DevLXNanoIII::Class::SetName(const TCHAR* text){
  Wide2LxAscii(text, name);
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
bool DevLXNanoIII::SendNmea(DeviceDescriptor_t* d, const TCHAR buf[], unsigned errBufSize, TCHAR errBuf[]){

  ScopeLock Lock(CritSec_Comm);
  if(!d || !d->Com) {
    return false;
  }

  char ascii_str[256] = "$";
  Wide2LxAscii(buf, std::size(ascii_str) - 1, ascii_str+1);
  unsigned char chksum = 0;

  auto pChar = (ascii_str + 1);
  for(; *pChar; ++pChar) {
    chksum ^= (*pChar);
  }
  sprintf(pChar, "*%02X\r\n",chksum);
  pChar += 5; 

  if (!ComWrite(d, ascii_str, std::distance(ascii_str, pChar), errBufSize, errBuf)) {
    return (false);
  }
  return (true);
} // SendNmea()


bool DevLXNanoIII::SendNmea(DeviceDescriptor_t* d, const TCHAR buf[]){
  TCHAR errBuf[100] = _T("");

  DevLXNanoIII::SendNmea(d, buf, std::size(errBuf), errBuf);
  if(_tcslen (errBuf) > 1)
  {
    DoStatusMessage(errBuf);
    return false;
  }
 // StartupStore(_T(" Nano3 SenadNmea %s %s"),buf, NEWLINE);
  return true;
} // SendNmea()

void DevLXNanoIII::GetDirections(WndButton* pWnd){
  if(pWnd) {
    WndForm * wf = pWnd->GetParentWndForm();
    if(wf) {
      int PortNum = Port();
      auto& Port = PortConfig[PortNum];
      auto& PortIO = Port.PortIO;

      WndProperty *wp;
      wp = wf->FindByName<WndProperty>(TEXT("prpMCDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.MCDir = (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpBUGDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BUGDir =  (DataBiIoDir) dfe->GetAsInteger();
      }

      wp = wf->FindByName<WndProperty>(TEXT("prpBALDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BALDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpSTFDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.STFDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpWINDDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.WINDDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpBARODir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BARODir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpVARIODir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.VARIODir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpSPEEDDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.SPEEDDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpR_TRGTDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.R_TRGTDir =  (DataTP_Type) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpGFORCEDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.GFORCEDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpOATDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.OATDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpBAT1Dir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BAT1Dir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpBAT2Dir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.BAT2Dir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpPOLARDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.POLARDir =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpDirectLink"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.DirLink =  (DataBiIoDir) dfe->GetAsInteger();
      }
      wp = wf->FindByName<WndProperty>(TEXT("prpT_TRGTDir"));
      if (wp) {
        DataField* dfe = wp->GetDataField();
        PortIO.T_TRGTDir =  (DataTP_Type) dfe->GetAsInteger();
      }
    }
  }
}

void DevLXNanoIII::OnCloseClicked(WndButton* pWnd)
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


void DevLXNanoIII::OnValuesClicked(WndButton* pWnd) {

  WndForm * wf = pWnd->GetParentWndForm();
  if(wf)
  {
    ShowValues (!ShowValues());
    WndButton *wBt = wf->FindByName<WndButton>(TEXT("cmdValues"));
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
    
    StartupStore(_T(" Nano3 CLEAR VALUES %s"), NEWLINE);
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
  }
}

void DevLXNanoIII::OnIGCDownloadClicked(WndButton* pWnd) {
(void)pWnd;
LockFlightData();
bool bInFlight    = CALCULATED_INFO.Flying;
UnlockFlightData();

  if(bInFlight) {
    MessageBoxX(MsgToken<2418>(), MsgToken<2397>(), mbOk);
    return;
  }
  TCHAR szTmp[MAX_NMEA_LEN];

  _sntprintf(szTmp,MAX_NMEA_LEN, _T("PLXVC,LOGBOOKSIZE,R"));

  SendNmea(Device(), szTmp);

  dlgLX_IGCSelectListShowModal();
}




bool  DevLXNanoIII::OnStartIGC_FileRead(TCHAR Filename[]) {
TCHAR szTmp[MAX_NMEA_LEN];
TCHAR IGCFilename[MAX_PATH];
LocalPath(IGCFilename, _T(LKD_LOGS), Filename);

  f = _tfopen( IGCFilename, TEXT("w"));
  if(f == NULL)   return false;
  // SendNmea(Device(), _T("PLXVC,KEEP_ALIVE,W"), errBufSize, errBuf);
  StartupStore(_T(" ******* NANO3  IGC Download START ***** %s") , NEWLINE);
  _sntprintf(szTmp,MAX_NMEA_LEN, _T("PLXVC,FLIGHT,R,%s,1,%u"),Filename,BLOCK_SIZE+1);
  _sntprintf(m_Filename, std::size(m_Filename), _T("%s"),Filename);
  SendNmea(Device(), szTmp);
  StartupStore(_T("> %s %s") ,szTmp, NEWLINE);
  IGCDownload(true);
#ifdef  NANO_PROGRESS_DLG
  CreateIGCProgressDialog();
#endif
return true;

}



BOOL DevLXNanoIII::AbortLX_IGC_FileRead(void)
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


BOOL DevLXNanoIII::PLXVC(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
 /*
  * $PLXVC,<key>,<type>,<values>*<checksum><cr><lf>
  */

  bool bCRCok = NMEAParser::NMEAChecksum(sentence);

  char ctemp[MAX_NMEA_LEN];
  char* params[MAX_NMEA_PARAMS];
  size_t n_params = NMEAParser::ExtractParameters(sentence, ctemp, params);
  if(n_params < 4) {
    return FALSE;
  }

  assert(std::string_view(params[1]) != "$PLXVC"); // reponsability of callers...

  std::string_view key(params[1]);

  if (key == "INFO") {
    return PLXVC_INFO(d, &params[2], n_params - 2, info);
  }

  if (key == "LOGBOOKSIZE") {
    unsigned size = strtol(params[3], nullptr, 10);
    if(size > 0) { // at least one file exists?
      TCHAR szCommand[MAX_NMEA_LEN];
      _sntprintf(szCommand,MAX_NMEA_LEN, _T("PLXVC,LOGBOOK,R,1,%u"), size + 1);
      SendNmea(d, szCommand);
    }
    return TRUE;
  }

  if(key == "LOGBOOK") {
    if (n_params < 10) {
      return FALSE;
    }

    tstring Line1 = from_unknown_charset(params[5]);

    tstring a = from_unknown_charset(params[6]);
    tstring b = from_unknown_charset(params[7]); 
    tstring c = from_unknown_charset(params[8]);
    uint32_t size = strtoul(params[9], nullptr, 10) / 1024;


    TCHAR Line2[MAX_NMEA_LEN];
    _sntprintf(Line2,MAX_NMEA_LEN, _T("%s (%s-%s) %ukB"), a.c_str() ,b.c_str() ,c.c_str(), size);
    AddElement(Line1.c_str(), Line2);
    return TRUE;
  }

  if (key == "FLIGHT" && IGCDownload()) {
    if( bCRCok) { // CRC OK?
      std::string_view data(params[6]);
      for (char c : data) {
        fputc(c, f); 
      }
      fputc('\n',f);

      m_CurLine = strtoul(params[4], nullptr, 10);
      uint TotalLines = strtoul(params[5], nullptr, 10);
      uint uPercent = 0;
      if(TotalLines > 0) {
        uPercent = (m_CurLine*100) / TotalLines;
      }
      TCHAR szString[MAX_NMEA_LEN];
      _sntprintf(szString, MAX_NMEA_LEN, _T("%s: %u%% %s ..."),MsgToken<2400>(), uPercent, m_Filename); // _@M2400_ "Downloading"

#ifdef NANO_PROGRESS_DLG
      IGCProgressDialogText(szString);
#endif
      if(m_CurLine == TotalLines) { 
      // reach end of file?
        if(f != NULL) { 
          fclose(f); 
          f= NULL; 
        }
        StartupStore(_T(" ******* NANO3  IGC Download END ***** %s") , NEWLINE);
        IGCDownload( false);
  //   MessageBoxX(  MsgToken<2406>(), MsgToken<2398>(), mbOk) ;  // // _@M2398_ "IGC Download"
#ifdef NANO_PROGRESS_DLG
        CloseIGCProgressDialog();
#endif
      }
    }  // CRC OK?

    if(IGCDownload()) {
      if((m_CurLine % (BLOCK_SIZE)==0) || !bCRCok) {
        IGCDownload( true );
        TCHAR szCommand[MAX_NMEA_LEN];
        _sntprintf(szCommand, MAX_NMEA_LEN, _T("PLXVC,FLIGHT,R,%s,%u,%u"),m_Filename,m_CurLine+1,m_CurLine+BLOCK_SIZE+1);
        SendNmea(Device(), szCommand);
      }
    }  // if(IGCDownload())
  }  // FLIGHT
  
  return FALSE;
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
BOOL DevLXNanoIII::LXWP0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
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
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if( !devGetAdvancedMode(d))
  {
    if (ParToDouble(sentence, 1, &fTmp))
    {

      if(Values(d))
      { TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp, MAX_NMEA_LEN,_T("%5.1fkm/h ($LXWP0)"),fTmp);
        SetDataText( d,_SPEED,   szTmp);
      }
      if(IsDirInput(PortIO.SPEEDDir  ))
      {
        airspeed = Units::From(unKiloMeterPerHour, fTmp);
        info->IndicatedAirspeed = airspeed;
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
        if (airspeed>0) {
          info->IndicatedAirspeed = IndicatedAirSpeed(airspeed, fTmp);
        }
        UpdateBaroSource(info, d, fTmp);
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

  if (ParToDouble(sentence, 10, &fDir) &&
  ParToDouble(sentence, 11, &fTmp))
  {

    if(Values(d))
    { TCHAR szTmp[MAX_NMEA_LEN];
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.1fkm/h %3.0f° ($LXWP0)"),fTmp,fDir);
      SetDataText( d, _WIND,   szTmp);
    }
    if(IsDirInput(PortIO.WINDDir  ))
    {
      info->ExternalWindDirection = fDir;
      info->ExternalWindSpeed =  Units::From(unKiloMeterPerHour, fTmp);
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
BOOL DevLXNanoIII::LXWP1(DeviceDescriptor_t* d, const char* String, NMEA_INFO* pGPS)
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
  char ctemp[MAX_NMEA_LEN];
  static int NoMsg=0;
  static int oldSerial=0;
  if (strlen(String) < 180)
    if ((( d->SerialNumber == 0)  || ( d->SerialNumber != oldSerial)) && (NoMsg < 5)) {
      NoMsg++ ;
      NMEAParser::ExtractParameter(String, ctemp, 0);
      from_unknown_charset(ctemp, d->Name);
      StartupStore(_T(". %s"), d->Name);

      NMEAParser::ExtractParameter(String, ctemp, 1);
      d->SerialNumber = StrToDouble(ctemp, nullptr);
      oldSerial = d->SerialNumber;
      StartupStore(_T(". %s Serial Number %i"), d->Name, d->SerialNumber);

      NMEAParser::ExtractParameter(String, ctemp, 2);
      d->SoftwareVer= StrToDouble(ctemp, nullptr);
      StartupStore(_T(". %s Software Vers.: %3.2f"), d->Name, d->SoftwareVer);

      NMEAParser::ExtractParameter(String, ctemp, 3);
      d->HardwareId = StrToDouble(ctemp, nullptr) * 10;
      StartupStore(_T(". %s Hardware Vers.: %3.2f"), d->Name, d->HardwareId / 10.0);

      TCHAR str[255];
      _stprintf(str, _T("%s (#%i) DETECTED"), d->Name, d->SerialNumber);
      DoStatusMessage(str);
      _stprintf(str, _T("SW Ver: %3.2f HW Ver: %3.2f "),  d->SoftwareVer, d->HardwareId / 10.0);
      DoStatusMessage(str);
  }
  // nothing to do
#endif
  return true;
} // LXWP1()



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
BOOL DevLXNanoIII::LXWP2(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO*)
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
  int iTmp;
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if (ParToDouble(sentence, 0, &fTmp))
  {
    iTmp =(int) (fTmp*100.0+0.5f);
    fTmp = (double)(iTmp)/100.0;
    if(Values(d))
    {
      TCHAR szTmp[MAX_NMEA_LEN];
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.2fm/s ($LXWP2)"),fTmp);
      SetDataText( d, _MC,   szTmp);
    }
    Nano3_bValid = true;
    if (IsDirInput(PortIO.MCDir)) {
      d->RecvMacCready(fTmp);
    }
  }

  if (ParToDouble(sentence, 1, &fTmp))
  {
    double fBALPerc = CalculateBalastFromLX(fTmp);
    if(Values(d))
    {
      TCHAR szTmp[MAX_NMEA_LEN];
      _sntprintf(szTmp,MAX_NMEA_LEN,  _T("%5.2f = %3.0f%% ($LXWP2)"),fTmp,(fBALPerc*100.0));
      SetDataText( d, _BAL,  szTmp);
    }
    if (IsDirInput(PortIO.BALDir)) {
      d->RecvBallast(fBALPerc);
    }
  }

  if(ParToDouble(sentence, 2, &fTmp))
  {
    if(Values(d))
    {
      TCHAR szTmp[MAX_NMEA_LEN];
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.0f%% ($LXWP2)"),fTmp);
      SetDataText( d,_BUGS,  szTmp);
    }
    if (IsDirInput(PortIO.BUGDir)) {
      fTmp = CalculateBugsFromLX(fTmp);
      d->RecvBugs(fTmp);
    }
  }

 double fa,fb,fc;
     if(ParToDouble(sentence, 3, &fa))
       if(ParToDouble(sentence, 4, &fb))
   if(ParToDouble(sentence, 5, &fc))
   {
      if(Values(d))
      {
          TCHAR szTmp[MAX_NMEA_LEN];
          _sntprintf(szTmp,MAX_NMEA_LEN, _T("a:%5.3f b:%5.3f c:%5.3f ($LXWP2)"),fa,fb,fc);
          SetDataText( d,  _POLAR,  szTmp);
      }
      if(IsDirInput(PortIO.POLARDir ))
      {
        double v;
        for (int i=0; i < 3; i++)
        {
          v=POLARV[i]/100;
          POLARLD[i] = -(fa*v*v + fb*v + fc);
#ifdef TESTBENCH
          TCHAR szTmp[MAX_NMEA_LEN];
          _sntprintf(szTmp,MAX_NMEA_LEN, _T("V[%i]:%5.0f    s[%i]:%6.2f  ($LXWP2)"),i,POLARV[i],i,POLARLD[i] );
          StartupStore(TEXT("Polar: %s"), szTmp);
#endif
        }
        _sntprintf (szPolarName ,80, _T("%s"), d->Name );
        PolarWinPilot2XCSoar(POLARV, POLARLD, WW);
        GlidePolar::SetBallast();
      }
    }



     if(ParToDouble(sentence, 6, &fTmp))
     {

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
BOOL DevLXNanoIII::LXWP3(DeviceDescriptor_t*, const char*, NMEA_INFO*)
{
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




BOOL DevLXNanoIII::LXWP4(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
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



BOOL DevLXNanoIII::PLXVF(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  TCHAR szTmp[MAX_NMEA_LEN];
  double alt=0, airspeed=0;
  devSetAdvancedMode(d,true);

  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if(IsDirInput(PortIO.GFORCEDir))
  {
    double fX,fY,fZ;
    if(ParToDouble(sentence, 1, &fX) &&
      ParToDouble(sentence, 2, &fY) &&
      ParToDouble(sentence, 3, &fZ))
    {
      if(Values(d))
      {
        _sntprintf(szTmp, MAX_NMEA_LEN, _T("%5.2f %5.2f %5.2f ($PLXVF)"),fZ,fY,fX);
        SetDataText( d, _GFORCE,  szTmp);
      }
      if(IsDirInput(PortIO.GFORCEDir))
      {
        info->AccelX  = fX;
        info->AccelY  = fY;
        info->AccelZ  = fZ;
        info->AccelerationAvailable = true;
      }
    }
  }


  if (ParToDouble(sentence, 5, &airspeed))
  {
    if(Values(d))
    {
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.0fkm/h ($PLXVF)"), Units::To(unKiloMeterPerHour, airspeed));
      SetDataText( d, _SPEED,  szTmp);
    }
//  airspeed = 135.0/TOKPH;
    if(IsDirInput(PortIO.SPEEDDir ))
    {
      info->IndicatedAirspeed = airspeed;
      info->AirspeedAvailable = TRUE;
    }
  }


  if (ParToDouble(sentence, 6, &alt))
  {
    if(Values(d))
    {
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.0fm ($PLXVF)"),alt);
      SetDataText( d,_BARO,  szTmp);
    }
    if(IsDirInput(PortIO.BARODir))
    {
      UpdateBaroSource( info, d, QNEAltitudeToQNHAltitude(alt));
      if (airspeed>0) {
        info->IndicatedAirspeed = IndicatedAirSpeed(airspeed, alt);
      }
    }
  }


  if (ParToDouble(sentence, 4, &alt))
  {
    if(Values(d))
    {
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%5.2fm/s ($PLXVF)"),alt);
      SetDataText( d,_VARIO,  szTmp);
    }
    if(IsDirInput(PortIO.VARIODir))
    {
      UpdateVarioSource(*info, *d, alt);
    }
  }


// Get STF switch
  double fTmp;
  if (ParToDouble(sentence, 7, &fTmp))
  {
    int  iTmp = (int)(fTmp+0.1);
    if(Values(d))
    {
      if(iTmp == 1)
        SetDataText( d, _STF,  _T("STF ($PLXVF)"));
      else
        SetDataText( d, _STF,  _T("VARIO ($PLXVF)"));
    }

    static int  iOldVarioSwitch=0;
    if(IsDirInput(PortIO.STFDir))
    {
      EnableExternalTriggerCruise = true;
      if(iTmp != iOldVarioSwitch)
      {
        iOldVarioSwitch = iTmp;
        if(iTmp==1)
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


  return(true);
} // PLXVF()


BOOL DevLXNanoIII::PLXVS(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  double Batt;
  double OAT;
  TCHAR szTmp[MAX_NMEA_LEN];
  devSetAdvancedMode(d,true);
  iS_SeriesTimeout = 30;

  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if (ParToDouble(sentence, 0, &OAT))
  {
    if(Values(d))
    {
      _sntprintf(szTmp, MAX_NMEA_LEN, _T("%3.1f°C ($PLXVS)"),OAT);
      SetDataText( d, _OAT,  szTmp);
    }

    if(IsDirInput(PortIO.OATDir))
    {
      info->OutsideAirTemperature = OAT;
      info->TemperatureAvailable  = TRUE;
    }
  }

  if (ParToDouble(sentence, 2, &Batt))
  {
    if(Values(d))
    {
      _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.1fV ($PLXVS)"),Batt);
      SetDataText( d, _BAT1,  szTmp);
    }
    if(IsDirInput(PortIO.BAT1Dir))
    {
      info->ExtBatt1_Voltage = Batt;
    }
  }

  return(true);
} // PLXVS()


BOOL DevLXNanoIII::PLXV0(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  char  szTmp1[MAX_NMEA_LEN], szTmp2[MAX_NMEA_LEN];
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  NMEAParser::ExtractParameter(sentence,szTmp1,1);
  if  (strcmp(szTmp1, "W")!=0)  // no write flag received
    return false;

  NMEAParser::ExtractParameter(sentence,szTmp1,0);
  if  (strcmp(szTmp1, "BRGPS") == 0)
  {
    NMEAParser::ExtractParameter(sentence,szTmp2,2);
    iNano3_GPSBaudrate = Nano3Baudrate( (int)( (StrToDouble(szTmp2,NULL))+0.1 ) );
    return true;
  }

  if (strcmp(szTmp1, "BRPDA") == 0)
  {
    NMEAParser::ExtractParameter(sentence,szTmp2,2);
    iNano3_PDABaudrate = Nano3Baudrate( (int) StrToDouble(szTmp2,NULL));
    return true;
  }

  NMEAParser::ExtractParameter(sentence,szTmp1,0);
  if  (strcmp(szTmp1, "QNH") == 0)
  {
    
    NMEAParser::ExtractParameter(sentence,szTmp2,2);
    double newQNH = StrToDouble(szTmp2,NULL)/100.0;
    TCHAR szQNH[128];
    _sntprintf(szQNH,std::size(szQNH), TEXT("%6.1f $PLXV"),newQNH);
    SetDataText( d, _QNH,   szQNH);
    if(IsDirInput(PortIO.QNHDir))
    {
      UpdateQNH(newQNH);
      StartupStore(_T("Nano3 QNH: %s"),szQNH);
    }
    return true;
  }

  /****************************************************************
   * MacCready
   ****************************************************************/
  if (strcmp(szTmp1, "MC") == 0) {
    double fTmp;
    if (ParToDouble(sentence, 2, &fTmp)) {
      if (Values(d)) {
        TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp, MAX_NMEA_LEN, _T("%5.2f PLXV0"), fTmp);
        SetDataText(d, _MC, szTmp);
      }
      if (IsDirInput(PortIO.MCDir)) {
        d->RecvMacCready(fTmp);
        StartupStore(_T("Nano3 MC: %5.2f"), fTmp);
        return true;
      }
    }
    return false;
  }

  /****************************************************************
   * BALlast
   ****************************************************************/
  if (strcmp(szTmp1, "BAL") == 0)
  {
    double fTmp;
    if (ParToDouble(sentence, 2, &fTmp)) {
      double fNewBal = CalculateBalastFromLX(fTmp);
      if (Values(d)) {
        TCHAR szTmp[MAX_NMEA_LEN];
        _sntprintf(szTmp, MAX_NMEA_LEN, _T("%2.1f %3.0f PLXV0"), fTmp, fNewBal);
        SetDataText(d, _BAL, szTmp);
      }
      if (IsDirInput(PortIO.BALDir)) {
        d->RecvBallast(fNewBal);
        return true;
      }
    }
    return false;
  }
  /****************************************************************
   * BUGs
   ****************************************************************/
  if (strcmp(szTmp1, "BUGS") == 0)
  {
    double fTmp;
    if (ParToDouble(sentence, 2, &fTmp)) {
      if (Values(d)) {
        TCHAR szTmp[20];
        _sntprintf(szTmp, std::size(szTmp), _T("%3.0f%% ($PLXV0)"), fTmp);
        SetDataText(d, _BUGS, szTmp);
      }
      if (IsDirInput(PortIO.BUGDir)) {
        d->RecvBugs(fTmp);
        return true;
      }
    }
    return false;
  }

  if (strcmp(szTmp1, "VOL")==0)
  {
    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
      StartupStore(_T("Nano3 VOL: %i"),(int)fTmp);
    return true;
  }

  /****************************************************************
   * POLAR
   ****************************************************************/
  if (strcmp(szTmp1, "POLAR") == 0)
  {
    double fLoad,fWeight,fMaxW, fEmptyW,fPilotW, fa,fb,fc;
    if( (ParToDouble(sentence, 2, &fa)) &&
        (ParToDouble(sentence, 3, &fb)) &&
        (ParToDouble(sentence, 4, &fc)) &&
        (ParToDouble(sentence, 5, &fLoad)) &&
        (ParToDouble(sentence, 6, &fWeight)) &&
        (ParToDouble(sentence, 7, &fMaxW)) &&
        (ParToDouble(sentence, 8, &fEmptyW)) &&
        (ParToDouble(sentence, 9, &fPilotW))
      )
      StartupStore(_T("Nano3 POLAR: a:%5.2f b:%5.2f c:%5.2f L:%3.1f W:%3.1f E:%3.1f P:%3.1f"),fa,fb,fc, fLoad,fWeight,fEmptyW,fPilotW)  ;
    return true;
  }

  if (strcmp(szTmp1, "CONNECTION") == 0)
  {
    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
      StartupStore(_T("Nano3 CONNECTION: %i"),(int)fTmp);
    return true;
  }

  if (strcmp(szTmp1, "NMEARATE") == 0)
  {
    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
      StartupStore(_T("Nano3 NMEARATE: %i"),(int)fTmp);
    return true;
  }

  return(false);
} // PLXV0()




BOOL Nano3_PutMacCready(DeviceDescriptor_t* d, double MacCready){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;
  if(Nano3_bValid == false) return false;

  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;


  if(!IsDirOutput(PortIO.MCDir)) return false;
  if(devGetAdvancedMode(d)) {
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PLXV0,MC,W,%3.1f"), MacCready );
  } else {
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PFLX2,%.2f,,,,,"), MacCready);
  }
  DevLXNanoIII::SendNmea(d,szTmp);
  return true;
}


BOOL Nano3_PutBallast(DeviceDescriptor_t* d, double Ballast){
  TCHAR  szTmp[MAX_NMEA_LEN];
  if(!d)  return false;
  if(Nano3_bValid == false) return false;
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if(!IsDirOutput(PortIO.BALDir)) return false;

  double BallastFact =  CalculateLXBalastFactor(Ballast);
  if(devGetAdvancedMode(d))
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PLXV0,BAL,W,%4.2f"),BallastFact);
  else
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PFLX2,,%.2f,,,,"), BallastFact);
  DevLXNanoIII::SendNmea(d,szTmp);

  return(TRUE);
}


BOOL Nano3_PutBugs(DeviceDescriptor_t* d, double Bugs){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;
  if(Nano3_bValid == false) return false;
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;
  if(!IsDirOutput(PortIO.BUGDir)) return false;
  double LXBugs = CalculateLXBugs( Bugs);

  if(devGetAdvancedMode(d))
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PLXV0,BUGS,W,%3.1f"),LXBugs);
  else
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PFLX2,,,%d,,,"), (int)LXBugs);

  DevLXNanoIII::SendNmea(d,szTmp);
  return(TRUE);
}


BOOL DevLXNanoIII::PutTarget(DeviceDescriptor_t* d, const WAYPOINT& wpt) {
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if(PortIO.T_TRGTDir == TP_Off) {
    return false;
  }

  TCHAR  szTmp[MAX_NMEA_LEN];

	int DegLat = wpt.Latitude;
	double MinLat = wpt.Latitude - DegLat;
	int DegLon = wpt.Longitude ;
	double MinLon = wpt.Longitude  - DegLon;

  char NoS = 'N';
  if((MinLat<0) || ((MinLat-DegLat==0) && (DegLat<0))) {
    NoS = 'S';
    DegLat *= -1; MinLat *= -1;
  }
  MinLat *= 60;


  char EoW = 'E';
  if((MinLon<0) || ((MinLon-DegLon==0) && (DegLon<0))) {
    EoW = 'W';
    DegLon *= -1; MinLon *= -1;
  }
  MinLon *=60;

  if( PortIO.T_TRGTDir  == TP_VTARG) {
    // PLXVTARG,KOLN,4628.80   ,N ,01541.167 ,E ,268.0
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PLXVTARG,%s,%02d%05.2f,%c,%03d%05.2f,%c,%i"),
              wpt.Name, DegLat, MinLat, NoS, DegLon, MinLon, EoW,
              (int)(wpt.Altitude + 0.5));

    SendNmea(d, szTmp);
    TestLog(TEXT("Send navigation Target LXNav: ($PLXVTARG) %s"), wpt.Name);
    SetDataText( d, _T_TRGT, wpt.Name);
  }
  else if( PortIO.T_TRGTDir  == TP_GPRMB) {
    // GPRMB,A,,,,H>TAKEOFF,5144.78,N,00616.70,E,,,A
    _sntprintf( szTmp,MAX_NMEA_LEN, TEXT("GPRMB,A,,,%s,%02d%05.2f,%c,%03d%05.2f,%c,,,,A"),
                wpt.Name, DegLat, MinLat, NoS, DegLon, MinLon, EoW);

    SendNmea(d, szTmp);
    TestLog(TEXT("Send navigation Target LXNav: ($GPRMB) %s"), wpt.Name);
    SetDataText( d, _T_TRGT, wpt.Name);
  }

  return true;
}



BOOL DevLXNanoIII::GPRMB(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if(PortIO.R_TRGTDir != TP_GPRMB) {
    return false;
  }

  DevLX::GPRMB( d,  sentence,  info);

  if(Values(d))
  {
    TCHAR  szTmp[MAX_NMEA_LEN];
    LockTaskData();
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%s ($GPRMB)"), WayPointList[RESWP_EXT_TARGET].Name);
    UnlockTaskData();
    SetDataText( d, _R_TRGT,  szTmp);
  }
  return false;
}


BOOL DevLXNanoIII::PLXVTARG(DeviceDescriptor_t* d, const char* sentence, NMEA_INFO* info)
{
  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if(PortIO.R_TRGTDir != TP_VTARG)
     return false;

  char szTmp[MAX_NMEA_LEN];

  double fTmp;

  ParToDouble(sentence, 1, &fTmp);
  double DegLat = (double)((int) (fTmp/100.0));
  double MinLat =  fTmp- (100.0*DegLat);
  double Latitude = DegLat+MinLat/60.0;

  NMEAParser::ExtractParameter(sentence,szTmp,2);
  if (szTmp[0]==_T('S')) {
    Latitude *= -1;
  }

  ParToDouble(sentence, 3, &fTmp);
  double DegLon =  (double) ((int) (fTmp/100.0));
  double MinLon =  fTmp- (100.0*DegLon);
  double Longitude = DegLon+MinLon/60.0;

  NMEAParser::ExtractParameter(sentence,szTmp,4);
  if (szTmp[0]==_T('W')) {
    Longitude *= -1;
  }

	
  NMEAParser::ExtractParameter(sentence,szTmp,0);
  tstring tname = from_unknown_charset(szTmp);

  double Altitude = RESWP_INVALIDNUMBER;
  if (!ParToDouble(sentence, 5, &Altitude)) {
    Altitude = RESWP_INVALIDNUMBER;
  }

  LockTaskData();
  {
    lk::snprintf(WayPointList[RESWP_EXT_TARGET].Name, TEXT("^%s"), tname.c_str());
    WayPointList[RESWP_EXT_TARGET].Latitude=Latitude;
    WayPointList[RESWP_EXT_TARGET].Longitude=Longitude;
    WayPointList[RESWP_EXT_TARGET].Altitude=Altitude;
    Alternate2 = RESWP_EXT_TARGET;
  }
  UnlockTaskData();

  if(Values(d))
  {
    tname += _T(" ($PLXVTARG)");
    SetDataText(d, _R_TRGT,  tname.c_str());
  }
  return false;
}



BOOL DevLXNanoIII::PLXVC_INFO(DeviceDescriptor_t* d, char** params, size_t size, NMEA_INFO* info) {
  if (size <= 2) {
    return FALSE;
  }

  if  (strcmp(params[2], "A") != 0) {  // no answer flag received
   return false;
  }

#ifdef DEVICE_SERIAL     
  if (size <= 5) {
    return FALSE;
  }
  d->SerialNumber = (int) StrToDouble(params[5],NULL);
#endif      

  if (size <= 7) {
    return FALSE;
  }

  double Batt = StrToDouble(params[7], nullptr);
  if(Values(d)) {
    TCHAR  szTmp[MAX_NMEA_LEN];
    _sntprintf(szTmp, MAX_NMEA_LEN, _T("%3.1fV ($PLXVC_INFO)"),Batt);
    SetDataText(d, _BAT1, szTmp);
  }

  const auto& Port = PortConfig[d->PortNumber];
  const auto& PortIO = Port.PortIO;

  if(IsDirInput(PortIO.BAT1Dir)) {
    info->ExtBatt1_Voltage = Batt;
  }

  if (size <= 8) {
    return FALSE;
  }

  Batt = StrToDouble(params[8], nullptr);
  if(Values(d)) {
    TCHAR szTmp[MAX_NMEA_LEN];
    _sntprintf(szTmp,MAX_NMEA_LEN, _T("%3.1fV (&PLXVC_INFO)"),Batt);
    SetDataText(d, _BAT2,  szTmp);
  }
  if(IsDirInput(PortIO.BAT2Dir)) {
    info->ExtBatt2_Voltage = Batt;
  }

  return true;
} // PLXVC

    void DevLXNanoIII::Device(DeviceDescriptor_t* d) 
	 {

		 if(d) 
			 StartupStore(TEXT("Config Device %i: %s"),d->PortNumber, d->Name);
		 else
			 StartupStore(TEXT("Remove Config Device %i: %s"),m_pDevice->PortNumber, m_pDevice->Name);
		 m_pDevice = d;
	 };