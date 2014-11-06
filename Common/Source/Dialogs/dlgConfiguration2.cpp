/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgConfiguration2.cpp,v 1.1 2011/12/21 10:29:29 root Exp root $
*/

#include "externs.h"
#include "WindowControls.h"
#include "Waypointparser.h"
#include "Dialogs.h"


extern short config_page[];
extern short configMode;

void OnInfoBoxHelp(WindowControl * Sender){

  WndProperty *wp = (WndProperty*)Sender;
  int type = wp->GetDataField()->GetAsInteger();
  TCHAR caption[100];
  TCHAR mode[20];
  TCHAR shelp[20];

  switch (config_page[configMode]) {
  case 15:
	// LKTOKEN  _@M835_ = "cruise" 
    _tcscpy(mode,gettext(TEXT("_@M835_")));
    break;
  case 16:
	// LKTOKEN  _@M834_ = "circling" 
    _tcscpy(mode,gettext(TEXT("_@M834_")));
    break;
  case 17:
	// LKTOKEN  _@M836_ = "final glide" 
    _tcscpy(mode,gettext(TEXT("_@M836_")));
    break;
  case 18:
	// LKTOKEN  _@M833_ = "auxiliary" 
    _tcscpy(mode,gettext(TEXT("_@M833_")));
    break;
  default:
	// LKTOKEN  _@M266_ = "Error" 
    _tcscpy(mode,gettext(TEXT("_@M266_")));
    return;
  }

  _stprintf(caption, TEXT("InfoBox %s in %s mode"), wp->GetCaption(), mode);

  _stprintf(shelp,_T("_@H%d_"),type+800);
  dlgHelpShowModal(caption, shelp);


}

