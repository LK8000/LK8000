/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "devFlarm.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "dlgIGCProgress.h"
#include "Util/Clamp.hpp"
#include "OS/Sleep.h"
#include "dlgFlarmIGCDownload.h"


#define MAX_FLARM_ANSWER_LEN  640  // FLARM Docu does not tell the max. answer len
                                   // The max. ever received length on IGC read was 485, so 640 seem to be a good value
#define GC_BLK_RECTIMEOUT     10000
#define GC_IDLETIME           10
#define REC_TIMEOUT           1000 // receive timeout in ms

#define LST_STRG_LEN          100
#define STATUS_TXT_LEN        100
#define PRPGRESS_DLG
#define deb_                  (0)  // debug output switch

#define IDLE_STATE            0
#define OPEN_BIN_STATE        1
#define PING_STATE_RX         2
#define PING_STATE_TX         3
#define READ_STATE_RX         4
#define READ_STATE_TX         5
#define SELECTRECORD_STATE_TX 6
#define SELECTRECORD_STATE_RX 7
#define READRECORD_STATE_TX   8
#define READRECORD_STATE_RX   9
#define START_DOWNLOAD_STATE  10
#define ABORT_STATE           11
#define ERROR_STATE           12
#define ALL_RECEIVED_STATE    13
#define CLOSE_STATE           14



static volatile int ThreadState =  IDLE_STATE;
static bool OnTimer(WndForm* pWnd);

static WndListFrame *wIGCSelectListList = NULL;
static WndOwnerDrawFrame *wIGCSelectListListEntry = NULL;



unsigned int IGC_DLIndex =0;         // selected File download index
unsigned int IGC_CurIndex =0;           // selected File index
unsigned int IGC_DrawListIndex=0;
bool bAbort = false;
bool bFLARM_BinMode = false;
int DownloadError =REC_NO_ERROR;     // global error variable


typedef struct
{
  TCHAR Line1[LST_STRG_LEN];
  TCHAR Line2[LST_STRG_LEN];
} ListElementType;

std::vector<ListElementType> IGCFileList;

int ReadFlarmIGCFile( DeviceDescriptor_t *d, uint8_t IGC_Index);

static WndForm *wf = NULL;
ListElement* pIGCResult = NULL;
TCHAR szStatusText[STATUS_TXT_LEN];

bool bShowMsg = false;

void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize);
uint8_t RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize, uint16_t Timeout);


uint16_t crc_update (uint16_t crc, uint8_t data) {

  crc = crc ^ ((uint16_t)data << 8);
  for (int i=0; i<8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
return crc;
}

uint16_t crc_update16 (uint16_t crc, uint16_t data) {
  crc = crc_update ( crc, lowbyte (data));
  crc = crc_update ( crc, highbyte (data));
  return crc;
}


void SendEscChar(DeviceDescriptor_t *d, uint8_t byte)
{
  if (d && d->Com)
  switch(byte)
  {
    case ESCAPE:
      d->Com->PutChar( ESCAPE);
      d->Com->PutChar( ESC_ESC);
    break;
    case STARTFRAME:
      d->Com->PutChar( ESCAPE);
      d->Com->PutChar( ESC_START);
    break;
    default:
      d->Com->PutChar( byte);
    break;
  }
}


void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command, uint8_t* pBlock, uint16_t blocksize)
{
uint16_t i;
uint16_t CRC=0;
uint8_t blk[8];


  blk[0] = lowbyte (8 +blocksize);  // length
  blk[1] = highbyte(8 +blocksize);  // length
  blk[2] = 1;                       // version
  blk[3] = lowbyte(Sequence);       // sequence
  blk[4] = highbyte(Sequence);      // sequence
  blk[5] = Command;

  d->Com->PutChar( STARTFRAME);
  for( i=0; i < 6; i++)
  {
    CRC =  crc_update(CRC, blk[i]);
  }

  if(pBlock)
  {
    for(i = 0; i < blocksize; i++)
    {
      CRC =  crc_update(CRC, pBlock[i]);
    }
  }
  blk[6] = lowbyte(CRC);
  blk[7] = highbyte(CRC);

  for(i=0; i < 8; i++)
  {
    SendEscChar(d,blk[i]);
  }

  for(i=0; i < blocksize; i++)
  {
    SendEscChar(d,pBlock[i]);
  }

  if(deb_)StartupStore(TEXT("\r\n===="));
 Poco::Thread::sleep(10);
}


