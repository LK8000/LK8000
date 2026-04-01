/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgAirspaceColours.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "../dlgSelectItem.h"

namespace {

class dlgAirspaceColors : public dlgSelectItem {
public:
  dlgAirspaceColors() = default;

protected:
  void DrawItem(LKSurface& Surface, const PixelRect& DrawRect, size_t ItemIndex) const override {
    Surface.SelectObject(LK_BLACK_PEN);
    if (ItemIndex == 0) {
      Surface.SetBackgroundTransparent();
      Surface.SelectObject(LKBrush_White);
      Surface.Rectangle(DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom);
      Surface.SetTextColor(RGB_BLACK);
      RECT rc = DrawRect;
      Surface.DrawText(MsgToken<1921>(), &rc, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
    }
    else {
      auto color = MapWindow::AirspaceColor(ItemIndex -1);
      auto brush = MapWindow::AirspaceBrush(color);
      Surface.SelectObject(brush);
      Surface.Rectangle(DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom);
    }
  }

  int GetItemCount() const override {
    return NUMAIRSPACECOLORS + 1;
  }

  const TCHAR* GetTitle() const override {
    return MsgToken<593>();
  }

};

}  // namespace

int dlgAirspaceColoursShowModal() {
  return dlgAirspaceColors().DoModal();
}
