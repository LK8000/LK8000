/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: MapWindowDisplayMode.cpp,v 8.29 2011/01/06 02:07:52 root Exp root $
*/


#include "externs.h"


MapWindow::Mode::Mode():
  _mode(MODE_CRUISE), _lastMode(MODE_NONE), _userForcedFlyMode(MODE_FLY_NONE),_autoNorthUP(false)
{
}


/**
 * @brief Changes given fly mode
 *
 * Method changes flight display mode based on provided data and mode forced
 * by the user (if valid).
 *
 * @param flyMode FLY mode to set
 */
void MapWindow::Mode::Fly(TModeFly flyMode)
{
  if(_userForcedFlyMode != MODE_FLY_NONE)
    flyMode = _userForcedFlyMode;

  _lastMode = _mode;
  _mode = flyMode | (_mode & SPECIAL_MASK);

  if(_mode != _lastMode)
    MapWindow::zoom.SwitchMode();
}


/**
 * @brief Sets given special mode
 *
 * Method enables/disables special display mode.
 *
 * @param specialMode Special mode to change
 * @param enable Specifies if the mode should be enabled or disabled
 */
void MapWindow::Mode::Special(TModeSpecial specialMode, bool enable)
{
  _lastMode = _mode;
  if(enable)
    _mode |= specialMode;
  else
    _mode &= ~specialMode;

  if(_mode != _lastMode)
    MapWindow::zoom.SwitchMode();
}
