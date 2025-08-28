/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   Mutex.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef _THREAD_MUTEX_HPP_
#define	_THREAD_MUTEX_HPP_

#include <functional>
#include <mutex>

#include "Poco/Mutex.h"
#include "Poco/NamedMutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/ScopedUnlock.h"

// template adaptor to make Poco::Mutex and Poco::FastMutex conform to C++ named requirements 'Lockable'
template<typename BaseMutex>
struct LockableMutex : BaseMutex {

    using BaseMutex::BaseMutex;

    bool try_lock() {
        return BaseMutex::tryLock();
    }
};

using Mutex = LockableMutex<Poco::Mutex>;
using FastMutex = LockableMutex<Poco::FastMutex>;
using NamedMutex = LockableMutex<Poco::NamedMutex>;

using ScopeLock = Poco::ScopedLock<Mutex>;
using ScopeUnlock = Poco::ScopedUnlock<Mutex>;

/**
 * Invoke a callable object within Mutex lock.
 */
template<typename _Mutex, typename _Callable, typename ..._Args>
std::invoke_result_t<_Callable, _Args...> 
WithLock(_Mutex& m, _Callable&& fn, _Args&& ...args) 
noexcept(std::is_nothrow_invocable_v<_Callable, _Args...>)
{
    Poco::ScopedLock<_Mutex> lock(m);
    return std::invoke(std::forward<_Callable>(fn), std::forward<_Args>(args)...);
}

#endif	/* _THREAD_MUTEX_HPP_ */
