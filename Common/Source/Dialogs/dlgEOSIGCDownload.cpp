/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "dlgTools.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "resource.h"
#include "Util/Clamp.hpp"
#include "OS/Sleep.h"
#include "Devices/devLX_EOS_ERA.h"
#include "dlgEOSIGCDownload.h"
#include "dlgIGCProgress.h"
#include "utils/tokenizer.h"
#include "utils/printf.h"

#define EOS_PRPGRESS_DLG    
  
#define LST_STRG_LEN          100
#define STATUS_TXT_LEN        100
#ifdef KOBO
#define GC_IDLETIME           10
#else
#define GC_IDLETIME           2
#endif

#define GC_TIMER_INTERVAL     750

#define deb_                  (0)  // debug output switch

 Mutex DLmutex;

int ReadEOS_IGCFile(DeviceDescriptor_t *d, uint8_t IGC_FileIndex) ;
static void UpdateList(void);

TCHAR DownoadIGCFilename[MAX_NMEA_PAR_LEN];
TCHAR szEOS_DL_StatusText[STATUS_TXT_LEN];
volatile bool  bDlgShown = false; 

enum thread_state {
  IDLE_STATE,
  READRECORD_STATE_TX,
  READRECORD_STATE_RX,
  START_DOWNLOAD_STATE,
  ABORT_STATE,
  ERROR_STATE,
  SIGNAL_STATE,
  ALL_RECEIVED_STATE,
  CLOSE_STATE
};

volatile thread_state EOS_ThreadState = IDLE_STATE;




typedef struct {
  TCHAR Line1[LST_STRG_LEN];
  TCHAR Line2[LST_STRG_LEN];
  uint32_t filesize;
} EOSListElementType;




static bool OnTimer(WndForm *pWnd) ;
class LK_EOS_IGCReadDlg {
public:
  LK_EOS_IGCReadDlg();

  ~LK_EOS_IGCReadDlg();


  void DrawIndex(uint uIdx) { m_IGC_DrawListIndex = uIdx; };

  uint DrawIndex() { return m_IGC_DrawListIndex; };

  void CurIndex(uint uIdx) { m_IGC_CurIndex = uIdx; };

  uint CurIndex() { return m_IGC_CurIndex; };

  void DownloadIndex(uint uIdx) { m_IGC_DownloadIndex = uIdx; };

  uint DownloadIndex() { return m_IGC_DownloadIndex; };

  std::vector<EOSListElementType> *FileList() { return &m_IGCFileList; };

  void DownloadError(int bDLE) { m_iEOSDownloadError = bDLE; };

  int DownloadError() { return m_iEOSDownloadError; };

  
  void ListFilled(bool bfilled) { m_ListFilled = bfilled; };

  int ListFilled() { return m_ListFilled; };


  void SelectList(WndListFrame *pLst) { m_wIGCSelectList = pLst; };

  WndListFrame *SelectList() { return m_wIGCSelectList; };

  void ListEntry(WndOwnerDrawFrame *pLst) { m_wIGCSelectListEntry = pLst; };

  WndOwnerDrawFrame *ListEntry() { return m_wIGCSelectListEntry; };

protected:

  int m_iEOSDownloadError;
  uint m_IGC_DrawListIndex;
  uint m_IGC_CurIndex;
  uint m_IGC_DownloadIndex;
  bool m_ListFilled ;
  WndListFrame *m_wIGCSelectList;
  WndOwnerDrawFrame *m_wIGCSelectListEntry;

  std::vector<EOSListElementType> m_IGCFileList;
};


LK_EOS_IGCReadDlg::LK_EOS_IGCReadDlg() {

  m_iEOSDownloadError = REC_NO_ERROR;
  m_IGC_DrawListIndex = 0;
  m_IGC_CurIndex = 0;
  m_IGC_DownloadIndex = 0;

  m_ListFilled = false;
  m_wIGCSelectList = NULL;
  m_wIGCSelectListEntry = NULL;
};

LK_EOS_IGCReadDlg::~LK_EOS_IGCReadDlg() {
  m_IGCFileList.clear();
};


LK_EOS_IGCReadDlg EOS_IGCReadDialog;



