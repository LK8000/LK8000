/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * $Id: dlgThermalDetails.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
 */

#include "externs.h"
#include "LKInterface.h"
#include "TeamCodeCalculation.h"
#include "dlgTools.h"
#include "resource.h"
#include "Calc/ThermalHistory.h"
#include "utils/printf.h"

namespace {

void OnSelectClicked(WndButton* pWnd, int thermal_idx) {

  SetThermalMultitarget(thermal_idx, _T("")); // update selected multitarget

  // switch to L> multitarget, and force moving map mode
  OvertargetMode=OVT_THER;
  SetModeType(LKMODE_MAP,MP_MOVING);

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

void SetValues(WndForm* pForm, int indexid) {
  auto thermal = GetThermalHistory(indexid);
  if (!thermal) { // TODO check
    StartupStore(_T("... LK thermal setvalues invalid indexid=%d"),indexid);
    DoStatusMessage(_T("ERR-216 INVALID THERMAL INDEXID"));
    return;
  }

  if (!thermal->Near.empty()) {
    TCHAR tcap[100];
    lk::snprintf(tcap,_T("%s %s: %s"),
                 MsgToken<905>(), // Thermal
                 MsgToken<456>(), // Near
                 thermal->Near.c_str());
    pForm->SetCaption(tcap);
  }

  auto wp = pForm->FindByName<WndProperty>(TEXT("prpName"));
  if (wp) {
    wp->SetText(thermal->Name.c_str());
    wp->RefreshDisplay();
  }

  TCHAR buffer[80];

  wp = pForm->FindByName<WndProperty>(TEXT("prpHTop"));
  if (wp) {
    Units::FormatAltitude(thermal->HTop, buffer, 80);
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = pForm->FindByName<WndProperty>(TEXT("prpHBase"));
  if (wp) {
    Units::FormatAltitude(thermal->HBase, buffer, 80);
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = pForm->FindByName<WndProperty>(TEXT("prpLift"));
  if (wp) {
    _stprintf(buffer,_T("%+.1f %s"),Units::ToVerticalSpeed(thermal->Lift), Units::GetVerticalSpeedName());
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }

  wp = pForm->FindByName<WndProperty>(TEXT("prpTeamCode"));
  if (wp) {
    // Taken from CalculateTeamBear..
    if (WayPointList.empty()) return;
    if (TeamCodeRefWaypoint < 0) return;

    double distance=0, bearing=0;

    DistanceBearing( WayPointList[TeamCodeRefWaypoint].Latitude,
            WayPointList[TeamCodeRefWaypoint].Longitude,
            thermal->position.latitude,
            thermal->position.longitude,
            &distance, &bearing);

    GetTeamCode(buffer, bearing, distance);

    buffer[5]='\0';
    wp->SetText(buffer);
    wp->RefreshDisplay();
  }
}

} // namespace

void dlgThermalDetails(int indexid) {
  using std::placeholders::_1;

  const CallBackTableEntry_t CallBackTable[] = {
    ClickNotifyCallbackEntry(OnCloseClicked),
    { "OnSelectClicked", std::bind(&OnSelectClicked, _1, indexid) },
    EndCallBackEntry()
  };

  std::unique_ptr<WndForm> pForm(dlgLoadFromXML(CallBackTable, IDR_XML_THERMALDETAILS));
  if (pForm) {
    SetValues(pForm.get(), indexid);
    pForm->ShowModal();
  }
}