uint8_t RecChar8( DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout)
{
  uint8_t Tmp;
  uint8_t err;
  err = RecChar(d, &Tmp, Timeout);

  if(!err)
  {
    if(Tmp == ESCAPE)
    {
      err = RecChar(d, &Tmp, Timeout);
      if(Tmp == ESC_ESC)
      {
        Tmp = 0x78;
        if(deb_) { StartupStore(TEXT("ESC_ESC" ));};
      }
      if(Tmp == ESC_START)
      {
        Tmp = 0x73;
        if(deb_) { StartupStore(TEXT("ESC_START"));};
      }
    }
  }
  *inchar = Tmp;
  return err;
}



uint8_t RecChar16( DeviceDescriptor_t *d,uint16_t *inchar, uint16_t Timeout)
{
  ConvUnion tmp;
  bool error  = RecChar8(d, &(tmp.byte[0]), Timeout);
   if(error == REC_NO_ERROR)
     error = RecChar8(d, &(tmp.byte[1]), Timeout);
  *inchar =  tmp.val;
return error;
}


uint8_t RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command, uint8_t* pBlock, uint16_t *blocksize, uint16_t Timeout)
{
uint8_t error  = REC_NO_ERROR ;
uint8_t inchar;
uint8_t Version;
uint16_t CRC_in, CRC_calc=0;

  do{
    error = RecChar(d, &inchar, Timeout);

    if(error)
    {
      if(deb_)StartupStore(TEXT("STARTFRAME timeout!" ));
      if(error) return error;
    }
  } while( (inchar != STARTFRAME) && !error);

  if(!error)
    { if(deb_ )StartupStore(TEXT("STARTFRAME OK!"));}
  else
    { if(deb_)StartupStore(TEXT("STARTFRAME fail!" )); }

  error |= RecChar16(d, blocksize , Timeout); CRC_calc = crc_update16 (CRC_calc,*blocksize);{ if(deb_)StartupStore(TEXT("Block Size %u"  ), *blocksize); }if(error != REC_NO_ERROR)  return error;
  error |= RecChar8(d, &Version   , Timeout); CRC_calc = crc_update   (CRC_calc,Version)   ;{ if(deb_)StartupStore(TEXT("Block Ver %u"   ), Version);    }if(error != REC_NO_ERROR)  return error;
  error |= RecChar16(d, Sequence  , Timeout); CRC_calc = crc_update16 (CRC_calc,*Sequence) ;{ if(deb_)StartupStore(TEXT("Block Seq %u"   ), *Sequence);  }if(error != REC_NO_ERROR)  return error;
  error |= RecChar8(d, Command    , Timeout); CRC_calc = crc_update   (CRC_calc,*Command)  ;{ if(deb_)StartupStore(TEXT("Block Cmd %02X" ), *Command);   }if(error != REC_NO_ERROR)  return error;
  error |= RecChar16(d,&CRC_in    , Timeout);                                               { if(deb_)StartupStore(TEXT("Block CRC %04X" ), CRC_in);     }if(error != REC_NO_ERROR)  return error;
  if(deb_) {StartupStore(TEXT("Header  received!" ));}

  if(*blocksize > 8)
    for (uint16_t i = 0 ; i < (*blocksize-8) ; i++)
    {
       error |= RecChar8(d,&inchar   , Timeout);
       if(error != REC_NO_ERROR)
       {
    	 StartupStore(TEXT("Rec Block Body error: %u!"), error);
         return error;
       }
       pBlock[i] = inchar;
       CRC_calc = crc_update (CRC_calc,pBlock[i]); if(error)  return error;
   //    if(deb_) { StartupStore(TEXT("Block[%u]  %02X" ),i, pBlock[i]);}
    }
 *blocksize -= 8;
 if(CRC_calc != CRC_in)
 {
   StartupStore(TEXT("Rec Block CRC error!" ));
   error = REC_CRC_ERROR;
 }
 else
 {
  if(error)
    StartupStore(TEXT("Rec Block error  received!" ));
  else
    if(deb_) StartupStore(TEXT("Rec Block received OK!" ));
 }
 return error;
}




static void UpdateList(void) {
  if(wIGCSelectListList != NULL)
  {
    wIGCSelectListList->ResetList();
    wIGCSelectListList->Redraw();
  }
}

static void OnUpClicked(WndButton* Sender) {
if(IGCFileList.size() == 0) return;
  if (IGC_CurIndex > 0) {
    IGC_CurIndex--;
  } else {
    LKASSERT(IGCFileList.size()>0);
    IGC_CurIndex = (IGCFileList.size() - 1);
  }
  if(wIGCSelectListList != NULL)
  {
    wIGCSelectListList->SetItemIndexPos(IGC_CurIndex);
    wIGCSelectListList->Redraw();
    if(wIGCSelectListListEntry)
      wIGCSelectListListEntry->SetFocus();
  }
}

