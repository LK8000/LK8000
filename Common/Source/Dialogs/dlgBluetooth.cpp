/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "WindowControls.h"
#include "Message.h"
#include "resource.h"

#ifndef NO_BLUETOOTH

namespace DlgBluetooth {

    WndForm *wfBth = NULL;
    size_t DrawListIndex = 0;
    size_t ItemIndex = 0;

    void OnClose(WndButton* pWnd) {
      if(pWnd) {
        WndForm * pForm = pWnd->GetParentWndForm();
        if(pForm) {
          pForm->SetModalResult(mrOK);
        }
      }
    }

    void OnPair(WndButton* pWnd) {
        CBtHandler* pBtHandler = CBtHandler::Get();
        if (pBtHandler) {
            CBtDevice * SelectedDevice = pBtHandler->GetDevice(ItemIndex);
            if (SelectedDevice && SelectedDevice->m_src == BDSRC_LOOKUP) {
                TCHAR szPin[20] = {0};
                dlgTextEntryShowModal(szPin, 20);

                if (!pBtHandler->Pair(SelectedDevice->m_ba, SelectedDevice->GetName().c_str(), szPin)) {
                    StartupStore(_T("Bluetooth pairing <%s> : Failed%s"), SelectedDevice->GetName().c_str(), NEWLINE);
                    MessageBoxX(MsgToken<1835>(), TEXT("Bluetooth"), mbOk, false);
                } else {
                    StartupStore(_T("Bluetooth pairing <%s> : success%s"), SelectedDevice->GetName().c_str(), NEWLINE);
                    SelectedDevice->m_src |= BDSRC_REGNAV;
                }
            }
            WndListFrame* BthList = wfBth->FindByName<WndListFrame>(TEXT("frmBthList"));
            if (BthList) {
                BthList->ResetList();
                BthList->Redraw();
            }
        }
    }

    void OnUnpair(WndButton* pWnd) {
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
            WndListFrame* BthList = wfBth->FindByName<WndListFrame>(TEXT("frmBthList"));
            if (BthList) {
                BthList->ResetList();
                BthList->Redraw();
            }
        }
    }

    void OnLookup(WndButton* pWnd) {
        StartHourglassCursor();
        CBtHandler * pBtHandler = CBtHandler::Get();
        if (pBtHandler && pBtHandler->StartHW() && pBtHandler->LookupDevices()) {
            WndListFrame* BthList = wfBth->FindByName<WndListFrame>(TEXT("frmBthList"));
            if (BthList) {
                BthList->ResetList();
                BthList->Redraw();
            }
        }
        StopHourglassCursor();
    }

    void OnPaintListItem(WndOwnerDrawFrame * Sender, LKSurface& Surface) {
        CBtHandler* pBtHandler = CBtHandler::Get();
        if (pBtHandler) {
            CBtDevice * bt = pBtHandler->GetDevice(DrawListIndex);
            if (bt) {
                int w1 = Surface.GetTextWidth(TEXT("PAIRED"));
                int w0 = Sender->GetWidth();
                Surface.DrawTextClip(DLGSCALE(2), DLGSCALE(2), bt->GetName().c_str(), w0 - w1 - DLGSCALE(5));
                if ((bt->m_src & (BDSRC_REGSVC | BDSRC_REGNAV | BDSRC_REGPIN))) {
                    Surface.DrawTextClip(DLGSCALE(2) + w0 - w1, DLGSCALE(2), _T("Paired"), w1);
                }
            }
        }
    }

    void OnListInfo(WndListFrame * Sender, WndListFrame::ListInfo_t * ListInfo) {

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
        CallbackEntry(OnClose),
        CallbackEntry(OnPair),
        CallbackEntry(OnUnpair),
        CallbackEntry(OnLookup),
        CallbackEntry(OnPaintListItem),
        CallbackEntry(OnListInfo),
        EndCallbackEntry()
    };

    void Show() {

        wfBth = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_BLUETOOTH_L : IDR_XML_BLUETOOTH_P);
        if (wfBth) {

            WndListFrame* BthList = wfBth->FindByName<WndListFrame>(TEXT("frmBthList"));
            if (BthList) {
                BthList->SetBorderKind(BORDERLEFT | BORDERTOP | BORDERRIGHT | BORDERBOTTOM);

                WndOwnerDrawFrame* BthListEntry = wfBth->FindByName<WndOwnerDrawFrame>(TEXT("frmBthListEntry"));
                if (BthListEntry) {
                    BthListEntry->SetCanFocus(true);
                }

                BthList->ResetList();
                BthList->Redraw();
            }

            if (wfBth->ShowModal()) {
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
