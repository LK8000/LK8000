/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   dlgSelectItem.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 31 April 2022
 */
#ifndef _DIALOGS_DLGSELECTITEM_H_
#define _DIALOGS_DLGSELECTITEM_H_

#include "Compiler.h"
#include "WindowControls.h"

class dlgSelectItem {
public:
  dlgSelectItem() = default;
  virtual ~dlgSelectItem() {}

  int DoModal();

protected:
  virtual void DrawItem(LKSurface& Surface, const PixelRect& DrawRect, size_t ItemIndex) const = 0;
  virtual int GetItemCount() const = 0;

  virtual const TCHAR* GetTitle() const = 0;

private:
  void OnPaintListItem(WndOwnerDrawFrame* Sender, LKSurface& Surface);
  void OnListInfo(WndListFrame* Sender, WndListFrame::ListInfo_t* ListInfo);
};

#endif // _DIALOGS_DLGSELECTITEM_H_