static void OnDownClicked(WndButton* pWnd) {
(void)pWnd;
if(IGCFileList.size() == 0) return;
  if (IGC_CurIndex < (IGCFileList.size() - 1)) {
    IGC_CurIndex++;
  } else {
    IGC_CurIndex = 0;
  }
  if(wIGCSelectListList != NULL)
  {
    wIGCSelectListList->SetItemIndexPos(IGC_CurIndex);
    wIGCSelectListList->Redraw();
    if(wIGCSelectListListEntry)
      wIGCSelectListListEntry->SetFocus();
  }
}




static void OnMultiSelectListListInfo(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {
(void) Sender;

  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = IGCFileList.size();
  } else {
    IGC_DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
    IGC_CurIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }

}

bool GetIGCFilename(TCHAR* IGCFilename, int Idx)
{
if(IGCFilename == NULL)
  return false;
TCHAR Tmp[MAX_PATH];

  _tcscpy(Tmp, IGCFileList.at(Idx).Line1 );
  TCHAR* remaining;
  TCHAR* Filename  = _tcstok_r(Tmp ,TEXT(" "),&remaining);
  LocalPath(IGCFilename, _T(LKD_LOGS), Filename);
  return true;
}


static void OnEnterClicked(WndButton* pWnd) {
TCHAR Tmp[MAX_PATH ];
if(IGCFileList.size() == 0) return;
    (void)pWnd;

  if ( IGC_CurIndex  >= IGCFileList.size()) {
      IGC_CurIndex = IGCFileList.size() - 1;
  }
  if(IGCFileList.size() < (uint)IGC_CurIndex) return;
  IGC_DLIndex = IGC_CurIndex;
  bAbort = false;
  bShowMsg = true;
  _stprintf(Tmp, _T("%s %s ?"),MsgToken(2404),IGCFileList.at(IGC_DLIndex).Line1 );
   if (MessageBoxX(Tmp, MsgToken(2404),  mbYesNo) == IdYes)  // _@2404 "Download"
   {
      /** check if file already exist and is not empty ************/
      TCHAR IGCFilename[MAX_PATH];
      if(GetIGCFilename(IGCFilename, IGC_DLIndex))
      {
	if(lk::filesystem::exist(IGCFilename))
	  if (MessageBoxX(MsgToken(2416), MsgToken(2398), mbYesNo) == IdNo) // _@M2416_ "File already exits\n download anyway?"
	  {
	    ThreadState = IDLE_STATE;
	    return ;
	  }
      }
	      /************************************************************/
     ThreadState =  START_DOWNLOAD_STATE;        // start thread IGC download
     if(wf) wf->SetTimerNotify(600, OnTimer); // check for end of download every 100ms
  #ifdef PRPGRESS_DLG
  CreateIGCProgressDialog();
  #endif
   }
}



static void OnMultiSelectListPaintListItem(WindowControl * Sender, LKSurface& Surface) {
#define PICTO_WIDTH 50
Surface.SetTextColor(RGB_BLACK);

  if(IGCFileList.size() == 0 ) return;
  if(IGCFileList.size() < (uint)IGC_DrawListIndex) return;
  if (IGC_DrawListIndex < IGCFileList.size())
  {
    TCHAR IGCFilename[MAX_PATH];
    TCHAR FileExist[5] = _T("");
    if( GetIGCFilename(IGCFilename, IGC_DrawListIndex))
    {
       if(lk::filesystem::exist(IGCFilename))    // file exists
       {
	 if(Appearance.UTF8Pictorials)           // use UTF8 symbols?
	   _tcscpy(FileExist,_T("✔"));   // check! already copied
	 else
	   _tcscpy(FileExist,_T("*"));	 // * already copied
       }
    }
    TCHAR text1[180] = {TEXT("IGC File")};
    TCHAR text2[180] = {TEXT("date")};
    _stprintf(text1, _T("%s %s"), FileExist, IGCFileList.at(IGC_DrawListIndex).Line1 );
    _stprintf(text2, _T("%s"), IGCFileList.at(IGC_DrawListIndex).Line2);
    Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));

    PixelRect rc = { 0, 0, 0, // DLGSCALE(PICTO_WIDTH),
	static_cast<PixelScalar>(Sender->GetHeight())
    };

    /********************
     * show text
     ********************/

    Surface.SetBackgroundTransparent();
    Surface.SetTextColor(RGB_BLACK);
    Surface.DrawText(rc.right + DLGSCALE(2), DLGSCALE(2), text1);
    int ytext2 = Surface.GetTextHeight(text1);
    Surface.SetTextColor(RGB_DARKBLUE);
    Surface.DrawText(rc.right + DLGSCALE(2), ytext2, text2);
  }
}



