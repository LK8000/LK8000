/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
      for (CAirspaceList::const_reverse_iterator it=airspaces_to_draw.rbegin(); it != airspaces_to_draw.rend(); ++it) {
        if ((*it)->DrawStyle()) {
          if ( asp_selected_flash && (*it)->Selected() ) {
            Surface.SelectObject(LK_BLACK_PEN);
          } else {
            Surface.SelectObject(hBigAirspacePens[(*it)->Type()]);
          }
          if((*it)->DrawStyle()==adsDisabled) {
            Surface.SelectObject(LKPen_Grey_N2 );
          }

          (*it)->Draw(Surface, false);
        }
      } // for

      /***********************************************************************
       * now draw airspaces on top (normal order) with thin pen
       ***********************************************************************/

      for (const auto& pAsp : airspaces_to_draw) {
        if(!((pAsp->Top().Base == abMSL) && (pAsp->Top().Altitude <= 0))) 
        {  // don't draw on map if upper limit is on sea level or below
          if (pAsp->DrawStyle() ) {
            if ( asp_selected_flash && pAsp->Selected() ) {
              Surface.SelectObject(LK_BLACK_PEN);
            } else {
              Surface.SelectObject(hAirspacePens[pAsp->Type()]);
            }
            if(pAsp->DrawStyle() != adsDisabled)
              pAsp->Draw(Surface, false);
          }
        }
      }//for


  Surface.SelectObject(oldBrush);
}


