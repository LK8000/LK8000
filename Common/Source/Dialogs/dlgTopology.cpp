/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTopology.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "Dialogs.h"
#include "WindowControls.h"
#include "dlgTools.h"
#include "resource.h"
#include <memory>
#include "MapDraw/ZoomTopology.h"

namespace {

void OnTopoActiveData(DataField *Sender, DataField::DataAccessKind_t Mode){
  // No Op...
}

void OnCloseClicked(WndButton* pWnd) {
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

void setTopologyPropertyValue(WndForm* wf, const TCHAR* propertyName,
                              topology_category category, double storedValue) {
  if (!wf || !propertyName || propertyName[0] == '\0') {
    return;
  }

  auto wp = wf->FindByName<WndProperty>(propertyName);
  if (!wp) {
    return;
  }

  auto prop = TopologyZoomLevel::get(category);
  if (!prop) {
    wp->SetVisible(false);
    return;
  }

  auto df = wp->GetDataField();
  if (df) {
    auto value = Units::From(Units_t::unKiloMeter, prop->Zoom());
    value = Units::ToDistance(value);
    df->SetAsFloat(value);
  }
  wp->RefreshDisplay();
}

void setVariables(WndForm* wf) {
  for (auto& prop : TopologyZoomLevel::list()) {
    setTopologyPropertyValue(wf, prop.name_, prop.category_, prop.ref_zoom_);
  }
}

void updateTopologyPropertyValue(WndForm* wf, const TCHAR* propertyName,
                                 topology_category category) {
  if (!wf || !propertyName || propertyName[0] == '\0') {
    return;
  }

  auto wp = wf->FindByName<WndProperty>(propertyName);
  if (wp) {
    auto df = wp->GetDataField();
    if (df) {
      auto value = Units::FromDistance(df->GetAsFloat());
      value = Units::To(Units_t::unKiloMeter, value);
      TopologyZoomLevel::set(category, std::clamp(value, 0.0, 100.0));
    }
  }
}

void OnResetTopologyClicked(WndButton* pWnd) {
  TopologyZoomLevel::list::reset();
  setVariables(pWnd->GetParentWndForm());

  //LKTOKEN _@M1223_ "Topology reset to default values"
  MessageBoxX(MsgToken<1223>(), TEXT(""), mbOk);
}

CallBackTableEntry_t CallBackTable[]={
  CallbackEntry(OnTopoActiveData),
  CallbackEntry(OnResetTopologyClicked),
  CallbackEntry(OnCloseClicked),
  EndCallbackEntry()
};

}  // namespace

void dlgTopologyShowModal() {
  if (LKTopo < 1) {
    // LKTOKEN  _@M502_ = "Only LKMaps can be configured, sorry!"
    MessageBoxX(MsgToken<502>(), TEXT(""), mbOk);
    return;
  }

  std::unique_ptr<WndForm> wf(dlgLoadFromXML(CallBackTable, IDR_XML_TOPOLOGY));
  if (!wf) {
    return;
  }

  setVariables(wf.get());

  wf->ShowModal();

  for (auto& prop : TopologyZoomLevel::list()) {
    updateTopologyPropertyValue(wf.get(), prop.name_, prop.category_);
  }
}
