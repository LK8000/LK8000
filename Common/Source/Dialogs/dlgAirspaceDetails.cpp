/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceDetails.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "InputEvents.h"
#include "Airspace/Airspace.h"
#include "Dialogs.h"
#include "LKObjects.h"
#include "Sound/Sound.h"
#include "resource.h"
#include "Radio.h"
#include "utils/printf.h"
#include "dlgAirspaceDetails.h"

dlgAirspaceDetails::dlgAirspaceDetails(const CAirspacePtr& pAirspace) {
  _pAirspace = pAirspace;
  if (_pAirspace) {
    _AirspaceCopy = CAirspaceManager::Instance().GetAirspaceCopy(_pAirspace);
  }
}

dlgAirspaceDetails::~dlgAirspaceDetails() {}

/*
 * only called by #InputEvents::ProcessQueue()
 * to display AirspaceDetails, use #InputEvents::processPopupDetails(im_airspace{pAsp});
 */
void dlgAirspaceDetails::DoModal() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  CallBackTableEntry_t CallBackTable[] = {
      callback_entry("OnAcknowledgeClicked", std::bind(&dlgAirspaceDetails::OnAcknowledgeClicked, this, _1)),
      callback_entry("OnDetailsClicked", std::bind(&dlgAirspaceDetails::OnDetailsClicked, this, _1)),
      callback_entry("OnFlyClicked", std::bind(&dlgAirspaceDetails::OnFlyClicked, this, _1)),
      callback_entry("OnCloseClicked", std::bind(&dlgAirspaceDetails::OnCloseClicked, this, _1)),
      callback_entry("OnSelectClicked", std::bind(&dlgAirspaceDetails::OnSelectClicked, this, _1)),
      callback_entry("OnSetFrequency", std::bind(&dlgAirspaceDetails::OnSetFrequency, this, _1)),
      callback_entry("OnSetSecFrequency", std::bind(&dlgAirspaceDetails::OnSetSecFrequency, this, _1)),
      callback_entry("OnPaintAirspacePicto", std::bind(&dlgAirspaceDetails::OnPaintAirspacePicto, this, _1, _2)),
      EndCallBackEntry()
  };

  _pForm = std::unique_ptr<WndForm>(dlgLoadFromXML(CallBackTable, IDR_XML_AIRSPACEDETAILS));
  if (!_pForm) {
    return;
  }

  _pForm->SetTimerNotify(1000, std::bind(&dlgAirspaceDetails::OnTimer, this, _1));

  SetValues();

  _pForm->ShowModal();
}

void dlgAirspaceDetails::OnPaintAirspacePicto(WndOwnerDrawFrame* Sender, LKSurface& Surface) const {
  const RECT rc = Sender->GetClientRect();

  Surface.SetBkColor(RGB_LIGHTGREY);
  /****************************************************************
   * for drawing the airspace pictorial, we need the original data.
   * copy contain only base class property, not geo data,
   * original data are shared ressources !
   * for that we need to grant all called methods are thread safe
   ****************************************************************/
  assert(_pAirspace);
  _pAirspace->DrawPicto(Surface, rc);
}

void dlgAirspaceDetails::OnFlyClicked(WndButton* pWnd) {
  assert(_pAirspace);
  CAirspaceManager::Instance().AirspaceFlyzoneToggle(*_pAirspace);
  SetValues();
  PlayResource(TEXT("IDR_WAV_CLICK"));
}

void dlgAirspaceDetails::OnSelectClicked(WndButton* pWnd) {
  assert(_pAirspace);
  CAirspaceManager::Instance().AirspaceSetSelect(_pAirspace);
  SetValues();
  PlayResource(TEXT("IDR_WAV_CLICK"));
}

void dlgAirspaceDetails::OnAcknowledgeClicked(WndButton* pWnd) {
  assert(_pAirspace);
  CAirspaceManager::Instance().AirspaceSetAckLevel(*_pAirspace, awNone);
  if (_AirspaceCopy.Enabled()) {
    CAirspaceManager::Instance().AirspaceDisable(*_pAirspace);
  }
  else {
    CAirspaceManager::Instance().AirspaceEnable(*_pAirspace);
  }

  SetValues();

  WndFrame* wPicto = (_pForm->FindByName<WndFrame>(TEXT("frmAirspacePicto")));
  if (wPicto) {
    wPicto->Redraw();
  }
  PlayResource(TEXT("IDR_WAV_CLICK"));
}

