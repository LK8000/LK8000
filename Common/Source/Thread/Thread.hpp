//
// Created by Bruno on 18/03/2019.
//

#ifndef _THREAD_THREAD_HPP_
#define _THREAD_THREAD_HPP_

#include "Poco/Thread.h"

class Thread : protected Poco::Runnable {
public:
    explicit Thread(const char* name) : _thread(name) {}

    bool Start() {
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

    void run() {
        Run();
    }

private:
    Poco::Thread _thread;
};

#endif //_THREAD_THREAD_HPP_
