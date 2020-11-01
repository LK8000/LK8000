/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKObjects.h"

void MapWindow::DrawAirSpaceBorders(LKSurface& Surface, const RECT& rc)
{
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;
  const auto oldBrush = Surface.SelectObject(LKBrush_Hollow);

  ScopeLock guard(CAirspaceManager::Instance().MutexRef());
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();

  /***********************************************************************
   * draw underlying aispaces first (reverse order) with bigger pen
   * *********************************************************************/
  for (CAirspaceList::const_reverse_iterator it = airspaces_to_draw.rbegin(); it != airspaces_to_draw.rend(); ++it) {
    if ((*it)->Visible()) {
      if ((*it)->DrawStyle() == adsDisabled) {
        Surface.SelectObject(LKPen_Grey_N2);
      } else {
        if (asp_selected_flash && (*it)->Selected()) {
          Surface.SelectObject(LK_BLACK_PEN);
        } else {
          Surface.SelectObject(hBigAirspacePens[(*it)->Type()]);
        }
      }
      (*it)->Draw(Surface, false);
    }
  } // for

  /***********************************************************************
   * now draw airspaces on top (normal order) with thin pen
   ***********************************************************************/
  for (CAirspaceList::const_iterator it = airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
    if ((*it)->Visible()) {
      if ((*it)->DrawStyle() != adsDisabled) {
        if (asp_selected_flash && (*it)->Selected()) {
          Surface.SelectObject(LK_BLACK_PEN);
        } else {
          Surface.SelectObject(hAirspacePens[(*it)->Type()]);
        }
        (*it)->Draw(Surface, false);
      }
    }
  } // for

  Surface.SelectObject(oldBrush);
}
