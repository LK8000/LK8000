/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "Dialogs.h"
#include "Devices/devFlarm.h"
#include "Devices/Flarm/BinaryProtocol.h"

#include "dlgIGCProgress.h"
#include "OS/Sleep.h"
#include "Util/Clamp.hpp"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"
#include "dlgFlarmIGCDownload.h"
#include "utils/tokenizer.h"
#include "utils/printf.h"
#include "LocalPath.h"

#define MAX_FLARM_ANSWER_LEN 640 
      // FLARM Docu does not tell the max. answer len
      // The max. ever received length on IGC read was 485, so 640 seem to be a
      // good value
#define WATCHDOG_TIMEOUT  15000
#define GC_BLK_RECTIMEOUT 1000
#define GC_IDLETIME       10
#define GC_TIMER_INTERVAL 500
#define REC_TIMEOUT       1000 // receive timeout in ms
#define MAX_RETRY         3
#define LST_STRG_LEN      100
#define STATUS_TXT_LEN    200
#define PRPGRESS_DLG

#define deb_
#ifdef deb_
  #define deb_Log(...) StartupStore(__VA_ARGS__)
#else
  #define deb_Log(...)
#endif

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
  DOWNLOAD_START_ANS,
  ABORT_STATE,
  ERROR_STATE,
  ALL_RECEIVED_STATE,
  CLOSE_STATE
};


class thread_state_with_timer final {
private:
  thread_state m_state = IDLE_STATE;
  PeriodClock timer;
      
public:

  // to change state
  void state(thread_state v) {
    if(m_state != v) {
      timer.Update();
      m_state = v;
    }
  }

  // to get current state
  thread_state state() const {
    return m_state;
  }

  // to get the number of milliseconds elapsed since the last state change
  int get_elapsed_time() const {
    return timer.Elapsed();
  }

  // to check whether the specified duration (in ms) has passed since the last state change
  bool check_timeout(unsigned duration) const {
    return timer.Check(duration);
  }
};

static thread_state_with_timer FlarmReadIGC;



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

TCHAR szStatusText[STATUS_TXT_LEN];

bool bShowMsg = false;

uint8_t RecChar8(DeviceDescriptor_t* d, uint8_t *inchar, uint16_t Timeout) {
  uint8_t Tmp;
  uint8_t err = RecChar(d, &Tmp, Timeout);
  if (err == REC_NO_ERROR) {
    if (Tmp == Flarm::ESCAPE) {
      err = RecChar(d, &Tmp, Timeout);
      if (err == REC_NO_ERROR) {
        if (Tmp == Flarm::ESC_ESC) {
          Tmp = Flarm::ESCAPE;
          deb_Log(TEXT("ESC_ESC"));
        }
        if (Tmp == Flarm::ESC_START) {
          Tmp = Flarm::STARTFRAME;
          deb_Log(TEXT("ESC_START"));
        }
      }
    }
  }
  *inchar = Tmp;
  return err;
}

uint8_t RecChar16(DeviceDescriptor_t* d, uint16_t *inchar, uint16_t Timeout) {
  ConvUnion tmp;
  bool error = RecChar8(d, &(tmp.byte[0]), Timeout);
  if (error == REC_NO_ERROR) {
    error = RecChar8(d, &(tmp.byte[1]), Timeout);
  }
  *inchar = tmp.val;
  return error;
}

