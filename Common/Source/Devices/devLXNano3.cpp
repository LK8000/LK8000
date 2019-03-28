/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

//__Version_1.0____________________________________________Vladimir Fux 12/2015_


//_____________________________________________________________________includes_

#include <time.h>
#include "externs.h"
#include "utils/stringext.h"
#include "devLXNano3.h"


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


BOOL m_bValues = false;

WndForm* wf = NULL;
extern bool UpdateQNH(const double newqnh);

#define PRPGRESS_DLG
#define BLOCK_SIZE 25

PDeviceDescriptor_t DevLXNanoIII::m_pDevice=NULL;
BOOL DevLXNanoIII::m_bLXNavS_series = false;
bool bIGC_Download = false;
static FILE *f= NULL;
uint uTimeout =0;
//______________________________________________________________________defines_

/// polynom for LX data CRC
#define LX_CRC_POLY 0x69

TCHAR m_Filename[200];
#define NANO_PROGRESS_DLG

int iRxUpdateTime=0;
int iNano3_RxUpdateTime=0;
double Nano3_oldMC = MACCREADY;
int Nano3_MacCreadyUpdateTimeout = 0;
int Nano3_BugsUpdateTimeout = 0;
int Nano3_BallastUpdateTimeout =0;
int iS_SeriesTimeout =0;
int iNano3_GPSBaudrate = 0;
int iNano3_PDABaudrate = 0;
//double fPolar_a=0.0, fPolar_b=0.0, fPolar_c=0.0, fVolume=0.0;
BOOL Nano3_bValid = false;
int Nano3_NMEAddCheckSumStrg( TCHAR szStrg[] );
BOOL Nano3_PutMacCready(PDeviceDescriptor_t d, double MacCready);
BOOL Nano3_PutBallast(PDeviceDescriptor_t d, double Ballast);
BOOL Nano3_PutBugs(PDeviceDescriptor_t d, double Bugs);

NMEA_INFO localVales;

BOOL IsDirInput( DataBiIoDir IODir)
{
  switch(IODir)
  {
    case BiDirOff  : return false; break;	 // OFF    no data exchange with this data (ignore data)
    case BiDirIn   : return true;  break;	 // IN     only reading of this data from external device
    case BiDirOut  : return false; break;	 // OUT    only sending this data to external device
    case BiDirInOut: return true;  break;	 // IN&OUT exchanga data from/to device in both directions (e.g. MC, Radio frequencies)
  }
  return false;
}

BOOL IsDirOutput( DataBiIoDir IODir)
{

  switch(IODir)
  {
    case BiDirOff  : return false; break;	 // OFF    no data exchange with this data (ignore data)
    case BiDirIn   : return false; break;	 // IN     only reading of this data from external device
    case BiDirOut  : return true ; break;	 // OUT    only sending this data to external device
    case BiDirInOut: return true ; break;	 // IN&OUT exchanga data from/to device in both directions (e.g. MC, Radio frequencies)
  }
  return false;
}


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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Registers device into device subsystem.
///
/// @retval true  when device has been registered successfully
/// @retval false device cannot be registered
///
//static
bool DevLXNanoIII::Register(){
  #ifdef UNIT_TESTS
    Wide2LxAsciiTest();
  #endif
  return(devRegister(GetName(),
    cap_gps | cap_baro_alt | cap_speed | cap_vario | cap_logger, Install));
} // Register()


BOOL PutMacCready(PDeviceDescriptor_t d, double MacCready);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Installs device specific handlers.
///
/// @param d  device descriptor to be installed
///
/// @retval true  when device has been installed successfully
/// @retval false device cannot be installed
///
//static
BOOL DevLXNanoIII::Install(PDeviceDescriptor_t d) {
  _tcscpy(d->Name, GetName());
  d->ParseNMEA    = ParseNMEA;
  d->PutMacCready = Nano3_PutMacCready;
  d->PutBugs      = Nano3_PutBugs;
  d->PutBallast   = Nano3_PutBallast;
  d->Open         = Open;
  d->Close        = NULL;
  d->Init         = NULL;
  d->LinkTimeout  = GetTrue;
  d->Declare      = DeclareTask;
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  d->Config       = Config;
  d->DirectLink   = Nano3_DirectLink;

  StartupStore(_T(". %s installed (platform=%s test=%u)%s"),
    GetName(),
    PlatfEndian::IsBE() ? _T("be") : _T("le"),
    PlatfEndian::To32BE(0x01000000), NEWLINE);
  return(true);
} // Install()





