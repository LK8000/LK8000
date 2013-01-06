/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


void MapWindow::DrawAirSpaceBorders(HDC hdc, const RECT rc)
{
  CAirspaceList::const_iterator it;
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;
  HBRUSH oldBrush;
  oldBrush=(HBRUSH)SelectObject(hdc,(HBRUSH)NULL);
  
  CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
      for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->DrawStyle()) {
          airspace_type = (*it)->Type();
          if ( asp_selected_flash && (*it)->Selected() ) {
            SelectObject(hdc, GetStockObject(BLACK_PEN));
          } else {
            SelectObject(hdc, hAirspacePens[airspace_type]);
          }
          (*it)->Draw(hdc, rc, false);
        }
      }//for

  SelectObject(hdc,oldBrush);
}


