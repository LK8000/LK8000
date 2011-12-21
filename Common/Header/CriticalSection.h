/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: CriticalSection.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

#include <windows.h>

class CCriticalSection
{
  CRITICAL_SECTION _criticalSection;
  void Lock()   { EnterCriticalSection(&_criticalSection); }
  void UnLock() { LeaveCriticalSection(&_criticalSection); }
public:
  class CGuard;
  CCriticalSection()  { InitializeCriticalSection(&_criticalSection); }
  ~CCriticalSection() { DeleteCriticalSection(&_criticalSection); }
};


class CCriticalSection::CGuard {
  CCriticalSection &_criticalSection;
public:
  explicit CGuard(CCriticalSection &criticalSection): _criticalSection(criticalSection) { _criticalSection.Lock(); }
  ~CGuard() { _criticalSection.UnLock(); }
};

#endif
