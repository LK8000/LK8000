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

#define PRPGRESS_DLG
#define BLOCK_SIZE 25

PDeviceDescriptor_t DevLXNanoIII::m_pDevice=NULL;
bool bIGC_Download = false;
static FILE *f= NULL;
uint uTimeout =0;
//______________________________________________________________________defines_

/// polynom for LX data CRC
#define LX_CRC_POLY 0x69

TCHAR m_Filename[200];


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
  d->PutMacCready = PutMacCready;
  d->PutBugs      = NULL;
  d->PutBallast   = NULL;
  d->Open         = Open;
  d->Close        = NULL;
  d->Init         = NULL;
  d->LinkTimeout  = GetTrue;
  d->Declare      = DeclareTask;
  d->IsLogger     = GetTrue;
  d->IsGPSSource  = GetTrue;
  d->IsBaroSource = GetTrue;
  d->Config       = Config;
  StartupStore(_T(". %s installed (platform=%s test=%u)%s"),
    GetName(),
    PlatfEndian::IsBE() ? _T("be") : _T("le"),
    PlatfEndian::To32BE(0x01000000), NEWLINE);
  return(true);
} // Install()



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
  return PLXVC( d,  sentence, info);
} // ParseNMEA()



BOOL DevLXNanoIII::Open( PDeviceDescriptor_t d) {
  m_pDevice = d;
  return TRUE;
}



BOOL PutMacCready(PDeviceDescriptor_t d, double MacCready){

  return true;

}


CallBackTableEntry_t DevLXNanoIII::CallBackTable[]={
  EndCallBackEntry()
};

BOOL DevLXNanoIII::Config(PDeviceDescriptor_t d){
  if(m_pDevice != d) {
	  StartupStore(_T(" Config : Invalide device descriptor%s"), NEWLINE);
	  return FALSE;
  }


  WndForm* wf = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_DEV_LXNAV_L : IDR_XML_DEV_LXNAV_P);
  if(wf) {

  WndButton *wBt = NULL;

  wBt = (WndButton *)wf->FindByName(TEXT("cmdClose"));
  if(wBt){
	  wBt->SetOnClickNotify(OnCloseClicked);
  }


  wBt = (WndButton *)wf->FindByName(TEXT("cmdIGCDownload"));
  if(wBt){
	  wBt->SetOnClickNotify(OnIGCDownloadClicked);
  }



    wf->ShowModal();

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
    status = status && DevLXNanoIII::SendNmea(d, buffer, errBufSize, errBuf);
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
//#ifdef TAKEOFF
	    // TakeOff point
      if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint)) {
        decl.WpFormat(DeclStrings[i++], &WayPointList[HomeWaypoint], Decl::tp_takeoff);
      } else {
        decl.WpFormat(DeclStrings[i++],NULL, Decl::tp_takeoff);
      }
//#endif
      // TurnPoints
      for (int ii = 0; ii < wpCount; ii++) {
        decl.WpFormat(DeclStrings[i++], lkDecl->waypoint[ii], Decl::tp_regular);
      }

//#ifdef LANDING
      // Landing point
      if (HomeWaypoint >= 0 && ValidWayPoint(HomeWaypoint)) {
        decl.WpFormat(DeclStrings[i++], &WayPointList[HomeWaypoint], Decl::tp_landing);
      } else {
        decl.WpFormat(DeclStrings[i++],NULL, Decl::tp_landing);
      }
//#endif
      for (int ii = 0; ii < i ; ii++){
	  StartupStore(_T(". NANO Decl: %s %s "),   DeclStrings[ii], NEWLINE);
      }
      // Send complete declaration to logger
      for (int ii = 0; ii < i ; ii++){
        if (status)
          status = status && DevLXNanoIII::SendDecl(d, ii+1, totalLines,
                                   DeclStrings[ii], errBufSize, errBuf);
        Poco::Thread::sleep(50);
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
  const TCHAR* wpName = _T("");
  if (wp != NULL) {
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
  _stprintf(buf, TEXT("C%02d%05.0f%c%03d%05.0f%c%s"),
             DegLat, MinLat*60000,NS,DegLon, MinLon*60000, EW,wpName  );
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
  TCHAR buffer[256];
  char retstr[20];
  _stprintf(buffer, TEXT("PLXVC,DECL,W,%u,%u,%s"), row, n_rows, content);
  bool status = DevLXNanoIII::SendNmea(d, buffer, errBufSize, errBuf); 
  if (status) {
    sprintf(retstr, "$PLXVC,DECL,C,%u", row);
    status = status && ComExpect(d, retstr, 512, NULL, errBufSize, errBuf);
  }
  return status;
} // SendDecl()





void DevLXNanoIII::OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}



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

CreateIGCProgressDialog();

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

  CloseIGCProgressDialog();

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

  if (_tcsncmp(_T("LOGBOOKSIZE"), Par[1],11) == 0)
  {
    _sntprintf(Par[0],MAX_NMEA_LEN, _T("PLXVC,LOGBOOK,R,1,%u"), _ttoi(Par[3])+1);
    SendNmea(d, Par[0], errBufSize, errBuf);
  }
  else
    if (_tcsncmp(_T("LOGBOOK"), Par[1],7) == 0)  // PLXVC but not declaration = IGC File transfer
    {
	TCHAR Line[2][MAX_NMEA_LEN];
	_sntprintf( Line[0],MAX_NMEA_LEN, _T("%s"),Par[5]);
	_sntprintf( Line[1],MAX_NMEA_LEN, _T("%s (%s-%s) %ukB"), Par[6] ,Par[7] ,Par[8], _ttoi(Par[9])/1024);
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
       CurLine = _ttoi(Par[4]);
       uint TotalLines = _ttoi(Par[5]);
       if((CurLine % (BLOCK_SIZE)==0))
       {
	 uint uPercent = 0;
	 if(TotalLines > 0)
	   uPercent = (CurLine*100) / TotalLines;
	   _sntprintf(Par[1],MAX_NMEA_LEN, _T("%s: %u%% %s ..."),MsgToken(2400), uPercent,m_Filename); // _@M2400_ "Downloading"

	 IGCProgressDialogText(Par[1]);

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
	// MessageBoxX(  MsgToken(2406), MsgToken(2398), mbOk) ;  // // _@M2398_ "IGC Download"

	 CloseIGCProgressDialog();
//	 StartRxThread(d,  errBufSize , errBuf);
       }
     }
    }
  }
  return(true);
}

