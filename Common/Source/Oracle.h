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
#include <atomic>

/**
 * Thread class used by "Oracle" for find Topology Item nearest to current position.
 */
class WhereAmI : public Thread {
public:
    WhereAmI() : Thread("WhereAmI") {
        toracle[0] = _T('\0');
    }

    ~WhereAmI() { Join(); }


    const TCHAR* getText() const {
        return toracle;
    }

    bool Start() override {
        toracle[0] = _T('\0');
        _done.store(false, std::memory_order_relaxed);
        return Thread::Start();
    }

    bool IsDone() const {
        return _done.load(std::memory_order_acquire);
    }

protected:
    void Run() override;

    TCHAR toracle[1000];

private:
    std::atomic<bool> _done{false};
};

#endif
