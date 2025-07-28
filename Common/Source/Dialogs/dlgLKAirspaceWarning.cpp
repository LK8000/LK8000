/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgLKAirspaceWarning.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgTools.h"
#include "resource.h"
#include "Sound/Sound.h"
#include "utils/printf.h"
#include "Event/Key.h"

namespace {

template <size_t size>
void FormatAirspaceMessage(TCHAR (&msgbuf)[size], const TCHAR* prefix, const CAirspaceBase& airspace) {
  TCHAR* pOut = msgbuf;
  size_t pOutSize = size;
  if (prefix) {
    size_t PrefixSize = lk::snprintf(msgbuf, _T("%s "), prefix);
    pOut += PrefixSize;
    pOutSize -= PrefixSize;
  }

  if (_tcsnicmp(airspace.Name(), airspace.TypeName(), _tcslen(airspace.TypeName())) == 0) {
    lk::strcpy(pOut, airspace.Name(), pOutSize);
  }
  else {
    lk::snprintf(pOut, pOutSize, _T("%s %s"), airspace.TypeName(), airspace.Name());
  }
}

class dlgAirspaceWarning final {
 public:
  dlgAirspaceWarning() = delete;

  dlgAirspaceWarning(AirspaceWarningMessage&& msg) : _Msg(std::move(msg)) {}
  ~dlgAirspaceWarning() = default;

  bool OnTimer(WndForm* pForm) {
    // Auto close dialog after some time
    if (!(--_TimerCounter)) {
      if (pForm) {
        pForm->SetModalResult(mrOK);
      }
      return true;
    }

    FillAirspaceData();
    return true;
  }

  bool OnKeyDown(WndForm* pWnd, unsigned KeyCode) {
    switch (KeyCode) {
      case KEY_RETURN:
        OnAckForTimeClicked(nullptr);
        return true;
      case KEY_ESCAPE:
        OnCloseClicked(nullptr);
        return true;
    }
    return false;
  }

  void OnAckForTimeClicked(WndButton* pWnd) {
    auto pAsp = _Msg.originator.lock();
    if (pAsp) {
      CAirspaceManager::Instance().AirspaceSetAckLevel(*pAsp, _Msg.warnlevel);
    }
    OnCloseClicked(pWnd);
  }

  void OnCloseClicked(WndButton* pWnd) {
    if (pWnd) {
      WndForm* pForm = pWnd->GetParentWndForm();
      if (pForm) {
        pForm->SetModalResult(mrOK);
      }
    }
  }

  void OnPaintAirspacePicto(WndOwnerDrawFrame* Sender, LKSurface& Surface) {
    if (Sender) {
      auto pAsp = _Msg.originator.lock();
      if (pAsp) {
        Surface.SetBkColor(RGB_LIGHTGREY);
        /****************************************************************
         * to draw the airspace pictorial, we need the original data.
         * copy contain only base class property, not geo data,
         * original data are shared ressources !
         * for that we need to grant all called methods are thread safe
         ****************************************************************/
        pAsp->DrawPicto(Surface, Sender->GetClientRect());
      }
    }
  }

  void DoModal() {
    using std::placeholders::_1;
    using std::placeholders::_2;

    CallBackTableEntry_t CallBackTable[] = {
        callback_entry("OnAckForTimeClicked", std::bind(&dlgAirspaceWarning::OnAckForTimeClicked, this, _1)),
        callback_entry("OnCloseClicked", std::bind(&dlgAirspaceWarning::OnCloseClicked, this, _1)),
        callback_entry("OnPaintAirspacePicto", std::bind(&dlgAirspaceWarning::OnPaintAirspacePicto, this, _1, _2)),
        EndCallbackEntry()
    };

    const unsigned resID = ScreenLandscape ? IDR_XML_LKAIRSPACEWARNING_L : IDR_XML_LKAIRSPACEWARNING_P;
    _pForm = std::unique_ptr<WndForm>(dlgLoadFromXML(CallBackTable, resID));
    if (!_pForm) {
      StartupStore(_T("------ LKAirspaceWarning setup FAILED!"));  //@ 101027
      return;
    }

    _pForm->SetKeyDownNotify(std::bind(&dlgAirspaceWarning::OnKeyDown, this, _1, _2));
    _pForm->SetTimerNotify(1000, std::bind(&dlgAirspaceWarning::OnTimer, this, _1));
    _TimerCounter = AirspaceWarningDlgTimeout;  // Auto closing dialog in x secs

    WndButton* wb = _pForm->FindByName<WndButton>(TEXT("cmdAckForTime"));
    if (wb) {
      TCHAR stmp2[40];
      lk::snprintf(stmp2, TEXT("%s (%dmin)"), MsgToken<46>(), AcknowledgementTime / 60);
      wb->SetCaption(stmp2);
    }

    FillAirspaceData();

    LKSound(_T("LK_AIRSPACE.WAV"));  // 100819

    _pForm->ShowModal();
  }

