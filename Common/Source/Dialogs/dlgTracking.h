#pragma once
#include "Compiler.h"
#include "WindowControls.h"
#include "Tracking/Tracking.h"

class dlgTracking final {
 public:
  dlgTracking() = delete;
  explicit dlgTracking(tracking::Profile& profile);

  dlgTracking(const dlgTracking&) = delete;
  dlgTracking& operator=(const dlgTracking&) = delete;

  int DoModal();

 private:
  void OnTrackingType(DataField* Sender, DataField::DataAccessKind_t Mode);
  void OnServer(DataField* Sender, DataField::DataAccessKind_t Mode);
  void OnStartConfig(DataField* Sender, DataField::DataAccessKind_t Mode);
  void OnInterval(DataField* Sender, DataField::DataAccessKind_t Mode);
  void OnPort(DataField* Sender, DataField::DataAccessKind_t Mode);
  void OnUser(DataField* Sender, DataField::DataAccessKind_t Mode);
  void OnPassword(DataField* Sender, DataField::DataAccessKind_t Mode);
  void OnRadar(DataField* Sender, DataField::DataAccessKind_t Mode);

  void ShowFrame(WndForm* pForm, const TCHAR* WndName, tracking::platform platform);

  void UpdateTypeUI(WndForm* pForm);

  tracking::Profile& _profile;
};
