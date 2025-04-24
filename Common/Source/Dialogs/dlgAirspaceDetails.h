/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgAirspaceDetails.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 25 April 2025
 */
#ifndef DIALOG_DLG_AIRSPACE_DETAILS_H
#define DIALOG_DLG_AIRSPACE_DETAILS_H

#include "Airspace/LKAirspace.h"

class WndButton;
class WndOwnerDrawFrame;
class WndForm;
class LKSurface;

class dlgAirspaceDetails final {
 public:
  dlgAirspaceDetails(const CAirspacePtr& pAirspace);
  ~dlgAirspaceDetails();

  void DoModal();

 private:
  void OnPaintAirspacePicto(WndOwnerDrawFrame* Sender, LKSurface& Surface) const;
  void OnFlyClicked(WndButton* pWnd);
  void OnSelectClicked(WndButton* pWnd);
  void OnAcknowledgeClicked(WndButton* pWnd);
  void OnCloseClicked(WndButton* pWnd) const;
  bool OnTimer(WndForm* pWnd);
  void OnSetFrequency(WndButton* pWnd) const;
  void OnSetSecFrequency(WndButton* pWnd) const;
  void SetValues();
  void OnDetailsClicked(WndButton* pWnd) const;

 private:
  CAirspacePtr _pAirspace; // pointer to the airspace
  CAirspaceBase _AirspaceCopy; // copy of the airspace with actual values
  std::unique_ptr<WndForm> _pForm;
};

#endif  // DIALOG_DLG_AIRSPACE_DETAILS_H