long Nano3Baudrate(int iIdx)
{
//	indexes are following:
//	enum { br4800=0, br9600, br19200, br38400, br57600,
//	br115200,br230400,br256000,br460800, br500k, br1M};
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




BOOL DevLXNanoIII::Nano3_DirectLink(PDeviceDescriptor_t d, BOOL bLinkEnable)
{
TCHAR  szTmp[MAX_NMEA_LEN];
#define CHANGE_DELAY 10

if(iNano3_GPSBaudrate ==0)
{
  _stprintf(szTmp, TEXT("PLXV0,BRGPS,R"));
  SendNmea(d,szTmp);
  Poco::Thread::sleep(CHANGE_DELAY);
  SendNmea(d, szTmp);
  Poco::Thread::sleep(CHANGE_DELAY);
}


  if(bLinkEnable)
  {
    #if TESTBENCH
    StartupStore(TEXT("LAXNav: enable LX LAXNav direct Link %s"), NEWLINE);
    #endif
    iNano3_PDABaudrate = d->Com->GetBaudrate();

    _stprintf(szTmp, TEXT("PLXV0,CONNECTION,W,DIRECT"));
    SendNmea(d, szTmp);
    Poco::Thread::sleep(CHANGE_DELAY);
    if(iNano3_PDABaudrate != iNano3_GPSBaudrate)
    {
      d->Com->SetBaudrate(iNano3_GPSBaudrate);
    #if TESTBENCH
      StartupStore(TEXT("LAXNav: Set Baudrate %i %s"),iNano3_GPSBaudrate, NEWLINE);
    #endif
      Poco::Thread::sleep(CHANGE_DELAY);
    }
    Poco::Thread::sleep(CHANGE_DELAY);
  }
  else
  {
    Poco::Thread::sleep(CHANGE_DELAY);

    if(iNano3_PDABaudrate != iNano3_GPSBaudrate)
    {
    #if TESTBENCH
      StartupStore(TEXT("LAXNav: Set Baudrate %i %s"),iNano3_PDABaudrate, NEWLINE);
    #endif
      d->Com->SetBaudrate(iNano3_PDABaudrate);
      Poco::Thread::sleep(CHANGE_DELAY);
    }

    #if TESTBENCH
    StartupStore(TEXT("LAXNav: Return from LAXNav link %s"), NEWLINE);
    #endif
    _stprintf(szTmp, TEXT("PLXV0,CONNECTION,W,VSEVEN"));
    SendNmea(d,szTmp);
    Poco::Thread::sleep(CHANGE_DELAY);

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
//static


BOOL DevLXNanoIII::ParseNMEA(PDeviceDescriptor_t d, TCHAR* sentence, NMEA_INFO* info)
{

  static int i=40;
  TCHAR  szTmp[MAX_NMEA_LEN];



  if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0)
  {
      Nano3_bValid = true;
    if(iNano3_RxUpdateTime > 0)
    {
      iNano3_RxUpdateTime--;
    }
    else
    {
      if(!IsDirOutput(PortIO[d->PortNumber].MCDir))
      {
      if(fabs(Nano3_oldMC - MACCREADY)> 0.005f)
      {
	Nano3_PutMacCready( d,  MACCREADY);
	Nano3_oldMC = MACCREADY;
	Nano3_MacCreadyUpdateTimeout = 2;
      }
      }
    }
  }

  PutTarget(d);

  /* configure LX after 10 GPS positions */
  if (_tcsncmp(_T("$GPGGA"), sentence, 6) == 0)
  {
    if(i++ > 10)
    {
      SetupLX_Sentence(d);
      i=0;
    }


      if(iS_SeriesTimeout-- < 0)
        S_Series(false);

    static int oldQFEOff =0;
    static int iOldQNH   =0;



    int iQNH = (int)(QNH*100.0);
    if(iQNH != iOldQNH)
    {
      iOldQNH = iQNH;
      _stprintf(szTmp, TEXT("PLXV0,QNH,W,%i"),(int)iQNH);
      SendNmea(d,szTmp);
    }


    int QFE = (int)QFEAltitudeOffset;
    if(QFE != oldQFEOff)
    {
       oldQFEOff = QFE;
      _stprintf(szTmp, TEXT("PLXV0,ELEVATION,W,%i"),(int)(QFEAltitudeOffset));
       SendNmea(d,szTmp);
    }
  }
#ifdef EEE
  if(iNano3_GPSBaudrate ==0)
  {
    _stprintf(szTmp, TEXT("PLXV0,BRGPS,R"));
    SendNmea(d,szTmp);
  }
#endif
  if (_tcsncmp(_T("$PLXVC"), sentence, 6) == 0)
  {
    return PLXVC( d,  sentence, info);
  }

    if (!NMEAParser::NMEAChecksum(sentence) || (info == NULL)){
      return FALSE;
    }
    if (_tcsncmp(_T("$PLXVF"), sentence, 6) == 0)
      return PLXVF(d, sentence + 7, info);
    else
      if (_tcsncmp(_T("$PLXVS"), sentence, 6) == 0)
        return PLXVS(d, sentence + 7, info);
      else
    	if (_tcsncmp(_T("$PLXV0"), sentence, 6) == 0)
    	  return PLXV0(d, sentence + 7, info);
    	else
          if (_tcsncmp(_T("$LXWP2"), sentence, 6) == 0)
            return LXWP2(d, sentence + 7, info);
          else
            if (_tcsncmp(_T("$LXWP0"), sentence, 6) == 0)
              return LXWP0(d, sentence + 7, info);
	    else
	      if(_tcsncmp(_T("$PLXVTARG"), sentence, 9) == 0)
		return PLXVTARG(d, sentence + 10, info);
	      else
		if (_tcsncmp(_T("$LXWP1"), sentence, 6) == 0)
		{
		  Nano3_bValid = true;
		  return LXWP1(d, sentence + 7, info);
		}


return false;
} // ParseNMEA()



BOOL DevLXNanoIII::Open( PDeviceDescriptor_t d) {


  m_pDevice = d;
  return TRUE;
}




CallBackTableEntry_t DevLXNanoIII::CallBackTable[]={

  EndCallBackEntry()
};




BOOL DevLXNanoIII::SetupLX_Sentence(PDeviceDescriptor_t d)
{
  if(S_Series())
  {
    SendNmea(d, TEXT("PLXV0,NMEARATE,W,2,5,10,10,1,5,5"));
  }
  return true;
}

BOOL DevLXNanoIII::ShowDataValue(WndForm* wf , PDeviceDescriptor_t d ,const TCHAR Select[],  const TCHAR ValueText[])
{
    if(m_pDevice != d) return false;
if(!m_bValues) return false;
WndProperty *wp;
wp = (WndProperty*)wf->FindByName(Select);
  if (wp) {
    DataField* dfe = wp->GetDataField();
    dfe->Clear();
    dfe->addEnumText(ValueText);
    wp->RefreshDisplay();
    return true;
  }
  return false;
}







BOOL DevLXNanoIII::ShowData(WndForm* wf ,PDeviceDescriptor_t d)
{
WndProperty *wp;
wp = (WndProperty*)wf->FindByName(TEXT("prpMCDir"));
int PortNum = d->PortNumber;
    StartupStore(_T(" ShowData : Device Number %i %s"),PortNum, NEWLINE);
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491));  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->addEnumText(MsgToken(2433)); // LKTOKEN  _@M2433_ "OUT"
    dfe->addEnumText(MsgToken(2434)); // LKTOKEN  _@M2434_ "IN & OUT"
    dfe->Set((uint) PortIO[PortNum].MCDir);
    wp->RefreshDisplay();
  }


  wp = (WndProperty*)wf->FindByName(TEXT("prpBUGDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491));  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->addEnumText(MsgToken(2433)); // LKTOKEN  _@M2433_ "OUT"
    dfe->addEnumText(MsgToken(2434)); // LKTOKEN  _@M2434_ "IN & OUT"
    dfe->Set((uint) PortIO[PortNum].BUGDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBALDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491));  // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->addEnumText(MsgToken(2433)); // LKTOKEN  _@M2433_ "OUT"
    dfe->addEnumText(MsgToken(2434)); // LKTOKEN  _@M2434_ "IN & OUT"
    dfe->Set((uint) PortIO[PortNum].BALDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpSTFDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->addEnumText(MsgToken(2433)); // LKTOKEN  _@M2433_ "OUT"
    dfe->Set((uint) PortIO[PortNum].STFDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpWINDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].WINDDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBARODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].BARODir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpSPEEDDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].SPEEDDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpVARIODir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].VARIODir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpTARGDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->addEnumText(MsgToken(2433)); // LKTOKEN  _@M2433_ "OUT"
    dfe->addEnumText(MsgToken(2434)); // LKTOKEN  _@M2434_ "IN & OUT"
    dfe->Set((uint) PortIO[PortNum].TARGETDir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpGFORCEDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].GFORCEDir);
    wp->RefreshDisplay();
  }
  wp = (WndProperty*)wf->FindByName(TEXT("prpOATDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].OATDir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBAT1Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].BAT1Dir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpBAT2Dir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
    dfe->Set((uint) PortIO[PortNum].BAT2Dir);
    wp->RefreshDisplay();
  }

  wp = (WndProperty*)wf->FindByName(TEXT("prpPOLARDir"));
  if (wp) {
    DataField* dfe = wp->GetDataField(); dfe->Clear();
    dfe->addEnumText(MsgToken(491)); // LKTOKEN  _@M491_ "OFF"
//    dfe->addEnumText(MsgToken(2432)); // LKTOKEN  _@M2432_ "IN"
//    dfe->addEnumText(MsgToken(2433)); // LKTOKEN  _@M2433_ "OUT"
    dfe->Set((uint) PortIO[PortNum].BAT2Dir);
    wp->RefreshDisplay();
  }
  return true;
}

