/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef LK8000_MACRO_H
#define LK8000_MACRO_H

// Be sure we are executing not faster than 1Hz!
#define ONEHZLIMITER {;static unsigned nextHB=0; if (LKHearthBeats< nextHB) return; nextHB=LKHearthBeats+2; }

// For debugging and trace tests, it should not be used out of testbench mode
// and not committed to masterbranch normally, for individual usage during debugging
#if TESTBENCH
#define IMHERE  StartupStore(_T(">>> %s:%u\n"), _T(__FILE__), __LINE__);
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif
