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

class Mutex : protected Poco::Mutex {
    friend class Poco::ScopedLock<Mutex>;
    friend class Poco::ScopedUnlock<Mutex>;
public:
    Mutex() {} 

    inline void Lock() { 
        Poco::Mutex::lock(); 
    }
    
    inline void Unlock() { 
        Poco::Mutex::unlock(); 
    }
};

class ScopeLock : public Poco::ScopedLock<Mutex> {
public:
    ScopeLock(Mutex& m) : Poco::ScopedLock<Mutex>(m) { }

};

class ScopeUnlock : public Poco::ScopedUnlock<Mutex> {
public:
    ScopeUnlock(Mutex& m) : Poco::ScopedUnlock<Mutex>(m) { }

};

#endif	/* MUTEX_HPP */

