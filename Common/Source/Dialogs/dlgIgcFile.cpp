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
#include "Comm/Obex/CObexPush.h"
#include "Message.h"
#include "utils/stl_utils.h"

namespace DlgIgcFile {
    WndForm *wfDlg = NULL;
    typedef std::vector<std::tstring> FileList_t;
    FileList_t FileList;
    size_t DrawListIndex = (~0);
    size_t ItemIndex = (~0);

    void ScanFile() {
        FileList.clear();
        TCHAR szPath[MAX_PATH] = _T("\0");
		LocalPath(szPath, _T(LKD_LOGS));
        size_t nLen = _tcslen(szPath);
        if (szPath[nLen - 1] != _T('\\')) {
            _tcscat(szPath, _T("\\"));
        }
        _tcscat(szPath, _T("*.igc"));

        for(lk::filesystem::directory_iterator It(szPath); It; ++It) {
            if(!It.isDirectory()) {
                FileList.push_back(It.getName());
            }
        }
        
        std::sort(FileList.rbegin(), FileList.rend()); // sort in desc order.
    }

    void OnClose(WindowControl * Sender) {
        (void) Sender;
        wfDlg->SetModalResult(mrOK);
    }

    void OnSend(WindowControl * Sender) {
        (void) Sender;
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
                    MessageBoxX(_T("No Device"), _T("Error"), MB_OK);
                    StartHourglassCursor();
                } else {
                    WndProperty* wp = (WndProperty*)wfDlg->FindByName(TEXT("prpDeviceList"));
                    DataFieldEnum* dfe = NULL;
                    if (wp) {
                        dfe = (DataFieldEnum*)wp->GetDataField();
                    }
                    if(dfe) {
                        dfe->Clear();
                        dfe->addEnumText(_T("none"));
                    }
                    for(size_t i = 0; i < nDevice; ++i) {
                        TCHAR szDeviceName[100] = {0};
                        if(!Obex.GetDeviceName(i, szDeviceName, std::distance(begin(szDeviceName), end(szDeviceName)))) {
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
                            MessageBoxX(_T("Send Failed"), _T("Error"), MB_OK);
                            StartHourglassCursor();
                        } else {
                            StopHourglassCursor();
                            MessageBoxX(_T("File sent!"), _T("Success"), MB_OK);
                            StartHourglassCursor();
                        }
                    }
                }
                Obex.Shutdown();
            } else {
                MessageBoxX(_T("Unsupported on this device"), _T("Error"), MB_OK);
            }
#else
            MessageBoxX(_T("Unsupported on this device"), _T("Error"), MB_OK);
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
            Surface.DrawTextClip(2 * ScreenScale, 2 * ScreenScale, ItFileName->c_str(), w0 - ScreenScale * 5);


        }
    }

    CallBackTableEntry_t CallBackTable[] = {
        DeclareCallBackEntry(OnClose),
        DeclareCallBackEntry(OnSend),
        DeclareCallBackEntry(OnIgcFileListInfo),
        DeclareCallBackEntry(OnPaintIgcFileListItem),
        DeclareCallBackEntry(NULL)
    };
}

using DlgIgcFile::wfDlg;

void dlgIgcFileShowModal() {
    TCHAR filename[MAX_PATH];
    const TCHAR *resName = NULL;
    if (!ScreenLandscape) {
        LocalPathS(filename, TEXT("dlgIgcFile.xml"));
        resName = TEXT("IDR_XML_IGCFILE");
    } else {
        LocalPathS(filename, TEXT("dlgIgcFile_L.xml"));
        resName = TEXT("IDR_XML_IGCFILE_L");
    }
    wfDlg = dlgLoadFromXML(DlgIgcFile::CallBackTable, filename, hWndMainWindow, resName);
    if (wfDlg) {

        WndListFrame* wndFileList = (WndListFrame*) wfDlg->FindByName(TEXT("frmIgcFileList"));
        if (wndFileList) {
            wndFileList->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT | BORDERBOTTOM);
            wndFileList->SetWidth(wfDlg->GetWidth() - wndFileList->GetLeft() - IBLSCALE(4));

            // Bug : we need ClientHeight, but Cleint Rect is Calculated by OnPaint
            // wndFileList->SetHeight(wfDlg->GetHeight() - wndFileList->GetTop() - 2);
            if (wndFileList->ScrollbarWidth == -1) {
                wndFileList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale);
            }

            WndOwnerDrawFrame* FileListEntry = (WndOwnerDrawFrame*) wfDlg->FindByName(TEXT("frmIgcFileListEntry"));
            if (FileListEntry) {
                FileListEntry->SetCanFocus(true);
                FileListEntry->SetWidth(wndFileList->GetWidth() - wndFileList->ScrollbarWidth - 5);
            }

            DlgIgcFile::ScanFile();

            wndFileList->ResetList();
            wndFileList->Redraw();
        }
    }

    if (wfDlg->ShowModal(true)) {

    }

    delete wfDlg;
    wfDlg = NULL;
}
