/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindowZoom.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/


#include "StdAfx.h"
#include "MapWindow.h"
#include "externs.h"

#include "utils/heapcheck.h"


MapWindow::Zoom::Zoom():
  _inited(false),
  _autoZoom(false), _circleZoom(true), _bigZoom(true),
  _scale(0), _requestedScale(&_modeScale[SCALE_CRUISE]),
  _scaleOverDistanceModify(0),
  _resScaleOverDistanceModify(0)
{
  for(unsigned i=0; i<SCALE_NUM; i++)
    _modeScale[i] = 0;
}


/** 
 * @brief Sets requested zoom scale for TARGET_PAN mode
 */
void MapWindow::Zoom::CalculateTargetPanZoom()
{
  // set scale exactly so that waypoint distance is the zoom factor across the screen
  *_requestedScale = LimitMapScale(TargetZoomDistance * DISTANCEMODIFY / 6.0);
}


/** 
 * @brief Sets requested zoom scale for AUTO_ZOOM mode
 */
void MapWindow::Zoom::CalculateAutoZoom()
{
  static int autoMapScaleWaypointIndex = -1;
  
  double wpd = DerivedDrawInfo.ZoomDistance; 
  if(wpd > 0) {
    double AutoZoomFactor;
    if( (DisplayOrientation == NORTHTRACK && !mode.Is(Mode::MODE_CIRCLING)) ||
        DisplayOrientation == NORTHUP ||
        DisplayOrientation == NORTHSMART ||
        ((DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TRACKCIRCLE) && mode.Is(Mode::MODE_CIRCLING)) )
      AutoZoomFactor = 2.5;
    else
      AutoZoomFactor = 4;
    
    if(wpd < AutoZoomFactor * _scaleOverDistanceModify) {
      // waypoint is too close, so zoom in
      _modeScale[SCALE_CRUISE] = LimitMapScale(wpd * DISTANCEMODIFY / AutoZoomFactor);
    }
  }
  
  LockTaskData();  // protect from external task changes
#ifdef HAVEEXCEPTIONS
  __try{
#endif
    // if we aren't looking at a waypoint, see if we are now
    if(autoMapScaleWaypointIndex == -1) {
      if(ValidTaskPoint(ActiveWayPoint))
        autoMapScaleWaypointIndex = Task[ActiveWayPoint].Index;
    }
    
    if(ValidTaskPoint(ActiveWayPoint)) {
      // if the current zoom focused waypoint has changed...
      if(autoMapScaleWaypointIndex != Task[ActiveWayPoint].Index) {
        autoMapScaleWaypointIndex = Task[ActiveWayPoint].Index;
        
        // zoom back out to where we were before
        _modeScale[SCALE_CRUISE] = _modeScale[SCALE_AUTO_ZOOM];
      }
    }
#ifdef HAVEEXCEPTIONS
  }__finally
#endif
     {
       UnlockTaskData();
     }
}


/** 
 * @brief Resets Map Zoom to initial values
 */
void MapWindow::Zoom::Reset()
{
  switch(AircraftCategory) {
  case umGlider:
  case umGAaircraft:
  case umCar:
    _modeScale[SCALE_CRUISE]   = SCALE_CRUISE_INIT;
    _modeScale[SCALE_CIRCLING] = SCALE_CIRCLING_INIT;
    _modeScale[SCALE_PANORAMA] = SCALE_PANORAMA_INIT;
    break;
    
  case umParaglider:
    switch(PGCruiseZoom) { // 091108
    case 0:
      _modeScale[SCALE_CRUISE] = 0.10;  // 088
      break;
    case 1:
      _modeScale[SCALE_CRUISE] = 0.12;  // 117
      break;
    case 2:
      _modeScale[SCALE_CRUISE] = 0.14;  // 205
      break;
    case 3:
      _modeScale[SCALE_CRUISE] = 0.16;  // 293
      break;
    case 4:
      _modeScale[SCALE_CRUISE] = 0.18; 
      break;
    case 5:
      _modeScale[SCALE_CRUISE] = 0.20; 
      break;
    case 6:
      _modeScale[SCALE_CRUISE] = 0.23; 
      break;
    case 7:
      _modeScale[SCALE_CRUISE] = 0.25; 
      break;
    case 8:
    default:
      _modeScale[SCALE_CRUISE] = 0.3; 
      break;
    }
    
    switch(PGClimbZoom) {
    case 0:
      _modeScale[SCALE_CIRCLING] = 0.05;
      break;
    case 1:
      _modeScale[SCALE_CIRCLING] = 0.07;
      break;
    case 2:
      _modeScale[SCALE_CIRCLING] = 0.09;
      break;
    case 3:
      _modeScale[SCALE_CIRCLING] = 0.14;
      break;
    case 4:
    default:
      _modeScale[SCALE_CIRCLING] = 0.03;
      break;
    }
    
    _modeScale[SCALE_PANORAMA] = SCALE_PG_PANORAMA_INIT;
    break;
    
  default:
    // make it an evident problem
    _modeScale[SCALE_CRUISE] = _modeScale[SCALE_CIRCLING] = _modeScale[SCALE_PANORAMA] = SCALE_INVALID_INIT;
    break;
  }
  
  if(_autoZoom)
    _modeScale[SCALE_AUTO_ZOOM] = _modeScale[SCALE_CRUISE];
  
  _requestedScale = &_modeScale[SCALE_CRUISE];
  _scale = *_requestedScale;
  _scaleOverDistanceModify = *_requestedScale / DISTANCEMODIFY;
  
  _inited = true;
  SwitchMode();
}