void dlgAirspaceDetails::OnCloseClicked(WndButton* pWnd) const {
  assert(_pForm);
  _pForm->SetModalResult(mrOK);
}

bool dlgAirspaceDetails::OnTimer(WndForm* pWnd) {
  SetValues();
  return true;
}

void dlgAirspaceDetails::OnSetFrequency(WndButton* pWnd) const{
  if (RadioPara.Enabled) {
    unsigned khz = ExtractFrequency(_AirspaceCopy.Name());
    if (!ValidFrequency(khz)) {
      khz = ExtractFrequency(_AirspaceCopy.Comment());
    }
    devPutFreqActive(khz, _AirspaceCopy.Name());
  }

  assert(_pForm);
  _pForm->SetModalResult(mrOK);
}

void dlgAirspaceDetails::OnSetSecFrequency(WndButton* pWnd) const {
  if (RadioPara.Enabled) {
    unsigned khz = ExtractFrequency(_AirspaceCopy.Name());
    if (!ValidFrequency(khz)) {
      khz = ExtractFrequency(_AirspaceCopy.Comment());
    }
    devPutFreqStandby(khz, _AirspaceCopy.Name());
  }

  assert(_pForm);
  _pForm->SetModalResult(mrOK);
}

void dlgAirspaceDetails::SetValues() {
  assert(_pForm);

  if (_pAirspace) {
    _AirspaceCopy = CAirspaceManager::Instance().GetAirspaceCopy(_pAirspace);
  }

  WndProperty* wp;
  WndButton* wb;
  TCHAR buffer[80];
  TCHAR buffer2[160];  // must contain buffer

  int bearing;
  int hdist;
  int vdist;

  // Get an object instance copy with actual values
  bool inside = CAirspaceManager::Instance().AirspaceCalculateDistance(_pAirspace, &hdist, &bearing, &vdist);

  const TCHAR* status = _AirspaceCopy.Enabled() ? MsgToken<1643>() : MsgToken<1600>();  // ENABLED / DISABLED
  TCHAR capbuffer[250];
  lk::snprintf(capbuffer, _T("%s (%s)"), _AirspaceCopy.Name(), status);
  _pForm->SetCaption(capbuffer);

  wp = _pForm->FindByName<WndProperty>(TEXT("prpType"));
  if (wp) {
    if (_AirspaceCopy.Flyzone()) {
      lk::snprintf(buffer, TEXT("%s %s"), _AirspaceCopy.TypeName(), TEXT("FLY"));
    }
    else {
      lk::snprintf(buffer, TEXT("%s %s"), TEXT("NOFLY"), _AirspaceCopy.TypeName());
    }
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = _pForm->FindByName<WndProperty>(TEXT("prpTop"));
  if (wp) {
    wp->SetText(_AirspaceCopy.Top().text_alternate());
    wp->RefreshDisplay();
  }

  wp = _pForm->FindByName<WndProperty>(TEXT("prpBase"));
  if (wp) {
    wp->SetText(_AirspaceCopy.Base().text_alternate());
    wp->RefreshDisplay();
  }

  wp = _pForm->FindByName<WndProperty>(TEXT("prpRange"));
  if (wp) {
    Units::FormatDistance(abs(hdist), buffer, 20);
    if (inside) {
      // LKTOKEN  _@M359_ = "Inside"
      wp->SetCaption(MsgToken<359>());
    }

    auto Suffix = [&]() {
      // LKTOKEN _@M1257_ "to leave"
      // LKTOKEN _@M1258_ "to enter"
      return (hdist < 0) ? MsgToken<1257>() : MsgToken<1258>();
    };
    lk::snprintf(buffer2, TEXT("%s %d%s %s"), buffer, iround(bearing), MsgToken<2179>(), Suffix());

    wp->SetText(buffer2);
    wp->RefreshDisplay();
  }

  WindowControl* wDetails = _pForm->FindByName(TEXT("cmdDetails"));
  if (wDetails) {
    bool HideDetail = _AirspaceCopy.Comment() && (_tcslen(_AirspaceCopy.Comment()) < 10);

    if (HideDetail) {
      WindowControl* wSelect = _pForm->FindByName(TEXT("cmdSelect"));
      if (wSelect) {
        unsigned left = wDetails->GetLeft();
        unsigned width = wSelect->GetRight() - wDetails->GetLeft();

        wSelect->SetLeft(left);
        wSelect->SetWidth(width);
      }
      wDetails->SetVisible(false);
    }
  }

  WindowControl* wFreq = _pForm->FindByName(TEXT("cmdSFrequency"));
  WindowControl* wSeqFreq = _pForm->FindByName(TEXT("cmdSecFrequency"));
  if (wFreq && wSeqFreq) {
    bool bRadio = false;

    if (RadioPara.Enabled) {
      unsigned khz = ExtractFrequency(_AirspaceCopy.Name());
      if (!ValidFrequency(khz)) {
        khz = ExtractFrequency(_AirspaceCopy.Comment());
      }

      if (ValidFrequency(khz)) {
        WindowControl* wClose = _pForm->FindByName(TEXT("cmdClose"));
        if (wClose) {
          wClose->SetLeft(IBLSCALE(155));
          wClose->SetWidth(IBLSCALE(80));
        }

        _stprintf(buffer2, _T("%s %7.3f"), GetActiveStationSymbol(Appearance.UTF8Pictorials), khz / 1000.);
        wFreq->SetCaption(buffer2);
        wFreq->Redraw();

        _stprintf(buffer2, _T("%s %7.3f"), GetStandyStationSymbol(Appearance.UTF8Pictorials), khz / 1000.);
        wSeqFreq->SetCaption(buffer2);
        wSeqFreq->Redraw();
        bRadio = true;
      }
    }
    wFreq->SetVisible(bRadio);
    wSeqFreq->SetVisible(bRadio);
  }

  wb = _pForm->FindByName<WndButton>(TEXT("cmdFly"));
  if (wb) {
    if (_AirspaceCopy.Flyzone()) {
      // LKTOKEN _@M1271_ "NOFLY"
      wb->SetCaption(MsgToken<1271>());
    }
    else {
      // LKTOKEN _@M1270_ "FLY"
      wb->SetCaption(MsgToken<1270>());
    }
    wb->Redraw();
  }

  wb = _pForm->FindByName<WndButton>(TEXT("cmdSelect"));
  if (wb) {
    if (_AirspaceCopy.Selected()) {
      wb->SetCaption(MsgToken<1656>());  // SELECTED!
    }
    else {
      wb->SetCaption(MsgToken<1654>());  // SELECT
    }
    wb->Redraw();
  }

  wb = _pForm->FindByName<WndButton>(TEXT("cmdAcknowledge"));
  if (wb) {
    if (_AirspaceCopy.Enabled()) {
      // LKTOKEN _@M1283_ "Disable"
      wb->SetCaption(MsgToken<1283>());
    }
    else {
      // LKTOKEN _@M1282_ "Enable"
      wb->SetCaption(MsgToken<1282>());
    }
    wb->Redraw();
  }
}

void dlgAirspaceDetails::OnDetailsClicked(WndButton* pWnd) const {
  TCHAR Details[READLINE_LENGTH + 1] = _T("");
  TCHAR Name[NAME_SIZE + 1] = _T("");

  if (_AirspaceCopy.Comment()) {
    lk::strcpy(Details, _AirspaceCopy.Comment());
  }
  else {
    lk::strcpy(Details, _AirspaceCopy.TypeName());
  }
  lk::snprintf(Name, _T("%s %s:"), _AirspaceCopy.TypeName(), MsgToken<231>()); // LKTOKEN  _@M231_ = "Details"

  dlgHelpShowModal(Name, Details, false);
}
