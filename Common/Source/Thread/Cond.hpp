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

#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__STDCPP_THREADS__)
// c++11 thread is not available with mingw
#include "Poco/Cond.hpp"
#else
#include "stdcpp/Cond.hpp"
#endif

#endif /* THREAD_COND_HPP */
