/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgWindSettings.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "WindowControls.h"
#include "resource.h"

using std::placeholders::_1;
using std::placeholders::_2;

static void OnCancelClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrCancel);
    }
  }
}

static void OnCloseClicked(WndButton* pWnd){
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnWindSpeedData(double& WindSpeed, DataField* Sender,
                            DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->SetMax(Units::ToWindSpeed(Units::From(unKiloMeterPerHour, 200.0)));
      Sender->Set(Units::ToWindSpeed(WindSpeed));
      break;
    case DataField::daPut:
    case DataField::daChange:
      WindSpeed = Units::FromWindSpeed(Sender->GetAsFloat());
      break;
    default:
      // calc alt...
      break;
  }
}

static void OnWindDirectionData(double& WindBearing, DataField* Sender,
                                DataField::DataAccessKind_t Mode) {
  switch (Mode) {
    case DataField::daGet:
      Sender->Set(AngleLimit360(WindBearing));
      break;
    case DataField::daPut:
    case DataField::daChange:
      WindBearing = AngleLimit360(Sender->GetAsFloat());
      break;
    default:
      break;
  }
}


void dlgWindSettingsShowModal(void) {
  auto wind= WithLock(CritSec_FlightData, []() -> WindData {
    return {
        .Speed = CALCULATED_INFO.WindSpeed,
        .Direction = CALCULATED_INFO.WindBearing,
    };
  });

  CallBackTableEntry_t CallBackTable[] = {
      callback_entry(
          "OnWindSpeedData",
          std::bind(&OnWindSpeedData, std::ref(wind.Speed), _1, _2)),
      callback_entry(
          "OnWindDirectionData",
          std::bind(&OnWindDirectionData, std::ref(wind.Direction), _1, _2)),

      CallbackEntry(OnCancelClicked),
      CallbackEntry(OnCloseClicked),
      EndCallbackEntry()};

  std::unique_ptr<WndForm> wf(
      dlgLoadFromXML(CallBackTable, IDR_XML_WINDSETTINGS));
  if (!wf) {
    return;
  }

  auto wp = wf->FindByName<WndProperty>(TEXT("prpSpeed"));
  if (wp) {
    wp->GetDataField()->SetUnits(Units::GetHorizontalSpeedName());
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoWind"));
  if (wp) {
    DataField* dfe = wp->GetDataField();
    if (dfe) {
      dfe->addEnumList({
        MsgToken<418>(),  // Manual
        MsgToken<175>(),  // Circling
        LKGetText(TEXT("ZigZag")),
        MsgToken<149>(),  // Both
        MsgToken<1793>()  // External
      });
    }
    wp->GetDataField()->Set(AutoWindMode);
    wp->RefreshDisplay();
  }

  wp = wf->FindByName<WndProperty>(TEXT("prpTrailDrift"));
  if (wp) {
    wp->GetDataField()->Set(MapWindow::EnableTrailDrift);
    wp->RefreshDisplay();
  }

  int res =wf->ShowModal();
  if (res == mrCancel) {
    return;
  }

  SetWindEstimate(wind.Speed, wind.Direction);

  wp = wf->FindByName<WndProperty>(TEXT("prpAutoWind"));
  if (wp) {
    if (AutoWindMode != wp->GetDataField()->GetAsInteger()) {
      AutoWindMode = wp->GetDataField()->GetAsInteger();
    }
  }
  wp = wf->FindByName<WndProperty>(TEXT("prpTrailDrift"));
  if (wp) {
    if (MapWindow::EnableTrailDrift != wp->GetDataField()->GetAsBoolean()) {
      MapWindow::EnableTrailDrift = wp->GetDataField()->GetAsBoolean();
    }
  }
}