static void OnIGCListEnter(WindowControl * Sender, WndListFrame::ListInfo_t *ListInfo) {
(void) Sender;
IGC_CurIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;

  if (IGC_CurIndex >= IGCFileList.size()) {
    IGC_CurIndex = IGCFileList.size() - 1;
  }


  if (IGC_CurIndex >= 0) {
    if(Sender) {
      WndForm * pForm = Sender->GetParentWndForm();
      if(pForm) {
	 IGC_DLIndex = IGC_CurIndex;
	 OnEnterClicked(NULL) ;

      }
    }
  }
}


void StopIGCRead(void )
{
  bAbort = true;
}


static void OnCloseClicked(WndButton* pWnd) {
StopIGCRead();
  if( ThreadState ==  IDLE_STATE)
  {
    if (MessageBoxX(MsgToken(2413), MsgToken(2403), mbYesNo) == IdYes) // _@M2413_ "FLARM need reboot for normal operation\n reboot now?"
    {
      if(pWnd) {
	WndForm * pForm = pWnd->GetParentWndForm();
	if(pForm) {
	  if(wf)wf->SetTimerNotify(0, NULL);    // reset Timer
	  pForm->SetModalResult(mrCancel);
	}
      }
    }
  }
}


static bool OnTimer(WndForm* pWnd) {
TCHAR Tmp [STATUS_TXT_LEN];

  if(pWnd)
  {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm)
    {
      UpdateList();
      if( ThreadState ==  IDLE_STATE)
      {
	WndButton* wb = (WndButton*)pForm->FindByName(TEXT("cmdClose"));
	wb->SetCaption(MsgToken(186));  // _@M186_ "Close"
	#ifdef PRPGRESS_DLG
	CloseIGCProgressDialog();
	#endif
	if(wf) wf->SetTimerNotify(0, NULL);    // reset Timer
	if(bShowMsg)
	{
	 switch (DownloadError)
	 {
	   case REC_NO_ERROR     : _sntprintf(Tmp, STATUS_TXT_LEN, _T("%s\n%s"), IGCFileList.at(IGC_DLIndex).Line1  ,MsgToken(2406)); break; // 	_@M2406_ "IGC File download complete"
	   case REC_TIMEOUT_ERROR: _tcscpy(Tmp,  MsgToken(2407)); break;  // _@M2407_ "Error: receive timeout
	   case REC_CRC_ERROR    : _tcscpy(Tmp,  MsgToken(2408)); break;  // _@M2408_ "Error: CRC checksum fail"
	   case REC_ABORTED      : _tcscpy(Tmp,  MsgToken(2415)); break;  // _@M2415_ "IGC Download aborted!"
	   case FILENAME_ERROR   : _tcscpy(Tmp,  MsgToken(2409)); break;  // _@M2409_ "Error: invalid filename"
	   case FILE_OPEN_ERROR  : _tcscpy(Tmp,  MsgToken(2410)); break;  // _@M2410_ "Error: can't open file"
	   case IGC_RECEIVE_ERROR: _tcscpy(Tmp,  MsgToken(2411)); break;  // _@M2411_ "Error: Block invalid"
	   case REC_NO_DEVICE    : _tcscpy(Tmp,  MsgToken(2401)); break;  // _@M2401_ "No Device found"


	   default               : _tcscpy(Tmp, MsgToken(2412)); break;  // _@M2412_ "Error: unknown"

	 }
	 if (MessageBoxX( Tmp, MsgToken(2398), mbOk) == IdYes)  // _@M2406_ "Error: communication timeout"Reboot"
	 {
	 }
       }
       DownloadError = REC_NO_ERROR;
     }
     else
     {
#ifdef PRPGRESS_DLG
       IGCProgressDialogText(szStatusText) ;  // update progress dialog text
#endif
     }
   }
 }
 return true;
}


static CallBackTableEntry_t IGCCallBackTable[] = {
  OnPaintCallbackEntry(OnMultiSelectListPaintListItem),
  OnListCallbackEntry(OnMultiSelectListListInfo),
  ClickNotifyCallbackEntry(OnCloseClicked),
  ClickNotifyCallbackEntry(OnUpClicked),
  ClickNotifyCallbackEntry(OnEnterClicked),
  ClickNotifyCallbackEntry(OnDownClicked),
  EndCallBackEntry()
};


