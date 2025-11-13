/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspacePatterns.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "../dlgSelectItem.h"

#ifdef HAVE_HATCHED_BRUSH

namespace {

class dlgAirspacePatterns : public dlgSelectItem {
public:
  dlgAirspacePatterns() = default;

protected:
  void DrawItem(LKSurface& Surface, const PixelRect& DrawRect, size_t ItemIndex) const override {
    Surface.SelectObject(LK_BLACK_PEN);
    Surface.SelectObject(LKBrush_White);
    Surface.SetBkColor(RGB_WHITE);
    Surface.SetTextColor(RGB_BLACK);
    Surface.SelectObject(MapWindow::GetAirspaceBrush(ItemIndex));

    Surface.Rectangle(DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom);
  }

  int GetItemCount() const override {
    return NUMAIRSPACEBRUSHES;
  }

  const TCHAR* GetTitle() const override {
    return MsgToken<594>();
  }

};

}  // namespace

int dlgAirspacePatternsShowModal() {
  return dlgAirspacePatterns().DoModal();
}

#endif
