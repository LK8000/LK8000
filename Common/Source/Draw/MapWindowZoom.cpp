/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindowZoom.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/


#include "externs.h"
#include "Multimap.h"


/**
 * @brief Sets requested zoom scale for TARGET_PAN mode
 */
void MapWindow::Zoom::CalculateTargetPanZoom()
{
  // set scale exactly so that waypoint distance is the zoom factor across the screen
  *_requestedScale = LimitMapScale(Units::ToDistance(TargetZoomDistance / 6.0));
}


/**
 * @brief Sets requested zoom scale for AUTO_ZOOM mode
 */
void MapWindow::Zoom::CalculateAutoZoom_Locked() {

  // Do not AutoZoom if we have CircleZoom enabled  and we are Circling
  if (CircleZoom_Locked() && mode.Is(Mode::MODE_CIRCLING)) {
    return;
  }

  double wpd = DerivedDrawInfo.ZoomDistance;
  if ((wpd > 0) && wpd < AutoZoomThreshold) {
    double AutoZoomFactor = 4;
    if ((DisplayOrientation == NORTHTRACK) || DisplayOrientation == NORTHUP || DisplayOrientation == NORTHSMART ||
        ( (MapWindow::mode.autoNorthUP() || DisplayOrientation == NORTHCIRCLE ||
            DisplayOrientation == TARGETCIRCLE || DisplayOrientation == TARGETUP)
            && mode.Is(Mode::MODE_CIRCLING))) {
      AutoZoomFactor = 2.5;
    }
    _modeScale[SCALE_CRUISE] = LimitMapScale(Units::ToDistance(wpd) / AutoZoomFactor);
  } else {
    _modeScale[SCALE_CRUISE] = GetZoomInitValue(CruiseZoom);
  }

}

double MapWindow::Zoom::ResScaleOverDistanceModify() const {
  ScopeLock Lock(_zoomMutex);
  return _resScaleOverDistanceModify;
}

double MapWindow::Zoom::DrawScale() const {
  ScopeLock Lock(_zoomMutex);
  return _drawScale;
}

double MapWindow::Zoom::InvDrawScale() const {
  ScopeLock Lock(_zoomMutex);
  return _invDrawScale;
}

double MapWindow::Zoom::GetZoomInitValue(int parameter_number) const {
  // Initial cruise/Climb zoom map scales. Parameter number equal to config dlg item index
  // Values are given in user units, km or mi what is selected.
  // These values used to select the best available mapscale from scalelist. See MapWindow::FillScaleListForEngineeringUnits(

  switch (Units::GetDistanceUnit()) {
    default:
      return ScaleListArrayMeters[parameter_number];
    case unStatuteMiles:
      return ScaleListArrayStatuteMiles[parameter_number];
    case unNauticalMiles:
      return ScaleListNauticalMiles[parameter_number];
  }
}
/**
 * @brief Resets Map Zoom to initial values
 */
void MapWindow::Zoom::Reset()
{
  constexpr double SCALE_PANORAMA_INIT    = 10.0;

  ScopeLock Lock(_zoomMutex);  // Protect initialization of zoom state
  _modeScale[SCALE_CRUISE]   = GetZoomInitValue(CruiseZoom);
  _modeScale[SCALE_CIRCLING] = GetZoomInitValue(ClimbZoom);
  _modeScale[SCALE_PANORAMA] = SCALE_PANORAMA_INIT;
  _modeScale[SCALE_TARGET_PAN] = _modeScale[SCALE_CRUISE];  // Initialize TARGET_PAN

  if(_autoZoom)
    _modeScale[SCALE_AUTO_ZOOM] = _modeScale[SCALE_CRUISE];

  _requestedScale = &_modeScale[SCALE_CRUISE];

  _inited = true;
  SwitchMode_Locked();
}

double MapWindow::Zoom::RequestedScale() const {
  ScopeLock Lock(_zoomMutex);
  return *_requestedScale;
}

