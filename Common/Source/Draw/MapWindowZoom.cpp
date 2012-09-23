/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindowZoom.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/


#include "externs.h"


const double MapWindow::Zoom::SCALE_CRUISE_INIT      = 3.5;
const double MapWindow::Zoom::SCALE_CIRCLING_INIT    = 0.1;
const double MapWindow::Zoom::SCALE_PANORAMA_INIT    = 10.0;
const double MapWindow::Zoom::SCALE_PG_PANORAMA_INIT = 7.5;
const double MapWindow::Zoom::SCALE_INVALID_INIT     = 50.0;

MapWindow::Zoom::Zoom():
  _bMapScale (true),
  _inited(false),
  _autoZoom(false), _circleZoom(true), _bigZoom(false),
  _scale(0), _realscale(0),  _requestedScale(&_modeScale[SCALE_CRUISE]),
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
  static int wait_for_new_wpt_distance = 0;
  double wpd = DerivedDrawInfo.ZoomDistance; 
  
  if (wait_for_new_wpt_distance>0) wait_for_new_wpt_distance--;		//This counter is needed to get new valid waypoint distance after wp changes
  if ( (wpd > 0) && (wait_for_new_wpt_distance==0) ) {
    double AutoZoomFactor;
    if( (DisplayOrientation == NORTHTRACK && !mode.Is(Mode::MODE_CIRCLING)) ||
        DisplayOrientation == NORTHUP ||
        DisplayOrientation == NORTHSMART ||
        ((DisplayOrientation == NORTHCIRCLE || DisplayOrientation == TRACKCIRCLE) && mode.Is(Mode::MODE_CIRCLING)) )
      AutoZoomFactor = 2.5;
    else
      AutoZoomFactor = 4;
    
    if ( ( !ISPARAGLIDER && (wpd < AutoZoomFactor * _scaleOverDistanceModify) ) ||
	 ( ISPARAGLIDER  && (wpd < PGAutoZoomThreshold)) ) {
      // waypoint is too close, so zoom in
      LKASSERT(AutoZoomFactor!=0);
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
        wait_for_new_wpt_distance = 3;
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

double MapWindow::Zoom::GetPgClimbZoomInitValue(int parameter_number) const
{
  // Initial PG circling zoom map scales. Parameter number equal to config dlg item index
  // Values are given in user units, km or mi what is selected.
  switch(parameter_number) {
    case 0: return 0.015;
    case 1: return 0.025;
    case 2: return 0.04;
    case 3: return 0.07;
    case 4: return 0.1;
    case 5: return 0.15;
    default: return 0.025;
  }
}

double MapWindow::Zoom::GetPgCruiseZoomInitValue(int parameter_number) const
{
  // Initial PG cruise zoom map scales. Parameter number equal to config dlg item index
  // Values are given in user units, km or mi what is selected.
  // These values used to select the best available mapscale from scalelist. See MapWindow::FillScaleListForEngineeringUnits()
  switch(parameter_number) { // 091108
    case 0: return 0.04;
    case 1: return 0.07;
    case 2: return 0.10;
    case 3: return 0.15;
    case 4: return 0.20;
    case 5: return 0.35; 
    case 6: return 0.50; 
    case 7: return 0.75; 
    case 8: return 1.00; 
    case 9: return 1.50;
    default: return 0.35; 
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
    _modeScale[SCALE_CRUISE]   = SCALE_CRUISE_INIT;
    _modeScale[SCALE_CIRCLING] = SCALE_CIRCLING_INIT;
    _modeScale[SCALE_PANORAMA] = SCALE_PANORAMA_INIT;
    break;
    
  case umParaglider:
  case umCar:
    _modeScale[SCALE_CRUISE]   = GetPgCruiseZoomInitValue(PGCruiseZoom);
    _modeScale[SCALE_CIRCLING] = GetPgClimbZoomInitValue(PGClimbZoom);
    _modeScale[SCALE_PANORAMA] = SCALE_PG_PANORAMA_INIT;
    break;
    
  default:
    // make it an evident problem
    _modeScale[SCALE_CRUISE] = _modeScale[SCALE_CIRCLING] = _modeScale[SCALE_PANORAMA] = SCALE_INVALID_INIT;
    break;
  }

  // Correct _modeScale[] values for internal use
  // You have to give values in user units (km,mi, what is selected), we need to divide it by 1.4
  // because of the size of the mapscale symbol
  _modeScale[SCALE_CRUISE]   /= 1.4;
  _modeScale[SCALE_CIRCLING] /= 1.4;
  _modeScale[SCALE_PANORAMA] /= 1.4;

  if(_autoZoom)
    _modeScale[SCALE_AUTO_ZOOM] = _modeScale[SCALE_CRUISE];
  
  _requestedScale = &_modeScale[SCALE_CRUISE];
  _scale = *_requestedScale;
  _scaleOverDistanceModify = *_requestedScale / DISTANCEMODIFY;
  _realscale = *_requestedScale/DISTANCEMODIFY/1000;
  
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
    // zoom._bigZoom = true; REMOVE
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
    // zoom._bigZoom = true; REMOVE
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
    // _bigZoom = true; REMOVE
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
    // zoom._bigZoom = true; REMOVE
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
    // zoom._bigZoom = true; REMOVE
    RefreshMap();
  }
}


/** 
 * @brief Updates current map scale
 */
void MapWindow::Zoom::UpdateMapScale()
{
  static bool pg_autozoom_turned_on = false;
  
  if(mode.Is(Mode::MODE_TARGET_PAN)) {
    // update TARGET_PAN
    CalculateTargetPanZoom();
    if(_scale != *_requestedScale)
      ModifyMapScale();
    return;
  }
  
  // in PG mode if autozoom is set to on, and waypoint distance drops below 
  // PGAutoZoomThreshold, we should turn on autozoom if it is off. Do this only once, let the user able to turn it off near WP
  if ( ISPARAGLIDER && AutoZoom_Config && !_autoZoom && (DerivedDrawInfo.ZoomDistance>0) && (DerivedDrawInfo.ZoomDistance < PGAutoZoomThreshold)) {
    if (!pg_autozoom_turned_on) {
      EventAutoZoom(1);
      pg_autozoom_turned_on = true;
    }
  }
  if (DerivedDrawInfo.ZoomDistance > (PGAutoZoomThreshold + 200.0)) {
    // Set state variable back to false, with some distance hysteresis
    pg_autozoom_turned_on = false;
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
  // limit zoomed in so doesn't reach silly levels
  if(_bMapScale)
    *_requestedScale = LimitMapScale(*_requestedScale); // FIX VENTA remove limit
  _scaleOverDistanceModify = *_requestedScale / DISTANCEMODIFY;
  LKASSERT(_scaleOverDistanceModify!=0);
  _resScaleOverDistanceModify = GetMapResolutionFactor() / _scaleOverDistanceModify;
  _drawScale = _scaleOverDistanceModify;
  _drawScale = _drawScale / 111194;
  LKASSERT(_drawScale!=0);
  _drawScale = GetMapResolutionFactor() / _drawScale;
  _invDrawScale = 1.0 / _drawScale;
  _scale = *_requestedScale;
  _realscale = *_requestedScale/DISTANCEMODIFY/1000;
}


bool MapWindow::Zoom::GetPgClimbInitMapScaleText(int init_parameter, TCHAR *out, size_t size) const
{
  double mapscale = GetPgClimbZoomInitValue(init_parameter);
 
  // Get nearest discrete value
  double ms = MapWindow::FindMapScale(mapscale/1.4)*1.4;
  return Units::FormatUserMapScale(NULL, Units::ToSysDistance(ms), out, size);
}

bool MapWindow::Zoom::GetPgCruiseInitMapScaleText(int init_parameter, TCHAR *out, size_t size) const
{
  double mapscale = GetPgCruiseZoomInitValue(init_parameter);
 
  // Get nearest discrete value
  double ms = MapWindow::FindMapScale(mapscale/1.4)*1.4;
  return Units::FormatUserMapScale(NULL, Units::ToSysDistance(ms), out, size);
}

