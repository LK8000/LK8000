/* 
 * File:   Mutex.hpp
 * Author: user
 *
 * Created on March 3, 2015, 2:20 PM
 */

#ifndef MUTEX_HPP
#define	MUTEX_HPP

#include "Poco/Mutex.h"

class Mutex : public Poco::Mutex {
public:
    void Lock() { Poco::Mutex::lock(); }
    void Unlock() { Poco::Mutex::unlock(); }
};

class ScopeLock : public Poco::ScopedLock<Mutex> {
public:
    ScopeLock(Mutex& m) : Poco::ScopedLock<Mutex>(m) { }

};

#endif	/* MUTEX_HPP */

