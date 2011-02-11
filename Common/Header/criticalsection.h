#ifndef _CRITICALSECTION_H_
#define _CRITICALSECTION_H_


//
// CCriticalSection class
//
// Usage example
//
// CCriticalSection csTest;
//
// int Test()
// {
//  // no lock needed
//  if(1) {
//    CCriticalSection::CGuard guard(csTest);
//    // do something here
//    // ..........
//    // Now provide many exit points and do not care to unlock because it will be done automatically
//    if(0)
//      return 0;
//    if(1)
//      return 1;
//    if(2)
//      return 2;
//  }
//  // no lock needed
// }
//
//
class CCriticalSection
{
  CRITICAL_SECTION _criticalSection;
  void Lock() { EnterCriticalSection(&_criticalSection); }
  void UnLock() { LeaveCriticalSection(&_criticalSection); }
public:
  CCriticalSection() { InitializeCriticalSection(&_criticalSection); }
  ~CCriticalSection() { DeleteCriticalSection(&_criticalSection); }

public:
  class CGuard {
    CCriticalSection &_criticalSection;
  public:
    explicit CGuard( CCriticalSection &criticalSection ): _criticalSection(criticalSection) { _criticalSection.Lock(); }
    ~CGuard() { _criticalSection.UnLock(); }
  };
};

#endif
