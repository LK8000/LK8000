/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgMacCready.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 7 July 2022, 22:58
 */

#include "dlgMacCready.h"
#include "externs.h"
#include "dlgTools.h"
#include "resource.h"
#include "Event/Key.h"


namespace {

void Refresh(WndForm* pForm) {
  if (!pForm) {
    return;
  }

  auto pWnd = pForm->FindByName(_T("frmValue"));
  if(pWnd) {
    TCHAR sTmp[32];
    _stprintf(sTmp, _T("%3.1f%s"),
              Units::ToVerticalSpeed(MACCREADY),
              Units::GetVerticalSpeedName());

    pWnd->SetCaption(sTmp);
  }
}

void OnUp(WndForm* pForm) {
  CALCULATED_INFO.AutoMacCready = false; // disable AutoMacCready when changing MC values
  CheckSetMACCREADY(Units::ToVerticalSpeed(MACCREADY + 0.1), nullptr);

  Refresh(pForm);
}

void OnUpClick(WndButton* pWnd) {
  if (pWnd) {
    OnUp(pWnd->GetParentWndForm());
  }
}

void OnDown(WndForm* pForm) {
  CALCULATED_INFO.AutoMacCready = false; // disable AutoMacCready when changing MC values
  CheckSetMACCREADY(Units::ToVerticalSpeed(MACCREADY - 0.1), nullptr);

  Refresh(pForm);
}

void OnDownClick(WndButton* pWnd) {
  if (pWnd) {
    OnDown(pWnd->GetParentWndForm());
  }
}

void OnClose(WndButton* pWnd) {
  if (pWnd) {
    WndForm* pForm = pWnd->GetParentWndForm();
    if (pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

bool OnKeyDown(WndForm* pForm, unsigned KeyCode) {
  if (pForm) {
    switch (KeyCode) {
      case KEY_UP:
        OnUp(pForm);
        return true;
      case KEY_DOWN:
        OnDown(pForm);
        return true;
      case KEY_RETURN:
      case KEY_ESCAPE:
        pForm->SetModalResult(mrOK);
        return true;
      default:
        break;
    }
  }
  return false;
}

} // namespace


void dlgMacCready::DoModal() {

  CallBackTableEntry_t CallBackTable[] = {
    ClickNotifyCallbackEntry(OnUpClick),
    ClickNotifyCallbackEntry(OnDownClick),
    ClickNotifyCallbackEntry(OnClose),
    EndCallBackEntry()
  };

  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(CallBackTable, IDR_XML_MACCREADY));
  if (pForm) {

    // center dialog
    PixelRect main_rect(main_window->GetClientRect());
    RasterPoint main_center = main_rect.GetCenter();
    pForm->SetTop(main_center.y - pForm->GetHeight() / 2);
    pForm->SetLeft(main_center.x - pForm->GetWidth() / 2);

    auto pWnd = dynamic_cast<WndFrame*>(pForm->FindByName(_T("frmValue")));
    if(pWnd) {
      pWnd->SetFont(LK8InfoBigFont);
      pWnd->SetCaptionStyle(DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }

    Refresh(pForm.get());

    pForm->SetKeyDownNotify(OnKeyDown);
    pForm->ShowModal();
  }
}
