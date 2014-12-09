/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 *
 * File:   Event.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 10 décembre 2014, 00:05
 */

#ifndef EVENT_H
#define	EVENT_H

#include "Globals.hpp"
#ifdef ANDROID
#include "Shared/Event.hpp"
#include "Android/Loop.hpp"
#elif defined(ENABLE_SDL)
#include "SDL/Event.hpp"
#include "SDL/Loop.hpp"
#elif defined(USE_CONSOLE) || defined(NON_INTERACTIVE)
#include "Shared/Event.hpp"
#include "Console/Loop.hpp"
#elif defined(USE_GDI)
#include "GDI/Event.hpp"
#include "GDI/Loop.hpp"
#endif
#include "Key.h"
#include "Compiler.h"


#endif	/* EVENT_H */