BOOL DevLXNanoIII::Config(PDeviceDescriptor_t d){
  if(m_pDevice != d) {

 // return FALSE;
  }
  m_pDevice = d;


  wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_DEV_LXNAV_L : IDR_XML_DEV_LXNAV_P);
  if(wf) {

    WndButton *wBt = NULL;

    wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
    if(wBt){
      wBt->SetOnClickNotify(OnCloseClicked);
    }

    wBt = (WndButton *)wf->FindByName(TEXT("cmdCancel"));
    if(wBt){
      wBt->SetOnClickNotify(OnCancelClicked);
    }

    wBt = (WndButton *)wf->FindByName(TEXT("cmdIGCDownload"));
    if(wBt){
      wBt->SetOnClickNotify(OnIGCDownloadClicked);
    }


    wBt = (WndButton *)wf->FindByName(TEXT("cmdValues"));
    if(wBt){
      wBt->SetOnClickNotify(OnValuesClicked);
    }
    ShowData(wf, d);

    wf->ShowModal();
    m_bValues = false;
    delete wf;
    wf=NULL;
  }
  return TRUE;
}






//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Returns device name (max length is @c DEVNAMESIZE).
///
//static
const TCHAR* DevLXNanoIII::GetName() {
  return(_T("LX Nano 3"));
} // GetName()



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
BOOL DevLXNanoIII::DeclareTask(PDeviceDescriptor_t d,
  Declaration_t* lkDecl, unsigned errBufSize, TCHAR errBuf[]) {
  Decl  decl;
  Class lxClass;
  byte t_DD, t_MM, t_YY, t_hh, t_mm, t_ss;
  TCHAR buffer[128];

  // we will use text-defined class
  lxClass.SetName(lkDecl->CompetitionClass);
  // stop RX thread
  if (!StopRxThread(d, errBufSize, errBuf))
    return(false);

  // set new Rx timeout
  int  orgRxTimeout;
  bool status = SetRxTimeout(d, 2000, orgRxTimeout, errBufSize, errBuf);
  if (status) {
    ShowProgress(decl_enable);

    // Establish connecttion and check two-way communication...
    _stprintf(buffer, _T("PLXVC,INFO,R"));
  	StartupStore(_T(". NANO Decl: %s %s "),   buffer, NEWLINE);
    status = status && SendNmea(d, buffer, errBufSize, errBuf);
    if (status)
      status = status && ComExpect(d, "$PLXVC,INFO,A,", 256, NULL, errBufSize, errBuf);
    Poco::Thread::sleep(300);
    if (status) {

      // Create and send task declaration...
      ShowProgress(decl_send);
      // to declared Start, TPs, and Finish we will add Takeoff and Landing,
      // so maximum NB of declared TPs is Decl::max_wp_count - 2
      if (!CheckWPCount(*lkDecl,Decl::min_wp_count - 2, Decl::max_wp_count - 2, errBufSize, errBuf)){
           SetRxTimeout(d, orgRxTimeout,orgRxTimeout, status ? errBufSize : 0, errBuf);
           StartRxThread(d, status ? errBufSize : 0, errBuf);
        return(false);
      }
      int wpCount = lkDecl->num_waypoints;
      int totalLines = 6 + 1 + wpCount + 1;
      TCHAR DeclStrings[totalLines][256];
      INT i = 0;

      // Metadata
      _stprintf(DeclStrings[i++], TEXT("HFPLTPILOT:%s"), lkDecl->PilotName);
      _stprintf(DeclStrings[i++], TEXT("HFGTYGGLIDERTYPE:%s"), lkDecl->AircraftType);
      _stprintf(DeclStrings[i++], TEXT("HFGIDGLIDERID:%s"), lkDecl->AircraftRego);
      _stprintf(DeclStrings[i++], TEXT("HFCIDCOMPETITIONID:%s"), lkDecl->CompetitionID);
      _stprintf(DeclStrings[i++], TEXT("HFCCLCOMPETITIONCLASS:%s"), lkDecl->CompetitionClass);

      // "C" record, first line acording to IGC GNSS specification 3.6.1
      if (!GPS_INFO.NAVWarning && GPS_INFO.SatellitesUsed > 0 &&
        GPS_INFO.Day >= 1 && GPS_INFO.Day <= 31 && GPS_INFO.Month >= 1 && GPS_INFO.Month <= 12) {
        t_DD = GPS_INFO.Day;   t_MM = GPS_INFO.Month;   t_YY = GPS_INFO.Year % 100;
        t_hh = GPS_INFO.Hour;  t_mm = GPS_INFO.Minute;  t_ss = GPS_INFO.Second;
      } else { // use system time
        time_t sysTime = time(NULL);
        struct tm tm_temp = {0};
        struct tm* utc = gmtime_r(&sysTime, &tm_temp);
        t_DD = utc->tm_mday;   t_MM = utc->tm_mon + 1;  t_YY = utc->tm_year % 100;
        t_hh = utc->tm_hour;   t_mm = utc->tm_min;      t_ss = utc->tm_sec;
      }
      _stprintf(DeclStrings[i++], TEXT("C%02d%02d%02d%02d%02d%02d000000%04d%02d"),
	                // DD    MM    YY    HH    MM    SS (DD MM YY) IIII  TT
	                 t_DD, t_MM, t_YY, t_hh, t_mm, t_ss,              1, wpCount-2);
#ifdef TAKEOFF
	    // TakeOff point
      if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint)) {
        decl.WpFormat(DeclStrings[i++], &WayPointList[HomeWaypoint], Decl::tp_takeoff);
      } else {
        decl.WpFormat(DeclStrings[i++],NULL, Decl::tp_takeoff);
      }
#endif
      // TurnPoints
      for (int ii = 0; ii < wpCount; ii++) {
        decl.WpFormat(DeclStrings[i++], lkDecl->waypoint[ii], Decl::tp_regular);
      }

#ifdef LANDING
      // Landing point
      if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint)) {
        decl.WpFormat(DeclStrings[i++], &WayPointList[HomeWaypoint], Decl::tp_landing);
      } else {
        decl.WpFormat(DeclStrings[i++],NULL, Decl::tp_landing);
      }
