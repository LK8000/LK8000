/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$

*/

#include <tchar.h>
#include "Dialogs/dlgProgress.h"

//
// The SPLASH screen for startup, shutdown and intermediate reloads
//

static dlgProgress* pWndProgress = NULL;

void CloseProgressDialog() {
    delete pWndProgress;
    pWndProgress = NULL;
}

void CreateProgressDialog(const TCHAR* text) {
    if(!pWndProgress) {
        pWndProgress = new dlgProgress();
    } 
    if(pWndProgress) {
        pWndProgress->SetProgressText(text);
    }
}
