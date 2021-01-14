/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
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
         nullptr_t>;

struct CallBackTableEntry_t {
  const TCHAR *Name;
  callback_variant callback;
};

inline CallBackTableEntry_t
make_OnHelpCallback(const TCHAR* Name, WindowControl::OnHelpCallback_t callback) {
  return { Name, callback };
}

#define OnHelpCallbackEntry(x) make_OnHelpCallback(_T(#x), x)


inline CallBackTableEntry_t
make_OnListCallback(const TCHAR* Name, WndListFrame::OnListCallback_t callback) {
  return { Name, callback };
}

#define OnListCallbackEntry(x) make_OnListCallback(_T(#x), x)


inline CallBackTableEntry_t
make_OnPaintCallback(const TCHAR* Name, WndOwnerDrawFrame::OnPaintCallback_t callback) {
  return { Name, callback };
}

#define OnPaintCallbackEntry(x) make_OnPaintCallback(_T(#x), x)


inline CallBackTableEntry_t
make_ClickNotifyCallback(const TCHAR* Name, WndButton::ClickNotifyCallback_t callback) {
  return { Name, callback };
}

#define ClickNotifyCallbackEntry(x) make_ClickNotifyCallback(_T(#x), x)


inline CallBackTableEntry_t 
make_DataChangeCallback(const TCHAR* Name, WndProperty::DataChangeCallback_t callback) {
  return { Name, callback };
}

#define DataChangeCallbackEntry(x) make_DataChangeCallback(_T(#x), x)


inline CallBackTableEntry_t
make_DataAccessCallback(const TCHAR* Name, DataField::DataAccessCallback_t callback) {
  return { Name, callback };
}

#define DataAccessCallbackEntry(x) make_DataAccessCallback(_T(#x), x)

#define EndCallBackEntry()          { nullptr, nullptr }

WndForm *dlgLoadFromXML(CallBackTableEntry_t *LookUpTable, unsigned resID);

#endif // __DLGTOOLS_H
