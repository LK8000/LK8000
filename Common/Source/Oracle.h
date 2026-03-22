/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: Oracle.h,v 1.1 2011/12/21 10:35:29 root Exp root $
*/

#ifndef Oracle_H
#define Oracle_H

#include "Thread/Thread.hpp"
#include "tchar.h"
#include <cassert>

/**
 * Thread class used by "Oracle" for find Topology Item nearest to current position.
 */
class WhereAmI : public Thread {
public:
    WhereAmI() : Thread("WhereAmI") {
        toracle[0] = _T('\0');
    }

    ~WhereAmI() { }


    const TCHAR* getText() const {
        // Caller must ensure thread is done before accessing result
        if (IsDefined()) {
            return nullptr;  // or consider throwing
        }
        return toracle;
    }

    bool Start() override {
        toracle[0] = _T('\0');
        return Thread::Start();
    }

    bool IsDone() const {
        return !IsDefined();
    }

protected:
    void Run() override;

    TCHAR toracle[1000];
};

#endif