void LeaveBinModeWithReset(DeviceDescriptor_t *d)
{
  if(d != NULL){
    int Sequence = 0;

    LockFlightData();
    GPS_INFO.FLARM_Available = false;
    UnlockFlightData();

    if(deb_) StartupStore(TEXT("EXIT & RESET"));
    SendBinBlock(d, Sequence, EXIT, NULL, 0);
     bFLARM_BinMode = false;
    StartupStore(TEXT("$PFLAR,0*55\r\n"));
    d->Com->WriteString(TEXT("$PFLAR,0*55\r\n"));
    if (deb_) StartupStore(TEXT("$PFLAR,0*55\r\n"));
  }
}

bool IsInBinaryMode (void)
{
  return bFLARM_BinMode;
}

bool SetBinaryModeFlag (bool bBinMode)
{
bool OldVal =	bFLARM_BinMode;
  bFLARM_BinMode = bBinMode;
  return OldVal;
}

class ResourceLock{
public:
  ResourceLock(){
    StartupStore(TEXT(".... Enter ResourceLock%s"),NEWLINE);
    MapWindow::SuspendDrawingThread();
    StartIGCReadThread() ;
  };
  ~ResourceLock(){
    StartupStore(TEXT(".... Leave ResourceLock%s"),NEWLINE);
    StopIGCReadThread() ;
    MapWindow::ResumeDrawingThread();
    if(wf)wf->SetTimerNotify(0, NULL);    // reset Timer

    IGCFileList.clear();
  }
};



ListElement* dlgIGCSelectListShowModal( DeviceDescriptor_t *d) {

ResourceLock ResourceGuard;  //simply need to exist for recource Lock/Unlock
StartupStore(TEXT(".... StartIGCReadThread%s"),NEWLINE);

bShowMsg = false;
bAbort = false;

  /*************************************************/
  ThreadState =  OPEN_BIN_STATE;
  /*************************************************/
  wf = dlgLoadFromXML(IGCCallBackTable, ScreenLandscape ? IDR_XML_MULTISELECTLIST_L : IDR_XML_MULTISELECTLIST_P);

  if (wf)
  {
    WndButton* wb = (WndButton*)wf->FindByName(TEXT("cmdClose"));
    wb->SetCaption(MsgToken(670));  // _@M670_ "Stop"

    wIGCSelectListList = (WndListFrame*) wf->FindByName(TEXT("frmMultiSelectListList"));
    LKASSERT(wIGCSelectListList != NULL);
    wIGCSelectListList->SetBorderKind(BORDERLEFT);
    wIGCSelectListList->SetEnterCallback(OnIGCListEnter);

    wIGCSelectListListEntry = (WndOwnerDrawFrame*) wf->FindByName(TEXT("frmMultiSelectListListEntry"));
    if(wIGCSelectListListEntry) {
      /*
       * control height must contains 2 text Line
       * Check and update Height if necessary
       */
      LKWindowSurface windowSurface(MainWindow);
      LKBitmapSurface tmpSurface(windowSurface, 1, 1);
      const auto oldFont = tmpSurface.SelectObject(wIGCSelectListListEntry->GetFont());
      const int minHeight = 2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
      tmpSurface.SelectObject(oldFont);
      const int wHeight = wIGCSelectListListEntry->GetHeight();
      if(minHeight > wHeight) {
	wIGCSelectListListEntry->SetHeight(minHeight);
      }
      wIGCSelectListListEntry->SetCanFocus(true);
    } else LKASSERT(0);
    UpdateList();

	wf->SetTimerNotify(600, OnTimer); // check for end of download every 600ms
    wf->ShowModal();
    delete wf;
    wf = NULL;
  }
  DownloadError = REC_NOMSG;               // don't show an error msg on initialisation
  LeaveBinModeWithReset(d);                // reset Flarm after leaving dialog

  ThreadState =  IDLE_STATE;
  return pIGCResult;
}




