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

#ifndef THREAD_POCO_MUTEX_HPP
#define THREAD_POCO_MUTEX_HPP

#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"
#include "Poco/ScopedUnlock.h"

// template adaptor to make Poco::Mutex conform to C++ named requirements 'Lockable'
template <typename BaseMutex>
struct LockableMutex : BaseMutex {
  using BaseMutex::BaseMutex;

  bool try_lock() {
    return BaseMutex::tryLock();
  }
};

using Mutex = LockableMutex<Poco::Mutex>;

using ScopeUnlock = Poco::ScopedUnlock<Mutex>;

#endif /* THREAD_POCO_MUTEX_HPP */
