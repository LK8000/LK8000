/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgConfiguration2.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "WindowControls.h"
#include "Waypointparser.h"
#include "Dialogs.h"


extern int page2mode(void);

void OnInfoBoxHelp(WndProperty * Sender){
  int type = Sender->GetDataField()->GetAsInteger();
  TCHAR caption[100];
  TCHAR mode[20];
  TCHAR shelp[20];

  switch (page2mode()) {
  case 0:
	// LKTOKEN  _@M835_ = "cruise"
    _tcscpy(mode,MsgToken(835));
    break;
  case 1:
	// LKTOKEN  _@M834_ = "circling"
    _tcscpy(mode,MsgToken(834));
    break;
  case 2:
	// LKTOKEN  _@M836_ = "final glide"
    _tcscpy(mode,MsgToken(836));
    break;
  case 3:
	// LKTOKEN  _@M833_ = "auxiliary"
    _tcscpy(mode,MsgToken(833));
    break;
  default:
	// LKTOKEN  _@M266_ = "Error"
    _tcscpy(mode,MsgToken(266));
    LKASSERT(false);
    return;
  }

  _stprintf(caption, TEXT("InfoBox %s in %s mode"), Sender->GetCaption(), mode);

  _stprintf(shelp,_T("_@H%d_"),type+800);
  dlgHelpShowModal(caption, shelp);


}
