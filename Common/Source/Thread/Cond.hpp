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
#include "Poco/Event.h"

class Cond : public Poco::Event {
public:
    void Wait(Mutex &mutex);
    void Broadcast() { set(); }
};


#endif	/* COND_HPP */

