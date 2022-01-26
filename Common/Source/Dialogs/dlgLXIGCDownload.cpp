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
#include "Devices/devLXNano3.h"
#include "dlgLXIGCDownload.h"
#include "utils/tokenizer.h"


#define LST_STRG_LEN          100

#define deb_                  (1)  // debug output switch


typedef struct {
  TCHAR Line1[LST_STRG_LEN];
  TCHAR Line2[LST_STRG_LEN];
} ListElementType;


class LKIGCReadDlg {
public:
  LKIGCReadDlg();

  ~LKIGCReadDlg();


  void DrawIndex(uint uIdx) { m_IGC_DrawListIndex = uIdx; };

  uint DrawIndex() { return m_IGC_DrawListIndex; };

  void CurIndex(uint uIdx) { m_IGC_CurIndex = uIdx; };

  uint CurIndex() { return m_IGC_CurIndex; };

  void DownloadIndex(uint uIdx) { m_IGC_DownloadIndex = uIdx; };

  uint DownloadIndex() { return m_IGC_DownloadIndex; };

  std::vector<ListElementType> *FileList() { return &m_IGCFileList; };

  void DownloadError(int bDLE) { m_iDownloadError = bDLE; };

  int DownloadError() { return m_iDownloadError; };


  void SelectList(WndListFrame *pLst) { m_wIGCSelectList = pLst; };

  WndListFrame *SelectList() { return m_wIGCSelectList; };

  void ListEntry(WndOwnerDrawFrame *pLst) { m_wIGCSelectListEntry = pLst; };

  WndOwnerDrawFrame *ListEntry() { return m_wIGCSelectListEntry; };

protected:

  int m_iDownloadError;
  uint m_IGC_DrawListIndex;
  uint m_IGC_CurIndex;
  uint m_IGC_DownloadIndex;

  WndListFrame *m_wIGCSelectList;
  WndOwnerDrawFrame *m_wIGCSelectListEntry;

  std::vector<ListElementType> m_IGCFileList;
};


LKIGCReadDlg::LKIGCReadDlg() {

  m_iDownloadError = REC_NO_ERROR;
  m_IGC_DrawListIndex = 0;
  m_IGC_CurIndex = 0;
  m_IGC_DownloadIndex = 0;

  m_wIGCSelectList = NULL;
  m_wIGCSelectListEntry = NULL;
};

LKIGCReadDlg::~LKIGCReadDlg() {
  m_IGCFileList.clear();
};


LKIGCReadDlg LX_IGCReadDialog;


BOOL ListUpdate = false;

void AddElement(TCHAR Line1[], TCHAR Line2[]) {
  ListElementType NewElement;
  _tcscpy(NewElement.Line1, Line1);
  _tcscpy(NewElement.Line2, Line2);
  LX_IGCReadDialog.FileList()->push_back(NewElement);

  ListUpdate = true;

}


static bool TimerUpdateList(WndForm *pWnd) {
//static void UpdateList(void) {
if (LX_IGCReadDialog.FileList()->size() == 0) return false;
if(ListUpdate)
  if (LX_IGCReadDialog.SelectList() != NULL) {
    LX_IGCReadDialog.SelectList()->ResetList();

    LX_IGCReadDialog.SelectList()->Redraw();
    LX_IGCReadDialog.SelectList()->SetItemIndexPos(LX_IGCReadDialog.CurIndex());
   // LX_IGCReadDialog.SelectList()->Redraw();
    ListUpdate = false;
  }
return true;
}

static void OnUpClicked(WndButton *Sender) {
  if (LX_IGCReadDialog.FileList()->size() == 0) return;
  if (LX_IGCReadDialog.CurIndex() > 0) {
    LX_IGCReadDialog.CurIndex(LX_IGCReadDialog.CurIndex() - 1);
  } else {
    LKASSERT(LX_IGCReadDialog.FileList()->size() > 0);
    LX_IGCReadDialog.CurIndex((LX_IGCReadDialog.FileList()->size() - 1));
  }
  if (LX_IGCReadDialog.SelectList() != NULL) {
    LX_IGCReadDialog.SelectList()->SetItemIndexPos(LX_IGCReadDialog.CurIndex());
    LX_IGCReadDialog.SelectList()->Redraw();
    if (LX_IGCReadDialog.ListEntry())
      LX_IGCReadDialog.ListEntry()->SetFocus();
  }
}