void MapWindow::Zoom::RequestedScale(double value) {
  ScopeLock Lock(_zoomMutex);
  *_requestedScale = value;
}

/**
 * @brief Assigns proper zoom ratio for new Display Mode (internal, called with lock held)
 */
void MapWindow::Zoom::SwitchMode_Locked() {
  if (!_inited) {
    return;
  }

  if (mode._mode & Mode::MODE_TARGET_PAN) {
    _requestedScale = &_modeScale[SCALE_TARGET_PAN];
  }
  else if (mode._mode & Mode::MODE_PAN) {
    if (!(mode._lastMode & Mode::MODE_PAN)) {
      // PAN enabled - use current map scale if PAN enabled
      _modeScale[SCALE_PAN] = *_requestedScale;
    }
    _requestedScale = &_modeScale[SCALE_PAN];
  }
  else if (mode._mode & Mode::MODE_PANORAMA) {
    // PANORAMA enabled
    _requestedScale = &_modeScale[SCALE_PANORAMA];
  }
  else if ((mode._mode & Mode::MODE_CIRCLING) && _circleZoom) {
    _requestedScale = &_modeScale[SCALE_CIRCLING];
  }
  else {
    _requestedScale = &_modeScale[SCALE_CRUISE];
  }
  RefreshMap();
}

void MapWindow::Zoom::AutoZoom(bool enable) {
  ScopeLock Lock(_zoomMutex);
  _autoZoom = enable;
  SwitchMode_Locked();
}

bool MapWindow::Zoom::AutoZoom() const {
  ScopeLock Lock(_zoomMutex);
  return _autoZoom;
}

void MapWindow::Zoom::CircleZoom(bool enable) {
  ScopeLock Lock(_zoomMutex);
  _circleZoom = enable;
  SwitchMode_Locked();
}

bool MapWindow::Zoom::CircleZoom() const {
  ScopeLock Lock(_zoomMutex);
  return CircleZoom_Locked();
}

void MapWindow::Zoom::BigZoom(bool enable) {
  ScopeLock Lock(_zoomMutex);
  _bigZoom = enable;
}

bool MapWindow::Zoom::BigZoom() const {
  ScopeLock Lock(_zoomMutex);
  return _bigZoom;
}

void MapWindow::Zoom::SetLimitMapScale(BOOL bOnOff) {
  ScopeLock Lock(_zoomMutex);
  _bMapScale = bOnOff;
}

/**
 * @brief Assigns proper zoom ratio for new Display Mode (thread-safe wrapper)
 */
void MapWindow::Zoom::SwitchMode() {
  ScopeLock Lock(_zoomMutex);  // Protect pointer and array access
  SwitchMode_Locked();
}

double MapWindow::Zoom::Scale() const {
  ScopeLock Lock(_zoomMutex);
  return _scale;
}

double MapWindow::Zoom::RealScale() const {
  ScopeLock Lock(_zoomMutex);
  return _realscale;
}

/**
 * @brief Switches AutoZoom state
 *
 * @param vswitch Switch value:
 *               - -1 - switch current state
 *               -  0 - disable
 *               -  1 - enable
 */
void MapWindow::Zoom::EventAutoZoom(int vswitch)
{
  ScopeLock Lock(_zoomMutex);  // Protect _autoZoom and _modeScale access
  bool lastAutoZoom = _autoZoom;
  if (vswitch == -1) {
    _autoZoom = !_autoZoom;
  }
  else {
    _autoZoom = vswitch;
  }

  if (_autoZoom) {
    // backup current zoom
    _modeScale[SCALE_AUTO_ZOOM] = _modeScale[SCALE_CRUISE];
  }

  if (_autoZoom != lastAutoZoom) {
    SwitchMode_Locked();
  }
}

/**
 * @brief Sets provided value as current zoom
 *
 * @param value zoom ratio to set
 */
