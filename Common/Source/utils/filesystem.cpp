/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   filesystem.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 juillet 2014, 13:45
 */
#define _FILESYSTEM_IMPL_

#include "filesystem.h"

namespace lk {
    namespace filesystem {
        // private class declaration

        class directory_iterator_impl {
        public:

            directory_iterator_impl() {
            }

            virtual ~directory_iterator_impl() {
            }

            virtual void operator++() = 0;
            virtual operator bool() = 0;

            virtual bool isDirectory() const = 0;
            virtual const TCHAR* getName() const = 0;

        };
    };
};

#ifdef _WIN32
#include "filesystem_WIN32.cpp"
#else
#include "filesystem_POSIX.cpp"
#endif

lk::filesystem::directory_iterator::~directory_iterator() {
    delete _impl;
}

lk::filesystem::directory_iterator& lk::filesystem::directory_iterator::operator++() {
    ++(*_impl);
    return (*this);
}

lk::filesystem::directory_iterator::directory_iterator::operator bool() {
    return (bool)(*_impl);
}

bool lk::filesystem::directory_iterator::isDirectory() const {
    return _impl->isDirectory();
}

const TCHAR* lk::filesystem::directory_iterator::getName() const {
    return _impl->getName();
}
