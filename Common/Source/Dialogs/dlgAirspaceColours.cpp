/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceColours.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "dlgSelectItem.h"

namespace {

class dlgAirspaceColors : public dlgSelectItem {
public:
  dlgAirspaceColors() = default;

protected:
  void DrawItem(LKSurface& Surface, const PixelRect& DrawRect, size_t ItemIndex) const override {
    Surface.SelectObject(LK_BLACK_PEN);
    Surface.SelectObject(MapWindow::GetAirspaceSldBrush(ItemIndex));  // this is the solid brush

    Surface.SetBkColor(LKColor(0xFF, 0xFF, 0xFF));
    Surface.SetTextColor(MapWindow::GetAirspaceColour(ItemIndex));

    Surface.Rectangle(DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom);
  }

  int GetItemCount() const override {
    return NUMAIRSPACECOLORS;
  }

  const TCHAR* GetTitle() const override {
    return MsgToken<593>();
  }

};

}  // namespace

int dlgAirspaceColoursShowModal() {
  return dlgAirspaceColors().DoModal();
}
