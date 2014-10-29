/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "dlgTools.h"

void MSG_NotEnoughMemory(void) {

  MessageBoxX(
    MsgToken(1663),  // NOT ENOUGH MEMORY
    MsgToken(1662),  // SYSTEM ERROR
    MB_OK|MB_ICONEXCLAMATION);

}

#if USELKASSERT
void MSG_ASSERTION(int line, const TCHAR *filename) {

  TCHAR ames[256];

  _stprintf(ames,_T("Execution failure in file\n%s\nat line %d\n\nLK8000 terminated!"),filename,line);
  MessageBoxX(ames,
    _T("CRITICAL ASSERTION FAILURE !"),
    MB_OK|MB_ICONEXCLAMATION,true);

}
#endif