int ReadFlarmIGCFile(DeviceDescriptor_t *d, uint8_t IGC_FileIndex)
{

if (d==NULL)
  return 0;

static FILE *file_ptr= NULL;
static int TimeCnt =0;
static uint16_t Sequence;
static uint8_t retrys = 0;

uint8_t   pByteBlk[MAX_FLARM_ANSWER_LEN];
pByteBlk[0] = IGC_FileIndex;
uint16_t  blocksize=1;
uint16_t RecSequence;
uint8_t RecCommand;
uint8_t err  = REC_NO_ERROR ;

  if(bAbort)
  {
    ThreadState =  ABORT_STATE;
    bAbort = false;
  }

  if (ThreadState == IDLE_STATE)
    return 0;

  if(d != NULL)
  {
    /******************************  OPEN_BIN_STATE ************************************/
    if( ThreadState ==  OPEN_BIN_STATE)
    {
      retrys =0;
      d->Com->WriteString(TEXT("$PFLAX\r\n"));  // set to binary
      if(deb_) StartupStore(TEXT("$PFLAX\r "));
      ThreadState =  PING_STATE_TX;
      bFLARM_BinMode = true;
      return 0;
    }

    /******************************  PING_STATE_TX     ************************************/
    if( ThreadState ==  PING_STATE_TX)
    {
      if(deb_) StartupStore(TEXT("PING "));

      if( retrys++ >= 15)
      {
	ThreadState = ERROR_STATE;
	return 0;
      }
      ListElementType NewElement;
      _sntprintf(NewElement.Line1, LST_STRG_LEN, _T("        PING Flarm %u/15"), retrys);
      _tcscpy(NewElement.Line2, _T("        ... "));
      IGCFileList.clear();
      IGCFileList.push_back(NewElement);
      SendBinBlock(d, Sequence++, PING, NULL, 0);
      TimeCnt =0;

      ThreadState = PING_STATE_RX;
    }
    /******************************  PING_STATE_RX     ************************************/
    if( ThreadState ==  PING_STATE_RX)
    {
      if(!BlockReceived())
      {
	if(deb_) StartupStore(TEXT("WAIT FOR PING ANSWER %ums"), TimeCnt*GC_IDLETIME);
	if(TimeCnt++ > (1000/GC_IDLETIME))
	{
	      err = REC_TIMEOUT_ERROR;
	  ThreadState = PING_STATE_TX;

	}
	return 0; // no data? leave thread and wait for next call
      }
      err = RecBinBlock(d, &RecSequence, &RecCommand, &pByteBlk[0], &blocksize, REC_TIMEOUT);
      ThreadState = PING_STATE_TX;
      if(err == REC_NO_ERROR )
      {
	retrys =0;
	ThreadState =  SELECTRECORD_STATE_TX;
	IGCFileList.clear();  // empty list
      }
      return 0;
    }
    /******************************  SELECTRECORD_STATE_TX    ************************************/
    if( ThreadState ==  SELECTRECORD_STATE_TX)
    {
      if(deb_) StartupStore(TEXT("RECORD_STATE_TX "));
      retrys=0;
      TimeCnt =0;
      pByteBlk[0] =IGCFileList.size();
      SendBinBlock(d, Sequence++, SELECTRECORD, &pByteBlk[0], 1);
      ThreadState = SELECTRECORD_STATE_RX;
    }

    /****************************  SELECTRECORD_STATE_RX    ************************************/
    if( ThreadState ==  SELECTRECORD_STATE_RX)
    {
      if(!BlockReceived())
      {
	if(deb_)  StartupStore(TEXT("SELECTRECORD_STATE_RX %ums"), TimeCnt*GC_IDLETIME);
	if(TimeCnt++ > (1000/GC_IDLETIME))
	{
	  err = REC_TIMEOUT_ERROR;
	  ThreadState = SELECTRECORD_STATE_TX;
	  if(retrys++ > 4)
	    ThreadState = ABORT_STATE;
	}
	return 0; // no data? leave thread and wait for next call
      }
      err = RecBinBlock(d, &RecSequence, &RecCommand, &pByteBlk[0], &blocksize, REC_TIMEOUT);
      if(RecCommand == ACK)
	ThreadState  = READRECORD_STATE_TX;
      else
	ThreadState = ALL_RECEIVED_STATE;
    }

    /******************************  READRECORD_STATE_RX     ************************************/
    if( ThreadState ==  READRECORD_STATE_TX)
    {
      if(deb_)  StartupStore(TEXT("READRECORD_STATE_RX "));
      retrys=0;
      TimeCnt =0;
      SendBinBlock(d, Sequence++, GETRECORDINFO, NULL, 0);

      ThreadState = READRECORD_STATE_RX;
    }
    /******************************  READRECORD_STATE_RX     ************************************/
    if( ThreadState ==  READRECORD_STATE_RX)
    {
      if(!BlockReceived())
      {
	if(deb_) StartupStore(TEXT("READRECORD_STATE_RX %ums"), TimeCnt*GC_IDLETIME);
	if(TimeCnt++ > (1000/GC_IDLETIME))
	{
	      err = REC_TIMEOUT_ERROR;
	      ThreadState = READRECORD_STATE_TX;
	      if(retrys++ > 4)
		ThreadState = ABORT_STATE;
	}
	return 0; // no data? leave thread and wait for next call
      }
      err = RecBinBlock(d, &RecSequence, &RecCommand, &pByteBlk[0], &blocksize, REC_TIMEOUT);
      pByteBlk[blocksize++] = 0;
      if(RecCommand == ACK)
      {
	TCHAR  TempString[255];
	ListElementType NewElement;
	for(uint16_t i = 0; i < blocksize-2; i++)
	  TempString[i]= (TCHAR) pByteBlk[i+2];
	if(deb_) StartupStore(TEXT("> %s "),TempString);
	TCHAR empty[3] = _T("");
	TCHAR* remaining=NULL;
	TCHAR* Filename  = _tcstok_r(TempString,TEXT("|"),&remaining); if(Filename == NULL)  {Filename = empty;};
	TCHAR* Date      = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Date     == NULL)  {Date     = empty;};
	TCHAR* Takeoff   = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Takeoff  == NULL)  {Takeoff  = empty;};
	TCHAR* Duration  = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Duration == NULL)  {Duration = empty;};
	TCHAR* Pilot     = _tcstok_r(NULL,TEXT("|"),&remaining);       if(Pilot    == NULL)  {Pilot    = empty;};
	TCHAR* CN        = _tcstok_r(NULL,TEXT("|"),&remaining);       if(CN       == NULL)  {CN       = empty;};
	_stprintf( NewElement.Line1  ,_T("%s (%s  [%5s])"),Filename, Date,Takeoff);
	_stprintf( NewElement.Line2  ,_T("%s"),Duration);
	if(Pilot) { _tcscat( NewElement.Line2 ,_T(" ") );_tcscat( NewElement.Line2 ,Pilot);};
	if(CN)    { _tcscat( NewElement.Line2 ,_T(" ")); _tcscat( NewElement.Line2 ,CN);};
	IGCFileList.push_back(NewElement);
      }

      if (RecCommand != ACK)
	ThreadState = ALL_RECEIVED_STATE;
      else
	ThreadState = SELECTRECORD_STATE_TX;
    }
    /******************************  ALL_RECEIVED_STATE *********************************/
    if( ThreadState ==  ALL_RECEIVED_STATE)
    {
      if(deb_) StartupStore(TEXT("ALL_RECEIVED_STATE"));
      ThreadState =  IDLE_STATE;
    }
    /******************************  ERROR_STATE     ************************************/
    if( ThreadState ==  ERROR_STATE)
    {
      if(deb_) StartupStore(TEXT("ERROR_STATE"));
      ListElementType NewElement;
      _tcscpy(NewElement.Line1, _T("        Error:"));
      _sntprintf(NewElement.Line2, LST_STRG_LEN, _T("         %s"),MsgToken(2401)); // _@M2401_ "No Device found"
      IGCFileList.clear();
      IGCFileList.push_back(NewElement);
      ThreadState =  IDLE_STATE;
      DownloadError = REC_NO_DEVICE;
    }
    /******************************  ABORT STATE     ************************************/
    if( ThreadState ==  ABORT_STATE)
    {
      if(deb_) StartupStore(TEXT("ABORT_STATE"));
      if(file_ptr != NULL) // file incomplete?
      {
	fclose(file_ptr);
	file_ptr = NULL;

	TCHAR IGCFilename[MAX_PATH];
	if(GetIGCFilename(IGCFilename, IGC_FileIndex))
	{
	  lk::filesystem::deleteFile(IGCFilename);  // delete incomplete file (after abort) to prevent "file exists warning
	  if(deb_) StartupStore(TEXT("delete incomplete IGC File: %s "),IGCFilename);
	}
      }
      DownloadError =  REC_ABORTED;
      ThreadState =  IDLE_STATE;
    }
    /****** START_DOWNLOAD_STATE ************************************************************************/
    if (ThreadState == START_DOWNLOAD_STATE)
    {
      Sequence =0;
      if(IGCFileList.size() < IGC_FileIndex)
	return 0;

      if(deb_)StartupStore(TEXT ("START_DOWNLOAD_STATE: %s"), IGCFileList.at(IGC_FileIndex).Line1);
      if(file_ptr) {fclose(file_ptr); file_ptr = NULL;}
      err = REC_NO_ERROR;

      TCHAR IGCFilename[MAX_PATH];
      GetIGCFilename(IGCFilename, IGC_FileIndex);
      file_ptr = _tfopen( IGCFilename, TEXT("w"));
      if(file_ptr == NULL)   { err = FILE_OPEN_ERROR;  }   // #define FILE_OPEN_ERROR 5

      _sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("IGC Dowlnoad File : %s "),IGCFileList.at(IGC_FileIndex).Line1);
      if(deb_)StartupStore(_T("%s"), szStatusText);
      SendBinBlock(d,  Sequence++,  SELECTRECORD, &IGC_FileIndex,  1);
      err = RecBinBlock(d,  &RecSequence, &RecCommand, &pByteBlk[0], &blocksize, REC_TIMEOUT);

      if(err != REC_NO_ERROR)
	err =  IGC_RECEIVE_ERROR;
      ThreadState =  READ_STATE_TX;
      Sequence =0;
      retrys =0;
    }

    /******READ STATE TX *******************************************************************************/
    if (ThreadState == READ_STATE_TX)
    {
      blocksize = 0;
      ThreadState = READ_STATE_RX;
      SendBinBlock(d,  Sequence,  GETIGCDATA, &pByteBlk[0], 0);
      TimeCnt =0;
    }

    /******READ STATE RX *******************************************************************************/
    if (ThreadState == READ_STATE_RX)
    {
      if(!BlockReceived())
      {
	if(TimeCnt++ > (GC_BLK_RECTIMEOUT/GC_IDLETIME))
	{
	  err = REC_TIMEOUT_ERROR;
	  if(retrys++ > 4)
	    ThreadState = ABORT_STATE;
	  else
	    ThreadState = READ_STATE_TX;
	}
	return 0; // no data? leave thread and wait for next call
      }
      Sequence++;
      retrys =0;
      err = RecBinBlock(d,  &RecSequence, &RecCommand, &pByteBlk[0], &blocksize,10* REC_TIMEOUT);
      ThreadState = READ_STATE_TX;

      if(err==REC_NO_ERROR)
	_sntprintf(szStatusText, STATUS_TXT_LEN, _T("%s: %u%% %s ..."),MsgToken(2400), pByteBlk[2], IGCFileList.at(IGC_FileIndex).Line1); // _@M2400_ "Downloading"

      if((Sequence %10) == 0)
      {
	StartupStore(TEXT("%u%% Block:%u  Response time:%ums  Size:%uByte"),pByteBlk[2],Sequence, TimeCnt*GC_IDLETIME,blocksize    );
      }
      for(int i=0; i < blocksize-3; i++)
      {
      	if(file_ptr)
	       	fputc(pByteBlk[3+i],file_ptr);
	if(pByteBlk[3+i] == EOF_)
	  RecCommand = EOF_;
      }

      if (err)
	ThreadState = CLOSE_STATE;

      if (RecCommand != ACK)
	ThreadState = CLOSE_STATE;
    }
    /******CLOSE ON ERROR *********************************************************************************/
    if(err != REC_NO_ERROR) ThreadState = CLOSE_STATE;

    /******CLOSE STATE *********************************************************************************/
    if (ThreadState == CLOSE_STATE)
    {
      if(file_ptr) {fclose(file_ptr); file_ptr= NULL;}

      ThreadState =  IDLE_STATE;
      if(deb_)StartupStore(TEXT ("IDLE_STATE"));
      if(err != REC_NO_ERROR)
      {
	_sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("Error Code:%u"), err);
	if(DownloadError ==REC_NO_ERROR)  // no prvious error=
	  DownloadError = err;
	err = REC_NO_ERROR;
      }
      else
      {
	_sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("%s"), MsgToken(2406)); // _@M2406_ "IGC File download complete!"
      }
      if(deb_)StartupStore(_T("IGC downlload complete"));

    }
  }  // if(d)
return 0;
}


class IGCReadThread : public Poco::Runnable
{
public:

  void Start() {
    if(!Thread.isRunning())
    {
      bStop = false;
      Thread.start(*this);
    }
  }

  void Stop() {
    if(Thread.isRunning())
    {
      bStop = true;
      Thread.join();
    }
  }

protected:
  bool bStop;
  Poco::Thread Thread;

  void run() {

    while(!bStop) {
      if( ThreadState !=  IDLE_STATE)
      {
	ReadFlarmIGCFile( CDevFlarm::GetDevice(), IGC_DLIndex);
      }
      Poco::Thread::sleep(GC_IDLETIME);
    }
  }
};

IGCReadThread IGCReadThreadThreadInstance;


void StartIGCReadThread() {
  if (deb_)StartupStore(TEXT("Start IGC Thread !"));
  IGCReadThreadThreadInstance.Start();
}

void StopIGCReadThread() {

  if (deb_) StartupStore(TEXT("Stop IGC Thread !"));
  IGCReadThreadThreadInstance.Stop();
}