void AddEOSElement(TCHAR Line1[], TCHAR Line2[], uint32_t size) {
  EOSListElementType NewElement;
  _tcscpy(NewElement.Line1, Line1);
  _tcscpy(NewElement.Line2, Line2);
  NewElement.filesize = size;
  EOS_IGCReadDialog.FileList()->push_back(NewElement);
  UpdateList();
}


void EOSListFilled(BOOL filled) {
  EOS_IGCReadDialog.ListFilled(filled);
}


static void OnUpClicked(WndButton *Sender) {
  if (EOS_IGCReadDialog.FileList()->size() == 0) return;
  if (EOS_IGCReadDialog.CurIndex() > 0) {
    EOS_IGCReadDialog.CurIndex(EOS_IGCReadDialog.CurIndex() - 1);
  } else {
    LKASSERT(EOS_IGCReadDialog.FileList()->size() > 0);
    EOS_IGCReadDialog.CurIndex((EOS_IGCReadDialog.FileList()->size() - 1));
  }
  if (EOS_IGCReadDialog.SelectList() != NULL) {
    EOS_IGCReadDialog.SelectList()->SetItemIndexPos(EOS_IGCReadDialog.CurIndex());
    EOS_IGCReadDialog.SelectList()->Redraw();
    if (EOS_IGCReadDialog.ListEntry())
      EOS_IGCReadDialog.ListEntry()->SetFocus();
  }
}

static void OnDownClicked(WndButton *pWnd) {
  (void) pWnd;
  if (EOS_IGCReadDialog.FileList()->size() == 0) return;
  if (EOS_IGCReadDialog.CurIndex() < (EOS_IGCReadDialog.FileList()->size() - 1)) {
    EOS_IGCReadDialog.CurIndex(EOS_IGCReadDialog.CurIndex() + 1);
  } else {
    EOS_IGCReadDialog.CurIndex(0);
  }
  if (EOS_IGCReadDialog.SelectList() != NULL) {
    EOS_IGCReadDialog.SelectList()->SetItemIndexPos(EOS_IGCReadDialog.CurIndex());
    EOS_IGCReadDialog.SelectList()->Redraw();
    if (EOS_IGCReadDialog.ListEntry())
      EOS_IGCReadDialog.ListEntry()->SetFocus();
  }
}


static void OnMultiSelectListListInfo(WndListFrame *Sender, WndListFrame::ListInfo_t *ListInfo) {

  if (EOS_IGCReadDialog.FileList()->size() == 0) return;
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = EOS_IGCReadDialog.FileList()->size();
  } else {
    EOS_IGCReadDialog.DrawIndex(ListInfo->DrawIndex + ListInfo->ScrollIndex);
    EOS_IGCReadDialog.CurIndex(ListInfo->ItemIndex + ListInfo->ScrollIndex);
  }

}

bool GetEOSIGCFilename(TCHAR *IGCFilename, TCHAR *InFilename) {
  if (IGCFilename == NULL)
    return false;
  TCHAR Tmp[MAX_PATH];

  _tcscpy(Tmp, InFilename);
  TCHAR *Filename = lk::tokenizer<TCHAR>(Tmp).Next({_T(' ')});
  if(Filename) {
    LocalPath(IGCFilename, _T(LKD_LOGS), Filename);
    return true;
  }
  return false;
}

