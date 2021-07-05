/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   Cond.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015
 */

#ifndef COND_HPP
#define	COND_HPP

#include "Thread/Mutex.hpp"
#include "Poco/Condition.h"

/// Exists only to avoids to change xcs original code...
class Cond : protected Poco::Condition {
public:
    Cond() {}
    ~Cond() {}

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


#endif	/* COND_HPP */