template<size_t size>
static uint8_t RecBinBlock(DeviceDescriptor_t* d, uint16_t *Sequence, uint8_t *Command,
                    uint8_t (&pBlock)[size], uint16_t *blocksize, uint16_t Timeout) {
  uint8_t error = REC_NO_ERROR;
  uint8_t inchar;
  uint8_t Version;
  uint16_t CRC_in, CRC_calc = 0;

  PeriodClock clock;
  clock.Update();
  do {
      error = RecChar(d, &inchar, Timeout);
  } while ((inchar != Flarm::STARTFRAME) && (error == REC_NO_ERROR));

  if(error != REC_NO_ERROR) {
    deb_Log(TEXT("STARTFRAME fail! Error code:%i"),error);
    return error;
  }

  deb_Log(TEXT("STARTFRAME OK!"));
                    
  error = RecChar16(d, blocksize, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }

  if ((*blocksize) > size) {
    StartupStore(TEXT("RecBinBlock : Invalid Block Size %u"), *blocksize);
    return REC_INVALID_SIZE;
  }

  CRC_calc = Flarm::crc_update(CRC_calc, *blocksize);
  deb_Log(TEXT("Block Size %u"), *blocksize);

  error = RecChar8(d, &Version, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  CRC_calc = Flarm::crc_update(CRC_calc, Version);
  deb_Log(TEXT("Block Ver %u"), Version);

  error = RecChar16(d, Sequence, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  CRC_calc = Flarm::crc_update(CRC_calc, *Sequence);
  deb_Log(TEXT("Block Seq %u"), *Sequence);

  error = RecChar8(d, Command, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  CRC_calc = Flarm::crc_update(CRC_calc, *Command);
  deb_Log(TEXT("Block Cmd %02X"), *Command);

  error = RecChar16(d, &CRC_in, Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }

  deb_Log(TEXT("Block CRC %04X"), CRC_in);
  deb_Log(TEXT("Header  received!"));
  if (*blocksize <= 8) {
    return REC_INVALID_SIZE;
  }

  (*blocksize) -= 8;

  if (*blocksize >= 2) {
    error = RecChar8(d, &inchar, Timeout);
    if (error != REC_NO_ERROR) {
      return error;
    }
    (*Sequence) = inchar;
    CRC_calc = Flarm::crc_update(CRC_calc, inchar);
    error = RecChar8(d, &inchar, Timeout);
    if (error != REC_NO_ERROR) {
      return error;
    }
    (*Sequence) = (*Sequence) << 8 | inchar;
    CRC_calc = Flarm::crc_update(CRC_calc, inchar);

    (*blocksize) -= 2;
  }

  for (uint16_t i = 0; i < *blocksize; i++) {
    error = RecChar8(d, &inchar, Timeout);
    if (error != REC_NO_ERROR) {
      StartupStore(TEXT("Rec Block Body error: %u!"), error);
      return error;
    }
    pBlock[i] = inchar;
    CRC_calc = Flarm::crc_update(CRC_calc, inchar);
  }
  if (CRC_calc != CRC_in) {
    error = REC_CRC_ERROR;
    StartupStore(TEXT("Rec Block CRC error!"));
  } else {
    error = REC_NO_ERROR;
    deb_Log(TEXT("Rec Block received!"));
  }
  deb_Log(TEXT("payload : %s"), data_string(pBlock, *blocksize).c_str());
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

static void OnMultiSelectListListInfo(WndListFrame *Sender,
                                      WndListFrame::ListInfo_t *ListInfo) {

  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = IGCFileList.size();
  } else {
    IGC_DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
    IGC_CurIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
  }
}

static
bool GetIGCFilename(TCHAR (&IGCFilename)[MAX_PATH], int Idx) {
  TCHAR Tmp[MAX_PATH];
  lk::strcpy(Tmp, IGCFileList.at(Idx).Line1);
  TCHAR *Filename = lk::tokenizer<TCHAR>(Tmp).Next(TEXT(" "));
  if(Filename) {
    LocalPath(IGCFilename, _T(LKD_LOGS), Filename);
    return true;
  }
  return false;
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
  bShowMsg = false;
  _stprintf(Tmp, _T("%s %s ?"), MsgToken<2404>(),
            IGCFileList.at(IGC_DLIndex).Line1);
  if (MessageBoxX(Tmp, MsgToken<2404>(), mbYesNo) == IdYes) // _@2404 "Download"
  {
    /** check if file already exist and is not empty ************/
    TCHAR IGCFilename[MAX_PATH];
    if (GetIGCFilename(IGCFilename, IGC_DLIndex)) {
      if (lk::filesystem::exist(IGCFilename))
        if (MessageBoxX(MsgToken<2416>(), MsgToken<2398>(), mbYesNo) ==
            IdNo) // _@M2416_ "File already exits\n download anyway?"
        {
          FlarmReadIGC.state(IDLE_STATE);
          return;
        }
    }
    /************************************************************/
    FlarmReadIGC.state(START_DOWNLOAD_STATE); // start thread IGC download
    pForm->SetTimerNotify(GC_TIMER_INTERVAL, OnTimer); // check for end of download every 250ms
#ifdef PRPGRESS_DLG
    CreateIGCProgressDialog();
#endif
  }
}

static void OnMultiSelectListPaintListItem(WndOwnerDrawFrame *Sender,
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
    lk::strcpy(text1, IGCFileList.at(IGC_DrawListIndex).Line1);
    if (GetIGCFilename(IGCFilename, IGC_DrawListIndex)) {
      if (lk::filesystem::exist(IGCFilename)) // file exists
      {
        if (Appearance.UTF8Pictorials) // use UTF8 symbols?
          lk::strcpy(FileExist, _T("✔")); // check! already copied
        else
          lk::strcpy(FileExist, _T("*")); // * already copied
        _stprintf(text1, _T("%s %s"), FileExist,IGCFileList.at(IGC_DrawListIndex).Line1);   
      }   
    }


    _stprintf(text2, _T("%s"), IGCFileList.at(IGC_DrawListIndex).Line2);
    Surface.SetBkColor(RGB_WHITE);

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

  IGC_CurIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;

  if (IGC_CurIndex >= IGCFileList.size()) {
    IGC_CurIndex = IGCFileList.size() - 1;
  }

  if (Sender) {
    WndForm *pForm = Sender->GetParentWndForm();
    if (pForm) {
      IGC_DLIndex = IGC_CurIndex;
      OnEnterClicked( pForm->FindByName<WndButton>(TEXT("cmdEnter")));
    }
  }
}

void StopIGCRead(void) {
  bAbort = true; 
}

static void OnCloseClicked(WndButton *pWnd) {
  StopIGCRead();
  if (FlarmReadIGC.state() == IDLE_STATE) {
    if (MessageBoxX(MsgToken<2413>(), MsgToken<2403>(), mbYesNo) ==
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
      pForm->SetTimerNotify(0, nullptr);
      UpdateList();
      if (FlarmReadIGC.state() == IDLE_STATE) {
        WndButton *wb = pForm->FindByName<WndButton>(TEXT("cmdClose"));
        wb->SetCaption(MsgToken<186>()); // _@M186_ "Close"
#ifdef PRPGRESS_DLG
        CloseIGCProgressDialog();
#endif
        pForm->SetTimerNotify(0, NULL); // reset Timer
        if (bShowMsg) {
          switch (DownloadError) {
          case REC_NO_ERROR:
            _sntprintf(Tmp, STATUS_TXT_LEN, _T("%s\n%s"),
                       IGCFileList.at(IGC_DLIndex).Line1, MsgToken<2406>());
            break; // 	_@M2406_ "IGC File download complete"
          case REC_TIMEOUT_ERROR:
            lk::strcpy(Tmp, MsgToken<2407>());
            break; // _@M2407_ "Error: receive timeout
          case REC_CRC_ERROR:
            lk::strcpy(Tmp, MsgToken<2408>());
            break; // _@M2408_ "Error: CRC checksum fail"
          case REC_ABORTED:
            lk::strcpy(Tmp, MsgToken<2415>());
            break; // _@M2415_ "IGC Download aborted!"
          case FILENAME_ERROR:
            lk::strcpy(Tmp, MsgToken<2409>());
            break; // _@M2409_ "Error: invalid filename"
          case FILE_OPEN_ERROR:
            lk::strcpy(Tmp, MsgToken<2410>());
            break; // _@M2410_ "Error: can't open file"
          case IGC_RECEIVE_ERROR:
            lk::strcpy(Tmp, MsgToken<2411>());
            break; // _@M2411_ "Error: Block invalid"
          case REC_NO_DEVICE:
            lk::strcpy(Tmp, MsgToken<2401>());
            break; // _@M2401_ "No Device found"

          default:
            lk::strcpy(Tmp, MsgToken<2412>());
            break; // _@M2412_ "Error: unknown"
          }
          bShowMsg = false;
          if (MessageBoxX(Tmp, MsgToken<2398>(), mbOk) ==
              IdYes) // _@M2406_ "Error: communication timeout"Reboot"
          {
          }
        }
        DownloadError = REC_NO_ERROR;
      } else {
#ifdef PRPGRESS_DLG
        IGCProgressDialogText(szStatusText); // update progress dialog text
#endif
        pForm->SetTimerNotify(GC_TIMER_INTERVAL, OnTimer); // recall if not idle
      }
    }
  }
  return true;
}

static CallBackTableEntry_t IGCCallBackTable[] = {
    CallbackEntry(OnMultiSelectListPaintListItem),
    CallbackEntry(OnMultiSelectListListInfo),
    CallbackEntry(OnCloseClicked),
    CallbackEntry(OnUpClicked),
    CallbackEntry(OnEnterClicked),
    CallbackEntry(OnDownClicked),
    EndCallbackEntry()};

void LeaveBinMode(DeviceDescriptor_t* d) {
  if (d != NULL) {
    int Sequence = 0;

    LockFlightData();
    GPS_INFO.FLARM_Available = false;
    UnlockFlightData();

    deb_Log(TEXT("Flarm exit BIN mode!"));
    Flarm::Exit(d, Sequence);
    SetBinaryModeFlag(false);

  }
}

void LeaveBinModeWithReset(DeviceDescriptor_t* d) {
  if (d != NULL) {
    LeaveBinMode(d); 
    d->Com->WriteString("$PFLAR,0*55\r\n");
    deb_Log(TEXT("Flarm Reset!\r\n"));	
  }
}

class FlarmResourceLock {
public:
  FlarmResourceLock() {
    MapWindow::SuspendDrawingThread();
    StartupStore(TEXT(".... Enter ResourceLock FLARM%s"), NEWLINE);
    StartIGCReadThread();
  }

  ~FlarmResourceLock() {
    StartupStore(TEXT(".... Leave ResourceLock%s"), NEWLINE);
    StopIGCReadThread();
    IGCFileList.clear();
    MapWindow::ResumeDrawingThread();
  }
};

void dlgIGCSelectListShowModal(DeviceDescriptor_t* d) {

  FlarmResourceLock ResourceGuard; // simply need to exist for recource Lock/Unlock
  StartupStore(TEXT(".... StartIGCReadThread%s"), NEWLINE);

  bShowMsg = false;
  bAbort = false;
  bFilled = false;
  /*************************************************/
  FlarmReadIGC.state(OPEN_BIN_STATE);
  /*************************************************/
  std::unique_ptr<WndForm> wf(dlgLoadFromXML(IGCCallBackTable, ScreenLandscape
                                            ? IDR_XML_MULTISELECTLIST_L
                                            : IDR_XML_MULTISELECTLIST_P));

  if (wf) {
    WndButton *wb = wf->FindByName<WndButton>(TEXT("cmdClose"));
    wb->SetCaption(MsgToken<670>()); // _@M670_ "Stop"

    wIGCSelectListList =
        wf->FindByName<WndListFrame>(TEXT("frmMultiSelectListList"));
    LKASSERT(wIGCSelectListList != NULL);
    wIGCSelectListList->SetBorderKind(BORDERLEFT);
    wIGCSelectListList->SetEnterCallback(OnIGCListEnter);

    wIGCSelectListListEntry = wf->FindByName<WndOwnerDrawFrame>(
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
  LeaveBinMode(d);  //  Flarm exit BIN mode

  FlarmReadIGC.state(IDLE_STATE);
}

void EnterBinMode(DeviceDescriptor_t* d)
{
  d->Com->WriteString("$PFLAX\r\n"); // set to binary
  deb_Log(TEXT("$PFLAX\r "));
  FlarmReadIGC.state(PING_STATE_TX);
  SetBinaryModeFlag(true);
  Sleep(100);
}


bool FormatListEntry(uint8_t* pByteBlk, uint16_t blocksize)
{
  if(blocksize >=MAX_FLARM_ANSWER_LEN)
    return false;

  TCHAR TempString[255];
  ListElementType NewElement;
  for (uint16_t i = 0; i < blocksize; i++) {
    TempString[i] = pByteBlk[i];
  }

  TCHAR empty[3] = _T("");
  lk::tokenizer<TCHAR> tok(TempString);
  TCHAR *Filename = tok.Next({_T('|')});
  if (Filename == NULL) {
    Filename = empty;
  };
  TCHAR *Date = tok.Next({_T('|')});
  if (Date == NULL) {
    Date = empty;
  };
  TCHAR *Takeoff = tok.Next({_T('|')});
  if (Takeoff == NULL) {
    Takeoff = empty;
  };
  TCHAR *Duration = tok.Next({_T('|')});
  if (Duration == NULL) {
    Duration = empty;
  };
  TCHAR *Pilot = tok.Next({_T('|')});
  if (Pilot == NULL) {
    Pilot = empty;
  };
  TCHAR *CN = tok.Next({_T('|')});
  if (CN == NULL) {
    CN = empty;
  };

  lk::snprintf(NewElement.Line1, _T("%s (%s  [%5s])"), Filename, Date, Takeoff);
  lk::snprintf(NewElement.Line2, _T("%s"), Duration);
  if (Pilot) {
    _tcscat(NewElement.Line2, _T(" "));
    _tcscat(NewElement.Line2, Pilot);
  };
  if (CN) {
    _tcscat(NewElement.Line2, _T(" "));
    _tcscat(NewElement.Line2, CN);
  };
  IGCFileList.push_back(NewElement);
  return true;
}

static
int ReadFlarmIGCFile(DeviceDescriptor_t* d, uint8_t IGC_FileIndex) {
  if (d == NULL)
    return 0; 
  static uint32_t TotalSize =0;
  static FILE *file_ptr = NULL;

  static uint16_t SeqNo = 0;
  uint16_t ResponseSeqNo = 0;

  static uint8_t retrys = 0;
  static uint8_t TimeOutFactor = 1;
  uint8_t pByteBlk[MAX_FLARM_ANSWER_LEN] = {};
  pByteBlk[0] = IGC_FileIndex;
  uint16_t blocksize = 1;
  uint8_t RecCommand = Flarm::NACK;
  uint8_t err = REC_NO_ERROR;


  if (bAbort) {
    FlarmReadIGC.state(ABORT_STATE);
    bAbort = false;
  }


  switch(FlarmReadIGC.state())
  {
      case IDLE_STATE:
      break;
      /********************  OPEN_BIN_STATE ******************************/
      case OPEN_BIN_STATE:
        retrys = 0;
        EnterBinMode(d);
        FlarmReadIGC.state(PING_STATE_TX);
      break;

      /*******************  PING_STATE_TX  ********************************/
      case PING_STATE_TX:
        deb_Log(TEXT("PING "));

#ifdef NO_FAKE_FLARM
        if (retrys++ >= 15) {
          FlarmReadIGC.state(ERROR_STATE);
          return 0;
        }
#endif

        ListElementType NewElement;
        _sntprintf(NewElement.Line1, LST_STRG_LEN, _T("        PING Flarm %u/15"),
                    retrys);
        lk::strcpy(NewElement.Line2, _T("        ... "));
        IGCFileList.clear();
        IGCFileList.push_back(NewElement);
        Flarm::Ping(d, ++SeqNo);

        FlarmReadIGC.state(PING_STATE_RX);
      break;
      /********************  PING_STATE_RX **********************************/
      case PING_STATE_RX:
        if (!BlockReceived()) {

          deb_Log(TEXT("WAIT FOR PING ANSWER %ums"),
                          FlarmReadIGC.get_elapsed_time());
          if (FlarmReadIGC.check_timeout(1000)) {
            err = REC_TIMEOUT_ERROR;
            FlarmReadIGC.state(PING_STATE_TX);
          }
        }
        else {
          err = RecBinBlock(d, &ResponseSeqNo, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);
          if (err == REC_NO_ERROR) { 
            if (ResponseSeqNo == SeqNo) {
              retrys = 0;
              FlarmReadIGC.state(SELECTRECORD_STATE_TX);
              IGCFileList.clear(); // empty list
            }
            else {
              deb_Log("Invalid response SeqNo");
            }
          }
          else {
            FlarmReadIGC.state(PING_STATE_TX);
          }
        }
      break;
      /*******************  SELECTRECORD_STATE_TX ***************************/
      case SELECTRECORD_STATE_TX:
        deb_Log(TEXT("RECORD_STATE_TX "));
        Flarm::SelectRecord(d, ++SeqNo, IGCFileList.size());
        FlarmReadIGC.state(SELECTRECORD_STATE_RX);
        DownloadError = REC_NO_ERROR;
      break;

      /*******************  SELECTRECORD_STATE_RX ***************************/
      case SELECTRECORD_STATE_RX:		
        if (!BlockReceived()) {

          deb_Log(TEXT("SELECTRECORD_STATE_RX %ums"),
                          FlarmReadIGC.get_elapsed_time());

          if  (FlarmReadIGC.check_timeout(GC_BLK_RECTIMEOUT)) 
          {
            FlarmReadIGC.state( SELECTRECORD_STATE_TX);
            if (retrys++ > MAX_RETRY) {
              FlarmReadIGC.state( ABORT_STATE);
              err = REC_TIMEOUT_ERROR;
            }
          }
        }
        else
        {
          err = RecBinBlock(d, &ResponseSeqNo, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);
          if (err) {
            FlarmReadIGC.state(ABORT_STATE);
          }
          else if (RecCommand == Flarm::ACK) {
            if (ResponseSeqNo == SeqNo) {          
              FlarmReadIGC.state( READRECORD_STATE_TX);
            }
            else {
              deb_Log("Invalid response SeqNo");
            }
            retrys = 0;
          }
          else {
            FlarmReadIGC.state(ALL_RECEIVED_STATE);
            retrys = 0;
          }
        }
      break;
      /******************  READRECORD_STATE_TX ******************************/
      case READRECORD_STATE_TX:
        deb_Log(TEXT("READRECORD_STATE_RX "));
        Flarm::GetRecordInfo(d, ++SeqNo);
        FlarmReadIGC.state( READRECORD_STATE_RX);
      break;
      /******************  READRECORD_STATE_RX ******************************/
      case READRECORD_STATE_RX:
        if (!BlockReceived()) {
          deb_Log(TEXT("READRECORD_STATE_RX %ums"),  FlarmReadIGC.get_elapsed_time());

          if  (FlarmReadIGC.check_timeout(GC_BLK_RECTIMEOUT)) 
          {
            err = REC_TIMEOUT_ERROR;
            FlarmReadIGC.state( READRECORD_STATE_TX);
            if (retrys++ > MAX_RETRY) {
              FlarmReadIGC.state( ABORT_STATE);
              err = REC_TIMEOUT_ERROR;
            }
          }
        }
        else
        {
          err = RecBinBlock(d, &ResponseSeqNo, &RecCommand, pByteBlk, &blocksize, REC_TIMEOUT);
          if (err) {
            FlarmReadIGC.state(ABORT_STATE);
          }
          else if (RecCommand == Flarm::ACK) {
            retrys = 0;
            if (ResponseSeqNo == SeqNo) {
              pByteBlk[blocksize++] = 0;
              if (ResponseSeqNo == SeqNo) {
                FormatListEntry( pByteBlk, blocksize);
                FlarmReadIGC.state( SELECTRECORD_STATE_TX);
              }
              else {
                deb_Log("Invalid response SeqNo");
              }
            }
          }
          else {
            retrys = 0;
            FlarmReadIGC.state(ALL_RECEIVED_STATE);
          }
        }
      break;
      /*******************  ALL_RECEIVED_STATE *****************************/
      case ALL_RECEIVED_STATE:		
        bFilled = true;
        deb_Log(TEXT("ALL_RECEIVED_STATE"));
        FlarmReadIGC.state( IDLE_STATE);
      break;

    /**********************  ERROR_STATE *********************************/
      case ERROR_STATE:		                    
        deb_Log(TEXT("ERROR_STATE"));
  //   ListElementType NewElement;
        lk::strcpy(NewElement.Line1, _T("        Error:"));
        _sntprintf(NewElement.Line2, LST_STRG_LEN, _T("         %s"),
                    MsgToken<2401>()); // _@M2401_ "No Device found"
        IGCFileList.clear();
        IGCFileList.push_back(NewElement);
        FlarmReadIGC.state( IDLE_STATE);
        err = REC_NO_DEVICE;
      break;

      /*********************  ABORT STATE ***********************************/
      case ABORT_STATE:
        deb_Log(TEXT("ABORT_STATE"));
        if (file_ptr != NULL) // file incomplete?
        {
          fclose(file_ptr);
          file_ptr = NULL;

          TCHAR IGCFilename[MAX_PATH];
          if (GetIGCFilename(IGCFilename, IGC_FileIndex)) {
            lk::filesystem::deleteFile(
                IGCFilename); // delete incomplete file (after abort) to prevent
                              // "file exists warning
            deb_Log(TEXT("delete incomplete IGC File: %s "), IGCFilename);
          }
        }
        if (!err)
          err = REC_ABORTED;
        FlarmReadIGC.state( IDLE_STATE);
      break;

      /******************* START_DOWNLOAD_STATE *****************************/
      case START_DOWNLOAD_STATE:
        SeqNo = 0;
        TotalSize =0;
        if (IGCFileList.size() < IGC_FileIndex)
        {
          FlarmReadIGC.state( PING_STATE_TX);
          return 0;
        }
        /*
            we must resend the binary mode command before a new IGC file donwload,
            because PowerFlarm automatcally return from binary mode after a while
            so we must re-enable it in case user waited too long to start download
        */
        EnterBinMode(d);
        deb_Log(TEXT("START_DOWNLOAD_STATE: %s"),
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
        
        deb_Log(_T("%s"), szStatusText);
  
        Flarm::SelectRecord(d, ++SeqNo, IGC_FileIndex);
        bShowMsg = true;
        retrys = 0;
        TimeOutFactor = 1;          
        FlarmReadIGC.state( DOWNLOAD_START_ANS);
      break;
        /*************************** DOWNLOAD_START_ANS **********************/      
      case DOWNLOAD_START_ANS:
        err = RecBinBlock(d, &ResponseSeqNo, &RecCommand, pByteBlk, &blocksize,
                          REC_TIMEOUT);
        if (err != REC_NO_ERROR) {
          err = IGC_RECEIVE_ERROR;
          FlarmReadIGC.state(ABORT_STATE);
        }
        else if (RecCommand == Flarm::ACK) {
          if (ResponseSeqNo == SeqNo) {
            FlarmReadIGC.state(READ_STATE_TX);
          }
          else {
            deb_Log("Invalid response SeqNo");
          }
        }
        break;

        /*************************** READ STATE TX ***************************/
      case READ_STATE_TX:
        blocksize = 0;
        DownloadError = REC_NO_ERROR;
        Flarm::GetIGCData(d, ++SeqNo);
        FlarmReadIGC.state( READ_STATE_RX);
      break;
      /************************** READ STATE RX *****************************/
      case READ_STATE_RX:
        if (!BlockReceived()) {
          if(FlarmReadIGC.check_timeout(TimeOutFactor*GC_BLK_RECTIMEOUT))
          {
            if (retrys++ > MAX_RETRY) {
              err = REC_TIMEOUT_ERROR;
              DownloadError = err;

              StartupStore(TEXT("%u%% Abort after %u Blocks, while wait for answer "
                              "time:%ums  Size:%uByte"),
                          pByteBlk[2], SeqNo, FlarmReadIGC.get_elapsed_time(),
                          TotalSize);
              FlarmReadIGC.state( ABORT_STATE);              
            } else {

              StartupStore(TEXT("%u%% %u. timeout %ums at Block %u (%u Bytes), request Block again!"),
                            pByteBlk[2], retrys, FlarmReadIGC.get_elapsed_time(), SeqNo ,TotalSize );
              FlarmReadIGC.state( READ_STATE_TX);
            }
          }
        }
        else
        {
          RecCommand = Flarm::NACK;
          err = RecBinBlock(d, &ResponseSeqNo, &RecCommand, pByteBlk, &blocksize,TimeOutFactor * REC_TIMEOUT);
          if (err) {
            if (retrys++ > MAX_RETRY) {
              DownloadError = err;
              StartupStore(
                TEXT("%u%% Block Error:%u Block %u  Abort after %u retrys read time:%ums  Size: %u Bytes"),
                pByteBlk[2],err, SeqNo, retrys, FlarmReadIGC.get_elapsed_time() , TotalSize);
              FlarmReadIGC.state( ABORT_STATE);
            }
            else
            {
              StartupStore(
                TEXT("%u%% Block Error:%u Block %u retry #%i after read time:%ums  Size:%u Bytes"),
                pByteBlk[2],err, SeqNo, retrys, FlarmReadIGC.get_elapsed_time(), TotalSize);              
                FlarmReadIGC.state( READ_STATE_TX);
            }
          }
          else if (RecCommand == Flarm::ACK) {
            if (ResponseSeqNo == SeqNo) {
              retrys  =0;
              TotalSize += blocksize;
            
              if (pByteBlk[0] > 50) { // if more that 50% read, increase TimeOutFactor
              TimeOutFactor =
                  WATCHDOG_TIMEOUT / GC_BLK_RECTIMEOUT; // reading last FLARM sentences takes up
                                          // to 8s, for whatever reason
            }

              _sntprintf(
                    szStatusText, STATUS_TXT_LEN, _T("%s: %u%% %s ..."), MsgToken<2400>(),
                    pByteBlk[0],
                    IGCFileList.at(IGC_FileIndex).Line1); // _@M2400_ "Downloading"

              static int  prevPercent =0;
              if (abs((int)pByteBlk[0] - prevPercent) >= 5) {
                prevPercent =  pByteBlk[0];
                StartupStore(TEXT("%u%% %u. Block (%u Bytes)  Response time:%ums  Total:%u Bytes"),
                              pByteBlk[0], SeqNo, blocksize, FlarmReadIGC.get_elapsed_time(), TotalSize);
              }
              for (int i = 1; i < blocksize; i++) {
                if (file_ptr)
                  fputc(pByteBlk[i], file_ptr);
                if (pByteBlk[i] == Flarm::EOF_)
                  RecCommand = Flarm::EOF_;
              }
              if (RecCommand == Flarm::EOF_) {
                FlarmReadIGC.state(CLOSE_STATE);
              }
              else {
                FlarmReadIGC.state(READ_STATE_TX);
              }
            }
          }
        }
      break;

      /************************* CLOSE STATE *********************************/		
      case CLOSE_STATE:
        if (file_ptr) {
          fclose(file_ptr);
          file_ptr = NULL;
        }

        FlarmReadIGC.state( IDLE_STATE);
        deb_Log(TEXT("IDLE_STATE"));
        if (err != REC_NO_ERROR) {
          _sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("Error Code:%u"), err);
          //    err = REC_NO_ERROR;
        } else {
          _sntprintf(szStatusText, STATUS_TXT_LEN, TEXT("%s"),
                      MsgToken<2406>()); // _@M2406_ "IGC File download complete!"
        }
        deb_Log(_T("IGC download complete"));
      break;
      /********************************************************************/
    } // case

    if(FlarmReadIGC.check_timeout(WATCHDOG_TIMEOUT))// no state change for a longer time ?
    {    
      StartupStore(TEXT("STATE WATCHDOG timeout after %ums in State %i"),
                   FlarmReadIGC.get_elapsed_time() , FlarmReadIGC.state());
      FlarmReadIGC.state( ABORT_STATE); // abort!
    }
 
  if (err)
    if (DownloadError == REC_NO_ERROR) // no previous error=
      DownloadError = err;
  return 0;
}

class IGCReadThread : public Thread {
public:
  IGCReadThread() : Thread("IGCReadThread") { }

  bool Start() override {
    if (!IsDefined()) {
      bStop = false;
      return Thread::Start();
    }
    return false;
  }

  void Stop() {
    if (IsDefined()) {
      bStop = true;
      Join();
    }
  }

protected:
  bool bStop = false;

  void Run() override {
    deb_Log(TEXT("IGC Thread Started !"));

    while (!bStop) {
      if (FlarmReadIGC.state() != IDLE_STATE) {
        ReadFlarmIGCFile(CDevFlarm::GetDevice(), IGC_DLIndex);
      }
      Sleep(GC_IDLETIME);
      Poco::Thread::yield();
    }
    deb_Log(TEXT("IGC Thread Stopped !"));

  }
};

IGCReadThread IGCReadThreadThreadInstance;

void StartIGCReadThread() {
  deb_Log(TEXT("Start IGC Thread !"));
  IGCReadThreadThreadInstance.Start();
}

void StopIGCReadThread() {
  deb_Log(TEXT("Stop IGC Thread !"));
  IGCReadThreadThreadInstance.Stop();
}
