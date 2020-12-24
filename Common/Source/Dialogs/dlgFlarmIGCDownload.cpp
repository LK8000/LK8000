/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "Dialogs.h"
#include "devFlarm.h"
#include "dlgIGCProgress.h"
#include "OS/Sleep.h"
#include "Util/Clamp.hpp"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"
#include "dlgFlarmIGCDownload.h"

#define MAX_FLARM_ANSWER_LEN 640 
      // FLARM Docu does not tell the max. answer len
      // The max. ever received length on IGC read was 485, so 640 seem to be a
      // good value
#define GC_BLK_RECTIMEOUT 1000
#define GC_IDLETIME 50
#define GC_TIMER_INTERVAL 750
#define REC_TIMEOUT 1000 // receive timeout in ms
#define MAX_RETRY 1
#define LST_STRG_LEN 100
#define STATUS_TXT_LEN 100
#define PRPGRESS_DLG
#define deb_ (0) // debug output switch

enum thread_state {
  IDLE_STATE,
  OPEN_BIN_STATE,
  PING_STATE_RX,
  PING_STATE_TX,
  READ_STATE_RX,
  READ_STATE_TX,
  SELECTRECORD_STATE_TX,
  SELECTRECORD_STATE_RX,
  READRECORD_STATE_TX,
  READRECORD_STATE_RX,
  START_DOWNLOAD_STATE,
  ABORT_STATE,
  ERROR_STATE,
  ALL_RECEIVED_STATE,
  CLOSE_STATE
};

static volatile thread_state ThreadState = IDLE_STATE;
static bool OnTimer(WndForm *pWnd);

static WndListFrame *wIGCSelectListList = NULL;
static WndOwnerDrawFrame *wIGCSelectListListEntry = NULL;

unsigned int IGC_DLIndex = 0;  // selected File download index
unsigned int IGC_CurIndex = 0; // selected File index
unsigned int IGC_DrawListIndex = 0;
bool bAbort = false;
int DownloadError = REC_NO_ERROR; // global error variable
bool bFilled = false;
typedef struct {
  TCHAR Line1[LST_STRG_LEN];
  TCHAR Line2[LST_STRG_LEN];
} ListElementType;

std::vector<ListElementType> IGCFileList;

int ReadFlarmIGCFile(DeviceDescriptor_t *d, uint8_t IGC_Index);

ListElement *pIGCResult = NULL;
TCHAR szStatusText[STATUS_TXT_LEN];

bool bShowMsg = false;

void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command,
                  uint8_t *pBlock, uint16_t blocksize);

