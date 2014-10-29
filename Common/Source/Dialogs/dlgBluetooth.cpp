/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   dlgBluetooth.cpp
 * Author: Bruno de Lacheisserie
 * 
 */

#include "externs.h"
#include "dlgTools.h"
#include "BtHandler.h"
#include "Dialogs.h"
#include "Message.h"

#ifndef NO_BLUETOOTH

namespace DlgBluetooth {

    WndForm *wfBth = NULL;
    size_t DrawListIndex = 0;
    size_t ItemIndex = 0;

    void OnClose(WindowControl * Sender) {
        (void) Sender;
        wfBth->SetModalResult(mrOK);
    }

    void OnPair(WindowControl * Sender) {
        (void) Sender;
        CBtHandler* pBtHandler = CBtHandler::Get();
        if (pBtHandler) {
            CBtDevice * SelectedDevice = pBtHandler->GetDevice(ItemIndex);
            if (SelectedDevice && SelectedDevice->m_src == BDSRC_LOOKUP) {
                TCHAR szPin[20] = {0};
                dlgTextEntryShowModal(szPin, 20, false);

                if (!pBtHandler->Pair(SelectedDevice->m_ba, SelectedDevice->GetName().c_str(), szPin)) {
                    StartupStore(_T("Bluetooth pairing <%s> : Failed%s"), SelectedDevice->GetName().c_str(), NEWLINE);
                    MessageBoxX(wfBth->GetHandle(), LKGetText(TEXT("_@M1835_")), TEXT("Bluetooth"), MB_OK, false);
                } else {
                    StartupStore(_T("Bluetooth pairing <%s> : success%s"), SelectedDevice->GetName().c_str(), NEWLINE);
                    SelectedDevice->m_src |= BDSRC_REGNAV;
                }
            }
            WndListFrame* BthList = (WndListFrame*) wfBth->FindByName(TEXT("frmBthList"));
            if (BthList) {
                BthList->ResetList();
                BthList->Redraw();
            }
        }
    }

    void OnUnpair(WindowControl * Sender) {
        (void) Sender;
        CBtHandler* pBtHandler = CBtHandler::Get();

        if (pBtHandler) {
            CBtDevice * SelectedDevice = pBtHandler->GetDevice(ItemIndex);
            if (SelectedDevice && SelectedDevice->m_src != BDSRC_LOOKUP) {

                if (!pBtHandler->Unpair(SelectedDevice->m_ba)) {
                    StartupStore(_T("%s[%s] : UnPairing Error%s"), SelectedDevice->GetName().c_str(), SelectedDevice->BTPortName().c_str(), NEWLINE);
                } else {
                    pBtHandler->RemoveDevice(SelectedDevice->m_ba);
                }
            }
            WndListFrame* BthList = (WndListFrame*) wfBth->FindByName(TEXT("frmBthList"));
            if (BthList) {
                BthList->ResetList();
                BthList->Redraw();
            }
        }
    }

    void OnLookup(WindowControl * Sender) {
        (void) Sender;
        StartHourglassCursor();
        CBtHandler * pBtHandler = CBtHandler::Get();
        if (pBtHandler && pBtHandler->StartHW() && pBtHandler->LookupDevices()) {
            WndListFrame* BthList = (WndListFrame*) wfBth->FindByName(TEXT("frmBthList"));
            if (BthList) {
                BthList->ResetList();
                BthList->Redraw();
            }
        }
        StopHourglassCursor();
    }

    void OnPaintListItem(WindowControl * Sender, LKSurface& Surface) {
        CBtHandler* pBtHandler = CBtHandler::Get();
        if (pBtHandler) {
            CBtDevice * bt = pBtHandler->GetDevice(DrawListIndex);
            if (bt) {
                int w1 = Surface.GetTextWidth(TEXT("PAIRED"));
                int w0 = Sender->GetWidth();
                Surface.DrawTextClip(2 * ScreenScale, 2 * ScreenScale, bt->GetName().c_str(), w0 - w1 - ScreenScale * 5);
                if ((bt->m_src & (BDSRC_REGSVC | BDSRC_REGNAV | BDSRC_REGPIN))) {
                    Surface.DrawTextClip(2 * ScreenScale + w0 - w1, 2 * ScreenScale, _T("Paired"), w1);
                }
            }
        }
    }

    void OnListInfo(WindowControl * Sender, WndListFrame::ListInfo_t * ListInfo) {
        (void) Sender;
        CBtHandler* pBtHandler = CBtHandler::Get();
        if (pBtHandler) {
            ListInfo->ItemCount = pBtHandler->m_devices.size();
            if (ListInfo->DrawIndex != -1) {
                DrawListIndex = ListInfo->DrawIndex + ListInfo->ScrollIndex;
                ItemIndex = ListInfo->ItemIndex + ListInfo->ScrollIndex;
            }
        }
    }


    CallBackTableEntry_t CallBackTable[] = {
        DeclareCallBackEntry(OnClose),
        DeclareCallBackEntry(OnPair),
        DeclareCallBackEntry(OnUnpair),
        DeclareCallBackEntry(OnLookup),
        DeclareCallBackEntry(OnPaintListItem),
        DeclareCallBackEntry(OnListInfo),
        DeclareCallBackEntry(NULL)
    };

    void Show() {
        TCHAR filename[MAX_PATH];
        const TCHAR *resName = NULL;
        if (!ScreenLandscape) {
            LocalPathS(filename, TEXT("dlgBluetooth_L.xml"));
            resName = TEXT("IDR_XML_BLUETOOTH_L");
        } else {
            LocalPathS(filename, TEXT("dlgBluetooth.xml"));
            resName = TEXT("IDR_XML_BLUETOOTH");
        }
        wfBth = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, resName);
        if (wfBth) {

            WndListFrame* BthList = (WndListFrame*) wfBth->FindByName(TEXT("frmBthList"));
            if (BthList) {
                BthList->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT | BORDERBOTTOM);
                BthList->SetWidth(wfBth->GetWidth() - BthList->GetLeft() - IBLSCALE(4));

                // Bug : we need ClientHeight, but Cleint Rect is Calculated by OnPaint
                // BthList->SetHeight(wfBth->GetHeight() - BthList->GetTop() - 2);
                if (BthList->ScrollbarWidth == -1) {
                    BthList->ScrollbarWidth = (int) (SCROLLBARWIDTH_INITIAL * ScreenDScale);
                }

                WndOwnerDrawFrame* BthListEntry = (WndOwnerDrawFrame*) wfBth->FindByName(TEXT("frmBthListEntry"));
                if (BthListEntry) {
                    BthListEntry->SetCanFocus(true);
                    BthListEntry->SetWidth(BthList->GetWidth() - BthList->ScrollbarWidth - 5);
                }

                BthList->ResetList();
                BthList->Redraw();
            }

            if (wfBth->ShowModal(true)) {
                CBtHandler * pBtHandler = CBtHandler::Get();
                if (pBtHandler) {
                    pBtHandler->ClearDevices();
                    pBtHandler->FillDevices();
                }
                RefreshComPortList();
            }

            delete wfBth;
            wfBth = NULL;
        }
    }

};

#endif