static void OnDownClicked(WndButton *pWnd) {
  (void) pWnd;
  if (LX_IGCReadDialog.FileList()->size() == 0) return;
  if (LX_IGCReadDialog.CurIndex() < (LX_IGCReadDialog.FileList()->size() - 1)) {
    LX_IGCReadDialog.CurIndex(LX_IGCReadDialog.CurIndex() + 1);
  } else {
    LX_IGCReadDialog.CurIndex(0);
  }
  if (LX_IGCReadDialog.SelectList() != NULL) {
    LX_IGCReadDialog.SelectList()->SetItemIndexPos(LX_IGCReadDialog.CurIndex());
    LX_IGCReadDialog.SelectList()->Redraw();
    if (LX_IGCReadDialog.ListEntry())
      LX_IGCReadDialog.ListEntry()->SetFocus();
  }
}


static void OnMultiSelectListListInfo(WindowControl *Sender, WndListFrame::ListInfo_t *ListInfo) {
  (void) Sender;
  if (LX_IGCReadDialog.FileList()->size() == 0) return;
  if (ListInfo->DrawIndex == -1) {
    ListInfo->ItemCount = LX_IGCReadDialog.FileList()->size();
  } else {
    LX_IGCReadDialog.DrawIndex(ListInfo->DrawIndex + ListInfo->ScrollIndex);
    LX_IGCReadDialog.CurIndex(ListInfo->ItemIndex + ListInfo->ScrollIndex);
  }

}

bool GetLXIGCFilename(TCHAR *IGCFilename, TCHAR *InFilename) {
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
  TCHAR Tmp[MAX_PATH];
  if (LX_IGCReadDialog.FileList()->size() == 0) return;
  (void) pWnd;

  if (LX_IGCReadDialog.CurIndex() >= LX_IGCReadDialog.FileList()->size()) {
    LX_IGCReadDialog.CurIndex(LX_IGCReadDialog.FileList()->size() - 1);
  }
  if (LX_IGCReadDialog.FileList()->size() < (uint) LX_IGCReadDialog.CurIndex()) return;
  LX_IGCReadDialog.DownloadIndex(LX_IGCReadDialog.CurIndex());
  TCHAR szTmp[MAX_NMEA_LEN];
  _tcscpy(szTmp, LX_IGCReadDialog.FileList()->at(LX_IGCReadDialog.DrawIndex()).Line1);

  TCHAR *IGCFilename = lk::tokenizer<TCHAR>(szTmp).Next({_T(' ')});

  _stprintf(Tmp, _T("%s %s ?"), MsgToken(2404), IGCFilename);
  if (MessageBoxX(Tmp, MsgToken(2404), mbYesNo) == IdYes)  // _@2404 "Download"
  {
    /** check if file already exist and is not empty ************/
    TCHAR PathIGCFilename[MAX_PATH];
    if (GetLXIGCFilename(PathIGCFilename, IGCFilename)) {
      if (lk::filesystem::exist(PathIGCFilename))
        if (MessageBoxX(MsgToken(2416), MsgToken(2398), mbYesNo) ==
            IdNo) // _@M2416_ "File already exits\n download anyway?"
        {
          return;
        }
    }
    DevLXNanoIII::OnStartIGC_FileRead(IGCFilename);
  }
}