void MapWindow::Zoom::EventSetZoom(double value) {
  ScopeLock Lock(_zoomMutex);  // Protect _requestedScale access
  double _lastRequestedScale = *_requestedScale;
  *_requestedScale = LimitMapScale(value);
  if (*_requestedScale != _lastRequestedScale) {
    RefreshMap();
  }
}

/**
 * @brief Modifies current zoom ratio
 *
 * @param vswitch Modifier value:
 */
void MapWindow::Zoom::EventScaleZoom(int vswitch)
{
  if (MapSpaceMode != MSM_MAP && !INPAN) {
    LKevent = (vswitch > 0) ? LKEVENT_UP : LKEVENT_DOWN;
    RefreshMap();
    return;
  }

  ScopeLock Lock(_zoomMutex);  // Protect _autoZoom, _circleZoom, and _requestedScale access
  // disable AutoZoom if possible
  if(_autoZoom &&
     mode.Special() == Mode::MODE_SPECIAL_NONE &&
     !(_circleZoom && mode.Is(Mode::MODE_CIRCLING))) {
    // Disable Auto Zoom only if not in Special or Circling Zoom
    DoStatusMessage(MsgToken<857>()); // AutoZoom OFF
    _autoZoom = false;
  }

  // For best results, zooms should be multiples or roots of 2
  double value = *_requestedScale;
  if(ScaleListCount > 0) {
    value = FindMapScale(*_requestedScale);
    value = StepMapScale(-vswitch);
  }
  else {
    if (abs(vswitch)>=4) {
      if (vswitch==4)
        vswitch = 1;
      if (vswitch==-4)
        vswitch = -1;
    }
    if (vswitch==1) { // zoom in a little
      value /= 1.414;
    }
    if (vswitch== -1) { // zoom out a little
      value *= 1.414;
    }
    if (vswitch==2) { // zoom in a lot
      value /= 2.0;
    }
    if (vswitch== -2) { // zoom out a lot
      value *= 2.0;
    }
  }
  double _lastRequestedScale = *_requestedScale;
  *_requestedScale = LimitMapScale(value);
  if(*_requestedScale != _lastRequestedScale) {
    RefreshMap();
  }
}


/**
 * @brief Updates current map scale
 */
void MapWindow::Zoom::UpdateMapScale() {
  ScopeLock Lock(_zoomMutex);  // Protect _requestedScale and _scale access

  if (mode.Is(Mode::MODE_TARGET_PAN)) {
    // update TARGET_PAN
    CalculateTargetPanZoom();
  }

  if (_autoZoom && mode.Special() == Mode::MODE_SPECIAL_NONE &&
      !(_circleZoom && mode.Is(Mode::MODE_CIRCLING))) {
    // Calculate Auto Zoom only if not in Special or Circling Zoom
    CalculateAutoZoom_Locked();
  }

  // if there is user intervention in the scale
  if (_scale != *_requestedScale) {
    ModifyMapScale();
  }
}

/**
 * @brief Recalculates zoom parameters
 */
void MapWindow::Zoom::ModifyMapScale()
{
  // limit zoomed in so doesn't reach silly levels
  if(_bMapScale) {
    *_requestedScale = LimitMapScale(*_requestedScale); // FIX VENTA remove limit
  }

  const double mapFactor = GetMapResolutionFactor();
  const double scaleOverDistanceModify = Units::FromDistance(*_requestedScale);

  _resScaleOverDistanceModify = mapFactor / scaleOverDistanceModify;
  _drawScale = mapFactor / (scaleOverDistanceModify / 111194); // what is this const value ?
  _invDrawScale = 1.0 / _drawScale;
  _scale = *_requestedScale;
  _realscale = Units::FromDistance(*_requestedScale) / 1000;
}


void MapWindow::Zoom::GetInitMapScaleText(int init_parameter, TCHAR *out, size_t size) const
{
  double mapscale = GetZoomInitValue(init_parameter);
  // Get nearest discrete value
  double ms = MapWindow::FindMapScale(mapscale);
  Units::FormatMapScale(Units::FromDistance(ms), out, size);
}


