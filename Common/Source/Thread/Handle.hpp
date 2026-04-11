/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   Handle.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015
 */

#ifndef _THREAD_HANDLE_HPP_
#define _THREAD_HANDLE_HPP_
#include "options.h"

#ifdef USE_STDCPP_THREADS
#include "stdcpp/Handle.hpp"
#elif defined(USE_POCO_THREADS)
#include "Poco/Handle.hpp"
#else
#error multithreading library is not defined
#endif

#endif /* _THREAD_HANDLE_HPP_ */