static void OnMultiSelectListPaintListItem(WndOwnerDrawFrame *Sender, LKSurface &Surface) {

  if (LX_IGCReadDialog.FileList()->size() == 0) return;
  if (LX_IGCReadDialog.FileList()->size() < (uint) LX_IGCReadDialog.DrawIndex()) return;
  if (LX_IGCReadDialog.DrawIndex() < LX_IGCReadDialog.FileList()->size()) {
    Surface.SetTextColor(RGB_BLACK);
    TCHAR szTmp[MAX_NMEA_LEN] ;
    TCHAR text1[MAX_NMEA_LEN] = {TEXT("IGC File")};
    TCHAR text2[MAX_NMEA_LEN] = {TEXT("date")};
    _tcscpy(szTmp, LX_IGCReadDialog.FileList()->at(LX_IGCReadDialog.DrawIndex()).Line1);
    _tcscpy(text1, LX_IGCReadDialog.FileList()->at(LX_IGCReadDialog.DrawIndex()).Line1);
    _tcscpy(text2, LX_IGCReadDialog.FileList()->at(LX_IGCReadDialog.DrawIndex()).Line2);

    /* extract filname */
    TCHAR *IGCFilename = lk::tokenizer<TCHAR>(szTmp).Next(TEXT(" "));

    TCHAR PathAndFilename[MAX_PATH];
   
    LocalPath(PathAndFilename, _T(LKD_LOGS), IGCFilename);     // add path
    TCHAR Tmp[MAX_NMEA_LEN];
    _sntprintf(Tmp, MAX_NMEA_LEN, _T("%s"),text1);     // missing
    if (Appearance.UTF8Pictorials)                             // use UTF8 symbols?
    {
      if (lk::filesystem::exist(PathAndFilename))                // check if file exists
        _sntprintf(text1, MAX_NMEA_LEN, _T("✔ %s"), Tmp); // already copied
    } else {
      if (lk::filesystem::exist(PathAndFilename))                // check if file exists
       _sntprintf(text1, MAX_NMEA_LEN, _T("* %s"), Tmp);// already copied
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


static void OnIGCListEnter(WindowControl *Sender, WndListFrame::ListInfo_t *ListInfo) {

  if (LX_IGCReadDialog.FileList()->size() == 0) return;
  LX_IGCReadDialog.CurIndex(ListInfo->ItemIndex + ListInfo->ScrollIndex);

  if (LX_IGCReadDialog.CurIndex() >= LX_IGCReadDialog.FileList()->size()) {
    LX_IGCReadDialog.CurIndex(LX_IGCReadDialog.FileList()->size() - 1);
  }


  if (Sender) {
    WndForm *pForm = Sender->GetParentWndForm();
    if (pForm) {
      LX_IGCReadDialog.DownloadIndex(LX_IGCReadDialog.CurIndex());
      OnEnterClicked(NULL);
    }
  }
}


static void OnCloseClicked(WndButton *pWnd) {
  if (pWnd) {
    WndForm *pForm = pWnd->GetParentWndForm();
    if (pForm) {

      pForm->SetModalResult(mrCancel);
    }
  }
}


void OnAbort_IGC_FileRead(void) {
  if (DevLXNanoIII::AbortLX_IGC_FileRead())  // was download in progress?
  {
    TCHAR PathAndFilename[MAX_PATH];
    LocalPath(PathAndFilename, _T(LKD_LOGS), LX_IGCReadDialog.FileList()->at(
            LX_IGCReadDialog.DownloadIndex()).Line1);   // add path
    lk::filesystem::deleteFile(PathAndFilename);              // delete incomplete file exists
  }
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


class LXResourceLock {
public:
  LXResourceLock() {
    //   StartupStore(TEXT(".... Enter ResourceLock%s"),NEWLINE);
    MapWindow::SuspendDrawingThread();

  };

  ~LXResourceLock() {
    //   StartupStore(TEXT(".... Leave ResourceLock%s"),NEWLINE);

    MapWindow::ResumeDrawingThread();

    LX_IGCReadDialog.FileList()->clear();

  }
};


ListElement *dlgLX_IGCSelectListShowModal(void) {

  LXResourceLock ResourceGuard;  //simply need to exist for recource Lock/Unlock

  ListElement *pIGCResult = NULL;


  WndForm* wf = dlgLoadFromXML(IGCCallBackTable,
                      ScreenLandscape ? IDR_XML_MULTISELECTLIST_L : IDR_XML_MULTISELECTLIST_P);

  if (wf) {
    LX_IGCReadDialog.SelectList((WndListFrame *) wf->FindByName(TEXT("frmMultiSelectListList")));
    LKASSERT(LX_IGCReadDialog.SelectList() != NULL);
    LX_IGCReadDialog.SelectList()->SetBorderKind(BORDERLEFT);
    LX_IGCReadDialog.SelectList()->SetEnterCallback(OnIGCListEnter);

    LX_IGCReadDialog.ListEntry(
            (WndOwnerDrawFrame *) wf->FindByName(TEXT("frmMultiSelectListListEntry")));
    if (LX_IGCReadDialog.ListEntry()) {
      /*
       * control height must contains 2 text Line
       * Check and update Height if necessary
       */
      LKWindowSurface windowSurface(*main_window);
      LKBitmapSurface tmpSurface(windowSurface, 1, 1);
      const auto oldFont = tmpSurface.SelectObject(LX_IGCReadDialog.ListEntry()->GetFont());
      const int minHeight = 2 * tmpSurface.GetTextHeight(_T("dp")) + 2 * DLGSCALE(2);
      tmpSurface.SelectObject(oldFont);
      const int wHeight = LX_IGCReadDialog.ListEntry()->GetHeight();
      if (minHeight > wHeight) {
        LX_IGCReadDialog.ListEntry()->SetHeight(minHeight);
      }
      LX_IGCReadDialog.ListEntry()->SetCanFocus(true);
    } else
      LKASSERT(0);


    LX_IGCReadDialog.FileList()->clear();
    LX_IGCReadDialog.SelectList()->ResetList();
    wf->SetTimerNotify(200, TimerUpdateList); // check for end of download every 200ms
    wf->ShowModal();
    LX_IGCReadDialog.FileList()->clear();
    wf->SetTimerNotify(0, NULL); // disable Timer
    delete wf;
    wf = NULL;
  }
  return pIGCResult;
}