static void OnEnterClicked(WndButton *pWnd) {
    (void) pWnd;
  if((!pWnd) || (!EOS_IGCReadDialog.ListFilled())){
    return;
  }
  
  WndForm* pForm = pWnd->GetParentWndForm();
  if(!pForm) {
    return;
  }


  TCHAR Tmp[MAX_PATH];
  if (EOS_IGCReadDialog.FileList()->size() == 0) return;


  if (EOS_IGCReadDialog.CurIndex() >= EOS_IGCReadDialog.FileList()->size()) {
    EOS_IGCReadDialog.CurIndex(EOS_IGCReadDialog.FileList()->size() - 1);
  }
  if (EOS_IGCReadDialog.FileList()->size() < (uint) EOS_IGCReadDialog.CurIndex()) return;
  EOS_IGCReadDialog.DownloadIndex(EOS_IGCReadDialog.CurIndex());
  
  
  TCHAR szTmp[MAX_NMEA_LEN];
  _tcscpy(szTmp, EOS_IGCReadDialog.FileList()->at(EOS_IGCReadDialog.CurIndex()).Line1);
  

  TCHAR *IGCFilename = lk::tokenizer<TCHAR>(szTmp).Next(TEXT(" "));
  _tcscat(IGCFilename, _T(".IGC"));  
  _tcscpy(DownoadIGCFilename, IGCFilename);
  _stprintf(Tmp, _T("%s %s ?"), MsgToken(2404), IGCFilename);
  if (MessageBoxX(Tmp, MsgToken(2404), mbYesNo) == IdYes)  // _@2404 "Download"
  {
    /** check if file already exist and is not empty ************/
    TCHAR PathIGCFilename  [MAX_PATH];
    if (GetEOSIGCFilename(PathIGCFilename, IGCFilename)) {
      if (lk::filesystem::exist(PathIGCFilename))
        if (MessageBoxX(MsgToken(2416), MsgToken(2398), mbYesNo) ==
            IdNo) // _@M2416_ "File already exits\n download anyway?"
        {
          EOS_ThreadState = IDLE_STATE;
          return;
        }
    }
    /************************************************************/
     StartupStore(TEXT("OnEnterClicked Size START_DOWNLOAD_STATE "));
    EOS_ThreadState = START_DOWNLOAD_STATE; // start thread IGC download
    pForm->SetTimerNotify(GC_TIMER_INTERVAL, OnTimer); // check for end of download every 250ms
#ifdef EOS_PRPGRESS_DLG
    CreateIGCProgressDialog();
#endif
  }
}


