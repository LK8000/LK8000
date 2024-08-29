/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Thread.hpp
 * Author: Bruno de Lacheisserie
 */

#ifndef _THREAD_THREAD_HPP_
#define _THREAD_THREAD_HPP_

#include "Poco/Thread.h"
#include <functional>

class Thread : protected Poco::Runnable {
public:
    explicit Thread(const char* name) : _thread(name) {}

    virtual bool Start() {
        _thread.start(*this);
        return _thread.isRunning();
    }

    void Join() {
        _thread.join();
    }

    bool IsInside() const {
        return (_thread.currentTid() == _thread.tid());
    }

    bool IsDefined() const {
        return _thread.isRunning();
    }

protected:
    virtual void Run() = 0;

    void run() override {
        Run();
    }

private:
    Poco::Thread _thread;
};

class InvokeThread : public Thread {
public:
    InvokeThread(const char* name, std::function<void()>&& func) : Thread(name) , _func(std::move(func)) {}

    void Run() override {
        _func();
    }

private:
    std::function<void()> _func;
};

#endif //_THREAD_THREAD_HPP_