/** 
 * @brief Assigns proper zoom ratio for new Display Mode
 */
void MapWindow::Zoom::SwitchMode()
{
  if(!_inited)
    return;
  
  if((mode._mode & Mode::MODE_TARGET_PAN) && !(mode._lastMode & Mode::MODE_TARGET_PAN)) {
    // TARGET_PAN enabled
    _requestedScale = &_modeScale[SCALE_TARGET_PAN];
    CalculateTargetPanZoom();
    zoom._bigZoom = true;
  }
  else if(mode._mode & Mode::MODE_TARGET_PAN) {
    // do not change zoom for other mode changes while in TARGET_PAN mode
    return;
  }
  else if(mode._mode & Mode::MODE_PAN) {
    if(!(mode._lastMode & Mode::MODE_PAN))
      // PAN enabled - use current map scale if PAN enabled
      _modeScale[SCALE_PAN] = *_requestedScale;
    
    _requestedScale = &_modeScale[SCALE_PAN];
    
    // do not change zoom for other mode changes while in PAN mode
    return;
  }
  else if((mode._mode & Mode::MODE_PANORAMA) && !(mode._lastMode & Mode::MODE_PANORAMA)) {
    // PANORAMA enabled
    _requestedScale = &_modeScale[SCALE_PANORAMA];
    zoom._bigZoom = true;
  }
  else if(mode._mode & Mode::MODE_PANORAMA) {
    // do not change zoom for mode changes while in PANORAMA mode
    return;
  }
  else {
    if((mode._mode & Mode::MODE_CIRCLING) && _circleZoom) {
      _requestedScale = &_modeScale[SCALE_CIRCLING];
    }
    else {
      _requestedScale = &_modeScale[SCALE_CRUISE];
      
      if(_autoZoom)
        CalculateAutoZoom();
    }
    _bigZoom = true;
  }
  *_requestedScale = LimitMapScale(*_requestedScale);
  
  RefreshMap();
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
  bool lastAutoZoom = _autoZoom;
  if(vswitch== -1)
    _autoZoom = !_autoZoom;
  else
    _autoZoom = vswitch;
  
  if(_autoZoom)
    // backup current zoom
    _modeScale[SCALE_AUTO_ZOOM] = _modeScale[SCALE_CRUISE];
  
  if(_autoZoom != lastAutoZoom)
    SwitchMode();
}


/** 
 * @brief Sets provided value as current zoom
 * 
 * @param value zoom ratio to set
 */
void MapWindow::Zoom::EventSetZoom(double value)
{
  double _lastRequestedScale = *_requestedScale;
  *_requestedScale = LimitMapScale(value);
  if(*_requestedScale != _lastRequestedScale) {
    zoom._bigZoom = true;
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
  // disable AutoZoom if possible
  if(_autoZoom &&
     mode.Special() == Mode::MODE_SPECIAL_NONE &&
     !(_circleZoom && mode.Is(Mode::MODE_CIRCLING))) {
    // Disable Auto Zoom only if not in Special or Circling Zoom
    DoStatusMessage(gettext(TEXT("_@M857_"))); // AutoZoom OFF
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
    zoom._bigZoom = true;
    RefreshMap();
  }
}


/** 
 * @brief Updates current map scale
 */
void MapWindow::Zoom::UpdateMapScale()
{
  if(mode.Is(Mode::MODE_TARGET_PAN)) {
    // update TARGET_PAN
    CalculateTargetPanZoom();
    if(_scale != *_requestedScale)
      ModifyMapScale();
    return;
  }
  
  if(_autoZoom &&
     mode.Special() == Mode::MODE_SPECIAL_NONE &&
     !(_circleZoom && mode.Is(Mode::MODE_CIRCLING))) {
    // Calculate Auto Zoom only if not in Special or Circling Zoom
    CalculateAutoZoom();
    if(_scale != *_requestedScale)
      ModifyMapScale();
    return;
  }
  
  // if there is user intervention in the scale
  if(_scale != *_requestedScale)
    ModifyMapScale();
}


/** 
 * @brief Recalculates zoom parameters
 */
void MapWindow::Zoom::ModifyMapScale()
{
  if(_scale == *_requestedScale)
    return;
  
  // limit zoomed in so doesn't reach silly levels
  *_requestedScale = LimitMapScale(*_requestedScale); // FIX VENTA remove limit
  _scaleOverDistanceModify = *_requestedScale / DISTANCEMODIFY;
  _resScaleOverDistanceModify = GetMapResolutionFactor() / _scaleOverDistanceModify;
  _drawScale = _scaleOverDistanceModify;
  _drawScale = _drawScale / 111194;
  _drawScale = GetMapResolutionFactor() / _drawScale;
  _invDrawScale = 1.0 / _drawScale;
  _scale = *_requestedScale;
}

