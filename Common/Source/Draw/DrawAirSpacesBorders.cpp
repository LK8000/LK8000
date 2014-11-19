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
  CAirspaceList::const_iterator it;
  const CAirspaceList& airspaces_to_draw = CAirspaceManager::Instance().GetNearAirspacesRef();
  int airspace_type;
  static bool asp_selected_flash = false;
  asp_selected_flash = !asp_selected_flash;
  const auto oldBrush = Surface.SelectObject(LKBrush_Hollow);
  
  CCriticalSection::CGuard guard(CAirspaceManager::Instance().MutexRef());
  //    for (it=airspaces_to_draw.end(); it != airspaces_to_draw.begin(); it--)

      /***********************************************************************
       * draw underlying aispaces first (reverse order) with bigger pen
       * *********************************************************************/
      it=airspaces_to_draw.end();
      if (it!=airspaces_to_draw.begin())
      do
      {
	it--;
        if ((*it)->DrawStyle()) {
          airspace_type = (*it)->Type();
          if ( asp_selected_flash && (*it)->Selected() ) {
            Surface.SelectObject(LK_BLACK_PEN);
          } else {
            Surface.SelectObject(hBigAirspacePens[airspace_type]);
          }
          if((*it)->DrawStyle()==adsDisabled)
            Surface.SelectObject(LKPen_Grey_N2 );

          (*it)->Draw(Surface, rc, false);
        }
      } while (it !=airspaces_to_draw.begin());

      /***********************************************************************
       * now draw aispaces on top (normal order) with thin pen
       ***********************************************************************/

      for (it=airspaces_to_draw.begin(); it != airspaces_to_draw.end(); ++it) {
        if ((*it)->DrawStyle()) {
          airspace_type = (*it)->Type();
          if ( asp_selected_flash && (*it)->Selected() ) {
            Surface.SelectObject(LK_BLACK_PEN);
          } else {
            Surface.SelectObject(hAirspacePens[airspace_type]);
          }
          if((*it)->DrawStyle()==adsDisabled)
            Surface.SelectObject(LKPen_Black_N0 );
          (*it)->Draw(Surface, rc, false);
        }
      }//for


  Surface.SelectObject(oldBrush);
}


