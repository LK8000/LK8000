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
#include "Comm/Obex/CObexPush.h"
#include "Message.h"
#include <iterator>
#include "utils/stl_utils.h"
#include "resource.h"

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

            //Start Bluetooth if needed...
#ifdef UNDER_CE
            CObexPush Obex;
            if(Obex.Startup()) {
                StartupStore(_T("Startup OK \n"));
                size_t nDevice = Obex.LookupDevice();
                StartupStore(_T("LookupDevice OK \n"));
                if(nDevice == 0) {
                    StopHourglassCursor();
                    // No Device, Error
                    MessageBoxX(MsgToken(1537), MsgToken(266), mbOk);
                    StartHourglassCursor();
                } else {
                    WndProperty* wp = (WndProperty*)wfDlg->FindByName(TEXT("prpDeviceList"));
                    DataField* dfe = NULL;
                    if (wp) {
                        dfe = wp->GetDataField();
                    }
                    if(dfe) {
                        dfe->Clear();
                        dfe->addEnumText(MsgToken(479)); // None
                    }
                    for(size_t i = 0; i < nDevice; ++i) {
                        TCHAR szDeviceName[100] = {0};
                        if(!Obex.GetDeviceName(i, szDeviceName, std::size(szDeviceName))) {
                            _stprintf(szDeviceName, _T("%s <%d>"), MsgToken(1538), i); // Unknown device
                        }
                        StartupStore(_T("GetDeviceName <%d><%s> \n"), i, szDeviceName);
                        if(dfe) {
                            dfe->addEnumText(szDeviceName);
                        }
                    }
                    if(wp) {
                        if(dfe) {
                            dfe->SetAsInteger(0);
                        }
                        wp->SetReadOnly(false);
                        wp->RefreshDisplay();
                    }
                    StopHourglassCursor();
                    size_t DeviceIndex = 0;
                    if(dfe && wp) {
                        dlgComboPicker(wp);
                        DeviceIndex = dfe->GetAsInteger();
                    }
                    StartHourglassCursor();
                    if(DeviceIndex != 0) {
                        DeviceIndex--;

                        FileList_t::const_iterator ItFileName = FileList.begin();
                        std::advance(ItFileName, ItemIndex);

                        TCHAR szFileFullPath[MAX_PATH];
                        LocalPath(szFileFullPath, _T(LKD_LOGS), ItFileName->c_str());

                        if(!Obex.SendFile(DeviceIndex, szFileFullPath)) {
                            StopHourglassCursor();
                            // Send Failed, Error
                            MessageBoxX(MsgToken(1539), MsgToken(266), mbOk);
                            StartHourglassCursor();
                        } else {
                            StopHourglassCursor();
                            // File sent! , Error
                            MessageBoxX(MsgToken(1540), MsgToken(1524), mbOk);
                            StartHourglassCursor();
                        }
                    }
                }
                Obex.Shutdown();
            } else {
                // "Unsupported on this device", "Error"
                MessageBoxX(MsgToken(1534), MsgToken(266), mbOk);
            }
#elif defined(ANDROID)

            LK8000Activity *activity = LK8000Activity::Get();
            if (activity) {

                auto It = std::next(FileList.begin(), ItemIndex);
                TCHAR szFileFullPath[MAX_PATH];
                LocalPath(szFileFullPath, _T(LKD_LOGS), It->c_str());

                activity->ShareFile(szFileFullPath);
            }
#else
            // "Unsupported on this device", "Error"
            MessageBoxX(MsgToken(1534), MsgToken(266), mbOk);
#endif
            StopHourglassCursor();
        }
    }

    void OnIgcFileListInfo(WindowControl * Sender, WndListFrame::ListInfo_t * ListInfo) {
        (void) Sender;
        ListInfo->ItemCount = FileList.size();
        if (ListInfo->DrawIndex != -1) {
            DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
            ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
        }
    }

    void OnPaintIgcFileListItem(WindowControl * Sender, LKSurface& Surface) {
        if (DrawListIndex < FileList.size()) {
            FileList_t::const_iterator ItFileName = FileList.begin();
            std::advance(ItFileName, DrawListIndex);
            int w0 = Sender->GetWidth();

            Surface.SetTextColor(RGB_BLACK);
            Surface.DrawTextClip(DLGSCALE(2), DLGSCALE(2), ItFileName->c_str(), w0 - DLGSCALE(2));
        }
    }

    CallBackTableEntry_t CallBackTable[] = {
        ClickNotifyCallbackEntry(OnClose),
        ClickNotifyCallbackEntry(OnSend),
        OnListCallbackEntry(OnIgcFileListInfo),
        OnPaintCallbackEntry(OnPaintIgcFileListItem),
        EndCallBackEntry()
    };
}

using DlgIgcFile::wfDlg;

void dlgIgcFileShowModal() {

    wfDlg = dlgLoadFromXML(DlgIgcFile::CallBackTable, ScreenLandscape ? IDR_XML_IGCFILE_L : IDR_XML_IGCFILE_P);
    if (wfDlg) {

        WndListFrame* wndFileList = (WndListFrame*) wfDlg->FindByName(TEXT("frmIgcFileList"));
        if (wndFileList) {
            wndFileList->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT | BORDERBOTTOM);

            WndOwnerDrawFrame* FileListEntry = (WndOwnerDrawFrame*) wfDlg->FindByName(TEXT("frmIgcFileListEntry"));
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
