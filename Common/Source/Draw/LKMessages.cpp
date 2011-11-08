/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"


void MSG_NotEnoughMemory(void) {

  MessageBoxX(hWndMapWindow,
    gettext(TEXT("_@M1663_")), // NOT ENOUGH MEMORY
    gettext(TEXT("_@M1662")),  // SYSTEM ERROR
    MB_OK|MB_ICONEXCLAMATION);

}


