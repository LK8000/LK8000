#include <memory>
#include "dlgTracking.h"
#include "dlgTools.h"
#include "resource.h"
#include "LKLanguage.h" 
#include "Tracking/http_session.h"

namespace {

void OnCloseClicked(WndButton* pWnd) {
  pWnd->GetParentWndForm()->SetModalResult(mrCancel);
}

} // namespace

dlgTracking::dlgTracking(tracking::Profile& profile)
    : _profile(profile) {}

void dlgTracking::OnTrackingType(DataField* Sender,
                                 DataField::DataAccessKind_t Mode) {
  if (Sender->getCount() == 0) {
    Sender->addEnumList({
        PlatformLabel(tracking::platform::none),
        PlatformLabel(tracking::platform::livetrack24),
        PlatformLabel(tracking::platform::skylines_aero),
    });

    if (http_session::ssl_available()) {
      Sender->addEnumList({
          PlatformLabel(tracking::platform::ffvl),
          PlatformLabel(tracking::platform::osmand),
          PlatformLabel(tracking::platform::traccar)
      });
    }

  }

  auto& wp = Sender->GetOwner();
  auto pForm = wp.GetParentWndForm();

  switch (Mode) {
    case DataField::daGet:
      Sender->Set(static_cast<unsigned>(_profile.protocol));
      break;
    case DataField::daPut:
    case DataField::daChange:
      _profile.protocol = static_cast<tracking::platform>(Sender->GetAsInteger());
      if (_profile.protocol == tracking::platform::livetrack24) {
        if (_profile.server.empty()) {
          _profile.server = "www.livetrack24.com";
        }
        if (_profile.port == 0) {
          _profile.port = 80;
        }
      }
      UpdateTypeUI(pForm);
      break;
    default:
      break;
  }
}

void dlgTracking::OnServer(DataField* Sender,
                           DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->SetAsString(to_tstring(_profile.server).c_str());
      break;
    case DataField::daPut:
    case DataField::daChange:
      _profile.server = to_utf8(Sender->GetAsString());
      break;
    default:
      break;
  }
}

void dlgTracking::OnStartConfig(DataField* Sender,
                                DataField::DataAccessKind_t Mode) {
  if (Sender) {
    if (Sender->getCount() == 0) {
      Sender->addEnumList({
          MsgToken<2334>(),  // _@M2334_ "In flight only (default)"
          MsgToken<2335>()   // _@M2335_ "permanent (test purpose)"
      });
    }

    switch (Mode) {
      case DataField::daGet:
        Sender->Set(_profile.always_on);
        break;
      case DataField::daPut:
      case DataField::daChange:
        _profile.always_on = Sender->GetAsBoolean();
        break;
      default:
        break;
    }
  }
}

void dlgTracking::OnInterval(DataField* Sender,
                             DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->SetAsInteger(_profile.interval);
      break;
    case DataField::daPut:
    case DataField::daChange:
      _profile.interval = Sender->GetAsInteger();
      break;
    default:
      break;
  }
}

void dlgTracking::OnPort(DataField* Sender,
                         DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->SetAsInteger(_profile.port);
      break;
    case DataField::daPut:
    case DataField::daChange:
      _profile.port = Sender->GetAsInteger();
      break;
    default:
      break;
  }
}

void dlgTracking::OnUser(DataField* Sender,
                         DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:

      Sender->SetAsString(to_tstring(_profile.user).c_str());
      break;
    case DataField::daPut:
    case DataField::daChange:
      _profile.user = to_utf8(Sender->GetAsString());
      break;
    default:
      break;
  }
}

void dlgTracking::OnPassword(DataField* Sender,
                             DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->SetAsString(to_tstring(_profile.password).c_str());
      break;
    case DataField::daPut:
    case DataField::daChange:
      _profile.password = to_utf8(Sender->GetAsString());
      break;
    default:
      break;
  }
}

void dlgTracking::OnRadar(DataField* Sender,
                          DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->SetAsBoolean(_profile.radar);
      break;
    case DataField::daPut:
    case DataField::daChange:
      _profile.radar = Sender->GetAsBoolean();
      break;
    default:
      break;
  }
}

void dlgTracking::ShowFrame(
    WndForm* pForm, const TCHAR* WndName,
    std::initializer_list<tracking::platform> platform) {
  auto frm = pForm->FindByName(WndName);
  if (frm) {
    bool visible = std::find(platform.begin(), platform.end(),
                             _profile.protocol) != platform.end();
    frm->SetVisible(visible);
    if (visible) {
      frm->ForEachChild([](WindowControl* pChild) {
        auto wp = dynamic_cast<WndProperty*>(pChild);
        if (wp) {
          wp->RefreshDisplay();
        }
      });
    }
  }
}

void dlgTracking::UpdateTypeUI(WndForm* pForm) {
  if (pForm) {
    ShowFrame(pForm, _T("frmLT24"), {tracking::platform::livetrack24});
    ShowFrame(pForm, _T("frmSkylines"), {tracking::platform::skylines_aero});
    ShowFrame(pForm, _T("frmVLSafe"), {tracking::platform::ffvl});
    ShowFrame(pForm, _T("frmOsmAnd"),
              {tracking::platform::osmand, tracking::platform::traccar});
  }
}

int dlgTracking::DoModal() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  CallBackTableEntry_t CallBackTable[] = {
      CallbackEntry(OnCloseClicked),
      callback_entry("OnTrackingType",
                     std::bind(&dlgTracking::OnTrackingType, this, _1, _2)),
      callback_entry("OnServer",
                     std::bind(&dlgTracking::OnServer, this, _1, _2)),
      callback_entry("OnStartConfig",
                     std::bind(&dlgTracking::OnStartConfig, this, _1, _2)),
      callback_entry("OnInterval",
                     std::bind(&dlgTracking::OnInterval, this, _1, _2)),
      callback_entry("OnPort",
                     std::bind(&dlgTracking::OnPort, this, _1, _2)),
      callback_entry("OnUser",
                     std::bind(&dlgTracking::OnUser, this, _1, _2)),
      callback_entry("OnPassword",
                     std::bind(&dlgTracking::OnPassword, this, _1, _2)),
      callback_entry("OnRadar",
                     std::bind(&dlgTracking::OnRadar, this, _1, _2)),                     
      EndCallbackEntry()
  };

  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(CallBackTable, IDR_XML_TRACKING));
  if (!pForm) {
    return mrCancel;
  }

  UpdateTypeUI(pForm.get());

  return pForm->ShowModal();
}
