/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   Cond.hpp
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2015
 */

#ifndef COND_HPP
#define	COND_HPP

/// Exists only for avoids to change xcs original code...
// unimplemented, never used so no linker error
class Cond {
public:
    void Wait(Mutex &mutex);
    void Broadcast() {}
};


#endif	/* COND_HPP */

