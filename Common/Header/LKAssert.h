/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#if !defined(LKASSERT_H)
#define LKASSERT_H

#if USELKASSERT

  extern void MSG_ASSERTION(int line, const TCHAR *filename);
  #define LKASSERT(arg) {;if (!(arg)) {; StartupStore(_T("[ASSERT FAILURE] in %S line %d\n"),_T(__FILE__),__LINE__); MSG_ASSERTION(__LINE__,_T(__FILE__)); exit(0);}}

#else

  #define LKASSERT(arg)

#endif







#endif