uint16_t crc_update(uint16_t crc, uint8_t data) {

  crc = crc ^ ((uint16_t)data << 8);
  for (int i = 0; i < 8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}

uint16_t crc_update16(uint16_t crc, uint16_t data) {
  crc = crc_update(crc, lowbyte(data));
  crc = crc_update(crc, highbyte(data));
  return crc;
}

void SendEscChar(DeviceDescriptor_t *d, uint8_t byte) {
  if (d && d->Com)
    switch (byte) {
    case ESCAPE:
      d->Com->Write(ESCAPE);
      d->Com->Write(ESC_ESC);
      break;
    case STARTFRAME:
      d->Com->Write(ESCAPE);
      d->Com->Write(ESC_START);
      break;
    default:
      d->Com->Write(byte);
      break;
    }
}

void SendBinBlock(DeviceDescriptor_t *d, uint16_t Sequence, uint8_t Command,
                  uint8_t *pBlock, uint16_t blocksize) {
  uint16_t i;
  uint16_t CRC = 0;
  uint8_t blk[8];

  if (d == NULL)
    return;

  blk[0] = lowbyte(8 + blocksize);  // length
  blk[1] = highbyte(8 + blocksize); // length
  blk[2] = 1;                       // version
  blk[3] = lowbyte(Sequence);       // sequence
  blk[4] = highbyte(Sequence);      // sequence
  blk[5] = Command;

  d->Com->Write(STARTFRAME);
  for (i = 0; i < 6; i++) {
    CRC = crc_update(CRC, blk[i]);
  }

  if (pBlock) {
    for (i = 0; i < blocksize; i++) {
      CRC = crc_update(CRC, pBlock[i]);
    }
  }
  blk[6] = lowbyte(CRC);
  blk[7] = highbyte(CRC);

  for (i = 0; i < 8; i++) {
    SendEscChar(d, blk[i]);
  }

  for (i = 0; i < blocksize; i++) {
    SendEscChar(d, pBlock[i]);
  }

  if (deb_) {
    StartupStore(TEXT("\r\n===="));
  }
  Poco::Thread::sleep(GC_IDLETIME);
  Poco::Thread::yield();
}

uint8_t RecChar8(DeviceDescriptor_t *d, uint8_t *inchar, uint16_t Timeout) {
  uint8_t Tmp;
  uint8_t err = RecChar(d, &Tmp, Timeout);
  if (err == REC_NO_ERROR) {
    if (Tmp == ESCAPE) {
      err = RecChar(d, &Tmp, Timeout);
      if (err == REC_NO_ERROR) {
        if (Tmp == ESC_ESC) {
          Tmp = ESCAPE;
          if (deb_) {
            StartupStore(TEXT("ESC_ESC"));
          }
        }
        if (Tmp == ESC_START) {
          Tmp = STARTFRAME;
          if (deb_) {
            StartupStore(TEXT("ESC_START"));
          }
        }
      }
    }
  }
  *inchar = Tmp;
  return err;
}

uint8_t RecChar16(DeviceDescriptor_t *d, uint16_t *inchar, uint16_t Timeout) {
  ConvUnion tmp;
  bool error = RecChar8(d, &(tmp.byte[0]), Timeout);
  if (error == REC_NO_ERROR) {
    error = RecChar8(d, &(tmp.byte[1]), Timeout);
  }
  *inchar = tmp.val;
  return error;
}

template<size_t size>
static uint8_t RecBinBlock(DeviceDescriptor_t *d, uint16_t *Sequence, uint8_t *Command,
                    uint8_t (&pBlock)[size], uint16_t *blocksize, uint16_t Timeout) {
  uint8_t error = REC_NO_ERROR;
  uint8_t inchar;
  uint8_t Version;
  uint16_t CRC_in, CRC_calc = 0;

  PeriodClock clock;
  clock.Update();
  do {
      error = RecChar(d, &inchar, Timeout);
  } while ((inchar != STARTFRAME) && (error == REC_NO_ERROR));

  if(error != REC_NO_ERROR) {
    if (deb_) {
      StartupStore(TEXT("STARTFRAME fail! Error code:%i"),error);
    }
    return error;
  }

  if (deb_) {
    StartupStore(TEXT("STARTFRAME OK!"));
  }
                    
  error = RecChar16(d, blocksize, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }

  if ((*blocksize) > size) {
    StartupStore(TEXT("RecBinBlock : Invalid Block Size %u"), *blocksize);
    return REC_INVALID_SIZE;
  }

  CRC_calc = crc_update16(CRC_calc, *blocksize);
  if (deb_) {
    StartupStore(TEXT("Block Size %u"), *blocksize);
  }

  error = RecChar8(d, &Version, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  CRC_calc = crc_update(CRC_calc, Version);
  if (deb_) {
    StartupStore(TEXT("Block Ver %u"), Version);
  }
  error = RecChar16(d, Sequence, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  CRC_calc = crc_update16(CRC_calc, *Sequence);
  if (deb_) {
    StartupStore(TEXT("Block Seq %u"), *Sequence);
  }
  error = RecChar8(d, Command, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  CRC_calc = crc_update(CRC_calc, *Command);
  if (deb_) {
    StartupStore(TEXT("Block Cmd %02X"), *Command);
  }
  error = RecChar16(d, &CRC_in, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  if (deb_) {
    StartupStore(TEXT("Block CRC %04X"), CRC_in);
    StartupStore(TEXT("Header  received!"));
  }

  if (*blocksize > 8) {
    for (uint16_t i = 0; i < (*blocksize - 8); i++) {
      error = RecChar8(d, &inchar, Timeout);
      if (error != REC_NO_ERROR) {
        StartupStore(TEXT("Rec Block Body error: %u!"), error);
        return error;
      }
      pBlock[i] = inchar;
      CRC_calc = crc_update(CRC_calc, pBlock[i]);
    }
  }
  *blocksize -= 8;
  if (CRC_calc != CRC_in) {
    error = REC_CRC_ERROR;
    StartupStore(TEXT("Rec Block CRC error!"));
  } else {
    error = REC_NO_ERROR;
    if (deb_) {
      StartupStore(TEXT("Rec Block received!"));
    }
  }
  return error;
}

static void UpdateList(void) {
  if (wIGCSelectListList != NULL) {
    wIGCSelectListList->ResetList();
    wIGCSelectListList->Redraw();
  }
}

static void OnUpClicked(WndButton *Sender) {
  if (IGCFileList.size() == 0)
    return;
  if (IGC_CurIndex > 0) {
    IGC_CurIndex--;
  } else {
    LKASSERT(IGCFileList.size() > 0);
    IGC_CurIndex = (IGCFileList.size() - 1);
  }
  if (wIGCSelectListList != NULL) {
    wIGCSelectListList->SetItemIndexPos(IGC_CurIndex);
    wIGCSelectListList->Redraw();
    if (wIGCSelectListListEntry)
      wIGCSelectListListEntry->SetFocus();
  }
}

static void OnDownClicked(WndButton *pWnd) {
  (void)pWnd;
  if (IGCFileList.size() == 0)
    return;
  if (IGC_CurIndex < (IGCFileList.size() - 1)) {
    IGC_CurIndex++;
  } else {
    IGC_CurIndex = 0;
  }
  if (wIGCSelectListList != NULL) {
    wIGCSelectListList->SetItemIndexPos(IGC_CurIndex);
    wIGCSelectListList->Redraw();
    if (wIGCSelectListListEntry)
      wIGCSelectListListEntry->SetFocus();
  }
}

static void OnMultiSelectListListInfo(WindowControl *Sender,
                                      WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;

  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = IGCFileList.size();
  } else {
    IGC_DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
    IGC_CurIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }
}

bool GetIGCFilename(TCHAR *IGCFilename, int Idx) {
  if (IGCFilename == NULL)
    return false;
  TCHAR Tmp[MAX_PATH];

  _tcscpy(Tmp, IGCFileList.at(Idx).Line1);
  TCHAR *remaining;
  TCHAR *Filename = _tcstok_r(Tmp, TEXT(" "), &remaining);
  LocalPath(IGCFilename, _T(LKD_LOGS), Filename);
  return true;
}

static void OnEnterClicked(WndButton *pWnd) {
  

  if((!pWnd) || (!bFilled)){
    return;
  }
  WndForm* pForm = pWnd->GetParentWndForm();
  if(!pForm) {
    return;
  }

  TCHAR Tmp[MAX_PATH];
  if (IGCFileList.size() == 0)
    return;

  if (IGC_CurIndex >= IGCFileList.size()) {
    IGC_CurIndex = IGCFileList.size() - 1;
  }
  if (IGCFileList.size() < (uint)IGC_CurIndex)
    return;
  IGC_DLIndex = IGC_CurIndex;
  bAbort = false;
  bShowMsg = true;
  _stprintf(Tmp, _T("%s %s ?"), MsgToken(2404),
            IGCFileList.at(IGC_DLIndex).Line1);
  if (MessageBoxX(Tmp, MsgToken(2404), mbYesNo) == IdYes) // _@2404 "Download"
  {
    /** check if file already exist and is not empty ************/
    TCHAR IGCFilename[MAX_PATH];
    if (GetIGCFilename(IGCFilename, IGC_DLIndex)) {
      if (lk::filesystem::exist(IGCFilename))
        if (MessageBoxX(MsgToken(2416), MsgToken(2398), mbYesNo) ==
            IdNo) // _@M2416_ "File already exits\n download anyway?"
        {
          ThreadState = IDLE_STATE;
          return;
        }
    }
    /************************************************************/
    ThreadState = START_DOWNLOAD_STATE; // start thread IGC download
    pForm->SetTimerNotify(GC_TIMER_INTERVAL, OnTimer); // check for end of download every 250ms
#ifdef PRPGRESS_DLG
    CreateIGCProgressDialog();
#endif
  }
}

static void OnMultiSelectListPaintListItem(WindowControl *Sender,
                                           LKSurface &Surface) {
#define PICTO_WIDTH 50
  Surface.SetTextColor(RGB_BLACK);

  if (IGCFileList.size() == 0)
    return;
  if (IGCFileList.size() < (uint)IGC_DrawListIndex)
    return;
  if (IGC_DrawListIndex < IGCFileList.size()) {
    TCHAR IGCFilename[MAX_PATH];
    TCHAR FileExist[5] = _T("");
    TCHAR text1[180] = {TEXT("IGC File")};
    TCHAR text2[180] = {TEXT("date")};   
    _tcscpy(text1, IGCFileList.at(IGC_DrawListIndex).Line1);
    if (GetIGCFilename(IGCFilename, IGC_DrawListIndex)) {
      if (lk::filesystem::exist(IGCFilename)) // file exists
      {
        if (Appearance.UTF8Pictorials) // use UTF8 symbols?
          _tcscpy(FileExist, _T("✔")); // check! already copied
        else
          _tcscpy(FileExist, _T("*")); // * already copied
        _stprintf(text1, _T("%s %s"), FileExist,IGCFileList.at(IGC_DrawListIndex).Line1);   
      }   
    }


    _stprintf(text2, _T("%s"), IGCFileList.at(IGC_DrawListIndex).Line2);
    Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));

    PixelRect rc = {0, 0, 0, // DLGSCALE(PICTO_WIDTH),
                    static_cast<PixelScalar>(Sender->GetHeight())};

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

static void OnIGCListEnter(WindowControl *Sender,
                           WndListFrame::ListInfo_t *ListInfo) {
  (void)Sender;
  IGC_CurIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;

  if (IGC_CurIndex >= IGCFileList.size()) {
    IGC_CurIndex = IGCFileList.size() - 1;
  }

  if (IGC_CurIndex >= 0) {
    if (Sender) {
      WndForm *pForm = Sender->GetParentWndForm();
      if (pForm) {
        IGC_DLIndex = IGC_CurIndex;
        OnEnterClicked( (WndButton *)pForm->FindByName(TEXT("cmdEnter")));

      }
    }
  }
}

void StopIGCRead(void) {
  bAbort = true; 
}

static void OnCloseClicked(WndButton *pWnd) {
  StopIGCRead();
  if (ThreadState == IDLE_STATE) {
    if (MessageBoxX(MsgToken(2413), MsgToken(2403), mbYesNo) ==
        IdYes) // _@M2413_ "FLARM need reboot for normal operation\n reboot
               // now?"
    {
      if (pWnd) {
        WndForm *pForm = pWnd->GetParentWndForm();
        if (pForm) {
          pForm->SetTimerNotify(0, NULL); // reset Timer
          pForm->SetModalResult(mrCancel);
        }
      }
    }
  }
}

static bool OnTimer(WndForm *pWnd) {
  TCHAR Tmp[STATUS_TXT_LEN];

  if (pWnd) {
    WndForm *pForm = pWnd->GetParentWndForm();
    if (pForm) {
      UpdateList();
      if (ThreadState == IDLE_STATE) {
        WndButton *wb = (WndButton *)pForm->FindByName(TEXT("cmdClose"));
        wb->SetCaption(MsgToken(186)); // _@M186_ "Close"
#ifdef PRPGRESS_DLG
        CloseIGCProgressDialog();
#endif
        pForm->SetTimerNotify(0, NULL); // reset Timer
        if (bShowMsg) {
          switch (DownloadError) {
          case REC_NO_ERROR:
            _sntprintf(Tmp, STATUS_TXT_LEN, _T("%s\n%s"),
                       IGCFileList.at(IGC_DLIndex).Line1, MsgToken(2406));
            break; // 	_@M2406_ "IGC File download complete"
          case REC_TIMEOUT_ERROR:
            _tcscpy(Tmp, MsgToken(2407));
            break; // _@M2407_ "Error: receive timeout
          case REC_CRC_ERROR:
            _tcscpy(Tmp, MsgToken(2408));
            break; // _@M2408_ "Error: CRC checksum fail"
          case REC_ABORTED:
            _tcscpy(Tmp, MsgToken(2415));
            break; // _@M2415_ "IGC Download aborted!"
          case FILENAME_ERROR:
            _tcscpy(Tmp, MsgToken(2409));
            break; // _@M2409_ "Error: invalid filename"
          case FILE_OPEN_ERROR:
            _tcscpy(Tmp, MsgToken(2410));
            break; // _@M2410_ "Error: can't open file"
          case IGC_RECEIVE_ERROR:
            _tcscpy(Tmp, MsgToken(2411));
            break; // _@M2411_ "Error: Block invalid"
          case REC_NO_DEVICE:
            _tcscpy(Tmp, MsgToken(2401));
            break; // _@M2401_ "No Device found"

          default:
            _tcscpy(Tmp, MsgToken(2412));
            break; // _@M2412_ "Error: unknown"
          }
          if (MessageBoxX(Tmp, MsgToken(2398), mbOk) ==
              IdYes) // _@M2406_ "Error: communication timeout"Reboot"
          {
          }
        }
        DownloadError = REC_NO_ERROR;
      } else {
#ifdef PRPGRESS_DLG
        IGCProgressDialogText(szStatusText); // update progress dialog text
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
    EndCallBackEntry()};

void LeaveBinModeWithReset(DeviceDescriptor_t *d) {
  if (d != NULL) {
    int Sequence = 0;

    LockFlightData();
    GPS_INFO.FLARM_Available = false;
    UnlockFlightData();

    if (deb_)
      StartupStore(TEXT("EXIT & RESET"));
    SendBinBlock(d, Sequence, EXIT, NULL, 0);
    SetBinaryModeFlag(false);
    StartupStore(TEXT("$PFLAR,0*55\r\n"));
    d->Com->WriteString(TEXT("$PFLAR,0*55\r\n"));
    if (deb_)
      StartupStore(TEXT("$PFLAR,0*55\r\n"));
  }
}

class FlarmResourceLock {
public:
  FlarmResourceLock() {
    StartupStore(TEXT(".... Enter ResourceLock FLARM%s"), NEWLINE);
    StartIGCReadThread();
  };
  ~FlarmResourceLock() {
    StartupStore(TEXT(".... Leave ResourceLock%s"), NEWLINE);
    StopIGCReadThread();
    IGCFileList.clear();
  }
};

ListElement *dlgIGCSelectListShowModal(DeviceDescriptor_t *d) {

  FlarmResourceLock ResourceGuard; // simply need to exist for recource Lock/Unlock
  StartupStore(TEXT(".... StartIGCReadThread%s"), NEWLINE);

  bShowMsg = false;
  bAbort = false;
  bFilled = false;
  /*************************************************/
  ThreadState = OPEN_BIN_STATE;
  /*************************************************/
  std::unique_ptr<WndForm> wf(dlgLoadFromXML(IGCCallBackTable, ScreenLandscape
                                            ? IDR_XML_MULTISELECTLIST_L
                                            : IDR_XML_MULTISELECTLIST_P));

  if (wf) {
    WndButton *wb = (WndButton *)wf->FindByName(TEXT("cmdClose"));
    wb->SetCaption(MsgToken(670)); // _@M670_ "Stop"

    wIGCSelectListList =
        (WndListFrame *)wf->FindByName(TEXT("frmMultiSelectListList"));
    LKASSERT(wIGCSelectListList != NULL);
    wIGCSelectListList->SetBorderKind(BORDERLEFT);
    wIGCSelectListList->SetEnterCallback(OnIGCListEnter);

    wIGCSelectListListEntry = (WndOwnerDrawFrame *)wf->FindByName(
        TEXT("frmMultiSelectListListEntry"));
    if (wIGCSelectListListEntry) {
      /*
       * control height must contains 2 text Line
       * Check and update Height if necessary
       */
      LKWindowSurface windowSurface(*main_window);
      LKBitmapSurface tmpSurface(windowSurface, 1, 1);
      const auto oldFont =
          tmpSurface.SelectObject(wIGCSelectListListEntry->GetFont());
      const int minHeight =
          2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
      tmpSurface.SelectObject(oldFont);
      const int wHeight = wIGCSelectListListEntry->GetHeight();
      if (minHeight > wHeight) {
        wIGCSelectListListEntry->SetHeight(minHeight);
      }
      wIGCSelectListListEntry->SetCanFocus(true);
    }
    UpdateList();

    wf->SetTimerNotify(GC_TIMER_INTERVAL, OnTimer); // check for end of download every 250ms
    wf->ShowModal();
  }
  DownloadError = REC_NOMSG; // don't show an error msg on initialisation
  LeaveBinModeWithReset(d);  // reset Flarm after leaving dialog

  ThreadState = IDLE_STATE;
  return pIGCResult;
}

int ReadFlarmIGCFile(DeviceDescriptor_t *d, uint8_t IGC_FileIndex) {

  if (d == NULL)
    return 0;

  static FILE *file_ptr = NULL;
  static int TimeCnt = 0;
  static uint16_t Sequence;
  static uint8_t retrys = 0;
  static uint8_t TimeOutFactor = 1;
  uint8_t pByteBlk[MAX_FLARM_ANSWER_LEN];
  pByteBlk[0] = IGC_FileIndex;
  uint16_t blocksize = 1;
  uint16_t RecSequence;
  uint8_t RecCommand;
  uint8_t err = REC_NO_ERROR;

  static thread_state OldThreadState = IDLE_STATE;

  if (OldThreadState == ThreadState) // state watchdog
  {
    if (TimeCnt++ > 3000 / GC_IDLETIME) // no state change for 3s ?
    {
      ThreadState = ABORT_STATE; // abort!
      StartupStore(TEXT("STATE WATCHDOG timeout after %ums in State %i"),
                   TimeCnt * GC_IDLETIME, ThreadState);
    }
  } else {
    OldThreadState = ThreadState; // remember new state
    TimeCnt = 0;
  }

  if (bAbort) {
    ThreadState = ABORT_STATE;
    bAbort = false;
  }

  if (ThreadState == IDLE_STATE)
    return 0;

  if (d != NULL)
  {
    switch(ThreadState)
    {
        case IDLE_STATE:
        break;
    /********************  OPEN_BIN_STATE ******************************/
        case OPEN_BIN_STATE:
          retrys = 0;
          d->Com->WriteString(TEXT("$PFLAX\r\n")); // set to binary
          if (deb_)
            StartupStore(TEXT("$PFLAX\r "));
          ThreadState = PING_STATE_TX;
          SetBinaryModeFlag(true);
        break;

        /*******************  PING_STATE_TX  ********************************/
        case PING_STATE_TX:
              if (deb_)
        StartupStore(TEXT("PING "));
#ifdef NO_FAKE_FLARM
          if (retrys++ >= 15) {
            ThreadState = ERROR_STATE;
            return 0;
          }
#endif
          ListElementType NewElement;
          _sntprintf(NewElement.Line1, LST_STRG_LEN, _T("        PING Flarm %u/15"),
                     retrys);
          _tcscpy(NewElement.Line2, _T("        ... "));
          IGCFileList.clear();
          IGCFileList.push_back(NewElement);
          SendBinBlock(d, Sequence++, PING, NULL, 0);
          TimeCnt = 0;

          ThreadState = PING_STATE_RX;
        break;
            /********************  PING_STATE_RX **********************************/
        case PING_STATE_RX:
          if (!BlockReceived()) {
            if (deb_)
              StartupStore(TEXT("WAIT FOR PING ANSWER %ums"),
                           TimeCnt * GC_IDLETIME);
            if (TimeCnt > (1000 / GC_IDLETIME)) {
              err = REC_TIMEOUT_ERROR;
              ThreadState = PING_STATE_TX;
            }
            return 0; // no data? leave thread and wait for next call
          }
          err = RecBinBlock(d, &RecSequence, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);
          ThreadState = PING_STATE_TX;
          if (err == REC_NO_ERROR) {
            retrys = 0;
            ThreadState = SELECTRECORD_STATE_TX;
            IGCFileList.clear(); // empty list
          }
        break;
        /*******************  SELECTRECORD_STATE_TX ***************************/
        case SELECTRECORD_STATE_TX:
          if (deb_)
            StartupStore(TEXT("RECORD_STATE_TX "));
          pByteBlk[0] = IGCFileList.size();
          SendBinBlock(d, Sequence++, SELECTRECORD, &pByteBlk[0], 1);
          ThreadState = SELECTRECORD_STATE_RX;
          DownloadError = REC_NO_ERROR;
        break;

        /*******************  SELECTRECORD_STATE_RX ***************************/
        case SELECTRECORD_STATE_RX:		
          if (!BlockReceived()) {
            if (deb_)
              StartupStore(TEXT("SELECTRECORD_STATE_RX %ums"),
                           TimeCnt * GC_IDLETIME);
            if (TimeCnt > (GC_BLK_RECTIMEOUT / GC_IDLETIME)) {
              err = REC_TIMEOUT_ERROR;
              ThreadState = SELECTRECORD_STATE_TX;
              if (retrys++ > MAX_RETRY) {
                ThreadState = ABORT_STATE;
                err = REC_TIMEOUT_ERROR;
              }
            }
            return 0; // no data? leave thread and wait for next call
          }
          err = RecBinBlock(d, &RecSequence, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);
          if (RecCommand == ACK)
            ThreadState = READRECORD_STATE_TX;
              else
        ThreadState = ALL_RECEIVED_STATE;

          if (err)
            ThreadState = ABORT_STATE;
          else
            retrys = 0;
        break;

        /******************  READRECORD_STATE_TX ******************************/
        case READRECORD_STATE_TX:
          if (deb_)
            StartupStore(TEXT("READRECORD_STATE_RX "));
          SendBinBlock(d, Sequence++, GETRECORDINFO, NULL, 0);

          ThreadState = READRECORD_STATE_RX;
        break;
        /******************  READRECORD_STATE_RX ******************************/
    case READRECORD_STATE_RX:
          if (!BlockReceived()) {
            if (deb_)
              StartupStore(TEXT("READRECORD_STATE_RX %ums"), TimeCnt * GC_IDLETIME);
            if (TimeCnt > (GC_BLK_RECTIMEOUT / GC_IDLETIME)) {
              err = REC_TIMEOUT_ERROR;
              ThreadState = READRECORD_STATE_TX;
              if (retrys++ > MAX_RETRY) {
                ThreadState = ABORT_STATE;
                err = REC_TIMEOUT_ERROR;
              }
            }

            return 0; // no data? leave thread and wait for next call
          }
          err = RecBinBlock(d, &RecSequence, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);
          if (err) {
            ThreadState = ABORT_STATE;
            return 0;
          }
          retrys = 0;
          pByteBlk[blocksize++] = 0;
          if (RecCommand == ACK) {
            TCHAR TempString[255];
            ListElementType NewElement;
            for (uint16_t i = 0; i < blocksize - 2; i++)
              TempString[i] = (TCHAR)pByteBlk[i + 2];
            if (deb_)
              StartupStore(TEXT("> %s "), TempString);
            TCHAR empty[3] = _T("");
            TCHAR *remaining = NULL;
            TCHAR *Filename = _tcstok_r(TempString, TEXT("|"), &remaining);
            if (Filename == NULL) {
              Filename = empty;
            };
            TCHAR *Date = _tcstok_r(NULL, TEXT("|"), &remaining);
            if (Date == NULL) {
              Date = empty;
            };
            TCHAR *Takeoff = _tcstok_r(NULL, TEXT("|"), &remaining);
            if (Takeoff == NULL) {
              Takeoff = empty;
            };
            TCHAR *Duration = _tcstok_r(NULL, TEXT("|"), &remaining);
            if (Duration == NULL) {
              Duration = empty;
            };
            TCHAR *Pilot = _tcstok_r(NULL, TEXT("|"), &remaining);
            if (Pilot == NULL) {
              Pilot = empty;
            };
            TCHAR *CN = _tcstok_r(NULL, TEXT("|"), &remaining);
            if (CN == NULL) {
              CN = empty;
            };
            _stprintf(NewElement.Line1, _T("%s (%s  [%5s])"), Filename, Date,
                      Takeoff);
            _stprintf(NewElement.Line2, _T("%s"), Duration);
            if (Pilot) {
              _tcscat(NewElement.Line2, _T(" "));
              _tcscat(NewElement.Line2, Pilot);
            };
            if (CN) {
              _tcscat(NewElement.Line2, _T(" "));
              _tcscat(NewElement.Line2, CN);
            };
            IGCFileList.push_back(NewElement);
          }

          if (RecCommand != ACK)
            ThreadState = ALL_RECEIVED_STATE;
          else
            ThreadState = SELECTRECORD_STATE_TX;
        break;
        /*******************  ALL_RECEIVED_STATE *****************************/
        case ALL_RECEIVED_STATE:		
          bFilled = true;
          if (deb_)
            StartupStore(TEXT("ALL_RECEIVED_STATE"));
          ThreadState = IDLE_STATE;
        break;

      /**********************  ERROR_STATE *********************************/
        case ERROR_STATE:		                    
          if (deb_)
            StartupStore(TEXT("ERROR_STATE"));
   //   ListElementType NewElement;
          _tcscpy(NewElement.Line1, _T("        Error:"));
          _sntprintf(NewElement.Line2, LST_STRG_LEN, _T("         %s"),
                     MsgToken(2401)); // _@M2401_ "No Device found"
          IGCFileList.clear();
          IGCFileList.push_back(NewElement);
          ThreadState = IDLE_STATE;
          err = REC_NO_DEVICE;
        break;

        /*********************  ABORT STATE ***********************************/
        case ABORT_STATE:
          if (deb_)
            StartupStore(TEXT("ABORT_STATE"));
          if (file_ptr != NULL) // file incomplete?
          {
            fclose(file_ptr);
            file_ptr = NULL;

            TCHAR IGCFilename[MAX_PATH];
            if (GetIGCFilename(IGCFilename, IGC_FileIndex)) {
              lk::filesystem::deleteFile(
                  IGCFilename); // delete incomplete file (after abort) to prevent
                                // "file exists warning
              if (deb_)
                StartupStore(TEXT("delete incomplete IGC File: %s "), IGCFilename);
            }
          }
          if (!err)
            err = REC_ABORTED;
          ThreadState = IDLE_STATE;
        break;

        /******************* START_DOWNLOAD_STATE *****************************/
        case START_DOWNLOAD_STATE:
          Sequence = 0;
          if (IGCFileList.size() < IGC_FileIndex)
            return 0;

          if (deb_)
            StartupStore(TEXT("START_DOWNLOAD_STATE: %s"),
                         IGCFileList.at(IGC_FileIndex).Line1);
          if (file_ptr) {
            fclose(file_ptr);
            file_ptr = NULL;
          }
          err = REC_NO_ERROR;

          TCHAR IGCFilename[MAX_PATH];
          GetIGCFilename(IGCFilename, IGC_FileIndex);
          file_ptr = _tfopen(IGCFilename, TEXT("w"));
          if (file_ptr == NULL) {
            err = FILE_OPEN_ERROR;
          } // #define FILE_OPEN_ERROR 5

          _sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("IGC Dowlnoad File : %s "),
                     IGCFileList.at(IGC_FileIndex).Line1);
          if (deb_)
            StartupStore(_T("%s"), szStatusText);
          SendBinBlock(d, Sequence++, SELECTRECORD, &IGC_FileIndex, 1);
          err = RecBinBlock(d, &RecSequence, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);

          if (err != REC_NO_ERROR) {
            err = IGC_RECEIVE_ERROR;
            ThreadState = ABORT_STATE;
          } else
            ThreadState = READ_STATE_TX;
          Sequence = 0;
          retrys = 0;
          TimeOutFactor = 1;
        break;
    
            /*************************** READ STATE TX ****************************/
        case READ_STATE_TX:		
          blocksize = 0;
          ThreadState = READ_STATE_RX;
          SendBinBlock(d, Sequence, GETIGCDATA, &pByteBlk[0], 0);  
        break;
                /************************** READ STATE RX *****************************/
    case READ_STATE_RX:		
          if (!BlockReceived()) {
            if (TimeCnt > (TimeOutFactor * GC_BLK_RECTIMEOUT /
                           GC_IDLETIME)) // Time factor needed fo the very last
                                         // Flarm Answer only, which need far longer
            {
              if (retrys++ > MAX_RETRY) {
                err = REC_TIMEOUT_ERROR;
                DownloadError = err;
                ThreadState = ABORT_STATE;
                StartupStore(TEXT("%u%% Block:%u  Abort while wait for answer "
                                  "time:%ums  Size:%uByte"),
                             pByteBlk[2], Sequence, TimeCnt * GC_IDLETIME,
                             blocksize);
              } else {
                ThreadState = READ_STATE_TX;
                StartupStore(TEXT("%u%% Block:%u timeout :%ums while wating for "
                                  "answer request Block again %i. time"),
                             pByteBlk[2], Sequence, TimeCnt * GC_IDLETIME, retrys);
              }
            }
            return 0;
          }
          Sequence++;
          retrys = 0;

          if (!err)
            err = RecBinBlock(d, &RecSequence, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);
          if (err) {
            ThreadState = ABORT_STATE;
            StartupStore(
                TEXT("%u%% Block:%u  Abort after read time:%ums  Size:%uByte"),
                pByteBlk[2], Sequence, TimeCnt * GC_IDLETIME, blocksize);
          } else
            ThreadState = READ_STATE_TX;

          if (pByteBlk[2] > 50) // if more that 50% read, increase TimeOutFactor
            TimeOutFactor =
                100000 / GC_BLK_RECTIMEOUT; // reading last FLARM sentences takes up
                                        // to 8s, for whatever reason

          if (err == REC_NO_ERROR)
            _sntprintf(
                szStatusText, STATUS_TXT_LEN, _T("%s: %u%% %s ..."), MsgToken(2400),
                pByteBlk[2],
                IGCFileList.at(IGC_FileIndex).Line1); // _@M2400_ "Downloading"

          if ((Sequence % 10) == 0) {
            StartupStore(TEXT("%u%% Block:%u  Response time:%ums  Size:%uByte"),
                         pByteBlk[2], Sequence, TimeCnt * GC_IDLETIME, blocksize);
          }
          for (int i = 0; i < blocksize - 3; i++) {
            if (file_ptr)
              fputc(pByteBlk[3 + i], file_ptr);
            if (pByteBlk[3 + i] == EOF_)
              RecCommand = EOF_;
          }

          if (err)
            ThreadState = CLOSE_STATE;

          if (RecCommand != ACK)
            ThreadState = CLOSE_STATE;
 
          if (err != REC_NO_ERROR)
            ThreadState = CLOSE_STATE;
        break;

      /************************* CLOSE STATE *********************************/		
        case CLOSE_STATE:
            if (file_ptr) {
              fclose(file_ptr);
              file_ptr = NULL;
            }

          ThreadState = IDLE_STATE;
          if (deb_)
            StartupStore(TEXT("IDLE_STATE"));
          if (err != REC_NO_ERROR) {
            _sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("Error Code:%u"), err);
            //    err = REC_NO_ERROR;
          } else {
            _sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("%s"),
                       MsgToken(2406)); // _@M2406_ "IGC File download complete!"
          }
          if (deb_)
            StartupStore(_T("IGC download complete"));
        break;
        /********************************************************************/
      } // case
    }	// if(d)
  if (err)
    if (DownloadError == REC_NO_ERROR) // no prvious error=
      DownloadError = err;
  return 0;
}

class IGCReadThread : public Poco::Runnable {
public:
  void Start() {
    if (!Thread.isRunning()) {
      bStop = false;
      Thread.start(*this);
    }
  }

  void Stop() {
    if (Thread.isRunning()) {
      bStop = true;
      Thread.join();
    }
  }

protected:
  bool bStop;
  Poco::Thread Thread;

  void run() {
    if (deb_)
      StartupStore(TEXT("IGC Thread Started !"));

    while (!bStop) {
      if (ThreadState != IDLE_STATE) {
        ReadFlarmIGCFile(CDevFlarm::GetDevice(), IGC_DLIndex);
      }
      Poco::Thread::sleep(GC_IDLETIME);
      Poco::Thread::yield();
    }

    if (deb_)
      StartupStore(TEXT("IGC Thread Stopped !"));

  }
};

IGCReadThread IGCReadThreadThreadInstance;

void StartIGCReadThread() {
  if (deb_)
    StartupStore(TEXT("Start IGC Thread !"));
  IGCReadThreadThreadInstance.Start();
}

void StopIGCReadThread() {
  if (deb_)
    StartupStore(TEXT("Stop IGC Thread !"));
  IGCReadThreadThreadInstance.Stop();
}
