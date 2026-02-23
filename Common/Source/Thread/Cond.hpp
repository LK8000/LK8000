/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   Cond.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015
 */

#ifndef THREAD_COND_HPP
#define THREAD_COND_HPP
#include "options.h"

#ifdef USE_STDCPP_THREADS
#include "stdcpp/Cond.hpp"
#elif defined(USE_POCO_THREADS)
#include "Poco/Cond.hpp"
#else
#error multithreading library is not defined
#endif

#endif /* THREAD_COND_HPP */
