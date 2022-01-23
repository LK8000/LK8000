/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTools.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __DLGTOOLS_H
#define __DLGTOOLS_H

#include <variant>
#include <utility>
#include "WindowControls.h"


using callback_variant = std::variant<
         WindowControl::OnHelpCallback_t,
         WndListFrame::OnListCallback_t,
         WndOwnerDrawFrame::OnPaintCallback_t,
         WndButton::ClickNotifyCallback_t,
         DataField::DataAccessCallback_t,
         std::nullptr_t>;

struct CallBackTableEntry_t {
  const char *Name;
  const callback_variant callback;
};

template<typename Callback_t>
CallBackTableEntry_t callback_entry(const char* Name, Callback_t&& callback) {
  return { Name, std::forward<Callback_t>(callback) };
}

#define OnHelpCallbackEntry(x) callback_entry<WindowControl::OnHelpCallback_t>(#x, x)

#define OnListCallbackEntry(x) callback_entry<WndListFrame::OnListCallback_t>(#x, x)

#define OnPaintCallbackEntry(x) callback_entry<WndOwnerDrawFrame::OnPaintCallback_t>(#x, x)

#define ClickNotifyCallbackEntry(x) callback_entry<WndButton::ClickNotifyCallback_t>(#x, x)

#define DataAccessCallbackEntry(x) callback_entry<DataField::DataAccessCallback_t>(#x, x)

#define EndCallBackEntry() { nullptr, nullptr }

WndForm *dlgLoadFromXML(const CallBackTableEntry_t *LookUpTable, unsigned resID);

#endif // __DLGTOOLS_H
