/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif


#if (WINDOWSPC>0)
typedef DWORD (_stdcall *GetIdleTimeProc) (void);
GetIdleTimeProc GetIdleTime;
#endif

#if DEBUG_MEM
int MeasureCPULoad() {
#if (WINDOWSPC>0) && !defined(__MINGW32__)
  static bool init=false;
  if (!init) {
    // get the pointer to the function
    GetIdleTime = (GetIdleTimeProc) 
      GetProcAddress(LoadLibrary(_T("coredll.dll")),
		     _T("GetIdleTime"));
    init=true;
  }
  if (!GetIdleTime) return 0;
#endif

  static int pi;
  static int PercentIdle;
  static int PercentLoad;
  static bool start=true;
  static DWORD dwStartTick;
  static DWORD dwIdleSt;
  static DWORD dwStopTick;
  static DWORD dwIdleEd;
  if (start) {
    dwStartTick = GetTickCount();
    dwIdleSt = GetIdleTime();
  }
  if (!start) {
    dwStopTick = GetTickCount();
    dwIdleEd = GetIdleTime();
    pi = ((100 * (dwIdleEd - dwIdleSt))/(dwStopTick - dwStartTick));
    PercentIdle = (PercentIdle+pi)/2;
  }
  start = !start;
  PercentLoad = 100-PercentIdle;
  return PercentLoad;
}
#endif



#ifdef CPUSTATS
// Warning this is called by several concurrent threads, no static variables here
void Cpustats(int *accounting, FILETIME *kernel_old, FILETIME *kernel_new, FILETIME *user_old, FILETIME *user_new) {
   __int64 knew=0, kold=0, unew=0, uold=0;
   int total=2; // show evident problem

   knew = kernel_new->dwHighDateTime;
   knew <<= 32;
   knew += kernel_new->dwLowDateTime;
    
   unew=user_new->dwHighDateTime;
   unew <<=32;
   unew+=user_new->dwLowDateTime;

   kold = kernel_old->dwHighDateTime;
   kold <<= 32;
   kold += kernel_old->dwLowDateTime;
   
   uold=user_old->dwHighDateTime;
   uold <<=32;
   uold+=user_old->dwLowDateTime;

#if (WINDOWSPC>0)   
   total = (int) ((knew+unew-kold-uold)/10.0);
   //if (total==0) return;
#else
   total = (int) ((knew+unew-kold-uold)/10000.0);
#endif
   *accounting=total;

}

#endif