static void OnMultiSelectListPaintListItem(WndOwnerDrawFrame *Sender, LKSurface &Surface) {

  if (EOS_IGCReadDialog.FileList()->size() == 0) return;
  if (EOS_IGCReadDialog.FileList()->size() < (uint) EOS_IGCReadDialog.DrawIndex()) return;
  if (EOS_IGCReadDialog.DrawIndex() < EOS_IGCReadDialog.FileList()->size()) {
    Surface.SetTextColor(RGB_BLACK);
    TCHAR szTmp[MAX_NMEA_LEN] ;
    TCHAR text1[MAX_NMEA_LEN] = {TEXT("IGC File")};
    TCHAR text2[MAX_NMEA_LEN] = {TEXT("date")};
    _tcscpy(szTmp, EOS_IGCReadDialog.FileList()->at(EOS_IGCReadDialog.DrawIndex()).Line1);
    _tcscpy(text1, EOS_IGCReadDialog.FileList()->at(EOS_IGCReadDialog.DrawIndex()).Line1);
    _tcscpy(text2, EOS_IGCReadDialog.FileList()->at(EOS_IGCReadDialog.DrawIndex()).Line2);

    /* extract filname */
    TCHAR *IGCFilename = lk::tokenizer<TCHAR>(szTmp).Next(TEXT(" "));

    TCHAR PathAndFilename[MAX_PATH];
    _tcscat(IGCFilename, _T(".IGC"));
    LocalPath(PathAndFilename, _T(LKD_LOGS), IGCFilename);     // add path
    TCHAR Tmp[MAX_NMEA_LEN];
    _tcscpy(Tmp, text1);     // missing
    if (Appearance.UTF8Pictorials)                             // use UTF8 symbols?
    {
      if (lk::filesystem::exist(PathAndFilename))                // check if file exists
        lk::snprintf(text1, _T("✔ %s"), Tmp); // already copied
    } else {
      if (lk::filesystem::exist(PathAndFilename))                // check if file exists
       lk::snprintf(text1, _T("* %s"), Tmp);// already copied
    }
    Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
    PixelRect rc = {0, 0, 0, // DLGSCALE(PICTO_WIDTH),
                    static_cast<PixelScalar>(Sender->GetHeight()) };

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
    if (Sender) {
      WndForm *pForm = Sender->GetParentWndForm();
      if (pForm) {
        WndButton* wb = ( WndButton*)pForm->FindByName(TEXT("cmdEnter"));

        if (wb)          
         OnEnterClicked( wb);
      }
    }
}



static void OnCloseClicked(WndButton *pWnd) {
  if (pWnd) {
    WndForm *pForm = pWnd->GetParentWndForm();
   
    if (pForm) {
      pForm->SetTimerNotify(0, NULL); // disable Timer
      pForm->SetModalResult(mrCancel);
    }
  }
}


void OnAbort_EOS_IGC_FileRead(void) {

      EOS_ThreadState = ABORT_STATE;

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


class EOSResourceLock {
public:
  EOSResourceLock() {
  StartupStore(TEXT(".... Enter ResourceLock EOS   %s"), NEWLINE);
    StartEOS_IGCReadThread();
      MapWindow::SuspendDrawingThread();
  };
  ~EOSResourceLock() {
    StartupStore(TEXT(".... Leave ResourceLock%s"), NEWLINE);
    StopEOS_IGCReadThread();
    EOS_IGCReadDialog.FileList()->clear();
    MapWindow::ResumeDrawingThread();
  }
};


ListElement *dlgEOSIGCSelectListShowModal(void) {

  EOSResourceLock ResourceGuard;  //simply need to exist for recource Lock/Unlock

  ListElement *pIGCResult = NULL;


  WndForm* wf = dlgLoadFromXML(IGCCallBackTable,
                      ScreenLandscape ? IDR_XML_MULTISELECTLIST_L : IDR_XML_MULTISELECTLIST_P);

  if (wf) {
    EOS_IGCReadDialog.SelectList((WndListFrame *) wf->FindByName(TEXT("frmMultiSelectListList")));
    LKASSERT(EOS_IGCReadDialog.SelectList() != NULL);
    EOS_IGCReadDialog.SelectList()->SetBorderKind(BORDERLEFT);
    EOS_IGCReadDialog.SelectList()->SetEnterCallback(OnIGCListEnter);

    EOS_IGCReadDialog.ListEntry(
            (WndOwnerDrawFrame *) wf->FindByName(TEXT("frmMultiSelectListListEntry")));
    if (EOS_IGCReadDialog.ListEntry()) {
      /*
       * control height must contains 2 text Line
       * Check and update Height if necessary
       */
      LKWindowSurface windowSurface(*main_window);
      LKBitmapSurface tmpSurface(windowSurface, 1, 1);
      const auto oldFont = tmpSurface.SelectObject(EOS_IGCReadDialog.ListEntry()->GetFont());
      const int minHeight = 2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
      tmpSurface.SelectObject(oldFont);
      const int wHeight = EOS_IGCReadDialog.ListEntry()->GetHeight();
      if (minHeight > wHeight) {
        EOS_IGCReadDialog.ListEntry()->SetHeight(minHeight);
      }
      EOS_IGCReadDialog.ListEntry()->SetCanFocus(true);
    } else
      LKASSERT(0);


    EOS_IGCReadDialog.FileList()->clear();
    EOS_IGCReadDialog.SelectList()->ResetList();
    wf->SetTimerNotify(GC_TIMER_INTERVAL, OnTimer); // check for end of download every 250ms
    wf->ShowModal();
    EOS_IGCReadDialog.FileList()->clear();


    delete wf;
    wf = NULL;
  }
  return pIGCResult;
}





int8_t CRC_Update(int8_t m_byCrc, uint8_t d)
{
  #define CRCPOLY    0x69

  int8_t tmp;
  uint8_t count;

  for (count = 0; ++count <= 8; d <<= 1) {
    tmp = m_byCrc ^ d;
    m_byCrc <<= 1;
    if (tmp < 0)
      m_byCrc ^= CRCPOLY;
  }
  return m_byCrc;
}

void SendBinBlock(DeviceDescriptor_t *d, uint8_t Command, uint8_t FileId, uint16_t Sequence) 
{
  if (d == NULL)
    return;

uint8_t m_datagram[20];
int8_t  m_byCrc = 0xFF;
uint16_t byteCount=0;   
ConvUnion BlkNo   ; 
ConvUnion FLightNo; 

  
      BlkNo.val = Sequence;
      FLightNo.val = FileId;
      m_datagram[byteCount++] = STX;                                     // 1
      m_datagram[byteCount++] = Command;                                 // 2
      m_datagram[byteCount++] = FLightNo.byte[0];                        // 3
      m_datagram[byteCount++] = FLightNo.byte[1];                        // 4
      m_datagram[byteCount++] = BlkNo.byte[0];                           // 5
      m_datagram[byteCount++] = BlkNo.byte[1];                           // 6

      for (uint16_t i =0; i < byteCount; i ++)
      {
        d->Com->Write( m_datagram[i]);
        m_byCrc= CRC_Update(m_byCrc, m_datagram[i]);
      }
      d->Com->Write(m_byCrc);
       if (deb_) StartupStore(TEXT("Send CRC: %02X"),(uint8_t) m_byCrc); 

  if (deb_) {
    StartupStore(TEXT("\r\n===="));
  }
}



static uint8_t RecBinBlock(DeviceDescriptor_t *d, FILE *pf_IGCFile, uint16_t Sequence, uint16_t* BytesRead) {
uint8_t error = REC_NO_ERROR;
uint8_t CRC_in;
uint16_t Timeout = REC_TIMEOUT;
#define MAX_BLK_SIZE 512
uint8_t m_datagram[MAX_BLK_SIZE];
uint8_t m_byCrc = 0xff;
uint8_t bRecByte = 0;;
uint16_t byteCount=0;   
ConvUnion BlkSize; 
ConvUnion BlkNo; 
 if(BytesRead)   *BytesRead =0;
  PeriodClock clock;
  clock.Update();
  uint16_t cnt =0;
  if (deb_) StartupStore(TEXT("=== RecBinBlock : "));
  cnt = 0;
  do {
      error = EOSRecChar(d, &bRecByte, 500);
      // expect ACK within next 40 char
      if((cnt++) > 512) {
        StartupStore(TEXT("ACK Timeout "));
        return REC_TIMEOUT_ERROR; 
      }
  } while ((bRecByte != ACK) && (error == REC_NO_ERROR));
  
  if(error != REC_NO_ERROR) {
    StartupStore(TEXT("ACK fail! Error code:%i"),error);    
    return error;
  }
  
  m_byCrc= CRC_Update(m_byCrc, bRecByte);

  if (deb_) {
     if (deb_) StartupStore(TEXT("ACK OK!"));
  }
                    
  error = EOSRecChar16(d, &BlkSize.val , Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  m_byCrc= CRC_Update(m_byCrc, BlkSize.byte[0]);
  m_byCrc= CRC_Update(m_byCrc, BlkSize.byte[1]);
 
  if (BlkSize.val > MAX_BLK_SIZE) {
     StartupStore(TEXT("RecBinBlock : Invalid Block Size %u"), BlkSize.val);
    return REC_INVALID_SIZE;
  }
 
  error = EOSRecChar16(d, &BlkNo.val , Timeout);
  if (error != REC_NO_ERROR) {
    return error;
  }
  m_byCrc= CRC_Update(m_byCrc, BlkNo.byte[0]);
  m_byCrc= CRC_Update(m_byCrc, BlkNo.byte[1]);
 
  
  if (deb_) StartupStore(TEXT("RecBinBlockNo : %u"), BlkNo.val);
  

  if (BlkSize.val > 0) {
    for (uint16_t i = 0; i < (BlkSize.val); i++) {
      error = EOSRecChar(d, &bRecByte , Timeout);
      if (error != REC_NO_ERROR) {
         if (deb_)  StartupStore(TEXT("Rec Block Body error: %u!"), error);
        return error;       
      }
      else
      {
        m_datagram[byteCount++] = bRecByte;
        m_byCrc= CRC_Update(m_byCrc, bRecByte);
      }
    }
  }
  
  error = EOSRecChar(d, &CRC_in , Timeout);
   
  if(BlkNo.val != Sequence)
  {
    StartupStore(TEXT("Error: Rec-Block wrong Block :expected %i received %i!"), Sequence, BlkNo.val);
    return REC_WRONG_BLOCK;
  }   
  if (m_byCrc != CRC_in) {
    error = REC_CRC_ERROR;
    StartupStore(TEXT("Rec Block CRC error!"));
  } else {
    error = REC_NO_ERROR;
      for (uint16_t i = 0; i < (BlkSize.val); i++) {
        fputc( m_datagram[i], pf_IGCFile);
      if(BytesRead)   *BytesRead = BlkSize.val;
    }    
    if (deb_) { StartupStore(TEXT("Rec Block %i received! size:%i"),BlkNo.val, BlkSize.val ); }
  }
  if(BlkSize.val <= 0)
    error = REC_ZERO_BLOCK;
  

  return error;
}


class EOS_IGCReadThread : public Poco::Runnable {
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
      StartupStore(TEXT("EOS IGC Thread Started !")); 
    while (!bStop) {

      ReadEOS_IGCFile(DevLX_EOS_ERA::GetDevice(), EOS_IGCReadDialog.DownloadIndex());

      Poco::Thread::sleep(GC_IDLETIME);
      Poco::Thread::yield();
    }
    SetEOSBinaryModeFlag(false);
    if (deb_)
      StartupStore(TEXT("EOS IGC Thread Stopped !"));

  }
};

EOS_IGCReadThread EOS_IGCReadThreadThreadInstance;




static void UpdateList(void) {
  if (EOS_IGCReadDialog.SelectList() != NULL) {
    EOS_IGCReadDialog.SelectList()->ResetList();
    EOS_IGCReadDialog.SelectList()->Redraw();
  }
}

void EOS_StopIGCRead(void) {
  ScopeLock lock(DLmutex);
  EOS_ThreadState = ABORT_STATE;
}


void StartEOS_IGCReadThread() {
  if (deb_)
    StartupStore(TEXT("Start IGC Thread !"));
  EOS_IGCReadThreadThreadInstance.Start();
}

void StopEOS_IGCReadThread() {
  if (deb_)
    StartupStore(TEXT("Stop IGC Thread !"));
  EOS_IGCReadThreadThreadInstance.Stop();
}



int ReadEOS_IGCFile(DeviceDescriptor_t *d, uint8_t IGC_FileIndex) {
 ScopeLock lock(DLmutex);
static volatile uint16_t BlockNo=1; 
static uint8_t ErrCnt = 0;
static uint32_t FileSize = 0;
static uint32_t  BytesRead =0;
#define MAX_ERROR_CNT 3
uint16_t error= REC_NO_ERROR;
  if (d == NULL)
    return 0;
  static FILE *pf_IGCFile= NULL;
  TCHAR PathIGCFilename  [MAX_PATH];
  switch (EOS_ThreadState)
  {
    
    case START_DOWNLOAD_STATE:    
      FileSize =  EOS_IGCReadDialog.FileList()->at(IGC_FileIndex).filesize;
      BytesRead =0;
      SetEOSBinaryModeFlag(true);
      StartupStore(TEXT("EOS/ERA/10k IGC File Download start"));
      BlockNo = 0;
      ErrCnt = 0;
      bDlgShown  = false;
      szEOS_DL_StatusText[0] = '\0';
    
      GetEOSIGCFilename(PathIGCFilename, DownoadIGCFilename);
      pf_IGCFile = _tfopen( PathIGCFilename, TEXT("w"));
      if(pf_IGCFile == NULL) 
      {  
        EOS_IGCReadDialog.DownloadError(FILE_OPEN_ERROR);  
        StartupStore(TEXT("EOS/ERA/10k IGC File open error"));
        EOS_ThreadState = ABORT_STATE;
      }  
      else
        EOS_ThreadState = READRECORD_STATE_TX;
    break;
    
    case READRECORD_STATE_TX:     
      SendBinBlock(d, GET_FLIGTH_BLK,IGC_FileIndex+1, BlockNo);
      EOS_ThreadState = READRECORD_STATE_RX;
    break;  
    
    case READRECORD_STATE_RX: 
      if(!EOSBlockReceived())
      {
         Poco::Thread::sleep(GC_IDLETIME);
        Poco::Thread::yield();
      }
      else
      {uint16_t Bytes;
        error = RecBinBlock(d, pf_IGCFile, BlockNo, &Bytes);
        BytesRead += Bytes;
        
        if(FileSize > 0) // (BytesRead*100)/FileSize
        {
          double fPercent = ((double)BytesRead*100.0)/(double)FileSize;
          lk::snprintf(szEOS_DL_StatusText, _T("%3.1f%% %s"),fPercent, DownoadIGCFilename); 
        }
        else
          lk::snprintf(szEOS_DL_StatusText, _T("%i %s"),BlockNo, DownoadIGCFilename);

        EOS_IGCReadDialog.DownloadError(error);
        
        EOS_ThreadState = READRECORD_STATE_TX;
        if (error == REC_ZERO_BLOCK)
          EOS_ThreadState = ALL_RECEIVED_STATE;
        else        
          if (error == REC_NO_ERROR)
          {
            BlockNo++;    
            ErrCnt = 0;
          }
          else
          { ErrCnt++;
            if (deb_) StartupStore(TEXT("ErrCnt Cnt: %i"),ErrCnt);
            if(ErrCnt > MAX_ERROR_CNT )          
              EOS_ThreadState = SIGNAL_STATE;
          }
      }
    break;     
      
    case ALL_RECEIVED_STATE:      
      StartupStore(TEXT("EOS/ERA/10k IGC File Download end (%i Blocks)"),BlockNo);    
      lk::snprintf(szEOS_DL_StatusText, _T("%s %s"), DownoadIGCFilename,MsgToken(2406));
      fclose (pf_IGCFile);
      pf_IGCFile = NULL;
      EOS_IGCReadDialog.DownloadError(REC_NO_ERROR);
      EOS_ThreadState = SIGNAL_STATE;     
    break;    
    

    case ABORT_STATE:
      fclose (pf_IGCFile);
       pf_IGCFile = NULL;
      GetEOSIGCFilename(PathIGCFilename, DownoadIGCFilename);   
      lk::snprintf(szEOS_DL_StatusText, _T("%s %s"), DownoadIGCFilename,MsgToken(2415));
      lk::filesystem::deleteFile( PathIGCFilename); 
      EOS_IGCReadDialog.DownloadError(REC_ABORTED);
      EOS_ThreadState = SIGNAL_STATE;       
    break;
    
    case SIGNAL_STATE:       
      if (bDlgShown)
        EOS_ThreadState =  IDLE_STATE;
      
    break;
    default:
    case  IDLE_STATE:
      SetEOSBinaryModeFlag(false);
    break;
  }    
      
  return 0;
}




static bool OnTimer(WndForm *pWnd) {
  if (pWnd) {    
    WndForm *pForm = pWnd->GetParentWndForm();
    if (pForm) {
     
      if (EOS_ThreadState == IDLE_STATE) 
      {
         pForm->SetTimerNotify(0,NULL); 
      }  
      else
      if (EOS_ThreadState == SIGNAL_STATE) 
      {
        pForm->SetTimerNotify(0,NULL); 
#ifdef EOS_PRPGRESS_DLG
        CloseIGCProgressDialog();
#endif     
        {
          TCHAR Tmp[STATUS_TXT_LEN];

          switch (EOS_IGCReadDialog.DownloadError()) {
          case REC_NO_ERROR:
            lk::snprintf(Tmp, STATUS_TXT_LEN, _T("%s\n%s"),
                       DownoadIGCFilename, MsgToken(2406));
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
            _tcscpy(Tmp, MsgToken(2475));
            break; // _@M2401_ "No Device found"
          case REC_WRONG_BLOCK: 
             _tcscpy(Tmp, MsgToken(2474)); // _@M2474_ "wrong block received"
            break;
          default:            
            _tcscpy(Tmp, MsgToken(2412));
            break; // _@M2412_ "Error: unknown"
          }
          if (MessageBoxX(Tmp, MsgToken(2398), mbOk) ==
              IdYes) // _@M2406_ "Error: communication timeout"Reboot"
          {

          }
          bDlgShown = true;
          
        }
 
      } else {

#ifdef EOS_PRPGRESS_DLG        
        IGCProgressDialogText(szEOS_DL_StatusText); // update progress dialog text
#endif
      }
    }
  }
  return true;
}