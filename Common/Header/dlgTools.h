/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTools.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __DLGTOOLS_H
#define __DLGTOOLS_H

#include <variant>
#include "WindowControls.h"


using callback_variant = std::variant<
         WindowControl::OnHelpCallback_t,
         WndListFrame::OnListCallback_t,
         WndOwnerDrawFrame::OnPaintCallback_t,
         WndButton::ClickNotifyCallback_t,
         WndProperty::DataChangeCallback_t,
         DataField::DataAccessCallback_t,
         std::nullptr_t>;

struct CallBackTableEntry_t {
  const char *Name;
  const callback_variant callback;
};

inline CallBackTableEntry_t
make_OnHelpCallback(const char* Name, WindowControl::OnHelpCallback_t callback) {
  return { Name, callback };
}

#define OnHelpCallbackEntry(x) make_OnHelpCallback(#x, x)


inline CallBackTableEntry_t
make_OnListCallback(const char* Name, WndListFrame::OnListCallback_t callback) {
  return { Name, callback };
}

#define OnListCallbackEntry(x) make_OnListCallback(#x, x)


inline CallBackTableEntry_t
make_OnPaintCallback(const char* Name, WndOwnerDrawFrame::OnPaintCallback_t callback) {
  return { Name, callback };
}

#define OnPaintCallbackEntry(x) make_OnPaintCallback(#x, x)


inline CallBackTableEntry_t
make_ClickNotifyCallback(const char* Name, WndButton::ClickNotifyCallback_t callback) {
  return { Name, callback };
}

#define ClickNotifyCallbackEntry(x) make_ClickNotifyCallback(#x, x)


inline CallBackTableEntry_t 
make_DataChangeCallback(const char* Name, WndProperty::DataChangeCallback_t callback) {
  return { Name, callback };
}

#define DataChangeCallbackEntry(x) make_DataChangeCallback(#x, x)


inline CallBackTableEntry_t
make_DataAccessCallback(const char* Name, DataField::DataAccessCallback_t callback) {
  return { Name, callback };
}

#define DataAccessCallbackEntry(x) make_DataAccessCallback(#x, x)

#define EndCallBackEntry()          { nullptr, nullptr }

WndForm *dlgLoadFromXML(const CallBackTableEntry_t *LookUpTable, unsigned resID);

#endif // __DLGTOOLS_H