  void FillAirspaceData() {
    // Get a new copy with current values from airspacemanager
    auto pAsp = _Msg.originator.lock();
    if (!pAsp) {
      return;  // no message to display airspace not valid
    }

    CAirspaceBase _AirspaceCopy = CAirspaceManager::Instance().GetAirspaceCopy(pAsp);

    if (_Msg.warnlevel != _AirspaceCopy.WarningLevel()) {
      // we can automatically close the dialog when the warning level changes, probably new msg waiting in the queue
      _pForm->SetModalResult(mrOK);
    }

    TCHAR msgbuf[128];
    FormatAirspaceMessage(msgbuf, nullptr, _AirspaceCopy);
    _pForm->SetCaption(msgbuf);

    // Fill up dialog data
    WndProperty* wp;
    WndButton* wb;

    wp = _pForm->FindByName<WndProperty>(TEXT("prpReason"));
    if (wp) {
      switch (_Msg.event) {
        default:
          // Normally not show
          // LKTOKEN _@M765_ "Unknown"
          wp->SetText(MsgToken<765>());
          break;

        case aweNone:
          // LKTOKEN _@M479_ "None"
          wp->SetText(MsgToken<479>());
          break;

        case aweMovingInsideFly:
          // LKTOKEN _@M1242_ "Flying inside FLY zone"
          wp->SetText(MsgToken<1242>());
          break;

        case awePredictedLeavingFly:
          // LKTOKEN _@M1243_ "Predicted leaving FLY zone"
          wp->SetText(MsgToken<1243>());
          break;

        case aweNearOutsideFly:
          // LKTOKEN _@M1244_ "Near leaving FLY zone"
          wp->SetText(MsgToken<1244>());
          break;

        case aweLeavingFly:
          // LKTOKEN _@M1245_ "Leaving FLY zone"
          wp->SetText(MsgToken<1245>());
          break;

        case awePredictedEnteringFly:
          // LKTOKEN _@M1246_ "Predicted entering FLY zone"
          wp->SetText(MsgToken<1246>());
          break;

        case aweEnteringFly:
          // LKTOKEN _@M1247_ "Entering FLY zone"
          wp->SetText(MsgToken<1247>());
          break;

        case aweMovingOutsideFly:
          // LKTOKEN _@M1248_ "Flying outside FLY zone"
          wp->SetText(MsgToken<1248>());
          break;

        // Events for NON-FLY zones
        case aweMovingOutsideNonfly:
          // LKTOKEN _@M1249_ "Flying outside NOFLY zone"
          wp->SetText(MsgToken<1249>());
          break;

        case awePredictedEnteringNonfly:
          // LKTOKEN _@M1250_ "Predicted entering NOFLY zone"
          wp->SetText(MsgToken<1250>());
          break;

        case aweNearInsideNonfly:
          // LKTOKEN _@M1251_ "Near entering NOFLY zone"
          wp->SetText(MsgToken<1251>());
          break;

        case aweEnteringNonfly:
          // LKTOKEN _@M1252_ "Entering NOFLY zone"
          wp->SetText(MsgToken<1252>());
          break;

        case aweMovingInsideNonfly:
          // LKTOKEN _@M1253_ "Flying inside NOFLY zone"
          wp->SetText(MsgToken<1253>());
          break;

        case aweLeavingNonFly:
          // LKTOKEN _@M1254_ "Leaving NOFLY zone"
          wp->SetText(MsgToken<1254>());
          break;

      }  // sw
      switch (_AirspaceCopy.WarningLevel()) {
        case awYellow:
          wp->SetBackColor(RGB_YELLOW);
          wp->SetForeColor(RGB_BLACK);
          break;
        case awRed:
          wp->SetBackColor(RGB_RED);
          break;
        default:
          break;
      }
      wp->RefreshDisplay();
    }

    int hdist;
    int vdist;
    int bearing;
    bool inside;

    wp = _pForm->FindByName<WndProperty>(TEXT("prpType"));
    if (wp) {
      TCHAR buffer[80];
      if (_AirspaceCopy.Flyzone()) {
        lk::snprintf(buffer, _T("%s %s"), _T("FLY"), _AirspaceCopy.TypeName());
      }
      else {
        lk::snprintf(buffer, _T("%s %s"), _T("NOFLY"), _AirspaceCopy.TypeName());
      }

      wp->SetText(buffer);
      wp->RefreshDisplay();
    }

    // Unfortunatelly virtual methods don't work on copied instances
    // we have to ask airspacemanager to perform the required calculations
    // inside = airspace_copy.CalculateDistance(&hdist, &bearing, &vdist);
    // inside = CAirspaceManager::Instance().AirspaceCalculateDistance(msg.originator, &hdist, &bearing, &vdist);
    bool distances_ready = _AirspaceCopy.GetDistanceInfo(inside, hdist, bearing, vdist);

    wp = _pForm->FindByName<WndProperty>(TEXT("prpHDist"));
    if (wp) {
      TCHAR stmp[21];
      TCHAR stmp2[40];
      if (distances_ready) {
        Units::FormatDistance((double)abs(hdist), stmp, 10);
        if (hdist < 0) {
          // LKTOKEN _@M1257_ "to leave"
          lk::snprintf(stmp2, _T("%s %s"), stmp, MsgToken<1257>());
        }
        else {
          // LKTOKEN _@M1258_ "to enter"
          lk::snprintf(stmp2, _T("%s %s"), stmp, MsgToken<1258>());
        }
      }
      else {
        // no distance info calculated
        // LKTOKEN _@M1259_ "Too far, not calculated"
        lk::strcpy(stmp2, MsgToken<1259>());
      }
      wp->SetText(stmp2);
      wp->RefreshDisplay();
    }

    wp = _pForm->FindByName<WndProperty>(TEXT("prpVDist"));
    if (wp) {
      TCHAR stmp[21];
      TCHAR stmp2[40];
      if (distances_ready) {
        Units::FormatAltitude((double)abs(vdist), stmp, 10);
        if (vdist < 0) {
          // LKTOKEN _@M1260_ "below"
          lk::snprintf(stmp2, _T("%s %s"), stmp, MsgToken<1260>());
        }
        else {
          // LKTOKEN _@M1261_ "above"
          lk::snprintf(stmp2, _T("%s %s"), stmp, MsgToken<1261>());
        }
      }
      else {
        // no distance info calculated
        // LKTOKEN _@M1259_ "Too far, not calculated"
        lk::strcpy(stmp2, MsgToken<1259>());
      }
      wp->SetText(stmp2);
      wp->RefreshDisplay();
    }

    wp = _pForm->FindByName<WndProperty>(TEXT("prpTopAlt"));
    if (wp) {
      wp->SetText(_AirspaceCopy.Top().text_alternate());
      wp->RefreshDisplay();
    }

    wp = _pForm->FindByName<WndProperty>(TEXT("prpBaseAlt"));
    if (wp) {
      wp->SetText(_AirspaceCopy.Base().text_alternate());
      wp->RefreshDisplay();
    }

    wb = _pForm->FindByName<WndButton>(TEXT("cmdClose"));
    if (wb) {
      TCHAR stmp2[40];
      lk::snprintf(stmp2, _T("%s (%d)"), MsgToken<186>(), _TimerCounter);
      wb->SetCaption(stmp2);
    }
  }

