/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   Cond.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015
 */

#ifndef _THREAD_COND_HPP_
#define	_THREAD_COND_HPP_

#include "Thread/Mutex.hpp"
#include "Poco/Condition.h"

class Cond : protected Poco::Condition {
public:
    Cond() = default;

    inline
    void Wait(Mutex &mutex) {
        Poco::Condition::wait<Mutex>(mutex);
    }

    inline
    bool Wait(Mutex &mutex, unsigned timeout_ms) {
        return Poco::Condition::tryWait<Mutex>(mutex, timeout_ms);
    }

    inline
    void Broadcast() {
        Poco::Condition::broadcast();
    }

    inline
    void Signal() {
        Poco::Condition::signal();
    }
};

#endif	/* _THREAD_COND_HPP_ */
