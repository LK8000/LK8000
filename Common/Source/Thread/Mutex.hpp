/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   Mutex.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef MUTEX_HPP
#define	MUTEX_HPP

#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/ScopedUnlock.h"

// template adaptor to make Poco::Mutex and Poco::FastMutex conform to C++ named requirements 'Lockable'
template<typename BaseMutex>
struct LockableMutex : BaseMutex {
    bool try_lock() {
        return BaseMutex::tryLock();
    }
};

using Mutex = LockableMutex<Poco::Mutex>;
using FastMutex = LockableMutex<Poco::FastMutex>;

class ScopeLock : public Poco::ScopedLock<Mutex> {
public:
    ScopeLock(Mutex& m) : Poco::ScopedLock<Mutex>(m) { }

};

class ScopeUnlock : public Poco::ScopedUnlock<Mutex> {
public:
    ScopeUnlock(Mutex& m) : Poco::ScopedUnlock<Mutex>(m) { }

};

#endif	/* MUTEX_HPP */