 private:
  std::unique_ptr<WndForm> _pForm;
  int _TimerCounter = AirspaceWarningDlgTimeout;

  AirspaceWarningMessage _Msg = {};
};

}  // namespace

// Called periodically to show new airspace warning messages to user
// This is called by WINMAIN thread, every second (1hz)
void ShowAirspaceWarningsToUser() {
  AirspaceWarningMessage msg;
  bool there_is_message = CAirspaceManager::Instance().PopWarningMessage(&msg);
  if (!there_is_message) {
    return;  // no message to display
  }

  auto pAsp = msg.originator.lock();
  if (!pAsp) {
    return;  // no message to display airspace not valid
  }

  auto airspace_copy = CAirspaceManager::Instance().GetAirspaceCopy(pAsp);

  bool ackdialog_required = false;
  TCHAR msgbuf[128];

  // which message we need to show?
  switch (msg.event) {
    default:
      // normally not show
      DoStatusMessage(_T("Unknown airspace warning message"));
      break;  // Unknown msg type

    case aweNone:
    case aweMovingInsideFly:       // normal, no msg, normally this msg type shouldn't get here
    case awePredictedEnteringFly:  // normal, no msg, normally this msg type shouldn't get here
    case aweMovingOutsideNonfly:   // normal, no msg, normally this msg type shouldn't get here
      break;

    case awePredictedLeavingFly:
    case aweNearOutsideFly:
    case aweLeavingFly:
    case awePredictedEnteringNonfly:
    case aweNearInsideNonfly:
    case aweEnteringNonfly:
    case aweMovingInsideNonfly:  // repeated messages
    case aweMovingOutsideFly:    // repeated messages
      ackdialog_required = true;
      break;

    case aweEnteringFly:
      // LKTOKEN _@M1240_ "Entering"
      FormatAirspaceMessage(msgbuf, MsgToken<1240>(), airspace_copy);
      DoStatusMessage(msgbuf);
      break;

    case aweLeavingNonFly:
      // don't warn on leaving acknowledged airspaces
      if (!airspace_copy.Acknowledged()) {
        // LKTOKEN _@M1241_ "Leaving"
        FormatAirspaceMessage(msgbuf, MsgToken<1241>(), airspace_copy);
        DoStatusMessage(msgbuf);
      }
      break;
  }

  // show dialog to user if needed
  if (ackdialog_required && (airspace_copy.WarningLevel() == msg.warnlevel)) {
    dlgAirspaceWarning(std::move(msg)).DoModal();
  }
}
