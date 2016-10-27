/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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

namespace DlgIgcFile {
    WndForm *wfDlg = NULL;
    typedef std::vector<tstring> FileList_t;
    FileList_t FileList;
    size_t DrawListIndex = (~0);
    size_t ItemIndex = (~0);

    void ScanFile() {
        FileList.clear();
        TCHAR szPath[MAX_PATH] = _T("\0");
        TCHAR tmpPath[MAX_PATH];
	LocalPath(szPath, _T(LKD_LOGS));
        size_t nLen = _tcslen(szPath);
        if (szPath[nLen - 1] != _T('\\')) {
            _tcscat(szPath, _T(DIRSEP));
        }
	_tcscpy(tmpPath,szPath);
        _tcscat(tmpPath, _T("*.igc"));

        for(lk::filesystem::directory_iterator It(tmpPath); It; ++It) {
            if(!It.isDirectory()) {
                FileList.push_back(It.getName());
            }
        }

	_tcscpy(tmpPath,szPath);
        _tcscat(tmpPath, _T("*.IGC"));

        for(lk::filesystem::directory_iterator It(tmpPath); It; ++It) {
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
                    MessageBoxX(_T("No Device"), _T("Error"), mbOk);
                    StartHourglassCursor();
                } else {
                    WndProperty* wp = (WndProperty*)wfDlg->FindByName(TEXT("prpDeviceList"));
                    DataField* dfe = NULL;
                    if (wp) {
                        dfe = wp->GetDataField();
                    }
                    if(dfe) {
                        dfe->Clear();
                        dfe->addEnumText(_T("none"));
                    }
                    for(size_t i = 0; i < nDevice; ++i) {
                        TCHAR szDeviceName[100] = {0};
                        if(!Obex.GetDeviceName(i, szDeviceName, array_size(szDeviceName))) {
                            _stprintf(szDeviceName, _T("Unknown device <%d>"), i);
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

                        TCHAR szFileFullPath[MAX_PATH] = _T("\0");
                        LocalPath(szFileFullPath, _T(LKD_LOGS));
                        size_t nLen = _tcslen(szFileFullPath);
                        if (szFileFullPath[nLen - 1] != _T('\\')) {
                            _tcscat(szFileFullPath, _T("\\"));
                        }
                        FileList_t::const_iterator ItFileName = FileList.begin();
                        std::advance(ItFileName, ItemIndex);
                        _tcscat(szFileFullPath, ItFileName->c_str());

                        if(!Obex.SendFile(DeviceIndex, szFileFullPath)) {
                            StopHourglassCursor();
                            MessageBoxX(_T("Send Failed"), _T("Error"), mbOk);
                            StartHourglassCursor();
                        } else {
                            StopHourglassCursor();
                            MessageBoxX(_T("File sent!"), _T("Success"), mbOk);
                            StartHourglassCursor();
                        }
                    }
                }
                Obex.Shutdown();
            } else {
                MessageBoxX(_T("Unsupported on this device"), _T("Error"), mbOk);
            }
#else
            MessageBoxX(_T("Unsupported on this device"), _T("Error"), mbOk);
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
            Surface.DrawTextClip(2 * ScreenScale, 2 * ScreenScale, ItFileName->c_str(), w0 - ScreenScale * 5);
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
    }

    if (wfDlg->ShowModal()) {

    }

    delete wfDlg;
    wfDlg = NULL;
}
