/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgTools.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#if !defined(__DLGTOOLS_H)
#define __DLGTOOLS_H

class WndForm;

#define OnHelpCallbackEntry(x)      {TEXT(#x), (void *)static_cast<WindowControl::OnHelpCallback_t>(x)}
#define OnListCallbackEntry(x)      {TEXT(#x), (void *)static_cast<WndListFrame::OnListCallback_t>(x)}
#define OnPaintCallbackEntry(x)     {TEXT(#x), (void *)static_cast<WndOwnerDrawFrame::OnPaintCallback_t>(x)}
#define ClickNotifyCallbackEntry(x) {TEXT(#x), (void *)static_cast<WndButton::ClickNotifyCallback_t>(x)}
#define DataChangeCallbackEntry(x)  {TEXT(#x), (void *)static_cast<WndProperty::DataChangeCallback_t>(x)}
#define DataAccessCallbackEntry(x)  {TEXT(#x), (void *)static_cast<DataField::DataAccessCallback_t>(x)}

#define EndCallBackEntry()          {TEXT(""), NULL}

int DLGSCALE(int x);

typedef struct{
  const TCHAR *Name;
  void *Ptr;
}CallBackTableEntry_t;

WndForm *dlgLoadFromXML(CallBackTableEntry_t *LookUpTable, unsigned resID);

#endif
