/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/


#include "externs.h"
#include "Dialogs.h"

#ifdef ANDROID
#include <sstream>
#include "Android/crashlytics.h"
#endif


void MSG_NotEnoughMemory(void) {

  MessageBoxX(
    MsgToken(1663),  // NOT ENOUGH MEMORY
    MsgToken(1662),  // SYSTEM ERROR
    mbOk);

}

#if USELKASSERT
void MSG_ASSERTION(int line, const TCHAR *filename) {

/*
 * we can't use MessageBoxX() outside main thread otherwise we report crash inside  MessageBoxX instead of LKASSERT place.
 *  message is log inside Runitime.log and reported to crashlytics instead.
 */
#ifdef ANDROID
  extern crashlytics_context_t* crash_context;

  std::ostringstream oss;
  oss << "LKASSERT : " << filename << ":" << line << std::endl;

  if(crash_context) {
    crash_context->log(crash_context, oss.str().c_str());
  }

#else
  TCHAR ames[256];

  _stprintf(ames,_T("Execution failure in file\n%s\nat line %d\n\nLK8000 terminated!"),filename,line);
  MessageBoxX(ames,
    _T("CRITICAL ASSERTION FAILURE !"),
    mbOk,true);
#endif
}
#endif
