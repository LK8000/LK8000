/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#if !defined(LKASSERT_H)
#define LKASSERT_H

#include "MessageLog.h"

#if USELKASSERT
#if YDEBUG
  #define LKASSERT(arg) assert(arg);
#else
  extern void MSG_ASSERTION(int line, const TCHAR *filename);
  #define LKASSERT(arg) {;if (!(arg)) {; StartupStore(_T("[ASSERT FAILURE] in %s line %d\n"),_T(__FILE__),__LINE__); MSG_ASSERTION(__LINE__,_T(__FILE__)); exit(0);}}
#endif
#else
  #define LKASSERT(arg)
#endif

#if BUGSTOP
  #define BUGSTOP_LKASSERT(arg) LKASSERT(arg)
#else
  #define BUGSTOP_LKASSERT(arg) 
#endif

#if TESTBENCH
  #define TESTBENCH_DO_ONLY(n,arg) {;static short rpt=n; if (rpt>0) {; rpt--; ;{;(arg); };};};
#else
  #define TESTBENCH_DO_ONLY(n,arg)
#endif






#endif
