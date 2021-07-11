/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: CriticalSection.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

#include "Thread/Mutex.hpp"

class CScopeLock {

    typedef void (*_Unlock)();
    typedef void (*_Lock)();
    
    _Unlock m_fUnlock;
public:
    
    explicit CScopeLock(_Lock fLock, _Unlock fUnlock) : m_fUnlock(fUnlock){
        fLock();
    }
    ~CScopeLock(){
        m_fUnlock();
    }
 };

#endif
