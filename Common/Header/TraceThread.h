/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

// Comment this line to disable tracing
// #define TRACETHREAD	1

#if TRACETHREAD
#if !defined(TRACETHREAD_H)
#define TRACETHREAD_H

extern int _THREADID_WINMAIN;
extern int _THREADID_CALC;
extern int _THREADID_DRAW;
extern int _THREADID_PORT1;
extern int _THREADID_PORT2;
extern int _THREADID_UNKNOWNPORT;

void TraceThread(const TCHAR *mes);

#define SHOWTHREAD(arg) {;TraceThread(arg);}

#endif
#else

#define SHOWTHREAD(arg)

#endif