#endif

      // Send complete declaration to logger
      for (int ii = 0; ii < i ; ii++){
        if (status)
          {
          status = status && DevLXNanoIII::SendDecl(d, ii+1, totalLines,
                                   DeclStrings[ii], errBufSize, errBuf);
        StartupStore(_T(". NANO Decl: %s %s "),   DeclStrings[ii], NEWLINE);
        Poco::Thread::sleep(50);
          }
      }
    }
    ShowProgress(decl_disable);
  }
  Poco::Thread::sleep(300);

  // restore Rx timeout (we must try that always; don't overwrite error descr)
  status = status && SetRxTimeout(d, orgRxTimeout,
           orgRxTimeout, status ? errBufSize : 0, errBuf);

  // restart RX thread (we must try that always; don't overwrite error descr)
  status = status && StartRxThread(d, status ? errBufSize : 0, errBuf);
  return(status);
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
bool DevLXNanoIII::Wide2LxAscii(const TCHAR* input, int outSize, char* output){
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
//   DevLXNanoIII::Decl
//
// *****************************************************************************
// #############################################################################


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Constructor - sets all data to 0.
///
DevLXNanoIII::Decl::Decl(){
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
/// Constructor - sets all data to 0.
///
DevLXNanoIII::Class::Class(){
  memset(this, 0, sizeof(*this));
} // Class()



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Sets the value of @c name member.
///
/// @param text  string to be set (will be converted into ASCII)
///
void DevLXNanoIII::Class::SetName(const TCHAR* text){
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
bool DevLXNanoIII::SendNmea(PDeviceDescriptor_t d, const TCHAR buf[], unsigned errBufSize, TCHAR errBuf[]){
  unsigned char chksum = 0;
  unsigned int i;
  char asciibuf[256];
  DevLXNanoIII::Wide2LxAscii(buf, 256, asciibuf);
  for(i = 0; i < strlen(asciibuf); i++) chksum ^= asciibuf[i];
  //sprintf(asciibuf, "%s*%02X\r\n", asciibuf, chksum);
  if (!ComWrite(d, '$', errBufSize, errBuf))  return (false);
  for(i = 0; i < strlen(asciibuf); i++)
    if (!ComWrite(d, asciibuf[i], errBufSize, errBuf)) return (false);
  sprintf(asciibuf, "*%02X\r\n",chksum);
  for(i = 0; i < strlen(asciibuf); i++)
    if (!ComWrite(d, asciibuf[i], errBufSize, errBuf)) return (false);

//  StartupStore(_T("request: $%s*%02X %s "),   buf,chksum, NEWLINE);

  return (true);
} // SendNmea()


bool DevLXNanoIII::SendNmea(PDeviceDescriptor_t d, const TCHAR buf[]){
TCHAR errBuf[10]= _T("");
TCHAR errBufSize=10;
  DevLXNanoIII::SendNmea(d,  buf,errBufSize,errBuf);
  if(_tcslen (errBuf) > 1)
  {
    DoStatusMessage(errBuf);
    return false;
  }
 // StartupStore(_T(" Nano3 SenadNmea %s %s"),buf, NEWLINE);
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
bool DevLXNanoIII::SendDecl(PDeviceDescriptor_t d, unsigned row, unsigned n_rows,
                   TCHAR content[], unsigned errBufSize, TCHAR errBuf[]){
  TCHAR buffer[MAX_NMEA_LEN];
  char retstr[20];
  _stprintf(buffer, TEXT("PLXVC,DECL,W,%u,%u,%s"), row, n_rows, content);
  bool status = DevLXNanoIII::SendNmea(d, buffer, errBufSize, errBuf); 
  if (status) {
    sprintf(retstr, "$PLXVC,DECL,C,%u", row);
    status = status && ComExpect(d, retstr, 512, NULL, errBufSize, errBuf);
  }
  return status;

} // SendDecl()



void DevLXNanoIII::OnCancelClicked(WndButton* pWnd){
  if(pWnd) {
      WndForm * wf = pWnd->GetParentWndForm();
      StartupStore(_T(" Nano3 OnCancel%s"), NEWLINE);
      wf->SetModalResult(mrCancel);
  }
}


void DevLXNanoIII::GetDirections(WndButton* pWnd)
{
if(pWnd) {
  WndForm * wf = pWnd->GetParentWndForm();
  if(wf) {
	int PortNum = m_pDevice->PortNumber;
	StartupStore(_T(" On Close : Device Number %i %s"),PortNum, NEWLINE);
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
    wp = (WndProperty*)wf->FindByName(TEXT("prpTARGDir"));
    if (wp) {
	DataField* dfe = wp->GetDataField();
	PortIO[PortNum].TARGETDir =  (DataBiIoDir) dfe->GetAsInteger();
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
  }
}
}

void DevLXNanoIII::OnCloseClicked(WndButton* pWnd)
{
  if(!m_bValues)
    GetDirections(pWnd);
  wf->SetModalResult(mrOK);
}





void DevLXNanoIII::OnValuesClicked(WndButton* pWnd) {

  m_bValues = !m_bValues;

  WndButton* wBt = (WndButton *)wf->FindByName(TEXT("cmdValues"));
  if(wBt){
     if( !m_bValues)
     {
       wBt->SetCaption(MsgToken(2447));// _@M2447_ "Values"
     }
     else
     {       GetDirections(pWnd);
       wBt->SetCaption(MsgToken(2448));//  _@M2448_ "Direction"
     }
  }
  if(m_bValues)
  {
    ShowDataValue( wf ,m_pDevice, _T("prpMCDir"),    (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpBUGDir"),   (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpBALDir"),   (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpSTFDir"),   (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpWINDDir"),  (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpBARODir"),  (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpVARIODir"), (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpSPEEDDir"), (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpTARGDir"),  (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpGFORCEDir"),(_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpOATDir"),   (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpBAT1Dir"),  (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpBAT2Dir"),  (_T("")));
    ShowDataValue( wf ,m_pDevice, _T("prpPOLARDir"), (_T("")));
  }
  else
    ShowData(wf, m_pDevice);

};

void DevLXNanoIII::OnIGCDownloadClicked(WndButton* pWnd) {
(void)pWnd;
LockFlightData();
bool bInFlight    = CALCULATED_INFO.Flying;
UnlockFlightData();

  if(bInFlight)	{
    MessageBoxX(MsgToken(2418), MsgToken(2397), mbOk);
    return;
  }
  TCHAR szTmp[MAX_NMEA_LEN];

  _stprintf(szTmp, _T("PLXVC,LOGBOOKSIZE,R"));
  TCHAR errBuf[10];
  TCHAR errBufSize=10;

  SendNmea(m_pDevice, szTmp, errBufSize, errBuf);

  if(m_pDevice) {
      dlgLX_IGCSelectListShowModal(m_pDevice);
  }
}




bool  DevLXNanoIII::OnStartIGC_FileRead(TCHAR Filename[]) {
TCHAR szTmp[MAX_NMEA_LEN];
TCHAR errBuf[10];
TCHAR errBufSize=10;
TCHAR IGCFilename[MAX_PATH];
LocalPath(IGCFilename, _T(LKD_LOGS), Filename);

  f = _tfopen( IGCFilename, TEXT("w"));
  if(f == NULL)   return false;
  // SendNmea(m_pDevice, _T("PLXVC,KEEP_ALIVE,W"), errBufSize, errBuf);
  StartupStore(_T(" ******* NANO3  IGC Download START ***** %s") , NEWLINE);
  _stprintf(szTmp, _T("PLXVC,FLIGHT,R,%s,1,%u"),Filename,BLOCK_SIZE+1);
  _stprintf(m_Filename, _T("%s"),Filename);
  SendNmea(m_pDevice, szTmp, errBufSize, errBuf);
  StartupStore(_T("> %s %s") ,szTmp, NEWLINE);
  bIGC_Download = true;
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
  bool bWasInProgress = bIGC_Download ;
  bIGC_Download = false;
#ifdef  NANO_PROGRESS_DLG
  CloseIGCProgressDialog();
#endif
  return bWasInProgress;
}


BOOL DevLXNanoIII::Close (PDeviceDescriptor_t d) {
  m_pDevice = NULL;
  return TRUE;
}




BOOL DevLXNanoIII::PLXVC(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR errBuf[10];
uint errBufSize=10;
static uint CurLine=0;   // musat be static to remember the last correct line in case of checksum error
BOOL RepeatRequest = false;

if (!NMEAParser::NMEAChecksum(sentence) /*|| (info == NULL)*/){
    StartupStore(_T("NANO3: Checksum Error %s %s") ,sentence, NEWLINE);
    RepeatRequest = true;
}


if(bIGC_Download) // IGC Download in progress?
{
  if(uTimeout++ > 20)
  {
    StartupStore(_T("NANO3: TIMEOUT while IGC File Download!!!%s") , NEWLINE);
    uTimeout =0;
    RepeatRequest = true;
  }
  if(  RepeatRequest == true)
  {
     TCHAR szTmp[MAX_NMEA_LEN];
     RepeatRequest = false;
     _sntprintf(szTmp,MAX_NMEA_LEN, _T("PLXVC,FLIGHT,R,%s,%u,%u"),m_Filename,CurLine+1,CurLine+BLOCK_SIZE+1);
     bIGC_Download = true;
     uTimeout      = 0;
     SendNmea(m_pDevice, szTmp, errBufSize, errBuf);
     StartupStore(_T("NANO3 request repeat: %s %s") ,szTmp, NEWLINE);
     return false;
  }
}

if (_tcsncmp(_T("$PLXVC"), sentence, 6) == 0)
{
  TCHAR Par[10][MAX_NMEA_LEN];

  for(uint i=0; i < 10 ; i++)
    NMEAParser::ExtractParameter(sentence,Par[i],i);

  if (_tcsncmp(_T("INFO"), Par[1],4) == 0)
    return PLXVC_INFO(d,sentence,info);

  if (_tcsncmp(_T("LOGBOOKSIZE"), Par[1],11) == 0)
  {
    _sntprintf(Par[0],MAX_NMEA_LEN, _T("PLXVC,LOGBOOK,R,1,%u"), atoi((char*)Par[3])+1);
    SendNmea(d, Par[0], errBufSize, errBuf);
  }
  else
    if (_tcsncmp(_T("LOGBOOK"), Par[1],7) == 0)  // PLXVC but not declaration = IGC File transfer
    {
      TCHAR Line[2][MAX_NMEA_LEN];
      _sntprintf( Line[0],MAX_NMEA_LEN, _T("%s"),Par[5]);
      _sntprintf( Line[1],MAX_NMEA_LEN, _T("%s (%s-%s) %ukB"), Par[6] ,Par[7] ,Par[8], atoi((char*)Par[9])/1024);
      AddElement(Line[0], Line[1]);
    }
    else
    {
//	StopRxThread(d, errBufSize, errBuf);
      if (bIGC_Download &&(_tcsncmp(_T("FLIGHT"), Par[1],6) == 0))
      {
	uTimeout=0;
	if(_tcslen( Par[3]) > 0)
	  StartupStore(_T(">>>> %s %s") ,sentence, NEWLINE);

	_ftprintf(f,_T("%s\n"),Par[6]);
       CurLine = atoi((char*)Par[4]);
       uint TotalLines = atoi((char*)Par[5]);
       if((CurLine % (BLOCK_SIZE)==0))
       {
	 uint uPercent = 0;
	 if(TotalLines > 0)
	   uPercent = (CurLine*100) / TotalLines;
	   _sntprintf(Par[1],MAX_NMEA_LEN, _T("%s: %u%% %s ..."),MsgToken(2400), uPercent,m_Filename); // _@M2400_ "Downloading"
#ifdef NANO_PROGRESS_DLG
	 IGCProgressDialogText(Par[1]);
#endif
	 _sntprintf(Par[0], MAX_NMEA_LEN, _T("PLXVC,FLIGHT,R,%s,%u,%u"),m_Filename,CurLine+1,CurLine+BLOCK_SIZE+1);
	 bIGC_Download = true;
	 uTimeout      = 0;
	 SendNmea(m_pDevice, Par[0], errBufSize, errBuf);
       }
       if(CurLine == TotalLines)  // reach end of file?
       {
	 if(f != NULL) { fclose(f); f= NULL; }
	 StartupStore(_T(" ******* NANO3  IGC Download END ***** %s") , NEWLINE);

	 bIGC_Download = false;
//	 MessageBoxX(  MsgToken(2406), MsgToken(2398), mbOk) ;  // // _@M2398_ "IGC Download"

	 CloseIGCProgressDialog();
//	 StartRxThread(d,  errBufSize , errBuf);
       }
     }
   }
 }
 return(true);
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
BOOL DevLXNanoIII::LXWP0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
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

  double fDir,fSpeed;

  if (ParToDouble(sentence, 1, &fSpeed))
  {

    if(m_bValues)
    { TCHAR szTmp[MAX_NMEA_LEN];
      _stprintf(szTmp, _T("%5.1fkm/h ($LXWP0)"),fSpeed);
      ShowDataValue( wf ,d,_T("prpSPEEDDir"),   szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].SPEEDDir  ))
    {
      info->Speed = fSpeed;
      info->AirspeedAvailable = TRUE;
    }
  }

  if (ParToDouble(sentence, 2, &fSpeed))
  {
    if(m_bValues)
    { TCHAR szTmp[MAX_NMEA_LEN];
      _stprintf(szTmp, _T("%5.1fm ($LXWP0)"),fSpeed);
      ShowDataValue( wf ,d,_T("prpBARODir"),   szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].BARODir  ))
    {
      info->Altitude = fSpeed;
      info->BaroAltitudeAvailable = TRUE;
    }
  }

  if (ParToDouble(sentence, 3, &fSpeed))
  {
    if(m_bValues)
    { TCHAR szTmp[MAX_NMEA_LEN];
      _stprintf(szTmp, _T("%5.1fm ($LXWP0)"),fSpeed/TOKPH);
      ShowDataValue( wf ,d, _T("prpVARIODir"),   szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].VARIODir  ))
    {
      info->Vario = fSpeed;
      info->VarioAvailable = TRUE;
      TriggerVarioUpdate();
    }
  }

  if (ParToDouble(sentence, 10, &fDir) &&
	ParToDouble(sentence, 11, &fSpeed))
  {

    if(m_bValues)
    { TCHAR szTmp[MAX_NMEA_LEN];
      _stprintf(szTmp, _T("%5.1fkm/h %3.0f° ($LXWP0)"),fSpeed,fDir);
      ShowDataValue( wf ,d, _T("prpWINDDir"),   szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].WINDDir  ))
    {
      info->ExternalWindDirection = fDir;
      info->ExternalWindSpeed =  fSpeed/TOKPH;
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
BOOL DevLXNanoIII::LXWP1(PDeviceDescriptor_t d, const TCHAR* String, NMEA_INFO* pGPS)
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
//	$LXWP1,LX5000IGC-2,15862,11.1 ,2.0*4A
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
      _stprintf(d->Name, _T("%s"),ctemp);
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
BOOL DevLXNanoIII::LXWP2(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO*)
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
TCHAR szTmp[MAX_NMEA_LEN];

  if(Nano3_MacCreadyUpdateTimeout > 0)
  {
    Nano3_MacCreadyUpdateTimeout--;
  }
  else
  {
    if (ParToDouble(sentence, 0, &fTmp))
    {
      iTmp =(int) (fTmp*100.0+0.5f);
      fTmp = (double)(iTmp)/100.0;
      if(m_bValues)
      {
	_stprintf(szTmp, _T("%5.2fm/s ($LXWP2)"),fTmp);
	ShowDataValue( wf ,d,_T("prpMCDir"),   szTmp);
      }
      Nano3_bValid = true;
      if(IsDirInput(PortIO[d->PortNumber].MCDir))
      {
	if(fabs(MACCREADY - fTmp)> 0.001)
	{
	  CheckSetMACCREADY(fTmp);
	  Nano3_MacCreadyUpdateTimeout =5;
	}
      }
    }
  }



  if(Nano3_BallastUpdateTimeout > 0)
  {
     Nano3_BallastUpdateTimeout--;
  }
  else
  {
    if (ParToDouble(sentence, 1, &fTmp))
    {
      double fBALPerc = CalculateBalastFromLX(fTmp);
      if(m_bValues)
      {
	_stprintf(szTmp, _T("%5.2f = %3.0f%% ($LXWP2)"),fTmp,(fBALPerc*100.0));
	ShowDataValue( wf ,d,_T("prpBALDir"),  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].BALDir  ))
      {
	if(fabs(fBALPerc- BALLAST) > 0.01 )
	{
	  CheckSetBallast(fBALPerc);
	}
      }
    }
  }


  if(Nano3_BugsUpdateTimeout > 0)
  {
      Nano3_BugsUpdateTimeout--;
  }
  else
  {
    if(ParToDouble(sentence, 2, &fTmp))
    {
      double tempBugs = 100.0-(fTmp);
      tempBugs /=  100.0;
      if(m_bValues)
      {
	_stprintf(szTmp, _T("%3.0f%% ($LXWP2)"),fTmp*100.0);
	ShowDataValue( wf ,d,_T("prpBUGDir"),  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].BUGDir ))
      {
	if(  fabs(fTmp -BUGS) >= 0.01)
	{
	  CheckSetBugs(fTmp);
	  Nano3_BugsUpdateTimeout = 5;
	}
      }
    }
  }


 double fa,fb,fc;
     if(ParToDouble(sentence, 3, &fa))
       if(ParToDouble(sentence, 4, &fb))
	 if(ParToDouble(sentence, 5, &fc))
	 {
	    if(m_bValues)
	    {
	      _stprintf(szTmp, _T("a:%5.3f b:%5.3f c:%5.3f ($LXWP2)"),fa,fb,fc);
	      ShowDataValue( wf ,d, _T("prpPOLARDir"),  szTmp);
	 //     _stprintf(szTmp, _T("a:%5.3f b:%5.3f c:%5.3f ($LXWP2)"),   POLAR[POLAR_A],   POLAR[POLAR_B],   POLAR[POLAR_C]);
	//      ShowDataValue( wf ,_T("prpBAT2Dir"),  szTmp);
	    }
	    /*
	    if(IsDirInput(PortIO[d->PortNumber].POLARDir ))
	    {
	      POLAR[POLAR_A] = fa;
	      POLAR[POLAR_B] = fb;
	      POLAR[POLAR_C] = fc;
	    }
	    */
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
BOOL DevLXNanoIII::LXWP3(PDeviceDescriptor_t, const TCHAR*, NMEA_INFO*)
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




BOOL DevLXNanoIII::LXWP4(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
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



BOOL DevLXNanoIII::PLXVF(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR szTmp[MAX_NMEA_LEN];
double alt=0, airspeed=0;
S_Series(true);
  if((m_bValues) || (IsDirInput(PortIO[d->PortNumber].GFORCEDir)))
  {
    double fX,fY,fZ;
    if(ParToDouble(sentence, 1, &fX) &&
      ParToDouble(sentence, 2, &fY) &&
      ParToDouble(sentence, 3, &fZ))
    {
      if(m_bValues)
      {
	_stprintf(szTmp, _T("%5.2f %5.2f %5.2f ($PLXVF)"),fZ,fY,fX);
	ShowDataValue( wf ,d, _T("prpGFORCEDir"),  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].GFORCEDir))
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
    if(m_bValues)
    {
      _stprintf(szTmp, _T("%3.0fkm/h ($PLXVF)"),airspeed*TOKPH);
      ShowDataValue( wf ,d,_T("prpSPEEDDir"),  szTmp);
    }
//	airspeed = 135.0/TOKPH;
    if(IsDirInput(PortIO[d->PortNumber].SPEEDDir ))
    {
      info->IndicatedAirspeed = airspeed;
      info->AirspeedAvailable = TRUE;
    }
  }


  if (ParToDouble(sentence, 6, &alt))
  {
    if(m_bValues)
    {
      _stprintf(szTmp, _T("%5.0fm ($PLXVF)"),alt);
      ShowDataValue( wf ,d,_T("prpBARODir"),  szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].BARODir))
    {
      UpdateBaroSource( info, 0, d, QNEAltitudeToQNHAltitude(alt));
      if (  info->IndicatedAirspeed>0)
	info->TrueAirspeed =  airspeed * AirDensityRatio(alt);
    }
  }


  if (ParToDouble(sentence, 4, &alt))
  {
    if(m_bValues)
    {
      _stprintf(szTmp, _T("%5.2fm/s ($PLXVF)"),alt);
      ShowDataValue( wf ,d,_T("prpVARIODir"),  szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].VARIODir))
    {
      info->Vario = alt;
      info->VarioAvailable = TRUE;
      TriggerVarioUpdate();
    }
  }


// Get STF switch
  double fTmp;
  if (ParToDouble(sentence, 7, &fTmp))
  {
    int  iTmp = (int)(fTmp+0.1);
    if(m_bValues)
    {
      if(iTmp == 1)
	ShowDataValue( wf ,d,_T("prpSTFDir"),  _T("STF ($PLXVF)"));
      else
	ShowDataValue( wf ,d,_T("prpSTFDir"),  _T("VARIO ($PLXVF)"));
    }

    static int  iOldVarioSwitch=0;
    if(IsDirInput(PortIO[d->PortNumber].STFDir))
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


BOOL DevLXNanoIII::PLXVS(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
double Batt;
double OAT;
TCHAR szTmp[MAX_NMEA_LEN];
S_Series(true);
iS_SeriesTimeout = 30;
  if (ParToDouble(sentence, 0, &OAT))
  {
    if(m_bValues)
    {
      _stprintf(szTmp, _T("%3.1f°C ($PLXVS)"),OAT);
      ShowDataValue( wf ,d,_T("prpOATDir"),  szTmp);
    }

    if(IsDirInput(PortIO[d->PortNumber].OATDir))
    {
      info->OutsideAirTemperature = OAT;
      info->TemperatureAvailable  = TRUE;
    }
  }

  if (ParToDouble(sentence, 2, &Batt))
  {
    if(m_bValues)
    {
      _stprintf(szTmp, _T("%3.1fV ($PLXVS)"),Batt);
      ShowDataValue( wf ,d,_T("prpBAT1Dir"),  szTmp);
    }
    if(IsDirInput(PortIO[d->PortNumber].BAT1Dir))
    {
      info->ExtBatt1_Voltage = Batt;
    }
  }

  return(true);
} // PLXVS()


BOOL DevLXNanoIII::PLXV0(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR  szTmp1[MAX_NMEA_LEN], szTmp2[MAX_NMEA_LEN];

  NMEAParser::ExtractParameter(sentence,szTmp1,1);
  if  (_tcscmp(szTmp1,_T("W"))!=0)  // no write flag received
    return false;

  NMEAParser::ExtractParameter(sentence,szTmp1,0);
  if  (_tcscmp(szTmp1,_T("BRGPS"))==0)
  {
    NMEAParser::ExtractParameter(sentence,szTmp2,2);
    iNano3_GPSBaudrate = Nano3Baudrate( (int)( (StrToDouble(szTmp2,NULL))+0.1 ) );
    return true;
  }

  if (_tcscmp(szTmp1,_T("BRPDA"))==0)
  {
    NMEAParser::ExtractParameter(sentence,szTmp2,2);
    iNano3_PDABaudrate = Nano3Baudrate( (int) StrToDouble(szTmp2,NULL));
    return true;
  }

  NMEAParser::ExtractParameter(sentence,szTmp1,0);
  if  (_tcscmp(szTmp1,_T("QNH"))==0)
  {
    NMEAParser::ExtractParameter(sentence,szTmp2,2);
    UpdateQNH((StrToDouble(szTmp2,NULL))/100.0);
    StartupStore(_T("Nano3 QNH: %s"),szTmp2);
    return true;
  }

  /****************************************************************
   * MacCready
   ****************************************************************/
  if (_tcscmp(szTmp1,_T("MC"))==0)
  {
    if(iRxUpdateTime > 0)
    {
      iRxUpdateTime--;
      return false;
    }
    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
    {
      if(m_bValues)
      {
	TCHAR szTmp[20];
	_stprintf(szTmp, _T("%5.2f PLXV0"),fTmp);
	ShowDataValue( wf ,d,_T("prpMCDir"),  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].MCDir))
      {
	if(fabs(MACCREADY - fTmp)> 0.001)
	{
	  CheckSetMACCREADY(fTmp);
	  iRxUpdateTime =5;
	  StartupStore(_T("Nano3 MC: %5.2f"),fTmp);
	  return true;
	}
      }
    }
    return false;
  }

  /****************************************************************
   * BALlast
   ****************************************************************/
  if (_tcscmp(szTmp1,_T("BAL"))==0)
  {
    if(Nano3_BallastUpdateTimeout > 0)
    {
	Nano3_BallastUpdateTimeout--;
      return false;
    }

    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
    {
      double fNewBal = CalculateBalastFromLX(fTmp);
      if(m_bValues)
      {
	TCHAR szTmp[20];
	_stprintf(szTmp, _T("%2.1f %3.0f PLXV0"),fTmp, fNewBal);
	ShowDataValue( wf ,d,_T("prpBALDir"),  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].BALDir))
      {
	if(fabs(BALLAST - fNewBal)> 0.001)
	{
	  CheckSetBallast(fNewBal);
	  Nano3_BallastUpdateTimeout =5;
	  return true;
	}
	StartupStore(_T("Nano3 BAL: %5.2f"),fTmp);
      }
    }
    return false;
  }
  /****************************************************************
   * BUGs
   ****************************************************************/
  if (_tcscmp(szTmp1,_T("BUGS"))==0)
  {
      if(Nano3_BugsUpdateTimeout > 0)
      {
	Nano3_BugsUpdateTimeout--;
        return false;
      }
      double fTmp;
      if(ParToDouble(sentence, 2, &fTmp))
      {
	if(m_bValues)
	{
	  TCHAR szTmp[20];
	  _stprintf(szTmp, _T("%3.0f%% ($PLXV0)"),fTmp);
	  ShowDataValue( wf ,d,_T("prpBUGDir"),  szTmp);
	}
	if(IsDirInput(PortIO[d->PortNumber].BUGDir))
	{
	  if(fabs(BUGS - fTmp)> 0.001)
	  {
	    Nano3_BugsUpdateTimeout =5;
	    StartupStore(_T("Nano3 BUG: %5.2f"),fTmp);
	    return true;
	  }
	}
      }
      return false;
  }


  if (_tcscmp(szTmp1,_T("VOL"))==0)
  {
    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
      StartupStore(_T("Nano3 VOL: %i"),(int)fTmp);
    return true;
  }

  /****************************************************************
   * POLAR
   ****************************************************************/
  if (_tcscmp(szTmp1,_T("POLAR"))==0)
  {
    double fLoad,fWeight,fMaxW, fEmptyW,fPilotW, fa,fb,fc;
    if(
       (ParToDouble(sentence, 2, &fa)) &&
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

  if (_tcscmp(szTmp1,_T("CONNECTION"))==0)
  {
    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
      StartupStore(_T("Nano3 CONNECTION: %i"),(int)fTmp);
    return true;
  }

  if (_tcscmp(szTmp1,_T("NMEARATE"))==0)
  {
    double fTmp;
    if(ParToDouble(sentence, 2, &fTmp))
      StartupStore(_T("Nano3 NMEARATE: %i"),(int)fTmp);
    return true;
  }

  return(false);
} // PLXV0()




BOOL Nano3_PutMacCready(PDeviceDescriptor_t d, double MacCready){
TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;
  if(Nano3_bValid == false) return false;
  if(!IsDirOutput(PortIO[d->PortNumber].MCDir)) return false;
  if(DevLXNanoIII::S_Series())
    _stprintf(szTmp, TEXT("PLXV0,MC,W,%3.1f"), MacCready );
  else
    _stprintf(szTmp, TEXT("PFLX2,%.2f,,,,,"), MacCready);
    DevLXNanoIII::SendNmea(d,szTmp);
  Nano3_MacCreadyUpdateTimeout = 5;
return true;

}


BOOL Nano3_PutBallast(PDeviceDescriptor_t d, double Ballast){
TCHAR  szTmp[MAX_NMEA_LEN];
  if(!d)  return false;
  if(Nano3_bValid == false) return false;
  if(!IsDirOutput(PortIO[d->PortNumber].BALDir)) return false;



  double BallastFact =  CalculateLXBalastFactor(Ballast);
  if(DevLXNanoIII::S_Series())
    _stprintf(szTmp, TEXT("PLXV0,BAL,W,%4.2f"),BallastFact);
  else
    _stprintf(szTmp, TEXT("PFLX2,,%.2f,,,,"), BallastFact);
  DevLXNanoIII::SendNmea(d,szTmp);

  Nano3_BallastUpdateTimeout =10;

return(TRUE);
}


BOOL Nano3_PutBugs(PDeviceDescriptor_t d, double Bugs){
  TCHAR  szTmp[MAX_NMEA_LEN];

  if(!d)  return false;
  if(Nano3_bValid == false) return false;
  if(!IsDirOutput(PortIO[d->PortNumber].BUGDir)) return false;
  double LXBugs = CalculateLXBugs( Bugs);

  if(DevLXNanoIII::S_Series())
    _sntprintf(szTmp,MAX_NMEA_LEN, TEXT("PLXV0,BUGS,W,%3.1f"),LXBugs);
  else
    _stprintf(szTmp, TEXT("PFLX2,,,%d,,,"), (int)LXBugs);

  DevLXNanoIII::SendNmea(d,szTmp);
  Nano3_BugsUpdateTimeout = 5;
return(TRUE);
}


BOOL DevLXNanoIII::PutTarget(PDeviceDescriptor_t d)
{
static int old_overindex = -99;
static int old_overmode = -99;
int overindex = GetOvertargetIndex();
int overmode  = OvertargetMode;

if(!IsDirOutput(PortIO[d->PortNumber].TARGETDir)) return false;

bool bTaskpresent = false; //ValidTaskPoint(0);
if(bTaskpresent)
  if(ValidTaskPoint(ActiveTaskPoint))
    overindex = Task[ActiveTaskPoint].Index;



if(overindex < 0)               /* valid waypoint ?*/
  return -1;
if(overindex == old_overindex)  /* same as before */
  if(overmode == old_overmode)  /* and same mode  */
    return 0;


old_overindex = overindex;
old_overmode  = overmode;
TCHAR  szTmp[512];


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


  if(bTaskpresent)
  {
    _stprintf( szTmp, TEXT("PLXVTARG,%s%s,%02d%05.2f,%c,%03d%05.2f,%c,%i"),
		      MsgToken(1323), // LKTOKEN _@M1323_ "T>"
		      WayPointList[overindex].Name,
		      DegLat, MinLat, NoS, DegLon, MinLon, EoW,
		      (int) (WayPointList[overindex].Altitude +0.5)
             );
    DevLXNanoIII::SendNmea(d,szTmp);

#if TESTBENCH
    StartupStore(TEXT("LXNav: %s"),szTmp);
#endif
  }
  else
  {
    _stprintf( szTmp,  TEXT("PLXVTARG,%s%s,%02d%05.2f,%c,%03d%05.2f,%c,%i"),
		GetOvertargetHeader(),
		WayPointList[overindex].Name,
		DegLat, MinLat, NoS, DegLon, MinLon, EoW,
		(int)(WayPointList[overindex].Altitude +0.5)
	     );
    DevLXNanoIII::SendNmea(d,szTmp);

#if TESTBENCH
    StartupStore(TEXT("LXNav: %s"),szTmp);
#endif
  }
return(true);
}

BOOL DevLXNanoIII::PLXVTARG(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR  szTmp[MAX_NMEA_LEN];
TCHAR  szTmp2[MAX_NMEA_LEN];

NMEAParser::ExtractParameter(sentence,szTmp,0);

if(m_bValues)
{
   _sntprintf(szTmp2,MAX_NMEA_LEN, TEXT("%s ($PLXVTARG)"),szTmp);
  ShowDataValue( wf ,d,_T("prpTARGDir"),  szTmp);
}

if(!IsDirInput(PortIO[d->PortNumber].TARGETDir)) return false;

_tcscpy(WayPointList[RESWP_EXT_TARGET].Name, _T("^") );
  _tcscat(WayPointList[RESWP_EXT_TARGET].Name, szTmp );



double fTmp;
  ParToDouble(sentence, 1, &fTmp);
  double DegLat = (double)((int) (fTmp/100.0));
  double MinLat =  fTmp- (100.0*DegLat);
  double Latitude = DegLat+MinLat/60.0;
  TCHAR NoS;
  NMEAParser::ExtractParameter(sentence,&NoS,2);
  if (NoS==_T('S')) {
	Latitude *= -1;
  }

  ParToDouble(sentence, 3, &fTmp);
  double DegLon =  (double) ((int) (fTmp/100.0));
  double MinLon =  fTmp- (100.0*DegLon);
  double Longitude = DegLon+MinLon/60.0;
  TCHAR EoW;
  NMEAParser::ExtractParameter(sentence,&EoW,4);
  if (EoW==_T('W')) {
      Longitude *= -1;
  }
  WayPointList[RESWP_EXT_TARGET].Latitude=Latitude;
  WayPointList[RESWP_EXT_TARGET].Longitude=Longitude;

  ParToDouble(sentence, 5, &fTmp);
  WayPointList[RESWP_EXT_TARGET].Altitude=fTmp;

//  StartupStore(_T("Nano3 TARGET:%s "),sentence);
//  StartupStore(_T("Nano3 TARGET:%s lat:%f  lon:%f  Alt:%f"),WayPointList[RESWP_EXT_TARGET].Name, Latitude, Longitude,  fTmp);

  Alternate2 = RESWP_EXT_TARGET;

  return false;

}


BOOL DevLXNanoIII::PLXVC_INFO(PDeviceDescriptor_t d, const TCHAR* sentence, NMEA_INFO* info)
{
TCHAR  szTmp[MAX_NMEA_LEN];

  NMEAParser::ExtractParameter(sentence,szTmp,2);
  if  (_tcscmp(szTmp,_T("A"))!=0)  // no answer flag received
   return false;

  NMEAParser::ExtractParameter(sentence,szTmp,0);
  {
    NMEAParser::ExtractParameter(sentence,szTmp,5);
    {
      info->SerialNumber = (int) StrToDouble(szTmp,NULL);
    }


    double Batt;
    if (ParToDouble(sentence, 7, &Batt))
    {
      if(m_bValues)
      {
	_stprintf(szTmp, _T("%3.1fV ($PLXVC_INFO)"),Batt);
	ShowDataValue( wf ,d,_T("prpBAT1Dir"),  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].BAT1Dir))
      {
	info->ExtBatt1_Voltage = Batt;
      }
    }

    if (ParToDouble(sentence, 8, &Batt))
    {
      if(m_bValues)
      {
	_stprintf(szTmp, _T("%3.1fV (&PLXVC_INFO)"),Batt);
	ShowDataValue( wf ,d,_T("prpBAT2Dir"),  szTmp);
      }
      if(IsDirInput(PortIO[d->PortNumber].BAT2Dir))
      {
	info->ExtBatt2_Voltage = Batt;
      }
    }
  }
  return true;

} // PLXVC

