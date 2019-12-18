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

#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__STDCPP_THREADS__)
// c++11 thread is not available with mingw
#include "Poco/Handle.hpp"
#else
#include "stdcpp/Handle.hpp"
#endif

#endif /* _THREAD_HANDLE_HPP_ */

