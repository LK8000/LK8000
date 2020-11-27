/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Dialogs.h"

#if USELKASSERT
void MSG_ASSERTION(int line, const TCHAR *filename) {

/*
 * we can't use MessageBoxX() outside main thread otherwise we report crash inside  MessageBoxX instead of LKASSERT place.
 *  message is log inside Runtime.log instead.
 */
#ifndef ANDROID
  TCHAR ames[256];

  _stprintf(ames,_T("Execution failure in file\n%s\nat line %d\n\nLK8000 terminated!"),filename,line);
  MessageBoxX(ames,
    _T("CRITICAL ASSERTION FAILURE !"),
    mbOk,true);
#endif
}
#endif
