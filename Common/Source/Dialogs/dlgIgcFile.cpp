/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgIgcFile.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 1 novembre 2013, 20:04
 */
#include "externs.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "Message.h"
#include <iterator>
#include "utils/stl_utils.h"
#include "resource.h"
#include "LocalPath.h"

#ifdef ANDROID
#include "Android/LK8000Activity.h"
#endif

namespace DlgIgcFile {
    WndForm *wfDlg = nullptr;
    typedef std::vector<tstring> FileList_t;
    FileList_t FileList;
    size_t DrawListIndex = (~0);
    size_t ItemIndex = (~0);

    void ScanFile() {
        FileList.clear();
        TCHAR szPath[MAX_PATH];
        LocalPath(szPath, _T(LKD_LOGS), _T("*.igc"));

        for(lk::filesystem::directory_iterator It(szPath); It; ++It) {
            if(!It.isDirectory()) {
                FileList.push_back(It.getName());
            }
        }

        std::sort(FileList.rbegin(), FileList.rend()); // sort in desc order.
    }

    void OnClose(WndButton* pWnd) {
      if(pWnd) {
        WndForm * pForm = pWnd->GetParentWndForm();
        if(pForm) {
          pForm->SetModalResult(mrOK);
        }
      }
    }

    void OnSend(WndButton* pWnd) {
        if(ItemIndex < FileList.size()) {
            StartHourglassCursor();
            // "Unsupported on this device", "Error"
            MessageBoxX(MsgToken<1534>(), MsgToken<266>(), mbOk);
            StopHourglassCursor();
        }
    }

    void OnIgcFileListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t * ListInfo) {

        ListInfo->ItemCount = FileList.size();
        if (ListInfo->DrawIndex != -1) {
            DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
            ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
        }
    }

    void OnPaintIgcFileListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface) {
        if (DrawListIndex < FileList.size()) {
            FileList_t::const_iterator ItFileName = FileList.begin();
            std::advance(ItFileName, DrawListIndex);
            int w0 = Sender->GetWidth();

            Surface.SetTextColor(RGB_BLACK);
            Surface.DrawTextClip(DLGSCALE(2), DLGSCALE(2), ItFileName->c_str(), w0 - DLGSCALE(2));
        }
    }

    CallBackTableEntry_t CallBackTable[] = {
        CallbackEntry(OnClose),
        CallbackEntry(OnSend),
        CallbackEntry(OnIgcFileListInfo),
        CallbackEntry(OnPaintIgcFileListItem),
        EndCallbackEntry()
    };
}

using DlgIgcFile::wfDlg;

void dlgIgcFileShowModal() {

    wfDlg = dlgLoadFromXML(DlgIgcFile::CallBackTable, ScreenLandscape ? IDR_XML_IGCFILE_L : IDR_XML_IGCFILE_P);
    if (wfDlg) {

        WndListFrame* wndFileList = wfDlg->FindByName<WndListFrame>(TEXT("frmIgcFileList"));
        if (wndFileList) {
            wndFileList->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT | BORDERBOTTOM);

            WndOwnerDrawFrame* FileListEntry = wfDlg->FindByName<WndOwnerDrawFrame>(TEXT("frmIgcFileListEntry"));
            if (FileListEntry) {
                FileListEntry->SetCanFocus(true);
            }

            DlgIgcFile::ScanFile();

            wndFileList->ResetList();
            wndFileList->Redraw();
        }

        if (wfDlg->ShowModal()) {

        }

        delete wfDlg;
        wfDlg = nullptr;
    }
}
