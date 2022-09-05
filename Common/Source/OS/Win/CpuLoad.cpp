/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "../CpuLoad.h"

#if !(WINDOWSPC>0)
class GetCpuLoad_Singleton {
public:
    GetCpuLoad_Singleton() : pfnGetIdleTime() {
        hCoreDll = LoadLibrary(_T("coredll.dll"));
        if(hCoreDll) { // coredll.dll found ?
            pfnGetIdleTime = (pfnGetIdleTime_t)GetProcAddress(hCoreDll, _T("GetIdleTime"));
            if(pfnGetIdleTime) {
                dwStartTick = GetTickCount();
                dwIdleSt = (*pfnGetIdleTime)();
                if(dwIdleSt == (MAXDWORD)) { // GetIdleTime is implemented in this platform ?
                    pfnGetIdleTime = NULL;
                    FreeLibrary(hCoreDll);
                    hCoreDll = NULL;
                }
            }
        }
    }

    ~GetCpuLoad_Singleton() {
        if(hCoreDll) {
            FreeLibrary(hCoreDll);
        }
    }

    DWORD operator()() {
        if(!pfnGetIdleTime){  // GetIdleTime is not implemented in this platform
            return MAXDWORD;
        }
        DWORD dwStopTick = GetTickCount();
        DWORD dwIdleEd = (*pfnGetIdleTime)();

        DWORD PercentUsage;
	if ((dwStopTick-dwStartTick)>0)
		PercentUsage = 100 - ((100*(dwIdleEd - dwIdleSt)) / (dwStopTick - dwStartTick));
	else
		PercentUsage = 100 - (100*(dwIdleEd - dwIdleSt));

        dwStartTick = dwStopTick;
        dwIdleSt = dwIdleEd;

        return PercentUsage;
    }

private:
    typedef DWORD (_stdcall *pfnGetIdleTime_t) (void);
    pfnGetIdleTime_t pfnGetIdleTime;
    HINSTANCE hCoreDll;
    DWORD dwStartTick;
    DWORD dwIdleSt;
};

GetCpuLoad_Singleton GetGpuLoad;
#endif

int CpuSummary() {
#if (WINDOWSPC>0 || (!PNA && PPC2003))
  return INVALID_VALUE;
#else
  int s = GetGpuLoad();
  if (s<0 || s>999) s=INVALID_VALUE;
  return s;
#endif
}
