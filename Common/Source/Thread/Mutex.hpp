/* 
 * File:   Mutex.hpp
 * Author: user
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef MUTEX_HPP
#define	MUTEX_HPP

#include "Poco/Mutex.h"

class Mutex : protected Poco::Mutex {
    friend class Poco::ScopedLock<Mutex>;
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

#endif	/* MUTEX_HPP */

