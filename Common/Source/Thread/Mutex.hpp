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

using ScopeLock = Poco::ScopedLock<Mutex>;
using ScopeUnlock = Poco::ScopedUnlock<Mutex>;

/**
 * Call the Function within Mutex lock.
 */
template<class _Mutex, class Function>
decltype(auto) WithLock(_Mutex& m, Function&& f) {
    Poco::ScopedLock<_Mutex> lock(m);
    return f();
}

#endif	/* MUTEX_HPP */

